#ifndef SINE_OSCILLATOR_H
#define SINE_OSCILLATOR_H

#include <JuceHeader.h>

class SineOscillator
{
public:
    SineOscillator();
    ~SineOscillator();

    void updateAngleDelta();
    void incrementAngle();

    void setSampleRate(float);
    void setFrequency(float);

    void fillBuffer(juce::AudioBuffer<float>&);

private:
    float sampleRate;
    float frequency;
    float level;

    float angleDelta;
    float currentAngle;
};

#endif // SINE_OSCILLATOR_H