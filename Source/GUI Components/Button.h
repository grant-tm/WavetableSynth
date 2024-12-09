#ifndef BUTTON_H
#define BUTTON_H

#include <JuceHeader.h>

struct ButtonLookAndFeel : public juce::LookAndFeel_V4
{

};

struct Button : public juce::Button
{
	Button();
	Button(const juce::String &buttonName, juce::Colour backgroundColor, juce::Colour borderColour, juce::Colour textColour);
	~Button(){}
	
	void paintButton (juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

	juce::Colour backgroundColor;
	juce::Colour borderColor;
	juce::Colour textColor;
};

#endif // BUTTON_H