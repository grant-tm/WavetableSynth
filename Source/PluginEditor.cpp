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

    // SCREEN ELEMENTS
    transposeBar(),
    wavetableDisplay(audioProcessor),
    adsrControls(),
    detuneVoicesAndWarpModeControls(),

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
    oscWavetablePositionKnobAttachment(audioProcessor.valueTree, "OSC_WAVETABLE_POSITION", oscWavetablePositionKnob)
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

    addAndMakeVisible(transposeBar);
    addAndMakeVisible(adsrControls);
    addAndMakeVisible(detuneVoicesAndWarpModeControls);

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
    //-----------------------------------------------------
    // GET BOUNDS
    auto bounds = getLocalBounds();

    //-----------------------------------------------------
    // CREATE BUTTON AREA
    
    // button spacing parameters
    const auto buttonHorizontalPadding = 4;
    const auto buttonVerticalPadding = 12;

    // create button area
    auto topButtonArea = bounds;
    topButtonArea = topButtonArea.removeFromTop(TOP_BAR_HEIGHT_PIXELS);
    topButtonArea.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    topButtonArea.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    topButtonArea.removeFromTop(buttonVerticalPadding);
    topButtonArea.removeFromBottom(buttonVerticalPadding);

    // load button
    auto loadButtonArea = topButtonArea;
    loadButtonArea.removeFromRight(3 * (loadButtonArea.getWidth() / 4) + 2 * buttonHorizontalPadding);
    loadButton.setBounds(loadButtonArea);
    
    // save button
    auto saveButtonArea = topButtonArea;
    saveButtonArea.removeFromRight(buttonHorizontalPadding + saveButtonArea.getWidth() / 2);
    saveButtonArea.removeFromLeft(buttonHorizontalPadding + saveButtonArea.getWidth() / 2);
    saveButton.setBounds(saveButtonArea);

    // edit button
    auto editButtonArea = topButtonArea;
    editButtonArea.removeFromLeft(buttonHorizontalPadding + editButtonArea.getWidth() / 2);
    editButtonArea.removeFromRight(buttonHorizontalPadding + editButtonArea.getWidth() / 2);
    editButton.setBounds(editButtonArea);

    // view button
    auto viewButtonArea = topButtonArea;
    viewButtonArea.removeFromLeft(3 * (viewButtonArea.getWidth() / 4) + 2 * buttonHorizontalPadding);
    viewButton.setBounds(viewButtonArea);

    //-----------------------------------------------------
    // SCREEN
    
    // get screen area
    auto screenArea = bounds;
    screenArea.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    screenArea.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    screenArea.removeFromTop(TOP_BAR_HEIGHT_PIXELS);
    
    // traspose bar
    auto transposeBarArea = screenArea;
    transposeBarArea = transposeBarArea.removeFromTop((WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4) + 5);
    transposeBar.setBounds(transposeBarArea);

    // wavetable display
    auto wavetableDisplayArea = screenArea;
    wavetableDisplayArea.removeFromTop(WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4);
    wavetableDisplayArea = wavetableDisplayArea.removeFromTop(WAVETABLE_DISPLAY_HEIGHT_PIXELS);
    wavetableDisplay.setBounds(wavetableDisplayArea);

    // adsr controls
    auto adsrArea = screenArea;
    adsrArea.removeFromTop((5 * (WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4)) - 5);
    adsrArea = adsrArea.removeFromTop((WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4) + 5);
    adsrControls.setBounds(adsrArea);

    // detune voices and warp mode controls
    auto altControlArea = screenArea;
    altControlArea.removeFromTop((6 * (WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4)) - 5);
    altControlArea = altControlArea.removeFromTop((WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4));
    detuneVoicesAndWarpModeControls.setBounds(altControlArea);

    //-----------------------------------------------------
    // LEFT HAND KNOBS

    auto leftControlArea = bounds;
    leftControlArea = leftControlArea.removeFromLeft((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    leftControlArea.removeFromTop(TOP_BAR_HEIGHT_PIXELS);
   
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

    //-----------------------------------------------------
    // RIGHT HAND KNOBS
    
    auto rightControlArea = bounds;
    rightControlArea = rightControlArea.removeFromRight((VST_WIDTH_PIXELS - WAVETABLE_DISPLAY_WIDTH_PIXELS) / 2);
    rightControlArea.removeFromTop(TOP_BAR_HEIGHT_PIXELS + (WAVETABLE_DISPLAY_HEIGHT_PIXELS / 4));

    auto oscWavetablePositionSliderArea = rightControlArea.removeFromTop(WAVETABLE_DISPLAY_HEIGHT_PIXELS);
    oscWavetablePositionKnob.setBounds(oscWavetablePositionSliderArea);

    rightControlArea.removeFromTop(15);
    auto oscWarpAmountKnobArea = rightControlArea.removeFromTop(SMALL_KNOB_DIAMETER_PIXELS);
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
