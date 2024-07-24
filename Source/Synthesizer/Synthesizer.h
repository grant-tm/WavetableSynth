#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <JuceHeader.h>
#include "Oscillator.h"

class Synthesizer
{
public:

	// CONSTRUCTOR / DESTRUCTORS
	Synthesizer();
	~Synthesizer();

	// GET / SET RENDER PARAMETERS
	
	float getSampleRate();
	void setSampleRate(float);

	void setFrequency(float);
	void setFrequencyByMidiNote(int, float);

	void setVolume(float);

	void setPan(float);

	// RENDER
	void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiBuffer);

	// OSCILLATORS
	void initializeOscillators();
	void updateOscillatorParameters(Oscillator &oscillator);
	void updateOscillatorDetuneIfChanged(int oscillatorId);

	// DETUNE
	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

	// WAVETABLE
	void setWavetable(Wavetable &);
	void setWavetableFrameIndex(int);
	const Wavetable *getWavetableReadPointer();
	int getNumWavetableFrames();

private:

	

	// OSCILLATORS
	Oscillator oscillators[4];
	
	// WAVETABLE
	Wavetable wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	float wavetableFrameIndex;

	// RENDER PARAMETERS
	float sampleRate = 22100;
	float frequency = 200;
	float volume = 1;
	float pan = 0;
	
	// DETUNE PARAMETERS
	int detuneVoices = 1;
	float detuneMix = 1;
	float detuneSpread = 1;

	// PITCH BEND
	float pitchBendWheelPosition;
	int pitchBendUpperBoundSemitones;
	int pitchBendLowerBoundSemitones;

	// MIDI
	float calculateFrequencyFromMidiInput(int midiNoteNuber, float pitchWheelPosition);
	float calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset);

	void render(juce::AudioBuffer<float> &buffer, int startSample, int endSample);

	void handleMidiEvent(const juce::MidiMessage &midiMessage);

	void startNote(int midiNoteNumber, int velocity, int pitchWheelPosition);
	void stopNote(int);

	void pitchWheelMoved(int newPitchWheelValue);
	void setPitchBendPosition(int position);
	float getPitchBendOffsetCents();
	float getPitchBendOffsetCents(float);

};

#endif // SYNTHESIZER_H
