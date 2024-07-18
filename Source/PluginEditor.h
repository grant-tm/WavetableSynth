/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "SynthesizerState.h"

#include "Knob.h"
#include "WavetableDisplay.h"

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

#define LARGE_KNOB_DIAMETER_PIXELS 80
#define SMALL_KNOB_DIAMETER_PIXELS 60

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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WavetableSynthAudioProcessor& audioProcessor;

    using KnobAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    Knob oscVolumeKnob;
    KnobAttachment oscVolumeKnobAttachment;

    Knob oscPanningKnob;
    KnobAttachment oscPanningKnobAttachment;

    struct WavetableDisplayComponent wavetableDisplay;

    std::vector<juce::Component*> getKnobs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
