#include "Knob.h"

enum KnobColors
{
    fillColor = 0xFF528187,
    borderColor = 0xFF0F1D1F,
    textColor = 0xFF0F1D1F
};

//====================================================================
// KNOB LOOK AND FEEL

void KnobLookAndFeel::drawRotarySlider(
    juce::Graphics &g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider &slider
) {
    using namespace juce;

    if (auto *knob = dynamic_cast<Knob *>(&slider)) {

        auto bounds = Rectangle<float>(float(x), float(y), float(width), float(height));
        auto knobBounds = bounds.removeFromTop(bounds.getWidth());
        knobBounds.removeFromTop(width * 0.04f);
        knobBounds.removeFromBottom(width * 0.04f);
        knobBounds.removeFromLeft(width * 0.04f);
        knobBounds.removeFromRight(width * 0.04f);

        //---------------------------------------------------------------------
        // DRAW KNOB BODY

        // draw knob background
        g.setColour(Colour(KnobColors::fillColor));
        g.fillEllipse(knobBounds);

        // draw knob border
        g.setColour(Colour(KnobColors::borderColor));
        g.drawEllipse(knobBounds, 5.f);

        //---------------------------------------------------------------------
        // DRAW KNOB THUMB

        // parameters to mess with
        const float thumbWidthProportionalToDiameter = 0.1f;
        const float thumbLengthProportionalToRadius = 0.66f;

        const auto knobCenter = knobBounds.getCentre();
        const auto knobDiameter = knobBounds.getWidth();
        const auto knobRadius = knobDiameter / 2;

        const auto thumbWidth = knobDiameter * thumbWidthProportionalToDiameter;
        const auto thumbX = knobCenter.getX() - (thumbWidth / 2);
        const auto thumbY = knobRadius * thumbLengthProportionalToRadius;
        const auto thumbHeight = knobBounds.getY() - thumbY;

        // create path according to knob position and dimensions
        Path thumbPath;
        thumbPath.addRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 2.5f);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        thumbPath.applyTransform(AffineTransform().rotated(sliderAngRad, knobCenter.getX(), knobCenter.getY()));

        // draw thumb as rounded rectangle
        g.setColour(Colour(KnobColors::borderColor));
        g.fillPath(thumbPath);
    }
}

 
//====================================================================
// KNOB

Knob::Knob(juce::RangedAudioParameter& rangedAudioParameter, const juce::String& label) : 
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    knobParameter(&rangedAudioParameter),
    knobLabel(label)
{
    setLookAndFeel(&knobLookAndFeel);
}

Knob::~Knob()
{
    setLookAndFeel(nullptr);
}

void Knob::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    
    auto labelBounds = getLocalBounds();
    labelBounds.removeFromTop(sliderBounds.getWidth());
    labelBounds = labelBounds.removeFromTop(getTextHeight() + 10);

    //---------------------------------------------------------------------
    // DEBUG: show knob bounding boxes
    
    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);
    g.setColour(Colours::magenta);
    g.drawRect(labelBounds);

    //---------------------------------------------------------------------
    // DRAW KNOB

    getLookAndFeel().drawRotarySlider(
        g,
        sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getWidth(),
        jmap(float(getValue()), float(range.getStart()), float(range.getEnd()), 0.f, 1.f),
        startAng, endAng,
        *this
    );

    //---------------------------------------------------------------------
    // DRAW LABEL
    
    juce::Font labelFont;
    labelFont.setBold(true);
    labelFont.setHeight(16);
    labelFont.setTypefaceName(labelFont.getDefaultMonospacedFontName());

    //g.setFont(float(getTextHeight()));
    auto strWidth = g.getCurrentFont().getStringWidth(knobLabel);

    Rectangle<float> r(labelBounds.getX(), labelBounds.getY(), labelBounds.getWidth(), labelBounds.getHeight());
    g.setColour(Colour(KnobColors::textColor));
    g.setFont(labelFont);
    g.drawFittedText(knobLabel, r.toNearestInt(), juce::Justification::centred, 1);
}

juce::Rectangle<int> Knob::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 2;

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2); // two pixels below top of component

    return r;
}