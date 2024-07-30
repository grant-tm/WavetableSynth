#include "Oscillator.h"

inline int clampInt (int input, int lowerBound, int upperBound)
{
    input = juce::jmax(lowerBound, input);
    return juce::jmin(upperBound, input);
}

inline float clampFloat (float input, float lowerBound, float upperBound)
{
    input = juce::jmax(lowerBound, input);
    return juce::jmin(upperBound, input);
}

//=============================================================================
// CONSTRUCTORS / DESTRUCTORS

Oscillator::Oscillator()
{
    wavetable = nullptr;
    wavetableSize = 0;
    wavetableNumFrames = 0;
    wavetableFrameIndex = 0;

    randomizePhases();
    deltaPhase = 0.f;
    sampleIndex = 0;
    sampleOffset = 0.f;
    sampleOffset = 0.f;

    sampleRate = 0.f;
    baseFrequency = 0.f;
    baseVolume = 0.f;
    basePan = 0.f;
    velocity = 1.f;

    renderFrequency = 150.f;
    renderVolume = 1.f;
    renderPanCoefficientLeft = 1.f;
    renderPanCoefficientRight = 1.f;

    detuneVoices = 1;
    detuneMix = 1.f;
    detuneSpread = 1.f;
    for (auto& detuneFrequencyCoefficient : detuneFrequencyCoefficients)
    {
        detuneFrequencyCoefficient = 1.f;
    }
    for (auto& detuneVolumeCoefficient : detuneVolumeCoefficients)
    {
        detuneVolumeCoefficient = 1.f;
    }
    for (auto& detunePanningOffset : detunePanningOffsets)
    {
        detunePanningOffset = 0.f;
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
        for (int outputSampleIndex = startSample; outputSampleIndex < (startSample + numSamples); outputSampleIndex++)
        {
            incrementPhase(detuneVoice);
            auto sampleValue = getNextSample() * renderVolume * velocity;
            
            // apply ADSR envelope
            sampleValue *= adsrEnvelope.isActive() ? adsrEnvelope.getNextSample() : 0.f;
            
            output[0][outputSampleIndex] += sampleValue * renderPanCoefficientLeft;
            output[1][outputSampleIndex] += sampleValue * renderPanCoefficientRight;
        }
    }
}

float Oscillator::getNextSample()
{
    // select 4 samples around sampleIndex
    auto values = wavetable->getReadPointer(wavetableFrameIndex);
    const float val0 = values[(sampleIndex - 1 + wavetableSize) % wavetableSize];
    const float val1 = values[(sampleIndex + 0) % wavetableSize];
    const float val2 = values[(sampleIndex + 1) % wavetableSize];
    const float val3 = values[(sampleIndex + 2) % wavetableSize];

    // calculate slopes to use at points val1 and val2 (avoid discontinuities)
    const float slope0 = (val2 - val0) * 0.5f;
    const float slope1 = (val3 - val1) * 0.5f;

    // calculate interpolation coefficients
    const float delta = val1 - val2;
    const float slopeSum = slope0 + delta;
    const float coefficientA = slopeSum + delta + slope1;
    const float coefficientB = slopeSum + coefficientA;

    // perform interpolation
    const float stage1 = coefficientA * sampleOffset - coefficientB;
    const float stage2 = stage1 * sampleOffset + slope0;
    const float result = stage2 * sampleOffset + val1;

    return result;
}

//=============================================================================
// APPLY RENDER PARAMETERS

void Oscillator::applyRenderParameters(int detuneVoice)
{
    if (detuneVoices > 1 && detuneMix > 0.f)
        applyDetuneRenderParameters(detuneVoice);
    else
        applyBaseRenderParameters();
}

void Oscillator::applyDetuneRenderParameters(int detuneVoice)
{
    renderFrequency = baseFrequency * detuneFrequencyCoefficients[detuneVoice];
    renderVolume = baseVolume * detuneVolumeCoefficients[detuneVoice];
    calculateRenderPanCoefficients(basePan + detunePanningOffsets[detuneVoice]);
}

void Oscillator::applyBaseRenderParameters()
{
    renderFrequency = baseFrequency;
    renderVolume = baseVolume;
    calculateRenderPanCoefficients(basePan);
}

void Oscillator::calculateRenderPanCoefficients(float pan)
{
    float leftAngle = (juce::MathConstants<float>::pi / 4.0f) * (1.0f + pan);
    renderPanCoefficientLeft = std::cos(leftAngle);

    float rightAngle = (juce::MathConstants<float>::pi / 4.0f) * (1.0f + pan);
    renderPanCoefficientRight = std::sin(rightAngle);
}

//=============================================================================
// PHASE UPDATE

// update the phase and calculate sampleIndex and sampleOffset
void Oscillator::incrementPhase(int phaseIndex)
{
    phases[phaseIndex] += deltaPhase;
    phases[phaseIndex] -= std::floor(phases[phaseIndex]);

    float scaledPhase = phases[phaseIndex] * wavetableSize;

    sampleIndex = (int) scaledPhase;
    sampleOffset = scaledPhase - (float) sampleIndex;
}

// calculate and update deltaPhase based on frequency and sampleRate
void Oscillator::updateDeltaPhase()
{
    if (sampleRate == 0.f)
        deltaPhase = 0.f;
    else
        deltaPhase = juce::jmax(0.f, renderFrequency / sampleRate);
}

void Oscillator::randomizePhases()
{
    juce::Random rng;
    for (auto &phase : phases)
    {
        phase = rng.nextFloat();
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
    const float maxFrequencyCoefficient = MAX_DETUNE_SPREAD * 0.2f * detuneSpread;
    float frequencyStep = maxFrequencyCoefficient / (float) std::floor(detuneVoices / 2);

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
    detuneVolumeCoefficients[numVoicesAssigned++] = 1.f;

    // even num voices: create a 2nd center voice
    if (detuneVoices % 2 == 0)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = 1.f;
    }

    // assign mixed volume to remaining voices
    int numVoicePairsToCreate = (detuneVoices - numVoicesAssigned) / 2;
    for (int voicePair = 1; voicePair <= numVoicePairsToCreate; voicePair++)
    {
        detuneVolumeCoefficients[numVoicesAssigned++] = 1.f * detuneMix;
        detuneVolumeCoefficients[numVoicesAssigned++] = 1.f * detuneMix;
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
    float panningStep = maxPanningOffset / (float) std::floor(detuneVoices / 2.0);

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
int Oscillator::getDetuneVoices() const
{
    return detuneVoices;
}

void Oscillator::setDetuneVoices(int newNumVoices)
{
    this->detuneVoices = clampInt(newNumVoices, 1, MAX_DETUNE_VOICES);
};

// detune mix
float Oscillator::getDetuneMix() const
{
    return detuneMix;
}

void Oscillator::setDetuneMix(float newDetuneMix)
{
    this->detuneMix = clampFloat(newDetuneMix, 0.f, 1.f);
}

// detune spread
float Oscillator::getDetuneSpread() const
{
    return detuneSpread;
}

void Oscillator::setDetuneSpread(float newDetuneSpread)
{
    this->detuneMix = clampFloat(newDetuneSpread, 0.f, 1.f);
}

//=============================================================================
// RENDER PARAMETERS

// sample rate [0, 192000]
void Oscillator::setSampleRate(float newSampleRate)
{
    newSampleRate = clampFloat(newSampleRate, 0.f, 192000.f);
    this->sampleRate = newSampleRate;
    this->adsrEnvelope.setSampleRate(newSampleRate);
}

// frequency [0, 200000]
void Oscillator::setFrequency(float newFrequency)
{
    this->baseFrequency = clampFloat(newFrequency, 0.f, 20000.f);
}

// volume [0, 1]
void Oscillator::setVolume(float newVolume)
{
    this->baseVolume = clampFloat(newVolume, 0.f, 1.f);
}

// velocity [0, 1]
void Oscillator::setVelocity(float newVelocity)
{
    this->velocity = clampFloat(newVelocity, 0.f, 1.f);
}

// pan [-1, 1]
void Oscillator::setPan(float newPan)
{
    this->basePan = clampFloat(newPan, -1.f, 1.f);
}

//=============================================================================
// WAVETABLE

void Oscillator::setWavetable(const Wavetable *wavetableToUse)
{
    wavetable = wavetableToUse;
    wavetableSize = wavetable->getNumSamples();
    wavetableNumFrames = wavetable->getNumChannels();
}

// [0, wavetableNumFrames]
void Oscillator::setWavetableFrameIndex(int newFrameIndex)
{
    this->wavetableFrameIndex = clampInt(newFrameIndex, 0, wavetableNumFrames);
}

//=============================================================================
// ADSR

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