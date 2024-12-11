#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <JuceHeader.h>
#include "Oscillator.h"

#define MAX_POLYPHONY 16

class Synthesizer
{
public:
	//==============================================================================

	Synthesizer();
	~Synthesizer() {};

	//==============================================================================
	void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiBuffer);
	
	void setWavetable(Wavetable &);
	void setWavetableFrameIndex(int);
	const Wavetable *getWavetableReadPointer() const;
	int getNumWavetableFrames() const;

	float getSampleRate() const;
	void setSampleRate(float);

	void setFrequency(float);
	void setFrequencyByMidiNote(int, float);
	
	void setVolume(float);
	void setPan(float);

	void setAdsrParameters(float attack, float decay, float sustain, float release);

	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

private:
	//==============================================================================
	Wavetable wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	int wavetableFrameIndex;

	//==============================================================================
	Oscillator oscillators[MAX_POLYPHONY];
	juce::ADSR::Parameters adsrParameters;

	float sampleRate;
	float frequency;
	float volume;
	float pan;
	
	int detuneVoices;
	float detuneMix;
	float detuneSpread;

	//==============================================================================
	struct Voice
	{
		int id;
		int noteNumber = 0;
		int age = -1;
	} voices[MAX_POLYPHONY];
	bool voiceStealingEnabled;

	float pitchBendWheelPosition;
	int pitchBendUpperBoundSemitones;
	int pitchBendLowerBoundSemitones;

	//==============================================================================
	void render(juce::AudioBuffer<float> &buffer, int startSample, int endSample);

	void updateOscillators();
	void updateOscillator(Oscillator &);
	void updateOscillatorDetuneParameters(Oscillator &);

	float calculateFrequencyFromMidiInput(int midiNoteNuber, float pitchWheelPosition) const;
	float calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset) const;

	void handleMidiEvent(const juce::MidiMessage &midiMessage);
	void startNote(int midiNoteNumber, float velocity);
	void stopNote(int);

	void updateVoiceAges();
	int findVoice(int) const;
	int findVoicePlayingNote(int) const;
	int findFreeVoice() const;
	int findOldestVoice() const;

	void  pitchWheelMoved(int newPitchWheelValue);
	void  setPitchBendPosition(int position);
	float getPitchBendOffsetCents() const;
	float getPitchBendOffsetCents(float) const;
};

#endif // SYNTHESIZER_H
