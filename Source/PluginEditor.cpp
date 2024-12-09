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
   
    // TOP BUTTONS
    loadButton("LOAD_BUTTON", juce::Colour(0xFFFF256D), juce::Colour(0xFFFF558D), juce::Colour(0xFF0F1D1F)),
    saveButton("SAVE_BUTTON", juce::Colour(0xFFFFEC47), juce::Colour(0xFFFFF59C), juce::Colour(0xFF0F1D1F)),
    editButton("EDIT_BUTTON", juce::Colour(0xFF61FF8D), juce::Colour(0xFF97FFB4), juce::Colour(0xFF0F1D1F)),
    viewButton("VIEW_BUTTON", juce::Colour(0xFF50C0FF), juce::Colour(0xFF6BCAFF), juce::Colour(0xFF0F1D1F)),

    // VOLUME KNOB
    oscVolumeKnob(*audioProcessor.valueTree.getParameter("OSC_VOLUME"), "VOL"),
    oscVolumeKnobAttachment(audioProcessor.valueTree, "OSC_VOLUME", oscVolumeKnob),
    
    // PANNING KNOB
    oscPanningKnob(*audioProcessor.valueTree.getParameter("OSC_PANNING"), "PAN"),
    oscPanningKnobAttachment(audioProcessor.valueTree, "OSC_PANNING", oscPanningKnob),
    
    // DETUNE MIX KNOB
    oscDetuneMixKnob(*audioProcessor.valueTree.getParameter("OSC_DETUNE_MIX"), "DETUNE"),
    oscDetuneMixKnobAttachment(audioProcessor.valueTree, "OSC_DETUNE_MIX", oscDetuneMixKnob),
    
    // WARP AMOUNT KNOB
    oscWarpAmountKnob(*audioProcessor.valueTree.getParameter("OSC_WARP_AMOUNT"), "WARP"),
    oscWarpAmountKnobAttachment(audioProcessor.valueTree, "OSC_WARP_AMOUNT", oscWarpAmountKnob),
    
    // WAVETABLE POSITION KNOB
    oscWavetablePositionKnob(*audioProcessor.valueTree.getParameter("OSC_WAVETABLE_POSITION"), "WT POS"),
    oscWavetablePositionKnobAttachment(audioProcessor.valueTree, "OSC_WAVETABLE_POSITION", oscWavetablePositionKnob),
    
    // WAVETABLE DISPLAY COMPONENT
    wavetableDisplay(audioProcessor)
{
    setSize (VST_WIDTH_PIXELS, VST_HEIGHT_PIXELS);

    loadButton.setButtonText("LOAD");
    addAndMakeVisible(loadButton);

    saveButton.setButtonText("SAVE");
    addAndMakeVisible(saveButton);

    editButton.setButtonText("EDIT");
    addAndMakeVisible(editButton);

    viewButton.setButtonText("VIEW");
    addAndMakeVisible(viewButton);

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
    g.fillAll (juce::Colour((uint32_t) ColorPalette::body));
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // GET BOUNDS
    auto bounds = getLocalBounds();
    auto topButtonArea = bounds.removeFromTop(TOP_BAR_HEIGHT_PIXELS);
    auto leftControlArea = bounds.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    auto rightControlArea = bounds.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);

    // TOP BUTTONS
    topButtonArea.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    topButtonArea.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    topButtonArea.removeFromTop(12.f);
    topButtonArea.removeFromBottom(12.f);

    const auto buttonPadding = 4.f;

    auto loadButtonArea = topButtonArea;
    loadButtonArea.removeFromRight(3 * (loadButtonArea.getWidth() / 4) + 2 * buttonPadding);
    loadButton.setBounds(loadButtonArea);
    
    auto saveButtonArea = topButtonArea;
    saveButtonArea.removeFromRight(buttonPadding + saveButtonArea.getWidth() / 2);
    saveButtonArea.removeFromLeft(buttonPadding + saveButtonArea.getWidth() / 2);
    saveButton.setBounds(saveButtonArea);

    auto editButtonArea = topButtonArea;
    editButtonArea.removeFromLeft(buttonPadding + editButtonArea.getWidth() / 2);
    editButtonArea.removeFromRight(buttonPadding + editButtonArea.getWidth() / 2);
    editButton.setBounds(editButtonArea);

    auto viewButtonArea = topButtonArea;
    viewButtonArea.removeFromLeft(3 * (viewButtonArea.getWidth() / 4) + 2 * buttonPadding);
    viewButton.setBounds(viewButtonArea);

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
    auto oscWavetablePositionSliderArea = rightControlArea.removeFromTop(WAVETABLE_DISPLAY_HEIGHT_PIXELS);
    oscWavetablePositionKnob.setBounds(oscWavetablePositionSliderArea);

    // warp amount
    /*auto oscWarpAmountKnobArea = oscMorphKnobArea.removeFromTop(SMALL_KNOB_DIAMETER_PIXELS + 15);
    oscWarpAmountKnobArea.removeFromRight(12);
    oscWarpAmountKnobArea.removeFromLeft(12);
    oscWarpAmountKnob.setBounds(oscWarpAmountKnobArea);*/
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
