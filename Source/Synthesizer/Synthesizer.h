#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <JuceHeader.h>
#include "Oscillator.h"

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

private:

	//==============================================================================
	
	Oscillator oscillators[4];

	Wavetable wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	float wavetableFrameIndex;

	//==============================================================================

	float sampleRate = 22100;
	float frequency = 200;
	float volume = 1;
	float pan = 0;
	
	int detuneVoices = 1;
	float detuneMix = 1;
	float detuneSpread = 1;

	//==============================================================================

	float pitchBendWheelPosition;
	int pitchBendUpperBoundSemitones;
	int pitchBendLowerBoundSemitones;
	
	//==============================================================================
	void render(juce::AudioBuffer<float> &buffer, int startSample, int endSample);

	float calculateFrequencyFromMidiInput(int midiNoteNuber, float pitchWheelPosition);
	float calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset);

	void handleMidiEvent(const juce::MidiMessage &midiMessage);
	void startNote(int midiNoteNumber, int velocity, int pitchWheelPosition);
	void stopNote(int);

	void pitchWheelMoved(int newPitchWheelValue);
	void setPitchBendPosition(int position);
	float getPitchBendOffsetCents();
	float getPitchBendOffsetCents(float);
};

#endif // SYNTHESIZER_H
