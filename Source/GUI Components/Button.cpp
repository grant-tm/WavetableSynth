#include "Button.h"

Button::Button(
	const juce::String &buttonName,
	juce::Colour backgroundColor,
	juce::Colour borderColour,
	juce::Colour textColor
) : juce::Button(buttonName) {
	this->backgroundColor = backgroundColor;
	this->borderColor = borderColour;
	this->textColor = textColor;
}

void Button::paintButton(
	juce::Graphics &g,
	bool shouldDrawButtonAsHighlighted,
	bool shouldDrawButtonAsDown
) {
	auto bounds = getLocalBounds();

	g.setColour(borderColor);
	g.fillRoundedRectangle(bounds.toFloat(), 5.f);

	bounds.removeFromTop(2.f);
	bounds.removeFromBottom(2.f);
	bounds.removeFromLeft(2.f);
	bounds.removeFromRight(2.f);

	g.setColour(backgroundColor);
	g.fillRoundedRectangle(bounds.toFloat(), 5.f);

	juce::FontOptions fontOptions;
	juce::Font labelFont(fontOptions);
	labelFont.setBold(true);
	labelFont.setHeight(16);
	labelFont.setTypefaceName(labelFont.getDefaultMonospacedFontName());
	g.setFont(labelFont);
	g.setColour(textColor);
	g.drawFittedText(getButtonText(), bounds, juce::Justification::centred, 1);
}