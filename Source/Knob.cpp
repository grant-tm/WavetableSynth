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
    juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider
) {
    using namespace juce;

    auto bounds = Rectangle<float>(float(x), float(y), float(width), float(height));

    // draw knob background
    g.setColour(Colour(KnobColors::fillColor));
    g.fillEllipse(bounds);

    // draw knob border
    g.setColour(Colour(KnobColors::borderColor));
    g.drawEllipse(bounds, 4.5f);

    if (auto* knob = dynamic_cast<Knob*>(&slider))
    {
        // draw knob position notch
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        auto center = bounds.getCentre();
        Rectangle<float> r;
        r.setLeft(center.getX() - 3);
        r.setRight(center.getX() + 3);
        r.setTop(bounds.getY());
        r.setBottom((center.getY() - bounds.getY()) * 0.66f);

        Path p;
        p.addRoundedRectangle(r, 3.f);
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.setColour(Colour(KnobColors::borderColor));
        g.fillPath(p);

        //// draw label
        //g.setFont(float(knob->getTextHeight()));
        //auto text = juce::String("TEST");
        //auto strWidth = g.getCurrentFont().getStringWidth(text);

        //r.setSize(float(strWidth + 4), float(knob->getTextHeight() + 2));
        //r.setCentre(bounds.getCentre());
        //g.setColour(Colour(KnobColors::fillColor));
        //g.fillRect(r);

        //g.setColour(Colour(KnobColors::textColor));
        //g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
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