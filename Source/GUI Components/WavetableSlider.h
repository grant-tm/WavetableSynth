#ifndef WAVETABLE_SLIDER_H
#define WAVETABLE_SLIDER_H

#include <JuceHeader.h>

//====================================================================
// WT SLIDER LOOK AND FEEL

struct WavetableSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider(juce::Graphics &,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, 
        juce::Slider &) override;
};

//====================================================================
// SLIDER

struct WavetableSlider : juce::Slider
{
    WavetableSlider(juce::RangedAudioParameter &rangedAudioParameter, const juce::String &label);
    ~WavetableSlider();

    void paint(juce::Graphics &g) override;

    juce::Rectangle<int> getSliderBounds() const;

    int getTextHeight() const { return 18; }

private:
    WavetableSliderLookAndFeel wavetableSliderLookAndFeel;
    juce::RangedAudioParameter *parameter;
    juce::String label;
};

#endif // WAVETABLE_SLIDER_H