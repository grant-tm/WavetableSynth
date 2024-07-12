#ifndef WAVETABLE_SYNTHESIZER_VOICE_H
#define WAVETABLE_SYNTHESIZER_VOICE_H

#include <JuceHeader.h>

using Wavetable = juce::AudioBuffer<float>;

class WavetableSynthesizerSound : public juce::SynthesiserSound
{
    bool appliesToNote(int)    override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class WavetableSynthesizerVoice : public juce::SynthesiserVoice
{
public:

    // CONSTRUCTORS / DESTRUCTORS
    WavetableSynthesizerVoice();
    WavetableSynthesizerVoice(Wavetable&);
    ~WavetableSynthesizerVoice();

    void setWavetable(Wavetable& table) {
        wavetable = table;
    }

    // RENDERING
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void updateDeltaPhase();
    void updatePhase();
    float getNextSample();

    // UPDATE RENDER CONTEXT
    void updateFrequency();
    
    // NOTE ON OFF
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int pitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    
    // PITCHBEND CONTROLS
    void pitchWheelMoved(int newPitchWheelValue) override;
    void setPitchBendPosition(int position);
    float getPitchBendOffsetCents();
    void controllerMoved(int controllerNumber, int newControllerValue) override {} // declared by necessity; not used

    // HELPERS
    static float getOffsetMidiNoteInHertz(int midiNoteNumber, float centsOffset);

protected:

    // RENDER CONTEXT
    float frequency;
    float velocity;

    // WAVETABLE DESCRIPTION
    Wavetable wavetable;
    int wavetableSize;

    // WAVETABLE POSITIONING
    float phase;
    float deltaPhase;
    int   sampleIndex;
    float sampleOffset;
    
    // PITCH BEND PARAMETERS
    float pitchBendWheelPosition;
    int pitchBendUpperBoundSemitones;
    int pitchBendLowerBoundSemitones;
};

#endif // WAVETABLE_SYNTHESIZER_VOICE_H