#include "WavetableSynthesizerVoice.h"

//==============================================================================
// CONSTRUCTORS / DESTRUCTORS

WavetableSynthesizerVoice::WavetableSynthesizerVoice() : juce::SynthesiserVoice()
{
    // initialize wavetable desc
    wavetable.clear();
    wavetableSize = 0;

    // initialize render context
    renderSampleRate = 0.f;
    renderFrequency = 0.f;
    renderLevel = 0.f;

    // initialize wavetable positioning
    updateDeltaPhase();
    phase = 0.f;
    sampleIndex = 0;
    sampleOffset = 0.f;

    // initialize wheel control parameters
    pitchBendWheelPosition = 0.f;
    pitchBendUpperBoundSemitones = 2;
    pitchBendLowerBoundSemitones = -2;
}

WavetableSynthesizerVoice::WavetableSynthesizerVoice(Wavetable& wavetableToUse) : WavetableSynthesizerVoice::WavetableSynthesizerVoice()
{
    setWavetable(wavetableToUse);
}

WavetableSynthesizerVoice::~WavetableSynthesizerVoice()
{
    wavetable.clear();
}

void WavetableSynthesizerVoice::setWavetable(Wavetable& wavetableToUse)
{
    // if wavetable is large enough, copy it and normalize it
    if (wavetableToUse.getNumChannels() == 1 && wavetableToUse.getNumSamples() >= 16)
    {
        // copy wavetable
        wavetable = wavetableToUse;
        wavetableSize = wavetable.getNumSamples();

        // get min and max values of wavetable
        auto tableRange = wavetable.findMinMax(0, 0, wavetableSize);
        auto min = tableRange.getStart();
        auto max = tableRange.getEnd();

        // normalize wavetable against max value
        for (int sampleIndex = 0; sampleIndex < wavetableSize; ++sampleIndex)
        {
            auto sampleValue = (float)wavetable.getSample(0, sampleIndex) / max;
            wavetable.setSample(0, sampleIndex, sampleValue);
        }
    }
}

//==============================================================================
// RENDERING

// idk what this really means but it has to override the base class
bool WavetableSynthesizerVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast <WavetableSynthesizerSound*>(sound) != nullptr;
}

// calculate deltaPhase based on frequency and sampleRate
// also updates frequency based on currently playing note and pitch wheel position
void WavetableSynthesizerVoice::updateDeltaPhase()
{
    deltaPhase = (renderSampleRate > 0.f) ? (renderFrequency / renderSampleRate) : 0.f;
}

// update the phase and calculate sampleIndex and sampleOffset
void WavetableSynthesizerVoice::updatePhase()
{
    // TODO: SIMD
    // TODO: use fixed point instead of wrapping with std::floor

    phase += deltaPhase;
    phase -= std::floor(phase);

    float scaledPhase = phase * wavetableSize;

    sampleIndex = (int) scaledPhase;
    sampleOffset = scaledPhase - (float) sampleIndex;
}

// calculate a single float sample by interpolating around the current sampleIndex and sampleOffset
float WavetableSynthesizerVoice::getNextSample()
{
    // TODO: SIMD
    
    // select 4 samples around sampleIndex
    float val0 = wavetable.getSample(0, (sampleIndex - 1 + wavetableSize) % wavetableSize);
    float val1 = wavetable.getSample(0, (sampleIndex + 0) % wavetableSize);
    float val2 = wavetable.getSample(0, (sampleIndex + 1) % wavetableSize);
    float val3 = wavetable.getSample(0, (sampleIndex + 2) % wavetableSize);

    // calculate slopes to use at points val1 and val2 (avoid discontinuities)
    float slope0 = (val2 - val0) * 0.5f;
    float slope1 = (val3 - val1) * 0.5f;

    // calculate interpolation coefficients
    float delta = val1 - val2;
    float slopeSum = slope0 + delta;
    float coefficientA = slopeSum + delta + slope1;
    float coefficientB = slopeSum + coefficientA;

    // perform interpolation
    float stage1 = coefficientA * sampleOffset - coefficientB;
    float stage2 = stage1 * sampleOffset + slope0;
    float result = stage2 * sampleOffset + val1;

    updatePhase();

    return result;
}

// fill a buffer with samples using the wavetable
void WavetableSynthesizerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // update rendering parameters
    updateRenderFrequencyFromMidiInput();
    setRenderSampleRate(getSampleRate());
    updateDeltaPhase();

    // render a wave into the oversampled block
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        float sampleValue = getNextSample() * renderLevel;
        for (int channel = 0; channel < outputBuffer.getNumChannels(); channel++)
        {
            outputBuffer.setSample(channel, sampleIndex, sampleValue);
        }
    }
}


//==============================================================================
// UPDATE RENDER CONTEXT

void WavetableSynthesizerVoice::setRenderSampleRate(float newRenderSampleRate)
{
    if (newRenderSampleRate > 0.f)
    {
        renderSampleRate = newRenderSampleRate;
    }
    else
    {
        renderSampleRate = 0.f;
    }
}

void WavetableSynthesizerVoice::setRenderFrequency(float newRenderFrequency)
{
    if (newRenderFrequency > 0.f)
    {
        renderFrequency = newRenderFrequency;
    }
    else 
    {
        renderFrequency = 0.f;
    }
}

void WavetableSynthesizerVoice::updateRenderFrequencyFromMidiInput()
{
    auto newFrequency = getOffsetMidiNoteInHertz(getCurrentlyPlayingNote(), getPitchBendOffsetCents());
    setRenderFrequency(newFrequency);
}

//==============================================================================
// NOTE ON OFF

void WavetableSynthesizerVoice::startNote(int midiNoteNumber, float velocity,
    juce::SynthesiserSound* sound, int pitchWheelPosition)
{
    setPitchBendPosition(pitchWheelPosition);
    updateRenderFrequencyFromMidiInput();
    renderLevel = velocity;
}

void WavetableSynthesizerVoice::stopNote(float velocity, bool allowTailOff)
{
    if (velocity == 0)
    {
        clearCurrentNote();
    }
    renderLevel = velocity;
}

//==============================================================================
// PITCHBEND CONTROLS

// pitch wheel move callback: store new position and update frequency
void WavetableSynthesizerVoice::pitchWheelMoved(int newPitchWheelValue)
{
    setPitchBendPosition(newPitchWheelValue);
}

// set the pitch bend position (normalized to a [-1, 1] float value)
void WavetableSynthesizerVoice::setPitchBendPosition(int position)
{
    if (position > 8192)
        pitchBendWheelPosition = (float) (position - 8192.f) / (16383.f - 8192.f);
    else
        pitchBendWheelPosition = (float) (8192.f - position) / -8192.f;
}

// convert pitch bend wheel position to a note offset in cents based on wheel upper or lower bounds
float WavetableSynthesizerVoice::getPitchBendOffsetCents()
{
    if (pitchBendWheelPosition >= 0.0f)
        // calculate cents based on position relative to UPPER bound
        return pitchBendWheelPosition * (float) pitchBendUpperBoundSemitones * 100.f;
    else
        // calculate cents based on position relative to LOWER lower
        return pitchBendWheelPosition * (float) pitchBendLowerBoundSemitones * 100.f;
}

//==============================================================================
// HELPERS

// convert a midi note number to Hz with a cent offset
float WavetableSynthesizerVoice::getOffsetMidiNoteInHertz(int midiNoteNumber, float centsOffset)
{
    float noteHz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    noteHz *= std::pow(2.0, centsOffset / 1200);
    return noteHz;
}