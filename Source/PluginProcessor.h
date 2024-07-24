/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Synthesizer/WavetableSynthesizer.h"
#include "Synthesizer/WavetableSynthesizerVoice.h"
#include "Synthesizer/WavetableSynthesizerSound.h"
#include "Synthesizer/Synthesizer.h"

#define BODY_COLOR_HEX              0xFF64BEA5
#define BORDER_COLOR_HEX            0xFF0F1D1F
#define SCREEN_MAIN_COLOR_HEX       0xFFD7FFEB
#define SCREEN_SHADOW_COLOR_HEX     0xFFB1E7CC
#define CONTROL_SURFACE_COLOR_HEX   0xFF528187

void generateSineWavetable(Wavetable& tableToFill, int resolution);
void generateSquareWavetable(Wavetable& tableToFill, int resolution);
void generateSawWavetable(Wavetable& tableToFill, int resolution);
void generateMultiSineWavetable(Wavetable& tableToFill, int resolution, int coefficientA, int coefficientB);
void generateSineFrames(Wavetable &tableToFill, int resolution);

//==============================================================================
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

    //==============================================================================

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState valueTree{ *this, nullptr, "Parameters", createParameterLayout() };
    
    //WavetableSynthesizer synthesizer;
    Synthesizer synthesizer;
    Wavetable wavetable;
    Oscillator osc;

private:
    const int oversampleCoefficient = 8;
    juce::dsp::Oversampling<float> oversamplingEngine;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessor)
};
