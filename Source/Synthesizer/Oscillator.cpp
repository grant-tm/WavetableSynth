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
    sampleOffset = 0.f;

    // SETTABLE RENDER PARAMETERS
    sampleRate = 0.f;
    baseFrequency = 0.f;
    baseVolume = 0.f;
    basePan = 0.f;
    velocity = 1.f;

    // DEPENDENT RENDER PARAMETERS
    renderFrequency = 150.f;
    renderVolume = 1.f;
    renderPanCoefficientLeft = 1.f;
    renderPanCoefficientRight = 1.f;

    // DETUNE PARAMETERS
    detuneVoices = 1;
    detuneMix = 1.f;
    detuneSpread = 1.f;
    for (int i = 0; i < MAX_DETUNE_VOICES; i++)
    {
        detuneFrequencyCoefficients[i] = 1;
        detuneVolumeCoefficients[i] = 1;
    }
}

Oscillator::Oscillator(const Wavetable *wavetableToUse) : Oscillator::Oscillator()
{
    setWavetable(wavetableToUse);
}

Oscillator::~Oscillator() {}

//=============================================================================
// RENDER

void Oscillator::render(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
    auto output = outputBuffer.getArrayOfWritePointers();
    for (int detuneVoice = 0; detuneVoice < detuneVoices; detuneVoice++)
    {
        applyRenderParameters(detuneVoice);
        updateDeltaPhase();

        // render the damn wave!
        for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
        {
            incrementPhase(detuneVoice);
            
            auto sampleValue = getNextSample() * renderVolume * velocity / detuneVoices;
            sampleValue *= adsrEnvelope.isActive() ? adsrEnvelope.getNextSample() : 0.f;
            
            output[0][sampleIndex] += sampleValue * renderPanCoefficientLeft;
            output[1][sampleIndex] += sampleValue * renderPanCoefficientRight;
        }
    }
}

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

//=============================================================================
// APPLY RENDER PARAMETERS

void Oscillator::applyRenderParameters(int detuneVoice)
{
    if (detuneVoices > 1 && detuneMix > 0)
    {
        applyDetuneRenderParameters(detuneVoice);
    }
    else
    {
        applyBaseRenderParameters();
    }
}

void Oscillator::applyDetuneRenderParameters(int detuneVoice)
{
    renderFrequency = baseFrequency * detuneFrequencyCoefficients[detuneVoice];

    renderVolume = baseVolume * detuneVolumeCoefficients[detuneVoice];

    auto renderPan = basePan + detunePanningOffsets[detuneVoice];
    
    auto leftAngle = (juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan);
    renderPanCoefficientLeft = std::cos(leftAngle);
    
    auto rightAngle = (juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan);
    renderPanCoefficientRight = std::sin(rightAngle);
}

void Oscillator::applyBaseRenderParameters()
{
    renderFrequency = baseFrequency;

    renderVolume = baseVolume;

    auto renderPan = basePan;
    renderPanCoefficientLeft = std::cos((juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan));
    renderPanCoefficientRight = std::sin((juce::MathConstants<float>::pi / 4.0f) * (1.0f + renderPan));
}

//=============================================================================
// PHASE UPDATE

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
// DETUNE

// detune frequency coefficients
void Oscillator::calculateDetuneFrequencyCoefficients()
{
    int numVoicesAssigned = 0;

    // if odd number of voices, assign baseFrequency to center voice
    if (detuneVoices % 2 != 0)
    {
        detuneFrequencyCoefficients[numVoicesAssigned++] = 1.f;
    }

    // assign frequency coefficients to remaining voices
    const float maxFrequencyCoefficient = MAX_DETUNE_SPREAD_PROPORTIONAL * detuneSpread;
    float frequencyStep = maxFrequencyCoefficient / std::floor(detuneVoices / 2);

    int numVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair <= numVoicePairsToCreate; voicePair++)
    {
        float frequencyCoefficient = frequencyStep * voicePair;
        //float frequencyCoefficient = 0.5f;
        detuneFrequencyCoefficients[numVoicesAssigned++] = 1 - frequencyCoefficient;
        detuneFrequencyCoefficients[numVoicesAssigned++] = 1 + frequencyCoefficient;
    }
}

// detune volume coefficients
void Oscillator::calculateDetuneVolumeCoefficients()
{
    int numVoicesAssigned = 0;

    // create 1 center voice
    detuneVolumeCoefficients[numVoicesAssigned++] = 1;

    // even num voices: create a 2nd center voice
    if (detuneVoices % 2 == 0)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = 1;
    }

    // assign mixed volume to remaining voices
    int numVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair <= numVoicePairsToCreate; voicePair++)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = detuneMix;
        detuneVolumeCoefficients[numVoicesAssigned++] = detuneMix;
    }
}

// detune panning values
void Oscillator::calculateDetunePanningOffsets()
{
    int numVoicesAssigned = 0;

    // if odd number of voices, assign basePan to center voice
    if (detuneVoices % 2 != 0)
    {
        detunePanningOffsets[numVoicesAssigned++] = basePan;
    }

    // offsets are symmetric from -maxPanningOffset to +maxPanningOffset
    const float maxPanningOffset = 0.5f * detuneSpread;

    // panning step is the panning offset for the next further out pair of voices
    float panningStep = maxPanningOffset / std::floor(detuneVoices / 2.0);

    // create voices
    int numVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair <= numVoicePairsToCreate; ++voicePair)
    {
        float panOffset = panningStep * voicePair;
        detunePanningOffsets[numVoicesAssigned++] = 0 - panOffset;
        detunePanningOffsets[numVoicesAssigned++] = 0 + panOffset;
    }
}

// recalculate all dependent values
void Oscillator::updateDetuneVoiceConfiguration()
{
    calculateDetuneFrequencyCoefficients();
    calculateDetuneVolumeCoefficients();
    calculateDetunePanningOffsets();
}

//=============================================================================
// DETUNE PARAMETERS

// detune num voices
void Oscillator::setDetuneVoices(int newNumVoices)
{
    // limit newNumVoices to [1, MAX_DETUNE_VOICES]
    newNumVoices = std::max(1, newNumVoices);
    newNumVoices = std::min(MAX_DETUNE_VOICES, newNumVoices);

    this->detuneVoices = newNumVoices;
};

int Oscillator::getDetuneVoices()
{
    return detuneVoices;
}

// detune mix
void Oscillator::setDetuneMix(float newDetuneMix)
{
    // limit newDetuneMix to [0, 1]
    newDetuneMix = std::max(0.f, newDetuneMix);
    newDetuneMix = std::min(1.f, newDetuneMix);

    this->detuneMix = newDetuneMix;
}

float Oscillator::getDetuneMix()
{
    return detuneMix;
}

// detune spread
void Oscillator::setDetuneSpread(float newDetuneSpread)
{
    // limit newDetuneSpread to [0, 1]
    newDetuneSpread = std::max(0.f, newDetuneSpread);
    newDetuneSpread = std::min(1.f, newDetuneSpread);

    this->detuneMix = newDetuneSpread;
}

float Oscillator::getDetuneSpread()
{
    return detuneSpread;
}

//=============================================================================
// RENDER PARAMETERS

// sample rate
void Oscillator::setSampleRate(float newSampleRate)
{
    // limit sample rate to [0, 192k]
    newSampleRate = juce::jmax(0.f, newSampleRate);
    newSampleRate = juce::jmin(192000.f, newSampleRate);

    this->sampleRate = newSampleRate;
    this->adsrEnvelope.setSampleRate(newSampleRate);
}

// frequency
void Oscillator::setFrequency(float newFrequency)
{
    // limit frequnecy to [0, 20k]
    newFrequency = juce::jmax(0.f, newFrequency);
    newFrequency = juce::jmin(20000.f, newFrequency);

    this->baseFrequency = newFrequency;
}

// volume
void Oscillator::setVolume(float newVolume)
{
    // limit volume to [0, 1]
    newVolume = juce::jmax(0.f, newVolume);
    newVolume = juce::jmin(1.f, newVolume);

    this->baseVolume = newVolume;
}

// velocity
void Oscillator::setVelocity(float newVelocity)
{
    // limit velocity to [0, 1]
    newVelocity = juce::jmax(0.f, newVelocity);
    newVelocity = juce::jmin(1.f, newVelocity);

    this->velocity = newVelocity;
}

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
//

void Oscillator::setAdsrParameters(juce::ADSR::Parameters adsrParameters)
{
    adsrEnvelope.setParameters(adsrParameters);
}

void Oscillator::startAdsrEnvelope()
{
    adsrEnvelope.reset();
    adsrEnvelope.noteOn();
}

void Oscillator::releaseAdsrEnvelope()
{
    adsrEnvelope.noteOff();
}

bool Oscillator::adsrEnvelopeIsActive() const
{
    return adsrEnvelope.isActive();
}