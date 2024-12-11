/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "Synthesizer/SynthesizerState.h"

#include "GUI Components/Button.h"
#include "GUI Components/TransposeBar.h"
#include "GUI Components/AdsrControls.h"
#include "GUI Components/Knob.h"
#include "GUI Components/WavetableSlider.h"
#include "GUI Components/WavetableDisplay.h"

enum ColorPalette : uint32_t
{
    body = 0xFF64BEA5,
    border = 0xFF0F1D1F,
    screenMain = 0xFFD7FFEB,
    screenShadow = 0xFFB1E7CC,
    controlSurface = 0xFF528187
};

#define VST_WIDTH_PIXELS 450
#define VST_HEIGHT_PIXELS 600

#define TOP_BAR_HEIGHT_PIXELS 60

#define WAVETABLE_DISPLAY_WIDTH_PIXELS 280
#define WAVETABLE_DISPLAY_HEIGHT_PIXELS 235

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

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using ControlSurfaceAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    WavetableSynthAudioProcessor& audioProcessor;

    Button loadButton;
    Button saveButton;
    Button editButton;
    Button viewButton;

    TransposeBar transposeBar;
    ControlSurfaceAttachment octaveTransposeAttachment;
    ControlSurfaceAttachment semitoneTransposeAttachment;
    ControlSurfaceAttachment fineTuneTransposeAttachment;
    ControlSurfaceAttachment coarsePitchTransposeAttachment;

    AdsrControlBar adsrControls;
    ControlSurfaceAttachment attackAttachment;
    ControlSurfaceAttachment decayAttachment;
    ControlSurfaceAttachment sustainAttachment;
    ControlSurfaceAttachment releaseAttachment;

    TransposeBar detuneVoicesAndWarpModeControls;

    struct WavetableDisplayComponent wavetableDisplay;

    Knob oscVolumeKnob;
    Knob oscPanningKnob;
    Knob oscDetuneMixKnob;
    Knob oscWarpAmountKnob;
    WavetableSlider oscWavetablePositionKnob;

    std::vector<juce::Component *> getKnobs();

    ControlSurfaceAttachment oscVolumeKnobAttachment;
    ControlSurfaceAttachment oscPanningKnobAttachment;
    ControlSurfaceAttachment oscDetuneMixKnobAttachment;
    ControlSurfaceAttachment oscWarpAmountKnobAttachment;
    ControlSurfaceAttachment oscWavetablePositionKnobAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
