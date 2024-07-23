#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <JuceHeader.h>
#include "WavetableSynthesizerVoice.h"

#define MAX_DETUNE_VOICES 12
#define MAX_DETUNE_SPREAD_PROPORTIONAL 0.01f

class Oscillator
{
//=============================================================================
// CONSTRUCTORS / DESTRUCTORS

public:
	
	Oscillator();
	Oscillator(const Wavetable *);
	~Oscillator();

//=============================================================================
// RENDERING

public:
	
	void render(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples);

private:

	float phases[MAX_DETUNE_VOICES];
	float deltaPhase;

	void incrementPhase(int);
	void updateDeltaPhase();

	float getNextSample();
	void setRenderParametersForDetunedVoice();

//=============================================================================
// RENDERING PARAMETERS

public:
	void setEnable(bool);
	void setSampleRate(float);
	void setFrequency(float);
	void setVolume(float);
	void setPan(float);

private:
	float sampleRate;
	bool  enable;
	float baseFrequency;
	float baseVolume;
	float basePan;

	float renderFrequency;
	float renderVolume;
	float renderPanCoefficientLeft;
	float renderPanCoefficientRight;

//=============================================================================
// WAVETABLE

public:

	void setWavetable(const Wavetable *);
	void setWavetableFrameIndex(int);

private:

	const Wavetable *wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	int wavetableFrameIndex;
	int sampleIndex;
	float sampleOffset;

//=============================================================================
// DETUNE PARAMETERS

public:

	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

private:

	int detuneVoices;
	float detuneMix;
	float detuneSpread;

//=============================================================================
// DETUNE DEPENDENT VALUES

private:

	float detuneFrequencyOffsets[MAX_DETUNE_VOICES];
	float detuneVolumeCoefficients[MAX_DETUNE_VOICES];
	float detunePanningValues[MAX_DETUNE_VOICES];

	void updateDetuneVoiceConfiguration();
	void calculateDetuneFrequencyOffsets();
	void calculateDetuneVolumeCoefficients();
	void calculateDetunePanningValues();
	void setRenderParametersForDetunedVoice(int);

};

#endif // OSCILLATOR_H