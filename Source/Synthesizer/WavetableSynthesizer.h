#ifndef WAVETABLE_SYNTHESIZER_H
#define WAVETABLE_SYNTHESIZER_H

#include <JuceHeader.h>

#include "SynthesizerState.h"
#include "WavetableSynthesizerVoice.h"
#include "WavetableSynthesizerSound.h"

class WavetableSynthesizer : public juce::Synthesiser
{
public:
	WavetableSynthesizer();
	~WavetableSynthesizer();

	void setWavetable(Wavetable&);
	const Wavetable *getWavetableReadPointer();
	int getNumWavetableFrames();

	const juce::AudioProcessorValueTreeState *stateValueTree;

private:

	Wavetable wavetable;

};

#endif // WAVETABLE_SYNTHESIZER_H