#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <JuceHeader.h>
#include "WavetableSynthesizerVoice.h"

#define MAX_DETUNE_VOICES 12
#define MAX_DETUNE_SPREAD_PROPORTIONAL 0.5f

class Oscillator
{

public:

	//=============================================================================
	// CONSTRUCTORS / DESTRUCTORS
	Oscillator();
	Oscillator(const Wavetable *);
	~Oscillator();

	//=============================================================================
	// RENDERING
	void render(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples);

	//=============================================================================
	// RENDERING PARAMETERS
	bool isEnabled();
	void setEnable(bool);
	void setSampleRate(float);
	void setFrequency(float);
	void setVolume(float);
	void setVelocity(float);
	void setPan(float);

	//=============================================================================
	// WAVETABLE

	void setWavetable(const Wavetable *);
	void setWavetableFrameIndex(int);

	//=============================================================================
	// DETUNE
	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

	int getDetuneVoices();
	float getDetuneMix();
	float getDetuneSpread();

	void updateDetuneVoiceConfiguration();

private:

	//=============================================================================
	// DATA

	// render parameters

	float sampleRate;
	bool  enable;
	float baseFrequency;
	float baseVolume;
	float velocity;
	float basePan;

	float renderFrequency;
	float renderVolume;
	float renderPanCoefficientLeft;
	float renderPanCoefficientRight;

	float phases[MAX_DETUNE_VOICES];
	float deltaPhase;

	// wavetable

	const Wavetable *wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	int wavetableFrameIndex;
	int sampleIndex;
	float sampleOffset;

	// detune

	int   detuneVoices;
	float detuneMix;
	float detuneSpread;
	float detuneFalloff;

	float detuneFrequencyCoefficients[MAX_DETUNE_VOICES];
	float detuneVolumeCoefficients[MAX_DETUNE_VOICES];
	float detunePanningOffsets[MAX_DETUNE_VOICES];

	//=============================================================================
	// FUNCTIONS

	void incrementPhase(int);
	void updateDeltaPhase();
	float getNextSample();

	void calculateDetuneFrequencyCoefficients();
	void calculateDetuneVolumeCoefficients();
	void calculateDetunePanningOffsets();

	void setRenderParametersForDetunedVoice(int);
	void passthroughBaseRenderParameters();


};

#endif // OSCILLATOR_H