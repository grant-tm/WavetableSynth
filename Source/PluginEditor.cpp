/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor(WavetableSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    
    oscVolumeKnob(*audioProcessor.valueTree.getParameter("OSC_VOLUME"), "%"),
    oscVolumeKnobAttachment(audioProcessor.valueTree, "OSC_VOLUME", oscVolumeKnob),
    
    oscPanningKnob(*audioProcessor.valueTree.getParameter("OSC_PANNING"), "%"),
    oscPanningKnobAttachment(audioProcessor.valueTree, "OSC_PANNING", oscPanningKnob),

    wavetableDisplay(audioProcessor)
{
    oscVolumeKnob.labels.add({ 0.f, "" });
    oscVolumeKnob.labels.add({ 100.f, "" });

    oscPanningKnob.labels.add({ -50.f, "" });
    oscPanningKnob.labels.add({ 50.f, "" });

    addAndMakeVisible(wavetableDisplay);

    for (auto* knob : getKnobs())
    {
        addAndMakeVisible(knob);
    }

    setSize (VST_WIDTH_PIXELS, VST_HEIGHT_PIXELS);
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

    // OSC VOLUME AND PANNING
    auto oscMixingKnobArea = leftControlArea;
    
    auto oscVolumeKnobArea = oscMixingKnobArea.removeFromTop(oscMixingKnobArea.getWidth() * 1.1f);
    oscVolumeKnob.setBounds(oscVolumeKnobArea);

    auto oscPanningKnobArea = oscMixingKnobArea.removeFromTop(oscVolumeKnobArea.getHeight());
    oscPanningKnob.setBounds(oscPanningKnobArea);
}

std::vector<juce::Component*> WavetableSynthAudioProcessorEditor::getKnobs()
{
    return
    {
        &oscVolumeKnob,
        &oscPanningKnob
    };
}
