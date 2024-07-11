#include "WavetableOscillator.h"

WavetableOscillator::WavetableOscillator() {
    sampleRate = 0.f;
    frequency = 100.f;
    level = 0.125f;
    wavetableSize = 0;
    phase = 0.f;
    sampleIndex = 0.f;
    sampleOffset = 0.f;
    updateDeltaPhase();
};

WavetableOscillator::~WavetableOscillator() {};

void WavetableOscillator::setSampleRate(float newSampleRate)
{
    this->sampleRate = (newSampleRate >= 0.f) ? newSampleRate : 0.f;
    updateDeltaPhase();
}

void WavetableOscillator::setFrequency(float newFrequency)
{
    this->frequency = (newFrequency >= 0.f) ? newFrequency : 0.f;
    updateDeltaPhase();
}

void WavetableOscillator::updateDeltaPhase()
{
    deltaPhase = (sampleRate > 0.f) ? (frequency / sampleRate) : 0.f;
}

// TODO: SIMD
void WavetableOscillator::updatePhase()
{
    // TODO: use fixed point instead of wrapping with std::floor
    phase += deltaPhase;
    phase -= std::floor(phase);

    float scaledPhase = phase * wavetableSize;

    sampleIndex = (int)scaledPhase;
    sampleOffset = scaledPhase - (float)sampleIndex;
}

// TODO: SIMD
float WavetableOscillator::getNextSample()
{
    // select 4 samples around sampleIndex
    float val0 = wavetable.getSample(0, (sampleIndex - 1) % wavetableSize);
    float val1 = wavetable.getSample(0, (sampleIndex + 0) % wavetableSize);
    float val2 = wavetable.getSample(0, (sampleIndex + 1) % wavetableSize);
    float val3 = wavetable.getSample(0, (sampleIndex + 2) % wavetableSize);

    // calculate slopes to use at points val1 and val2 (avoid discontinuities)
    float slope0 = (val2 - val1) * 0.5f;
    float slope1 = (val3 - val1) * 0.5f;

    // calculate interpolation coefficients
    float delta = val1 - val2;
    float slopeSum = slope0 + delta;
    float coefficientA = slopeSum + delta + slope0;
    float coefficientB = slopeSum + coefficientA;

    // perform interpolation
    float stage1 = coefficientA * sampleOffset - coefficientB;
    float stage2 = stage1 * sampleOffset + slope0;
    float result = stage2 * sampleOffset + val1;

    return result;
}

void WavetableOscillator::fillBuffer(juce::AudioBuffer<float>& buffer)
{
    auto leftBuffer = buffer.getWritePointer(0);
    auto rightBuffer = buffer.getWritePointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto currentSample = getNextSample();
        updatePhase();

        leftBuffer[sample] = currentSample * level;
        rightBuffer[sample] = currentSample * level;
    }
}