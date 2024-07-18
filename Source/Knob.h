#ifndef KNOB_H
#define KNOB_H

#include <JuceHeader.h>

//====================================================================
// KNOB LOOK AND FEEL

struct KnobLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;
};

//====================================================================
// KNOB

struct Knob : juce::Slider
{
    Knob(juce::RangedAudioParameter& rangedAudioParameter, const juce::String& label);
    ~Knob();

    void paint(juce::Graphics& g) override;
    
    juce::Rectangle<int> getSliderBounds() const;
    
    int getTextHeight() const { return 18; }

private:
    KnobLookAndFeel knobLookAndFeel;
    juce::RangedAudioParameter* knobParameter;
    juce::String knobLabel;
};

#endif // KNOB_H
