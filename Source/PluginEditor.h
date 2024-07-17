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

enum ColorPalette
{
    body = 0xFF64BEA5,
    border = 0xFF0F1D1F,
    screenMain = 0xFFD7FFEB,
    screenShadow = 0xFFB1E7CC,
    controlSurface = 0xFF528187
};

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

    std::vector<juce::Component*> getKnobs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
