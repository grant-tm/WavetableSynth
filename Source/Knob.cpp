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

Knob::Knob(juce::RangedAudioParameter& rangedAudioParameter, const juce::String& unitSuffix)
    : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rangedAudioParameter),
    suffix(unitSuffix)
{
    setLookAndFeel(&lookAndFeel);
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

    // DEBUG: show knob bounding boxes
    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(
        g,
        sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(),
        jmap(float(getValue()), float(range.getStart()), float(range.getEnd()), 0.f, 1.f),
        startAng, endAng,
        *this
    );

    //auto center = sliderBounds.toFloat().getCentre();
    //auto radius = sliderBounds.getWidth() * 0.5f;

    //// draw labels
    //g.setColour(Colour(KnobColors::textColor));
    //g.setFont(float(getTextHeight()));

    //auto numChoices = labels.size();
    //for (int i = 0; i < numChoices; ++i)
    //{
    //    auto pos = labels[i].position;
    //    jassert(0.f <= pos);
    //    jassert(1.f >= pos);

    //    auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

    //    auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

    //    Rectangle<float> r;
    //    auto str = labels[i].label;
    //    r.setSize(float(g.getCurrentFont().getStringWidth(str)), float(getTextHeight()));
    //    r.setCentre(c);
    //    r.setY(r.getY() + getTextHeight());
    //    g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    //}

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

juce::String Knob::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = float(getValue());

        if (val >= 1000.f)
        {
            val /= 1000.f;
            addK = true;
        }

        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
        {
            str << "k";
        }
        str << suffix;
    }
    return str;
}