#ifndef ADSR_CONTROLS_H
#define ADSR_CONTROLS_H

#include <JuceHeader.h>

class AdsrSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics &g,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider &slider) override;
};

class SustainSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics &g,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider &slider) override;
};

class AdsrControlBar : public juce::Component
{
public:

    AdsrControlBar();
    ~AdsrControlBar();

    AdsrSliderLookAndFeel sliderLNF;
    SustainSliderLookAndFeel sustainSliderLNF;

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    void resized() override;
    void paint(juce::Graphics &g) override;

};

#endif // ADSR_CONTROLS_H