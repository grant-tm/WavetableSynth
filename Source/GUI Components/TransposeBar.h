#ifndef TRANSPOSE_BAR_H
#define TRANSPOSE_BAR_H

#include <JuceHeader.h>

class TransposeSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics &g,
        int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider &slider) override;
};

class TransposeBar : public juce::Component
{
public:
    
    TransposeBar();
    ~TransposeBar();

    TransposeSliderLookAndFeel sliderLNF;

    juce::Slider octaveSlider;
    juce::Slider semitoneSlider;
    juce::Slider fineSlider;
    juce::Slider coarseSlider;

    void resized() override;
    void paint(juce::Graphics &g) override;
    
};

#endif // TRANSPOSE_BAR_H