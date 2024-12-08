#include "WavetableSlider.h"

enum WavetableSliderColors : uint32_t
{
    fillColor = 0xFF528187,
    borderColor = 0xFF0F1D1F,
    textColor = 0xFF0F1D1F
};

void WavetableSliderLookAndFeel::drawLinearSlider(
    juce::Graphics &g,
    int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    juce::Slider::SliderStyle sliderStyle,
    juce::Slider &slider
) {
   
    const auto trackWidth = 7;
    const auto thumbWidth = 35;
    const auto thumbHeight = 14;
    
    auto bounds = juce::Rectangle<float>(float(x), float(y), float(width), float(height));

    //---------------------------------------------------------------------
    // DRAW SLIDER TRACK
    
    auto trackBounds = bounds;
    trackBounds.removeFromLeft((bounds.getWidth() / 2) - (trackWidth / 2));
    trackBounds.removeFromRight((bounds.getWidth() / 2) - (trackWidth / 2));

    g.setColour(juce::Colour(WavetableSliderColors::borderColor));
    g.fillRoundedRectangle(trackBounds, 4.f);

    //---------------------------------------------------------------------
    // DRAW SLIDER THUMB

    auto thumbY = y - juce::jmap(sliderPos, float(y - height), float(y));
    thumbY = juce::jmin(trackBounds.getBottom() - thumbHeight, thumbY);
    auto thumbBounds = juce::Rectangle<float>(
        bounds.getCentreX() - (thumbWidth / 2),
        thumbY,
        thumbWidth,
        thumbHeight
    );

    g.setColour(juce::Colour(WavetableSliderColors::fillColor));
    g.fillRect(thumbBounds);
    
    g.setColour(juce::Colour(WavetableSliderColors::borderColor));
    g.drawRect(thumbBounds, 3.f);

}

//====================================================================
// SLIDER

WavetableSlider::WavetableSlider(juce::RangedAudioParameter &rangedAudioParameter, const juce::String &label) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    parameter(&rangedAudioParameter),
    label(label)
{
    setLookAndFeel(&wavetableSliderLookAndFeel);
}

WavetableSlider::~WavetableSlider()
{
    setLookAndFeel(nullptr);
}

void WavetableSlider::paint(juce::Graphics &g)
{
    using namespace juce;

    auto minValue = 0.f;
    auto maxValue = 100.f;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();

    auto labelBounds = getLocalBounds();
    labelBounds.removeFromTop(sliderBounds.getWidth());
    labelBounds = labelBounds.removeFromTop(getTextHeight() + 10);

    //---------------------------------------------------------------------
    // DEBUG: show knob bounding boxes

    /*
    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);
    g.setColour(Colours::magenta);
    g.drawRect(labelBounds);
    */

    //---------------------------------------------------------------------
    // DRAW KNOB

    getLookAndFeel().drawLinearSlider(
        g,
        sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getWidth(),
        jmap(float(getValue()), float(range.getStart()), float(range.getEnd()), 0.f, 1.f),
        minValue, maxValue,
        juce::Slider::SliderStyle::LinearVertical,
        *this
    );

    //---------------------------------------------------------------------
    // DRAW LABEL

    juce::FontOptions fontOptions;
    juce::Font labelFont(fontOptions);
    labelFont.setBold(true);
    labelFont.setHeight(16);
    labelFont.setTypefaceName(labelFont.getDefaultMonospacedFontName());

    auto floatLabelBounds = labelBounds.toFloat();
    Rectangle<float> r(floatLabelBounds.getX(), floatLabelBounds.getY(), floatLabelBounds.getWidth(), floatLabelBounds.getHeight());
    g.setColour(Colour(WavetableSliderColors::textColor));
    g.setFont(labelFont);
    g.drawFittedText(label, r.toNearestInt(), juce::Justification::centred, 1);
}

juce::Rectangle<int> WavetableSlider::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    auto size = bounds.getHeight();
    size -= getTextHeight();

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2); // two pixels below top of component

    return r;
}