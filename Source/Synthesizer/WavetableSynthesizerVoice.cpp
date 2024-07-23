#include "WavetableSynthesizerVoice.h"

//=============================================================================
// CONSTRUCTORS / DESTRUCTORS

WavetableSynthesizerVoice::WavetableSynthesizerVoice() : juce::SynthesiserVoice()
{
    // initialize wavetable desc
    wavetable = nullptr;
    wavetableSize = 0;
    wavetableNumFrames = 0;
    wavetableFrameIndex = 0;

    // initialize render context
    renderSampleRate = 0.f;
    renderFrequency = 0.f;
    renderLevel = 0.f; 
    renderPanCoefficientLeft = 0.f;
    renderPanCoefficientRight = 0.f;
    noteVelocity = 0.f;

    // initialize wavetable positioning
    updateDeltaPhase();
    phase = 0.f;
    sampleIndex = 0;
    sampleOffset = 0.f;

    // initialize wheel control parameters
    pitchBendWheelPosition = 0.f;
    pitchBendUpperBoundSemitones = 2;
    pitchBendLowerBoundSemitones = -2;

    juce::ADSR::Parameters adsrDefaultParameters(0.8f, 1.f, 0.3f, 1.f);
    adsrEnvelope.setParameters(adsrDefaultParameters);
    adsrEnvelope.setSampleRate(renderSampleRate);
}

WavetableSynthesizerVoice::WavetableSynthesizerVoice(const Wavetable* wavetableToUse) : 
    WavetableSynthesizerVoice::WavetableSynthesizerVoice()
{
    setWavetable(wavetableToUse);
}

WavetableSynthesizerVoice::~WavetableSynthesizerVoice() {}

//=============================================================================
// WAVETABLE MANAGEMENT

void WavetableSynthesizerVoice::setWavetable(const Wavetable* wavetableToUse)
{
    wavetable = wavetableToUse;
    wavetableSize = wavetable->getNumSamples();
    wavetableNumFrames = wavetable->getNumChannels();
}

void WavetableSynthesizerVoice::setWavetableFrameIndex(int newFrameIndex)
{
    newFrameIndex = juce::jmax(0, newFrameIndex);
    newFrameIndex = juce::jmin(newFrameIndex, wavetableNumFrames);
    wavetableFrameIndex = newFrameIndex;
}

//=============================================================================
// RENDERING

// fill a buffer with samples using the wavetable
void WavetableSynthesizerVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, 
    int startSample, int numSamples
) {
    // update rendering parameters
    updateRenderFrequencyFromMidiInput();
    setRenderSampleRate((float) getSampleRate());
    updateDeltaPhase();

    // render a wave into the oversampled block
    auto output = outputBuffer.getArrayOfWritePointers();
    for (int renderIndex = startSample; renderIndex < numSamples; ++renderIndex)
    {
        // get raw sample
        float rawSampleValue = getInterpolatedSampleFromCurrentPhase();
        updatePhase();

        // apply level changes from volume knob and note velocity
        float sampleValue = rawSampleValue * renderLevel * noteVelocity;

        // apply panning coefficients while writing to output
        output[0][renderIndex] = sampleValue * renderPanCoefficientLeft;
        output[1][renderIndex] = sampleValue * renderPanCoefficientRight;
    }

    adsrEnvelope.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
}

// interpolate sample from current wave phase
float WavetableSynthesizerVoice::getInterpolatedSampleFromCurrentPhase()
{
    // TODO: SIMD

    // select 4 samples around sampleIndex
    auto values = wavetable->getReadPointer(wavetableFrameIndex);
    float val0 = values[(sampleIndex - 1 + wavetableSize) % wavetableSize];
    float val1 = values[(sampleIndex + 0) % wavetableSize];
    float val2 = values[(sampleIndex + 1) % wavetableSize];
    float val3 = values[(sampleIndex + 2) % wavetableSize];

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

    return result;
}

// calculate and update deltaPhase based on frequency and sampleRate
void WavetableSynthesizerVoice::updateDeltaPhase()
{
    if (renderSampleRate == 0.f)
    {
        deltaPhase = 0.f;
    }
    else 
    {
        deltaPhase = juce::jmax(0.f, renderFrequency / renderSampleRate);
    }
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

// idk what this really means but it has to override the base class
bool WavetableSynthesizerVoice::canPlaySound(juce::SynthesiserSound *sound)
{
    return dynamic_cast <WavetableSynthesizerSound *>(sound) != nullptr;
}

//=============================================================================
// UPDATE RENDER PARAMETERS

//-------------------------------------
// sample rate

void WavetableSynthesizerVoice::setRenderSampleRate(float newRenderSampleRate)
{
    // limit sample rate to [0, 192k]
    newRenderSampleRate = juce::jmax(0.f, newRenderSampleRate);
    newRenderSampleRate = juce::jmin(newRenderSampleRate, 192000.f);

    renderSampleRate = newRenderSampleRate;
    adsrEnvelope.setSampleRate(renderSampleRate);

}

//-------------------------------------
// frequency

void WavetableSynthesizerVoice::setRenderFrequency(float newRenderFrequency)
{
    // limit frequnecy to [0, 20k]
    newRenderFrequency = juce::jmax(0.f, newRenderFrequency);
    newRenderFrequency = juce::jmin(newRenderFrequency, 20000.f);
    
    renderFrequency = newRenderFrequency;
}

void WavetableSynthesizerVoice::updateRenderFrequencyFromMidiInput()
{
    auto currentNote = getCurrentlyPlayingNote();
    auto pitchBendOffsetCents = getPitchBendOffsetCents();
    auto newFrequency = getOffsetMidiNoteInHertz(currentNote, pitchBendOffsetCents);
    setRenderFrequency(newFrequency);
}

void WavetableSynthesizerVoice::updateRenderFrequencyFromMidiNote(int midiNoteNumber, int pitchWheelPosition)
{
    auto pitchBendOffsetCents = getPitchBendOffsetCents(pitchWheelPosition);
    auto newFrequency = getOffsetMidiNoteInHertz(midiNoteNumber, pitchWheelPosition);
    setRenderFrequency(newFrequency);
}

//-------------------------------------
// level

void WavetableSynthesizerVoice::setRenderLevel(float newRenderLevel)
{
    // limit level to [0, 1]
    newRenderLevel = juce::jmax(0.f, newRenderLevel);
    newRenderLevel = juce::jmin(newRenderLevel, 1.f);
    
    renderLevel = newRenderLevel;
}

//-------------------------------------
// pan

void WavetableSynthesizerVoice::setRenderPan(float newPanningValue)
{
    // limit pan to [-1, 1]
    newPanningValue = juce::jmax(-1.f, newPanningValue);
    newPanningValue = juce::jmin(newPanningValue, 1.f);

    // calculate coefficients to be applied to each channel
    renderPanCoefficientLeft = std::cos((juce::MathConstants<float>::pi / 4.0f) * (1.0f + newPanningValue));
    renderPanCoefficientRight = std::sin((juce::MathConstants<float>::pi / 4.0f) * (1.0f + newPanningValue));
}

//=============================================================================
// NOTE ON OFF

void WavetableSynthesizerVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int pitchWheelPosition)
{
    // TODO: use midi note number and pitch wheel position instead of this
    updateRenderFrequencyFromMidiNote(midiNoteNumber, pitchWheelPosition);
    noteVelocity = velocity;
    adsrEnvelope.noteOn();

    juce::ignoreUnused(sound);
    juce::ignoreUnused(midiNoteNumber);
    juce::ignoreUnused(pitchWheelPosition);
}

void WavetableSynthesizerVoice::stopNote(float velocity, bool allowTailOff)
{
    adsrEnvelope.noteOff();
    noteVelocity = velocity;

    if(!allowTailOff)
    {
        clearCurrentNote();
    }
}

//=============================================================================
// MIDI

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
    {
        // calculate cents based on position relative to UPPER bound
        return pitchBendWheelPosition * (float) pitchBendUpperBoundSemitones * 100.f;
    }
    else
    {
        // calculate cents based on position relative to LOWER lower
        return pitchBendWheelPosition * (float) pitchBendLowerBoundSemitones * 100.f;
    }
}

// convert pitch bend wheel position to a note offset in cents based on wheel upper or lower bounds
float WavetableSynthesizerVoice::getPitchBendOffsetCents(float pitchWheelPosition)
{
    if (pitchWheelPosition >= 0.0f)
    {
        // calculate cents based on position relative to UPPER bound
        return pitchWheelPosition * (float) pitchBendUpperBoundSemitones * 100.f;
    }
    else
    {
        // calculate cents based on position relative to LOWER lower
        return pitchWheelPosition * (float) pitchBendLowerBoundSemitones * 100.f;
    }
}

// convert a midi note number to Hz with a cent offset
float WavetableSynthesizerVoice::getOffsetMidiNoteInHertz(int midiNoteNumber, float centsOffset)
{
    auto noteHz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    noteHz *= std::pow(2.0, centsOffset / 1200);
    return (float) noteHz;
}

void WavetableSynthesizerVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused(controllerNumber);
    juce::ignoreUnused(newControllerValue);
    return;
}