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
        angleDelta *= 1.005f;
    }
}

void generateRandomSineCombinations(Wavetable &tableToFill)
{
    int numFrames = 10;
    int numSamples = 1024;

    tableToFill.setSize(numFrames, numSamples);

    auto angleDelta = juce::MathConstants<float>::twoPi / (float)(numSamples - 1);

    juce::Random rng;
    for (int frame = 0; frame < numFrames; frame++)
    {
        auto currentAngle = 0.f;
        auto secondAngle = 0.f;
        auto randomDelta = angleDelta * std::floor(1 + (rng.nextFloat() * 6));
        //auto randomDelta = angleDelta * rng.nextFloat() * 10;
        auto *samples = tableToFill.getWritePointer(frame);
        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            auto sampleValue = std::sin(currentAngle) * std::cos(secondAngle);
            samples[sampleIndex] = (float)sampleValue;
            currentAngle += angleDelta;
            secondAngle += randomDelta;
        }
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

    for (int i = 0; i < resolution/2; i++)
    {
        samples[i] = 0.f + (2.f * (float)i / (float)resolution);
    }
    for (int i = resolution / 2; i < resolution; i++)
    {
        samples[i] = -2.f + (2.f * (float)i / (float)resolution);
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
    //generateSineWavetable(wavetable, 1024);
    //generateSawWavetable(wavetable, 1024);
    //generateSquareWavetable(wavetable, 1024);

    //generateSineFrames(wavetable, 512);
    //generateManySineFrames(wavetable);
    generateRandomSineCombinations(wavetable);
    
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
    juce::ignoreUnused(index);
}

const juce::String WavetableSynthAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void WavetableSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void WavetableSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synthesizer.setSampleRate((float) sampleRate);

    oversamplingEngine.setUsingIntegerLatency(true);
    oversamplingEngine.initProcessing(static_cast<size_t>(samplesPerBlock));

    setLatencySamples((int) oversamplingEngine.getLatencyInSamples());
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

    // CLEAR
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // UPDATE
    updateSynthesizerParametersFromValueTree();
    
    // RENDER
    renderOversampledBlock(buffer, midiMessages);
}

void WavetableSynthAudioProcessor::updateSynthesizerParametersFromValueTree()
{
    // set transposition parameters
    auto octave = valueTree.getRawParameterValue("OCTAVE_TRANSPOSE")->load();
    auto semitone = valueTree.getRawParameterValue("SEMITONE_TRANSPOSE")->load();
    auto fine = valueTree.getRawParameterValue("FINE_TRANSPOSE")->load();
    auto coarse = valueTree.getRawParameterValue("COARSE_TRANSPOSE")->load();
    //synthesizer.setTranspose(octave, semitone, fine, coarse);

    // set adsr parameters
    auto attack = valueTree.getRawParameterValue("ADSR_ATTACK")->load();
    auto decay = valueTree.getRawParameterValue("ADSR_DECAY")->load();
    auto sustain = valueTree.getRawParameterValue("ADSR_SUSTAIN")->load();
    auto release = valueTree.getRawParameterValue("ADSR_RELEASE")->load();
    synthesizer.setAdsrParameters(attack, decay, sustain, release);

    // set mixing parameters
    synthesizer.setVolume(valueTree.getRawParameterValue("OSC_VOLUME")->load());
    synthesizer.setPan(valueTree.getRawParameterValue("OSC_PANNING")->load());

    // set detune parameters
    auto detuneMix = valueTree.getRawParameterValue("OSC_DETUNE_MIX")->load();
    synthesizer.setDetuneVoices(int(1 + std::floor(10.f * detuneMix)));
    synthesizer.setDetuneSpread(0.5f * detuneMix);
    synthesizer.setDetuneMix(detuneMix);

    // set wavetable parameters
    auto wavetablePositionKnobValue = valueTree.getRawParameterValue("OSC_WAVETABLE_POSITION")->load();
    int wavetablePosition = (int) std::floor(wavetablePositionKnobValue * (std::max(0, synthesizer.getNumWavetableFrames() - 1)));
    valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->store((float) wavetablePosition);
    synthesizer.setWavetableFrameIndex((int) valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->load());
}

void WavetableSynthAudioProcessor::renderOversampledBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversamplingEngine.processSamplesUp(block);

    float *channels[2] = {oversampledBlock.getChannelPointer(0), oversampledBlock.getChannelPointer(1)};
    juce::AudioBuffer<float> oversampledBuffer{channels, 2, static_cast<int>(oversampledBlock.getNumSamples())};

    synthesizer.setSampleRate((float) getSampleRate() * oversampleCoefficient);
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
    // TRANSPOSE PARAMETERS

    auto octaveTransposeRange = juce::NormalisableRange<float>(-4.f, 4.f, 1.f, 1.f);
    auto semitoneTransposeRange = juce::NormalisableRange<float>(-12.f, 12.f, 1.f, 1.f);
    auto fineTuneTransposeRange = juce::NormalisableRange<float>(-100.f, 100.f, 1.f, 1.f);
    auto coarsePitchTransposeRange = juce::NormalisableRange<float>(-48.f, 48.f, 0.01f, 1.f);

    layout.add(std::make_unique<juce::AudioParameterFloat>("OCTAVE_TRANSPOSE", "OCTAVE_TRANSPOSE", octaveTransposeRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SEMITONE_TRANSPOSE", "SEMITONE_TRANSPOSE", semitoneTransposeRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("FINE_TRANSPOSE", "FINE_TRANSPOSE", fineTuneTransposeRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("COARSE_TRANSPOSE", "COARSE_TRANSPOSE", coarsePitchTransposeRange, 0.f));

    //----------------------------------
    // ADSR PARAMETERS
    
    auto attackRange = juce::NormalisableRange<float>(0.0001f, 15.f, 0.00001f, 0.15f);
    auto decayRange = juce::NormalisableRange<float>(0.0001f, 15.f, 0.00001f, 0.15f);
    auto sustainRange = juce::NormalisableRange<float>(0.0f, 1.f, 0.01f, 1.f);
    auto releaseRange = juce::NormalisableRange<float>(0.0001f, 15.f, 0.0001f, 0.15f);

    layout.add(std::make_unique<juce::AudioParameterFloat>("ADSR_ATTACK", "ADSR_ATTACK", attackRange, 0.0005f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ADSR_DECAY", "ADSR_DECAY", decayRange, 1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ADSR_SUSTAIN", "ADSR_SUSTAIN", sustainRange, 1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ADSR_RELEASE", "ADSR_RELEASE", releaseRange, 0.015f));

    //----------------------------------
    // OSC PARAMETERS

    // osc volume
    auto oscVolumeRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    auto oscPanningRange = juce::NormalisableRange<float>(-1.f, 1.f, .01f, 1.f);
    auto oscDetuneMixRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    auto oscWarpAmountRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    auto oscWavetablePositionRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_VOLUME", "OSC_VOLUME", oscVolumeRange, 0.75f));

    // osc panning
    auto oscPanningRange = juce::NormalisableRange<float>(-1.f, 1.f, .01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_PANNING", "OSC_PANNING", oscPanningRange, 0.f));

    // osc detune mix
    auto oscDetuneMixRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("OSC_DETUNE_MIX", "OSC_DETUNE_MIX", oscDetuneMixRange, 0.f));

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
