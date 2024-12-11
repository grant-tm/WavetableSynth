#include "AdsrControls.h"

void AdsrSliderLookAndFeel::drawLinearSlider(
	juce::Graphics &g,
	int x, int y, int width, int height,
	float sliderPos, float minSliderPos, float maxSliderPos,
	juce::Slider::SliderStyle style, juce::Slider &slider
) {
	auto bounds = juce::Rectangle<int>(x, y - 4, width, height);
	bounds.removeFromBottom(0);
	bounds.removeFromTop(2);
	bounds.removeFromLeft(2);
	bounds.removeFromRight(2);

	// draw shadow
	g.setColour(juce::Colour(0xFF97C6AE));
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY()); // draw top edge
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom()); // draw left edge

	//--------------------------------------------------------
	// TEXT

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
	
	// time in miliseconds: show as whole numbers
	juce::String sliderText;
	if (sliderValue < 1.0) {
		sliderValue *= 1000;
		if (sliderValue > 100.0) {
			sliderValue = (int) sliderValue;
		}
		sliderText = juce::String(sliderValue) + juce::String(" ms");
	}
	else {
		sliderValue *= 10;
		sliderValue = (int) sliderValue;
		sliderValue /= 10;
		sliderText = juce::String(sliderValue) + juce::String(" s");
	}

	labelFont.setHeight(12);
	g.setFont(labelFont);
	g.setColour(juce::Colour(0xFF97C6AE));
	textBounds.removeFromTop(4);
	g.drawFittedText(sliderText, textBounds, juce::Justification::bottomLeft, 1);

	//--------------------------------------------------------
	// SLIDER

	auto sliderBounds = bounds;
	sliderBounds.removeFromTop(6);
	sliderBounds.removeFromBottom(20);
	sliderBounds.removeFromLeft(4 + sliderBounds.getWidth() / 2);
	sliderBounds.removeFromRight(4);

	auto range = slider.getRange();
	auto min = range.getStart();
	auto max = range.getEnd();
	sliderBounds.removeFromRight((max - slider.getValue()) * sliderBounds.getWidth() / (max - min));

	g.setColour(juce::Colour(0xFF97C6AE));
	g.fillRect(sliderBounds);

};

void SustainSliderLookAndFeel::drawLinearSlider(
	juce::Graphics &g,
	int x, int y, int width, int height,
	float sliderPos, float minSliderPos, float maxSliderPos,
	juce::Slider::SliderStyle style, juce::Slider &slider
) {
	auto bounds = juce::Rectangle<int>(x, y - 4, width, height);
	bounds.removeFromBottom(0);
	bounds.removeFromTop(2);
	bounds.removeFromLeft(2);
	bounds.removeFromRight(2);

	// draw shadow
	g.setColour(juce::Colour(0xFF97C6AE));
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY()); // draw top edge
	g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom()); // draw left edge

	//--------------------------------------------------------
	// TEXT

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

	// time in miliseconds: show as whole numbers
	juce::String sliderText(sliderValue);

	labelFont.setHeight(12);
	g.setFont(labelFont);
	g.setColour(juce::Colour(0xFF97C6AE));
	textBounds.removeFromTop(4);
	g.drawFittedText(sliderText, textBounds, juce::Justification::bottomLeft, 1);

	//--------------------------------------------------------
	// SLIDER

	auto sliderBounds = bounds;
	sliderBounds.removeFromTop(6);
	sliderBounds.removeFromBottom(20);
	sliderBounds.removeFromLeft(4 + sliderBounds.getWidth() / 2);
	sliderBounds.removeFromRight(4);

	auto range = slider.getRange();
	auto min = range.getStart();
	auto max = range.getEnd();
	sliderBounds.removeFromRight((max - slider.getValue()) * sliderBounds.getWidth() / (max - min));

	g.setColour(juce::Colour(0xFF97C6AE));
	g.fillRect(sliderBounds);

};

AdsrControlBar::AdsrControlBar() :
	juce::Component()
{
	// configure octave slider
	attackSlider.setName("ATK");
	attackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	attackSlider.setVelocityBasedMode(true);
	attackSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	attackSlider.setRange(0.0001, 15.0, 0.0001);
	attackSlider.setSkewFactor(0.15, false);
	attackSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	attackSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(attackSlider);

	// configure semitone slider
	decaySlider.setName("DEC");
	decaySlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	decaySlider.setVelocityBasedMode(true);
	decaySlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	decaySlider.setRange(0.0, 15.0, 0.01);
	decaySlider.setSkewFactor(0.15, false);
	decaySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	decaySlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(decaySlider);

	// configure fine tune slider
	sustainSlider.setName("SUS");
	sustainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	sustainSlider.setVelocityBasedMode(true);
	sustainSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	sustainSlider.setRange(0.0, 1.0, 0.01);
	sustainSlider.setSkewFactor(1, false);
	sustainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	sustainSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(sustainSlider);

	// configure coarse pitch slider
	releaseSlider.setName("REL");
	releaseSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 0, 0);
	releaseSlider.setVelocityBasedMode(true);
	releaseSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	releaseSlider.setRange(0.0, 15.0, 0.01);
	releaseSlider.setSkewFactor(0.15, false);
	releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	releaseSlider.setLookAndFeel(&sliderLNF);
	addAndMakeVisible(releaseSlider);
}

AdsrControlBar::~AdsrControlBar()
{
	attackSlider.setLookAndFeel(nullptr);
	decaySlider.setLookAndFeel(nullptr);
	sustainSlider.setLookAndFeel(nullptr);
	releaseSlider.setLookAndFeel(nullptr);
}

void AdsrControlBar::resized()
{
	auto bounds = getLocalBounds();
	bounds.removeFromLeft(5);
	bounds.removeFromRight(5);

	attackSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 4));
	decaySlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 3));
	sustainSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
	releaseSlider.setBounds(bounds.removeFromLeft(bounds.getWidth()));
}

void AdsrControlBar::paint(juce::Graphics &g)
{
	auto bounds = getLocalBounds();

	g.setColour(juce::Colour(0xFFD7FFEB));
	g.fillAll();

	g.setColour(juce::Colour(0xFF0F1D1F));
	g.drawRect(bounds, 5);
}