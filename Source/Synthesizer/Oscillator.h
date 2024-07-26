#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <JuceHeader.h>

#define MAX_DETUNE_VOICES 12
#define MAX_DETUNE_SPREAD_PROPORTIONAL 0.05f

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

	bool isEnabled();
	void setEnable(bool);
	void setSampleRate(float);
	void setFrequency(float);
	void setVolume(float);
	void setVelocity(float);
	void setPan(float);

	//=============================================================================
	void setDetuneVoices(int);
	void setDetuneMix(float);
	void setDetuneSpread(float);

	int   getDetuneVoices();
	float getDetuneMix();
	float getDetuneSpread();

	void updateDetuneVoiceConfiguration();

	//=============================================================================

	void setWavetable(const Wavetable *);
	void setWavetableFrameIndex(int);

	//=============================================================================

	void setAdsrParameters(juce::ADSR::Parameters adsrParameters);
	void startAdsrEnvelope();
	void releaseAdsrEnvelope();
	bool adsrEnvelopeIsActive() const;

	int   noteNumber;
	double timeStarted;

private:

	juce::ADSR adsrEnvelope;

	//=============================================================================

	float sampleRate;
	bool  enable;
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
	float detuneFalloff;

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

};

#endif // OSCILLATOR_H