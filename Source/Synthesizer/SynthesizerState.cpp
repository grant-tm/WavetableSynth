#include "SynthesizerState.h"

//SynthesizerState getSynthesizerStateFromValueTree(juce::AudioProcessorValueTreeState& valueTree);

SynthesizerState getSynthesizerStateFromValueTree(juce::AudioProcessorValueTreeState& valueTree)
{
    SynthesizerState state;

    state.oscVolume = valueTree.getRawParameterValue("OSC_VOLUME")->load();
    state.oscPanning = valueTree.getRawParameterValue("OSC_PANNING")->load();

    state.oscDetuneMix = valueTree.getRawParameterValue("OSC_DETUNE_MIX")->load();
    state.oscWarpAmount = valueTree.getRawParameterValue("OSC_WARP_AMOUNT")->load();

    state.oscWavetablePosition = valueTree.getRawParameterValue("OSC_WAVETABLE_POSITION")->load();
    state.oscWavetableNumFrames = (int) valueTree.getRawParameterValue("OSC_WAVETABLE_NUM_FRAMES")->load();
    state.oscWavetableCurrentFrame = (int) valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->load();


    return state;
}
