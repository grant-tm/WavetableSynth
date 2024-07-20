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

void generateSquareWavetable(Wavetable& tableToFill, int resolution)
{
    tableToFill.setSize(1, resolution);
    auto* samples = tableToFill.getWritePointer(0);

    for (int i = 0; i < resolution; i++)
    {
        samples[i] = (i >= (resolution / 2)) ? -1.f : 1.f;
    }
}

void generateSawWavetable(Wavetable& tableToFill, int resolution)
{
    tableToFill.setSize(1, resolution);
    auto* samples = tableToFill.getWritePointer(0);

    for (int i = 0; i < resolution; i++)
    {
        samples[i] = -1.f + (2.f * (float)i / (float)resolution);
    }
}

void generateMultiSineWavetable(Wavetable& tableToFill, int resolution, int coefficientA, int coefficientB)
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

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] *= (float)sample;
        currentAngle += coefficientA * angleDelta;
    }

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] *= (float)sample;
        currentAngle += coefficientB * angleDelta;
    }
}

//==============================================================================
WavetableSynthAudioProcessor::WavetableSynthAudioProcessor() :
oversamplingEngine(2, (size_t)std::log(oversampleCoefficient), juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false)
#ifndef JucePlugin_PreferredChannelConfigurations
    , AudioProcessor (BusesProperties()
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
    generateSawWavetable(wavetable, 512);

    synthesizer.stateValueTree = &valueTree;

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
    oversamplingEngine.initProcessing(static_cast<size_t>(samplesPerBlock));
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

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversamplingEngine.processSamplesUp(block);
    
    setLatencySamples(oversamplingEngine.getLatencyInSamples());

    float* channels[2] = { oversampledBlock.getChannelPointer(0), oversampledBlock.getChannelPointer(1) };
    juce::AudioBuffer<float> oversampledBuffer{channels, 2, static_cast<int>(oversampledBlock.getNumSamples())};
    
    auto voice = dynamic_cast<WavetableSynthesizerVoice *>(synthesizer.getVoice(0));
    voice->setRenderLevel(synthesizer.stateValueTree->getRawParameterValue("OSC_VOLUME")->load());

    synthesizer.setCurrentPlaybackSampleRate(getSampleRate() * oversampleCoefficient);
    synthesizer.renderNextBlock(oversampledBuffer, midiMessages, 0, oversampledBuffer.getNumSamples());
    
    oversamplingEngine.processSamplesDown(block);
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
    juce::MemoryOutputStream stateOutputStream(destData, true);
    valueTree.state.writeToStream(stateOutputStream);
}

void WavetableSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        valueTree.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout WavetableSynthAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //----------------------------------
    // OSC PARAMETERS

    // osc volume
    auto oscVolumeRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_VOLUME", "OSC_VOLUME", oscVolumeRange, 0.75f));

    // osc panning
    auto oscPanningRange = juce::NormalisableRange<float>(-1.f, 1.f, .02f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_PANNING", "OSC_PANNING", oscVolumeRange, 0.f));

    // osc detune mix
    auto oscDetuneMixRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_DETUNE_MIX", "OSC_DETUNE_MIX", oscDetuneMixRange, 0.f));

    // osc warp amount
    auto oscWarpAmountRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_WARP_AMOUNT", "OSC_WARP_AMOUNT", oscWarpAmountRange, 0.f));

    // osc wavetable position
    auto oscWavetablePositionRange = juce::NormalisableRange<float>(0.f, 255.f, 1.f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_WAVETABLE_POSITION", "OSC_WAVETABLE_POSITION", oscWavetablePositionRange, 0.f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
