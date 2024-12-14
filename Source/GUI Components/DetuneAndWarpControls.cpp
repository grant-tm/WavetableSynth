#include "DetuneAndWarpControls.h"

void DetuneVoiceSliderLookAndFeel::drawLinearSlider(
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
	g.setColour(juce::Colour(0xFF78A18C));
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
	juce::String sliderText(sliderValue);
	g.setColour(juce::Colour(0xFF78A18C));
	g.drawFittedText(sliderText, textBounds, juce::Justification::bottomLeft, 1);
};

void WarpModeSliderLookAndFeel::drawLinearSlider(
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
	g.setColour(juce::Colour(0xFF78A18C));
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
	juce::String sliderText;

	switch ((int)slider.getValue())
	{
	case WarpModes::Sync:
		sliderText = "Sync";
		break;
	case WarpModes::WindowedSync:
		sliderText = "Windowed Sync";
		break;
	case WarpModes::Bend:
		sliderText = "Bend";
		break;
	case WarpModes::PWM:
		sliderText = "PWM";
		break;
	case WarpModes::Asym:
		sliderText = "Asym";
		break;
	case WarpModes::Flip:
		sliderText = "Flip";
		break;
	case WarpModes::Mirror:
		sliderText = "Mirror";
		break;
	case WarpModes::Quantize:
		sliderText = "Quantize";
		break;
	default:
		sliderText = "--";
	}

	g.setColour(juce::Colour(0xFF78A18C));
	g.drawFittedText(sliderText, textBounds, juce::Justification::bottomLeft, 1);
};

DetuneAndWarpControlBar::DetuneAndWarpControlBar() :
	juce::Component()
{
	// configure detune voice slider
	detuneVoiceSlider.setName("DET. VOICES");
	detuneVoiceSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	detuneVoiceSlider.setVelocityBasedMode(true);
	detuneVoiceSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	detuneVoiceSlider.setRange(1, 16, 1);
	detuneVoiceSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	detuneVoiceSlider.setLookAndFeel(&detuneVoiceSliderLNF);
	addAndMakeVisible(detuneVoiceSlider);

	// configure warp mode slider
	warpModeSlider.setName("WARP MODE");
	warpModeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	warpModeSlider.setVelocityBasedMode(true);
	warpModeSlider.setVelocityModeParameters(0.25, 1, 0.1, true);
	warpModeSlider.setRange(0, WarpModes::NumWarpModes - 1, 1);
	warpModeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	warpModeSlider.setLookAndFeel(&warpModeSliderLNF);
	addAndMakeVisible(warpModeSlider);
}

DetuneAndWarpControlBar::~DetuneAndWarpControlBar()
{
	detuneVoiceSlider.setLookAndFeel(nullptr);
	warpModeSlider.setLookAndFeel(nullptr);
}

void DetuneAndWarpControlBar::resized()
{
	auto bounds = getLocalBounds();
	bounds.removeFromLeft(5);
	bounds.removeFromRight(5);

	detuneVoiceSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
	warpModeSlider.setBounds(bounds);
}

void DetuneAndWarpControlBar::paint(juce::Graphics &g)
{
	auto bounds = getLocalBounds();

	g.setColour(juce::Colour(0xFFD7FFEB));
	g.fillAll();

	g.setColour(juce::Colour(0xFF0F1D1F));
	g.drawRect(bounds, 5);
}