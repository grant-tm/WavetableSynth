#ifndef DETUNE_AND_WARP_CONTROLS_H
#define DETUNE_AND_WARP_CONTROLS_H

#include <JuceHeader.h>

enum WarpModes {
    Sync = 0,
    WindowedSync,
    Bend,
    PWM,
    Asym,
    Flip,
    Mirror,
    Quantize,
    NumWarpModes
};

class DetuneVoiceSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics &g,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider &slider) override;
};

class WarpModeSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics &g,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider &slider) override;
};

class DetuneAndWarpControlBar : public juce::Component
{
public:

    DetuneAndWarpControlBar();
    ~DetuneAndWarpControlBar();

    DetuneVoiceSliderLookAndFeel detuneVoiceSliderLNF;
    WarpModeSliderLookAndFeel warpModeSliderLNF;

    juce::Slider detuneVoiceSlider;
    juce::Slider warpModeSlider;

    void resized() override;
    void paint(juce::Graphics &g) override;

};

#endif // DETUNE_AND_WARP_CONTROLS_H