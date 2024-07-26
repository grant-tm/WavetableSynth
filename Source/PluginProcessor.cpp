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

void generateSineFrames(Wavetable &tableToFill, int resolution)
{
    tableToFill.setSize(3, resolution);

    auto *samples = tableToFill.getWritePointer(0);
    auto angleDelta = juce::MathConstants<float>::twoPi / (float)(resolution - 1);
    auto currentAngle = 0.f;

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] = (float)sample;
        currentAngle += angleDelta;
    }

    samples = tableToFill.getWritePointer(1);
    angleDelta *= 2.f;
    currentAngle = 0.f;

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] = (float)sample;
        currentAngle += angleDelta;
    }

    samples = tableToFill.getWritePointer(2);
    angleDelta *= 2.f;
    currentAngle = 0.0;

    for (int i = 0; i < resolution; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] = (float)sample;
        currentAngle += angleDelta;
    }
}

void generateManySineFrames(Wavetable &tableToFill)
{
    int numFrames = 255;
    int numSamples = 1024;
    
    tableToFill.setSize(numFrames, numSamples);

    auto angleDelta = juce::MathConstants<float>::twoPi / (float)(numSamples - 1);

    for (int frame = 0; frame < numFrames; frame++)
    {
        auto currentAngle = 0.f;
        auto *samples = tableToFill.getWritePointer(frame);
        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            auto sampleValue = std::sin(currentAngle);
            samples[sampleIndex] = (float) sampleValue;
            currentAngle += angleDelta;
        }
        angleDelta *= 1.01;
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
    //generateSineFrames(wavetable, 512);
    generateManySineFrames(wavetable);
    synthesizer.setWavetable(wavetable);
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
    synthesizer.setSampleRate(sampleRate);
   
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

    // UPDATE
    updateSynthesizerParametersFromValueTree();
    
    // RENDER
    renderOversampledBlock(buffer, midiMessages);
}

void WavetableSynthAudioProcessor::updateSynthesizerParametersFromValueTree()
{
    synthesizer.setVolume(valueTree.getRawParameterValue("OSC_VOLUME")->load());
    synthesizer.setPan(valueTree.getRawParameterValue("OSC_PANNING")->load());

    synthesizer.setDetuneVoices(1);
    synthesizer.setDetuneSpread(0.4f);
    synthesizer.setDetuneMix(valueTree.getRawParameterValue("OSC_DETUNE_MIX")->load());

    auto wavetablePositionKnobValue = valueTree.getRawParameterValue("OSC_WAVETABLE_POSITION")->load();
    auto wavetablePosition = std::floor(wavetablePositionKnobValue * (std::max(0, synthesizer.getNumWavetableFrames() - 1)));
    valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->store(wavetablePosition);
    synthesizer.setWavetableFrameIndex(valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->load());
}

void WavetableSynthAudioProcessor::renderOversampledBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversamplingEngine.processSamplesUp(block);

    setLatencySamples(oversamplingEngine.getLatencyInSamples());

    float *channels[2] = {oversampledBlock.getChannelPointer(0), oversampledBlock.getChannelPointer(1)};
    juce::AudioBuffer<float> oversampledBuffer{channels, 2, static_cast<int>(oversampledBlock.getNumSamples())};

    synthesizer.setSampleRate(getSampleRate() * oversampleCoefficient);
    synthesizer.processBlock(oversampledBuffer, midiMessages);

    oversamplingEngine.processSamplesDown(block);
}

//==============================================================================
bool WavetableSynthAudioProcessor::hasEditor() const
{
    return true;
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
    auto oscPanningRange = juce::NormalisableRange<float>(-1.f, 1.f, .01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_PANNING", "OSC_PANNING", oscPanningRange, 0.f));

    // osc detune mix
    auto oscDetuneMixRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_DETUNE_MIX", "OSC_DETUNE_MIX", oscDetuneMixRange, 1.f));

    // osc warp amount
    auto oscWarpAmountRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_WARP_AMOUNT", "OSC_WARP_AMOUNT", oscWarpAmountRange, 0.f));

    // osc wavetable position
    auto oscWavetablePositionRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_WAVETABLE_POSITION", "OSC_WAVETABLE_POSITION", oscWavetablePositionRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterInt>("OSC_WAVETABLE_NUM_FRAMES", "OSC_WAVETABLE_NUM_FRAMES", 0, 256, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("OSC_WAVETABLE_CURRENT_FRAME", "OSC_WAVETABLE_CURRENT_FRAME", 0, 512, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
