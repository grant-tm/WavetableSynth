/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "Synthesizer/SynthesizerState.h"

#include "GUI Components/Knob.h"
#include "GUI Components/WavetableSlider.h"
#include "GUI Components/WavetableDisplay.h"

enum ColorPalette
{
    body = 0xFF64BEA5,
    border = 0xFF0F1D1F,
    screenMain = 0xFFD7FFEB,
    screenShadow = 0xFFB1E7CC,
    controlSurface = 0xFF528187
};

#define VST_WIDTH_PIXELS 500
#define VST_HEIGHT_PIXELS 450

#define TOP_BAR_HEIGHT_PIXELS 60

#define WAVETABLE_DISPLAY_WIDTH_PIXELS 305
#define WAVETABLE_DISPLAY_HEIGHT_PIXELS 245

#define LARGE_KNOB_DIAMETER_PIXELS 100
#define SMALL_KNOB_DIAMETER_PIXELS 80

//==============================================================================
/**
*/
class WavetableSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&);
    ~WavetableSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WavetableSynthAudioProcessor& audioProcessor;

    struct WavetableDisplayComponent wavetableDisplay;

    Knob oscVolumeKnob;
    Knob oscPanningKnob;
    Knob oscDetuneMixKnob;
    Knob oscWarpAmountKnob;
    WavetableSlider oscWavetablePositionKnob;

    std::vector<juce::Component *> getKnobs();

    using KnobAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    KnobAttachment oscVolumeKnobAttachment;
    KnobAttachment oscPanningKnobAttachment;
    KnobAttachment oscDetuneMixKnobAttachment;
    KnobAttachment oscWarpAmountKnobAttachment;
    KnobAttachment oscWavetablePositionKnobAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
