#ifndef WAVETABLE_SYNTHESIZER_H
#define WAVETABLE_SYNTHESIZER_H

#include <JuceHeader.h>

#include "SynthesizerState.h"

#include "WavetableSynthesizerVoice.h"
#include "WavetableSynthesizerSound.h"

class WavetableSynthesizer : public juce::Synthesiser
{
public:

	const juce::AudioProcessorValueTreeState *stateValueTree;
};

#endif // WAVETABLE_SYNTHESIZER_H