/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SineOscillator
{
public:
    SineOscillator() {};
    ~SineOscillator() {};

    void updateAngleDelta(void)
    {
        auto cyclesPerSample = 0.0f;
        if (sampleRate > 0.0f)
        {
            cyclesPerSample = frequency / sampleRate;
        }
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    void incrementAngle()
    {
        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<float>::twoPi)
        {
            currentAngle -= juce::MathConstants<float>::twoPi;
        }
    }

    void setSampleRate(float sampleRate)
    {
        this->sampleRate = sampleRate;
        updateAngleDelta();
    }

    void setFrequency(float frequency)
    {
        this->frequency = frequency;
        updateAngleDelta();
    }

    void fillBuffer(juce::AudioBuffer<float>& buffer)
    {
        auto leftBuffer = buffer.getWritePointer(0);
        auto rightBuffer = buffer.getWritePointer(1);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto currentSample = (float)std::sin(currentAngle);
            incrementAngle();

            leftBuffer[sample] = currentSample * level;
            rightBuffer[sample] = currentSample * level;
        }
    }

private:
    float sampleRate = 0.0f;
    float frequency = 100.0f;
    float angleDelta = 0.0f;
    float currentAngle = 0.0f;
    float level = 0.125f;
};

//==============================================================================
/**
*/
class WavetableSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WavetableSynthAudioProcessor();
    ~WavetableSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

    SineOscillator sineOscillator;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessor)
};
