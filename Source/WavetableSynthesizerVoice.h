#ifndef WAVETABLE_SYNTHESIZER_VOICE_H
#define WAVETABLE_SYNTHESIZER_VOICE_H

#include <JuceHeader.h>

#include "WavetableSynthesizerSound.h"

using Wavetable = juce::AudioBuffer<float>;


class WavetableSynthesizerVoice : public juce::SynthesiserVoice
{
public:

    // CONSTRUCTORS / DESTRUCTORS
    WavetableSynthesizerVoice();
    WavetableSynthesizerVoice(Wavetable&);
    ~WavetableSynthesizerVoice();

    void setWavetable(Wavetable& wavetableToUse);

    // RENDERING
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void updateDeltaPhase();
    void updatePhase();
    float getNextSample();

    // UPDATE RENDER CONTEXT
    void setRenderSampleRate(float);
    void setRenderFrequency(float);
    void setRenderLevel(float);
    void updateRenderFrequencyFromMidiInput();
   
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

    // RENDER 
    float renderSampleRate;
    float renderFrequency;
    float renderLevel;
    float noteVelocity;

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