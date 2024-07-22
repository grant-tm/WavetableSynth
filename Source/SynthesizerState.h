#ifndef SYNTHESIZER_STATE_H
#define SYNTHESIZER_STATE_H

#include <JuceHeader.h>

// contains parameters for peak, low cut, and high cut filters
struct SynthesizerState
{
    //========================================================================
    // OSC

    // Mixing Parameters
    float oscVolume{ 0.f };
    float oscPanning{ 0.f };

    float oscDetuneMix{0.f};
    float oscWarpAmount{0.f};

    float oscWavetablePosition{0.f};
    int   oscWavetableNumFrames{0};
    int   oscWavetableCurrentFrame{0};
};

SynthesizerState getSynthesizerStateFromValueTree(juce::AudioProcessorValueTreeState& valueTree);

#endif // SYNTHESIZER_STATE_H
