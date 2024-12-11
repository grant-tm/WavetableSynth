#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <JuceHeader.h>

#define MAX_DETUNE_VOICES 12
#define MAX_DETUNE_SPREAD 0.05f

using Wavetable = juce::AudioBuffer<float>;

class Oscillator
{

public:

	//=============================================================================
	Oscillator();
	Oscillator(const Wavetable *);
	~Oscillator();

	//=============================================================================
	void render(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples);

	void setSampleRate(float);
	void setFrequency(float);
	void setVolume(float);
	void setVelocity(float);
	void setPan(float);

	//=============================================================================
	void randomizePhases();
	
	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

	int   getDetuneVoices() const;
	float getDetuneMix() const;
	float getDetuneSpread() const;
	void updateDetuneVoiceConfiguration();

	//=============================================================================
	void setWavetable(const Wavetable *);
	void setWavetableFrameIndex(int);

	//=============================================================================
	void setAdsrParameters(juce::ADSR::Parameters adsrParameters);
	void startAdsrEnvelope();
	void releaseAdsrEnvelope();
	bool adsrEnvelopeIsActive() const;

private:
	//=============================================================================
	juce::ADSR adsrEnvelope;
	juce::AudioBuffer<float> adsrScalars;
	
	float sampleRate;

	float baseFrequency;
	float baseVolume;
	float basePan;

	float renderFrequency;
	float renderVolume;
	float renderPanCoefficientLeft;
	float renderPanCoefficientRight;

	float velocity;

	float phases[MAX_DETUNE_VOICES];
	float deltaPhase;

	//=============================================================================
	int   detuneVoices;
	float detuneMix;
	float detuneSpread;

	float detuneFrequencyCoefficients[MAX_DETUNE_VOICES];
	float detuneVolumeCoefficients[MAX_DETUNE_VOICES];
	float detunePanningOffsets[MAX_DETUNE_VOICES];

	//=============================================================================
	const Wavetable *wavetable;
	int wavetableSize;
	int wavetableNumFrames;
	int wavetableFrameIndex;
	
	int sampleIndex;
	float sampleOffset;

	//=============================================================================
	void incrementPhase(int);
	void updateDeltaPhase();

	float getNextSample();

	void calculateDetuneFrequencyCoefficients();
	void calculateDetuneVolumeCoefficients();
	void calculateDetunePanningOffsets();

	void applyRenderParameters(int);
	void applyDetuneRenderParameters(int);
	void applyBaseRenderParameters();

	void calculateRenderPanCoefficients(float);
};

#endif // OSCILLATOR_H