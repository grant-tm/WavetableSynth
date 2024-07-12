/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void generateSineWavetable(Wavetable& tableToFill, int resolution)
{
    tableToFill.setSize(1, resolution);
    auto* samples = tableToFill.getWritePointer(0);

    auto angleDelta = juce::MathConstants<float>::twoPi / (float)(resolution - 1);
    auto currentAngle = 0.0;

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] = (float)sample;
        currentAngle += angleDelta;
    }
}

//==============================================================================
WavetableSynthAudioProcessor::WavetableSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        #endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
    )
#endif
{
    Wavetable wavetable;
    generateSineWavetable(wavetable, 256);

    synthesizer.clearVoices();
    synthesizer.addVoice(new WavetableSynthesizerVoice(wavetable));

    synthesizer.clearSounds();
    synthesizer.addSound(new WavetableSynthesizerSound());
}

WavetableSynthAudioProcessor::~WavetableSynthAudioProcessor()
{
}

//==============================================================================
const juce::String WavetableSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WavetableSynthAudioProcessor::acceptsMidi() const
{
    #if JucePlugin_WantsMidiInput
        return true;
    #else
        return false;
    #endif
}

bool WavetableSynthAudioProcessor::producesMidi() const
{
    #if JucePlugin_ProducesMidiOutput
        return true;
    #else
        return false;
    #endif
}

bool WavetableSynthAudioProcessor::isMidiEffect() const
{
    #if JucePlugin_IsMidiEffect
        return true;
    #else
        return false;
    #endif
}

double WavetableSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WavetableSynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int WavetableSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WavetableSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WavetableSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void WavetableSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WavetableSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synthesizer.setCurrentPlaybackSampleRate(sampleRate);
}

void WavetableSynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WavetableSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
        juce::ignoreUnused (layouts);
        return true;
    #else
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        #if ! JucePlugin_IsSynth
            if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
                return false;
        #endif
        return true;
    #endif
}
#endif

void WavetableSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    synthesizer.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool WavetableSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WavetableSynthAudioProcessor::createEditor()
{
    return new WavetableSynthAudioProcessorEditor (*this);
}

//==============================================================================
void WavetableSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WavetableSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
