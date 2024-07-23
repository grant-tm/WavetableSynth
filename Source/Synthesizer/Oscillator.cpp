#include "Oscillator.h"

//=============================================================================
// CONSTRUCTORS / DESTRUCTORS

Oscillator::Oscillator()
{
    // WAVETABLE
    wavetable = nullptr;
    wavetableSize = 0;
    wavetableNumFrames = 0;
    wavetableFrameIndex = 0;

    // WAVETABLE POSITION
    for (int i = 0; i < MAX_DETUNE_VOICES; i++)
    {
        phases[i] = 0;
    }
    deltaPhase = 0.f;
    sampleIndex = 0;
    sampleOffset = 0.f;

    // SETTABLE RENDER PARAMETERS
    enable = false;
    sampleRate = 0.f;
    baseFrequency = 0.f;
    baseVolume = 0.f;
    basePan = 0.f;

    // DEPENDENT RENDER PARAMETERS
    renderFrequency = 0.f;
    renderVolume = 0.f;
    renderPanCoefficientLeft = 0.f;
    renderPanCoefficientRight = 0.f;

    // DETUNE PARAMETERS
    detuneVoices = 1;
    detuneMix = 0.f;
    detuneSpread = 0.f;
    for (int i = 0; i < MAX_DETUNE_VOICES; i++)
    {
        detuneFrequencyOffsets[i] = 0;
        detuneVolumeCoefficients[i] = 0;
    }
}

Oscillator::Oscillator(const Wavetable *wavetableToUse) : Oscillator::Oscillator()
{
    setWavetable(wavetableToUse);
}

Oscillator::~Oscillator() {}

//=============================================================================
// RENDERING

void Oscillator::render(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
    if (!this->enable)
    {
        return;
    }

    auto output = outputBuffer.getArrayOfWritePointers();
    for (int detuneVoice = 0; detuneVoice < this->detuneVoices; detuneVoice++)
    {
        setRenderParametersForDetunedVoice(detuneVoice);
        updateDeltaPhase();

        // render the damn wave!
        for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
        {
            incrementPhase(detuneVoice);
            auto sampleValue = getNextSample();
            output[0][sampleIndex] = sampleValue * renderPanCoefficientLeft;
            output[1][sampleIndex] = sampleValue * renderPanCoefficientRight;
        }
    }

    /*for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
    {
        output[0][sampleIndex] /= this->detuneVoices;
        output[1][sampleIndex] /= this->detuneVoices;
    }*/
}

// interpolate sample from current wave phase
float Oscillator::getNextSample()
{
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

// update the phase and calculate sampleIndex and sampleOffset
void Oscillator::incrementPhase(int phaseIndex)
{
    phases[phaseIndex] += deltaPhase;
    phases[phaseIndex] -= std::floor(phases[phaseIndex]);

    float scaledPhase = phases[phaseIndex] * wavetableSize;

    sampleIndex = (int)scaledPhase;
    sampleOffset = scaledPhase - (float)sampleIndex;
}

// calculate and update deltaPhase based on frequency and sampleRate
void Oscillator::updateDeltaPhase()
{
    if (sampleRate == 0.f)
    {
        deltaPhase = 0.f;
    }
    else
    {
        deltaPhase = juce::jmax(0.f, renderFrequency / sampleRate);
    }
}

//=============================================================================
// RENDERING PARAMETERS

//-------------------------------------
// enable

void Oscillator::setEnable(bool newEnable)
{
    enable = newEnable;
}

//-------------------------------------
// sample rate

void Oscillator::setSampleRate(float newSampleRate)
{
    // limit sample rate to [0, 192k]
    newSampleRate = juce::jmax(0.f, newSampleRate);
    newSampleRate = juce::jmin(192000.f, newSampleRate);

    this->sampleRate = newSampleRate;
}

//-------------------------------------
// frequency

void Oscillator::setFrequency(float newFrequency)
{
    // limit frequnecy to [0, 20k]
    newFrequency = juce::jmax(0.f, newFrequency);
    newFrequency = juce::jmin(20000.f, newFrequency);

    this->baseFrequency = newFrequency;
}

//-------------------------------------
// volume

void Oscillator::setVolume(float newVolume)
{
    // limit volume to [0, 1]
    newVolume = juce::jmax(0.f, newVolume);
    newVolume = juce::jmin(1.f, newVolume);

    this->baseVolume = newVolume;
}

//-------------------------------------
// pan

void Oscillator::setPan(float newPan)
{
    // limit pan to [-1, 1]
    newPan = juce::jmax(-1.f, newPan);
    newPan = juce::jmin(1.f, newPan);

    this->basePan = newPan;
}

//=============================================================================
// WAVETABLE

void Oscillator::setWavetable(const Wavetable *wavetableToUse)
{
    wavetable = wavetableToUse;
    wavetableSize = wavetable->getNumSamples();
    wavetableNumFrames = wavetable->getNumChannels();
}

void Oscillator::setWavetableFrameIndex(int newFrameIndex)
{
    // limit new frame index to [0, numFrames]
    newFrameIndex = juce::jmax(0, newFrameIndex);
    newFrameIndex = juce::jmin(wavetableNumFrames, newFrameIndex);
    
    this->wavetableFrameIndex = newFrameIndex;
}

//=============================================================================
// DETUNE PARAMETERS

//-------------------------------------
// detune num voices
void Oscillator::setDetuneVoices(int newNumVoices)
{
    newNumVoices = std::max(1, newNumVoices);
    newNumVoices = std::min(MAX_DETUNE_VOICES, newNumVoices);
    
    this->detuneVoices = newNumVoices;
    updateDetuneVoiceConfiguration();
};

//-------------------------------------
// detune frequency offsets

void Oscillator::setDetuneMix(float newDetuneMix)
{
    newDetuneMix = std::max(0.f, newDetuneMix);
    newDetuneMix = std::min(1.f, newDetuneMix);

    this->detuneMix = newDetuneMix;
    updateDetuneVoiceConfiguration();

}

//-------------------------------------
// detune frequency offsets
void Oscillator::setDetuneSpread(float newDetuneSpread)
{
    newDetuneSpread = std::max(0.f, newDetuneSpread);
    newDetuneSpread = std::min(1.f, newDetuneSpread);

    this->detuneMix = newDetuneSpread;
    updateDetuneVoiceConfiguration();
}

//=============================================================================
// DETUNE DEPENDENT VALUES

//--------------------------------------
// recalcualte all dependent values

void Oscillator::updateDetuneVoiceConfiguration()
{
    calculateDetuneFrequencyOffsets();
    calculateDetuneVolumeCoefficients();
    calculateDetunePanningValues();
}

//-------------------------------------
// detune frequency offsets

void Oscillator::calculateDetuneFrequencyOffsets()
{
    auto spread = MAX_DETUNE_SPREAD_PROPORTIONAL * detuneSpread;
    auto spreadPerLayer = spread / std::floor(detuneVoices / 2);
    int numVoicesAssigned = 0;

    // if odd number of voices, assign baseFrequency to center voice
    if (detuneVoices % 2 != 0)
    {
        detuneFrequencyOffsets[numVoicesAssigned++] = baseFrequency;
    }

    // assign frequency offsets to remaining voices
    int numOuterVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair < numOuterVoicePairsToCreate; voicePair++)
    {
        detuneFrequencyOffsets[numVoicesAssigned++] = baseFrequency - (spreadPerLayer * voicePair);
        detuneFrequencyOffsets[numVoicesAssigned++] = baseFrequency + (spreadPerLayer * voicePair);
    }
}

//-------------------------------------
// detune volume coefficients

void Oscillator::calculateDetuneVolumeCoefficients()
{
    int numVoicesAssigned = 0;

    // if odd number of voices, assign baseVolume to center voice
    if (detuneVoices % 2 != 0)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = baseVolume;
    }

    // assign mixed volume to remaining voices
    int numOuterVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair < numOuterVoicePairsToCreate; voicePair++)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = baseVolume * detuneMix;
        detuneVolumeCoefficients[numVoicesAssigned++] = baseVolume * detuneMix;
    }
}

//-------------------------------------
// detune panning values

void Oscillator::calculateDetunePanningValues()
{
    auto spread = 1.f * detuneSpread;
    auto spreadPerLayer = (2 * spread) / detuneVoices;
    int numVoicesAssigned = 0;

    // if odd number of voices, assign basePan to center voice
    if (detuneVoices % 2 != 0)
    {
        detunePanningValues[numVoicesAssigned++] = basePan;
    }

    // assign shifted pan to remaining voices
    int numOuterVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair < numOuterVoicePairsToCreate; voicePair++)
    {
        float panCoefficient = basePan + ((float) voicePair * spreadPerLayer);
        panCoefficient = std::max(-1.f, panCoefficient);
        panCoefficient = std::min(1.f, panCoefficient);
        detunePanningValues[numVoicesAssigned++] = -panCoefficient;
        detunePanningValues[numVoicesAssigned++] = panCoefficient;
    }
}

//-------------------------------------
// set all parameters

void Oscillator::setRenderParametersForDetunedVoice(int detuneVoice)
{
    renderFrequency = baseFrequency + detuneFrequencyOffsets[detuneVoice];
    
    renderVolume = baseVolume * detuneVolumeCoefficients[detuneVoice];

    auto renderPan = basePan * detunePanningValues[detuneVoice];
    renderPanCoefficientLeft = std::cos((juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan));
    renderPanCoefficientRight = std::sin((juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan));
}