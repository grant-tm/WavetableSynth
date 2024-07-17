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

    setSize (400, 460);
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
    bounds.removeFromTop(5);
    bounds.removeFromBottom(5);
    bounds.removeFromLeft(5);
    bounds.removeFromRight(5);

    // WAVETABLE DISPLAY AREA
    auto wavetableDisplayArea = bounds.removeFromRight(250);
    wavetableDisplayArea = wavetableDisplayArea.removeFromTop(250);
    wavetableDisplay.setBounds(wavetableDisplayArea);

    // OSC VOLUME AND PANNING
    auto oscMixingKnobArea = bounds;
    oscMixingKnobArea.removeFromRight(oscMixingKnobArea.getWidth() * 0.25f);
    oscMixingKnobArea.removeFromBottom(oscMixingKnobArea.getHeight() * 0.25f);
    
    auto oscVolumeKnobArea = oscMixingKnobArea.removeFromTop(100);
    oscVolumeKnob.setBounds(oscVolumeKnobArea);

    auto oscPanningKnobArea = oscMixingKnobArea.removeFromTop(100);
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
