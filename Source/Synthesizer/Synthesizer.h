#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <JuceHeader.h>
#include "Oscillator.h"

#define MAX_POLYPHONY 16

struct Voice
{
	int noteNumber = 0;
	int order = 0;
};

class Synthesizer
{
public:
	//==============================================================================
	Synthesizer();
	~Synthesizer();
	
	//==============================================================================

	void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiBuffer);

	//==============================================================================
	float getSampleRate();
	void setSampleRate(float);

	void setFrequency(float);
	void setFrequencyByMidiNote(int, float);

	void setVolume(float);
	void setPan(float);
	//==============================================================================
	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

	//==============================================================================
	void initializeOscillators();
	void updateOscillatorParameters(Oscillator &oscillator);
	void updateOscillatorDetuneIfChanged(int oscillatorId);
	
	//==============================================================================
	void setWavetable(Wavetable &);
	void setWavetableFrameIndex(int);
	const Wavetable *getWavetableReadPointer();
	int getNumWavetableFrames();

	void mapBufferToRange(juce::AudioBuffer<float> &, int, int);
	void setAdsrEnvelopeParameters(juce::ADSR::Parameters adsrParameters);


private:

	//==============================================================================
	
	juce::ADSR::Parameters adsrParameters{0.05f, 1.f, 0.8f, 1.f};

	Oscillator oscillators[MAX_POLYPHONY];

	bool voiceStealingEnabled = false;
	int maxPolyphony = MAX_POLYPHONY;
	struct Voice activeVoices[MAX_POLYPHONY];
	int numVoicesActive = 0;

	void disableInactiveOscillators();
	int findVoice();
	int findVoiceToSteal();
	int findVoicePlayingNote(int);

	//==============================================================================

	Wavetable wavetable;
	int wavetableSize = 0;
	int wavetableNumFrames = 0;
	int wavetableFrameIndex = 0;

	//==============================================================================

	float sampleRate = 22100;
	float frequency = 200;
	float volume = 1;
	float pan = 0;
	
	int detuneVoices = 1;
	float detuneMix = 1;
	float detuneSpread = 1;

	//==============================================================================

	float pitchBendWheelPosition = 0;
	int pitchBendUpperBoundSemitones = 2;
	int pitchBendLowerBoundSemitones = -2;
	
	//==============================================================================
	void render(juce::AudioBuffer<float> &buffer, int startSample, int endSample);

	float calculateFrequencyFromMidiInput(int midiNoteNuber, float pitchWheelPosition);
	float calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset);

	void handleMidiEvent(const juce::MidiMessage &midiMessage);
	void startNote(int midiNoteNumber, float velocity, int pitchWheelPosition);
	void stopNote(int);
	void stopAllNotes();

	void pitchWheelMoved(int newPitchWheelValue);
	void setPitchBendPosition(int position);
	float getPitchBendOffsetCents();
	float getPitchBendOffsetCents(float);

};

#endif // SYNTHESIZER_H
