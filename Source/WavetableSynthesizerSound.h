#ifndef WAVETABLE_SYNTHESIZER_SOUND_H
#define WAVETABLE_SYNTHESIZER_SOUND_H

#include <JuceHeader.h>

class WavetableSynthesizerSound : public juce::SynthesiserSound
{
    bool appliesToNote(int)    override;
    bool appliesToChannel(int) override;
};

#endif // WAVETABLE_SYNTHESIZER_SOUND_H