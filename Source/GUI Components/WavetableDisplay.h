#ifndef WAVETABLE_DISPLAY_H
#define WAVETABLE_DISPLAY_H

#include <JuceHeader.h>
#include "../PluginProcessor.h"

struct WavetableDisplayComponent : 
    juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
public:
    
    // CONSTRUCTORS / DESTRUCTORS
    WavetableDisplayComponent(WavetableSynthAudioProcessor&);
    ~WavetableDisplayComponent();
    
    // COMPONENT OVERRIDES
    void paint(juce::Graphics& g) override;
    void resized() override;

    // TIMER SETS REFRESH RATE
    void timerCallback() override;

    // WAVETABLE UPDATE SIGNAL
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };

private:
    
    // REFERENCE TO AUDIO PROCESSOR
    WavetableSynthAudioProcessor& audioProcessor;

    const Wavetable *wavetableRef;
    int wavetableCurrentFrameIndex;

    // WAVETABLE
    juce::AudioBuffer<float> wavetable;
    juce::Atomic<bool> wavetableChanged{ false };
    void updateWavetable();
    float getHermiteInterpolatedWavetableSample(float phase);
    float getLinearlyInterpolatedWavetableSample(float phase);

    // PATH PRODUCERS
    juce::Path createPathFromWavetable();
    juce::Path createLineLevelPath();
};

#endif // WAVETABLE_DISPLAY_H