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
    WavetableSynthesizerVoice(const Wavetable*);
    ~WavetableSynthesizerVoice();

    // WAVETABLE MANAGEMENT
    void setWavetable(const Wavetable* wavetableToUse);
    void setWavetableFrameIndex(int);

    // RENDERING
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    float getInterpolatedSampleFromCurrentPhase();
    void updateDeltaPhase();
    void updatePhase();
    bool canPlaySound(juce::SynthesiserSound *sound) override;

    // UPDATE RENDER PARAMETERS
    void setRenderSampleRate(float);
    void setRenderFrequency(float);
    void updateRenderFrequencyFromMidiInput();
    void setRenderLevel(float);
    void setRenderPan(float);
   
    // NOTE ON OFF
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int pitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    
    // MIDI
    void pitchWheelMoved(int newPitchWheelValue) override;
    void setPitchBendPosition(int position);
    float getPitchBendOffsetCents();
    static float getOffsetMidiNoteInHertz(int midiNoteNumber, float centsOffset);
    void controllerMoved(int controllerNumber, int newControllerValue) override;

protected:

    juce::ADSR adsrEnvelope;

    // WAVETABLE DESCRIPTION
    const Wavetable *wavetable;
    int wavetableSize;
    int wavetableNumFrames;
    int wavetableFrameIndex;

    // RENDER PARAMETERS
    float renderSampleRate;
    float renderFrequency;
    float renderLevel;
    float renderPanCoefficientLeft;
    float renderPanCoefficientRight;
    float noteVelocity;

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