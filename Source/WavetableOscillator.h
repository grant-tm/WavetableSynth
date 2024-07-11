#ifndef WAVETABLE_OSCILLATOR_H
#define WAVETABLE_OSCILLATOR_H

#include <JuceHeader.h>

using Wavetable = juce::AudioBuffer<float>;

class WavetableOscillator
{
public:
    WavetableOscillator();
    ~WavetableOscillator();

    void setSampleRate(float);
    void setFrequency(float);

    void updateDeltaPhase();
    void updatePhase();

    float getNextSample();
    void fillBuffer(juce::AudioBuffer<float>&);

protected:

    // wavetable
    Wavetable wavetable;
    int wavetableSize;

    // oscillator parameters
    float sampleRate;
    float frequency;
    float level;

    // phase
    float phase;
    float deltaPhase;

    // sample index
    int sampleIndex;
    float sampleOffset;
};

#endif // WAVETABLE_OSCILLATOR_H