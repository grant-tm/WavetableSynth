#include "TransposeBar.h"

void TransposeSliderLookAndFeel::drawLinearSlider(
	juce::Graphics &g,
	int x, int y, int width, int height,
	float sliderPos, float minSliderPos, float maxSliderPos,
	juce::Slider::SliderStyle style, juce::Slider &slider
) {
	auto bounds = juce::Rectangle<int>(x, y - 4, width, height);
	bounds.removeFromBottom(0);
	bounds.removeFromTop(0);
	bounds.removeFromLeft(2);
	bounds.removeFromRight(2);

	// draw shadow
	g.setColour(juce::Colour(0xFF97C6AE));
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY()); // draw top edge
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom()); // draw left edge

	// set text bounds
	auto textBounds = bounds;
	textBounds.removeFromBottom(0);
	textBounds.removeFromTop(4);
	textBounds.removeFromLeft(4);
	textBounds.removeFromRight(2);

	// set font
	juce::FontOptions fontOptions;
	juce::Font labelFont(fontOptions);
	labelFont.setHeight(16);
	labelFont.setTypefaceName(labelFont.getDefaultMonospacedFontName());
	g.setFont(labelFont);
	g.setColour(juce::Colour(0xFF1F464B));

	// draw slider name
	g.drawFittedText(slider.getName(), textBounds, juce::Justification::topLeft, 1);

	// draw slider value
	auto sliderValue = slider.getValue();
	juce::String sliderText;
	if (sliderValue == 0) {
		sliderText = juce::String("--");
	}
	else {
		sliderText = juce::String(sliderValue);
		if (sliderValue > 0)
			sliderText = "+" + sliderText;
	}
	g.drawFittedText(sliderText, textBounds, juce::Justification::bottomLeft, 1);
};

TransposeBar::TransposeBar() : 
	juce::Component()
{
	// configure octave slider
	octaveSlider.setName("OCT");
	octaveSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	octaveSlider.setVelocityBasedMode(true);
	octaveSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	octaveSlider.setRange(-4, 4, 1);
	octaveSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	octaveSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(octaveSlider);

	// configure semitone slider
	semitoneSlider.setName("SEMI");
	semitoneSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	semitoneSlider.setVelocityBasedMode(true);
	semitoneSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	semitoneSlider.setRange(-12, 12, 1);
	semitoneSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	semitoneSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(semitoneSlider);

	// configure fine tune slider
	fineSlider.setName("FINE");
	fineSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	fineSlider.setVelocityBasedMode(true);
	fineSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	fineSlider.setRange(-100, 100, 1);
	fineSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	fineSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(fineSlider);

	// configure coarse pitch slider
	coarseSlider.setName("CRSE");
	coarseSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	coarseSlider.setVelocityBasedMode(true);
	coarseSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	coarseSlider.setRange(-48, 48, 0.01);
	coarseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	coarseSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(coarseSlider);
}

TransposeBar::~TransposeBar()
{
	octaveSlider.setLookAndFeel(nullptr);
	semitoneSlider.setLookAndFeel(nullptr);
	fineSlider.setLookAndFeel(nullptr);
	coarseSlider.setLookAndFeel(nullptr);
}

void TransposeBar::resized()
{
	auto bounds = getLocalBounds();
	bounds.removeFromLeft(5);
	bounds.removeFromRight(5);

	octaveSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 4));
	semitoneSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 3));
	fineSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
	coarseSlider.setBounds(bounds.removeFromLeft(bounds.getWidth()));
}

void TransposeBar::paint(juce::Graphics &g)
{
	auto bounds = getLocalBounds();

	g.setColour(juce::Colour(0xFFD7FFEB));
	g.fillAll();

	g.setColour(juce::Colour(0xFF0F1D1F));
	g.drawRect(bounds, 5);
}