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

	void updateVoiceAges();

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

	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

private:
	//==============================================================================
	Wavetable wavetable;
	int wavetableSize = 0;
	int wavetableNumFrames = 0;
	int wavetableFrameIndex = 0;

	//==============================================================================
	Oscillator oscillators[MAX_POLYPHONY];
	juce::ADSR::Parameters adsrParameters{0.05f, 1.f, 0.8f, 1.f};

	float sampleRate = 22100;
	float frequency = 200;
	float volume = 1;
	float pan = 0;
	
	int detuneVoices = 1;
	float detuneMix = 1;
	float detuneSpread = 1;

	//==============================================================================
	struct Voice
	{
		int id;
		int noteNumber = 0;
		int age = -1;
	} voices[MAX_POLYPHONY];
	bool voiceStealingEnabled = true;

	float pitchBendWheelPosition = 0;
	int pitchBendUpperBoundSemitones = 2;
	int pitchBendLowerBoundSemitones = -2;

	//==============================================================================
	void render(juce::AudioBuffer<float> &buffer, int startSample, int endSample);

	void updateOscillators();
	void updateOscillator(Oscillator &);
	void updateOscillatorDetuneParameters(Oscillator &);

	float calculateFrequencyFromMidiInput(int midiNoteNuber, float pitchWheelPosition);
	float calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset);

	void handleMidiEvent(const juce::MidiMessage &midiMessage);
	void startNote(int midiNoteNumber, float velocity, int pitchWheelPosition);
	void stopNote(int);

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
