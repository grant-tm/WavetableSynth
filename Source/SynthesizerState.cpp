#include "SynthesizerState.h"

SynthesizerState getSynthesizerStateFromValueTree(juce::AudioProcessorValueTreeState& valueTree);

SynthesizerState getSynthesizerState(juce::AudioProcessorValueTreeState& valueTree)
{
    SynthesizerState state;

    state.oscVolume = valueTree.getRawParameterValue("OSC_VOLUME")->load();
    state.oscPanning = valueTree.getRawParameterValue("OSC_PANNING")->load();

    return state;
}
