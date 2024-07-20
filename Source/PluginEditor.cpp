/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    // VOLUME KNOB
    oscVolumeKnob(*audioProcessor.valueTree.getParameter("OSC_VOLUME"), "VOL"),
    oscVolumeKnobAttachment(audioProcessor.valueTree, "OSC_VOLUME", oscVolumeKnob),
    // PANNING KNOB
    oscPanningKnob(*audioProcessor.valueTree.getParameter("OSC_PANNING"), "PAN"),
    oscPanningKnobAttachment(audioProcessor.valueTree, "OSC_PANNING", oscPanningKnob),
    // DETUNE MIX KNOB
    oscDetuneMixKnob(*audioProcessor.valueTree.getParameter("OSC_DETUNE_AMT"), "DET MIX"),
    oscDetuneMixKnobAttachment(audioProcessor.valueTree, "OSC_DETUNE_AMT", oscDetuneMixKnob),
    // WARP AMOUNT KNOB
    oscWarpAmountKnob(*audioProcessor.valueTree.getParameter("OSC_WARP_AMT"), "WARP"),
    oscWarpAmountKnobAttachment(audioProcessor.valueTree, "OSC_WARP_AMT", oscWarpAmountKnob),
    // WAVETABLE POSITION KNOB
    oscWavetablePositionKnob(*audioProcessor.valueTree.getParameter("OSC_WAVETABLE_POSITION"), "WT POS"),
    oscWavetablePositionKnobAttachment(audioProcessor.valueTree, "OSC_WAVETABLE_POSITION", oscWavetablePositionKnob),
    // WAVETABLE DISPLAY COMPONENT
    wavetableDisplay(audioProcessor)
{
    setSize (VST_WIDTH_PIXELS, VST_HEIGHT_PIXELS);

    addAndMakeVisible(wavetableDisplay);
    for (auto* knob : getKnobs())
    {
        addAndMakeVisible(knob);
    }
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
}

//==============================================================================
void WavetableSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(ColorPalette::body));
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // GET BOUNDS
    auto bounds = getLocalBounds();
    auto topButtonArea = bounds.removeFromTop(TOP_BAR_HEIGHT_PIXELS);
    auto leftControlArea = bounds.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    auto rightControlArea = bounds.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);

    // WAVETABLE DISPLAY AREA
    auto wavetableDisplayArea = bounds;
    wavetableDisplayArea = wavetableDisplayArea.removeFromTop(WAVETABLE_DISPLAY_HEIGHT_PIXELS);
    wavetableDisplay.setBounds(wavetableDisplayArea);

    // LEFT HAND KNOBS
    auto oscMixingKnobArea = leftControlArea;
    
    // volume
    auto oscVolumeKnobArea = oscMixingKnobArea.removeFromTop(LARGE_KNOB_DIAMETER_PIXELS + 15);
    oscVolumeKnob.setBounds(oscVolumeKnobArea);

    // panning
    auto oscPanningKnobArea = oscMixingKnobArea.removeFromTop(LARGE_KNOB_DIAMETER_PIXELS + 15);
    oscPanningKnob.setBounds(oscPanningKnobArea);

    // detune mix
    auto oscDetuneMixKnobArea = oscMixingKnobArea.removeFromTop(SMALL_KNOB_DIAMETER_PIXELS + 15);
    oscDetuneMixKnobArea.removeFromRight(12);
    oscDetuneMixKnobArea.removeFromLeft(12);
    oscDetuneMixKnob.setBounds(oscDetuneMixKnobArea);

    // RIGHT HAND KNOBS
    auto oscMorphKnobArea = rightControlArea;
    oscMorphKnobArea.removeFromTop((LARGE_KNOB_DIAMETER_PIXELS * 1) + (1 * 15));

    // wavetable positition
    auto oscWavetablePositionKnobArea = oscMorphKnobArea.removeFromTop((LARGE_KNOB_DIAMETER_PIXELS * 1) + (1 * 15));
    oscWavetablePositionKnob.setBounds(oscWavetablePositionKnobArea);

    // warp amount
    auto oscWarpAmountKnobArea = oscMorphKnobArea.removeFromTop(SMALL_KNOB_DIAMETER_PIXELS + 15);
    oscWarpAmountKnobArea.removeFromRight(12);
    oscWarpAmountKnobArea.removeFromLeft(12);
    oscWarpAmountKnob.setBounds(oscWarpAmountKnobArea);
}

std::vector<juce::Component*> WavetableSynthAudioProcessorEditor::getKnobs()
{
    return
    {
        &oscVolumeKnob,
        &oscPanningKnob,
        &oscDetuneMixKnob,
        &oscWarpAmountKnob,
        &oscWavetablePositionKnob
    };
}
