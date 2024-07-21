#include "WavetableSynthesizerVoice.h"

//==============================================================================
// CONSTRUCTORS / DESTRUCTORS

WavetableSynthesizerVoice::WavetableSynthesizerVoice() : juce::SynthesiserVoice()
{
    // initialize wavetable desc
    wavetable = nullptr;
    wavetableSize = 0;

    // initialize render context
    renderSampleRate = 0.f;
    renderFrequency = 0.f;
    renderLevel = 0.f;
    
    noteVelocity = 0.f;
    
    renderPanCoefficientLeft = 0.f;
    renderPanCoefficientRight = 0.f;

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

WavetableSynthesizerVoice::WavetableSynthesizerVoice(const Wavetable* wavetableToUse) : WavetableSynthesizerVoice::WavetableSynthesizerVoice()
{
    setWavetable(wavetableToUse);
}

WavetableSynthesizerVoice::~WavetableSynthesizerVoice()
{
}

void WavetableSynthesizerVoice::setWavetable(const Wavetable* wavetableToUse)
{
    wavetable = wavetableToUse;
    wavetableSize = wavetable->getNumSamples();
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
    auto values = wavetable->getReadPointer(0);
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

    if (renderSampleRate == 0.f 
        || renderFrequency < 15.f 
        || renderLevel == 0 
        || wavetableSize == 0
    ) {
        outputBuffer.clear(0, startSample, numSamples);
        outputBuffer.clear(1, startSample, numSamples);
        return;
    }

    // render a wave into the oversampled block
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        float sampleValue = getNextSample() * renderLevel * noteVelocity;
        outputBuffer.setSample(0, sampleIndex, sampleValue * renderPanCoefficientLeft);
        outputBuffer.setSample(1, sampleIndex, sampleValue * renderPanCoefficientRight);
    }
}

void WavetableSynthesizerVoice::calculatePanningCoefficients(float pan) {
    // Clamp the pan value to the range [-1, 1]
    if (pan < -1.f) pan = -1.f;
    if (pan > 1.f) pan = 1.f;

    // Calculate the coefficients
    renderPanCoefficientLeft = std::cos((juce::MathConstants<float>::pi / 4.0f) * (1.0f + pan));
    renderPanCoefficientRight = std::sin((juce::MathConstants<float>::pi / 4.0f) * (1.0f + pan));
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

void WavetableSynthesizerVoice::setRenderLevel(float newRenderLevel)
{
    if (newRenderLevel < 0.f)
    {
        renderLevel = 0.f;
    }
    else if (newRenderLevel > 1.f)
    {
        renderLevel = 1.f;
    }
    else {
        renderLevel = newRenderLevel;
    }
}

void WavetableSynthesizerVoice::setRenderPan(float newPan)
{
    if (newPan < -1.f)
    {
        calculatePanningCoefficients(-1.f);
    }
    else if (newPan >= 1.f)
    {
        calculatePanningCoefficients(1.f);
    }
    else {
        calculatePanningCoefficients(newPan);
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
    noteVelocity = velocity;
}

void WavetableSynthesizerVoice::stopNote(float velocity, bool allowTailOff)
{
    if (velocity == 0)
    {
        clearCurrentNote();
    }
    noteVelocity = velocity;
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