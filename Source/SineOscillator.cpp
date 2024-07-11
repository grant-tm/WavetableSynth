#include "SineOscillator.h"

SineOscillator::SineOscillator()
{
    sampleRate = 0.0f;
    frequency = 100.0f;
    angleDelta = 0.0f;
    currentAngle = 0.0f;
    level = 0.125f;

    updateAngleDelta();
}

SineOscillator::~SineOscillator(){}

void SineOscillator::updateAngleDelta()
{
    auto cyclesPerSample = 0.0f;
    if (sampleRate > 0.0f)
    {
        cyclesPerSample = frequency / sampleRate;
    }
    angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
}

void SineOscillator::incrementAngle()
{
    currentAngle += angleDelta;
    if (currentAngle >= juce::MathConstants<float>::twoPi)
    {
        currentAngle -= juce::MathConstants<float>::twoPi;
    }
}

void SineOscillator::setSampleRate(float newSampleRate)
{
    this->sampleRate = (newSampleRate >= 0.f) ? newSampleRate : 0.f;
    updateAngleDelta();
}

void SineOscillator::setFrequency(float newFrequency)
{
    this->frequency = (newFrequency >= 0.f) ? newFrequency : 0.f;
    updateAngleDelta();
}

void SineOscillator::fillBuffer(juce::AudioBuffer<float>& buffer)
{
    auto leftBuffer = buffer.getWritePointer(0);
    auto rightBuffer = buffer.getWritePointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto currentSample = (float) std::sin(currentAngle);
        incrementAngle();

        leftBuffer[sample] = currentSample * level;
        rightBuffer[sample] = currentSample * level;
    }
}