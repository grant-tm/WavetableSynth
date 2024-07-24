#include "Synthesizer.h"

//=============================================================================
// CONSTRUCTORS / DESTRUCTORS

Synthesizer::Synthesizer()
{
    initializeOscillators();
}

Synthesizer::~Synthesizer()
{

}


//=============================================================================
// SET PARAMETERS

//-------------------------------------
// sample rate

void Synthesizer::setSampleRate(float newSampleRate)
{
    newSampleRate = juce::jmax(0.f, newSampleRate);
    newSampleRate = juce::jmin(192000.f, newSampleRate);

    this->sampleRate = newSampleRate;
}

float Synthesizer::getSampleRate()
{
    return this->sampleRate;
}


//-------------------------------------
// frequency

void Synthesizer::setFrequency(float newFrequency)
{
    // limit frequnecy to [0, 20k]
    newFrequency = juce::jmax(0.f, newFrequency);
    newFrequency = juce::jmin(newFrequency, 20000.f);

    this->frequency = newFrequency;
}

void Synthesizer::setFrequencyByMidiNote(int midiNoteNumber, float pitchWheelPosition)
{
    auto newFrequency = calculateFrequencyFromOffsetMidiNote(midiNoteNumber, pitchWheelPosition);
    setFrequency(newFrequency);
}

float Synthesizer::calculateFrequencyFromMidiInput(int midiNoteNumber, float pitchWheelPosition)
{
    auto pitchBendOffsetCents = getPitchBendOffsetCents(pitchWheelPosition);
    auto calculatedFrequency = calculateFrequencyFromOffsetMidiNote(midiNoteNumber, pitchBendOffsetCents);
    return calculatedFrequency;
}

float Synthesizer::calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset)
{
    auto noteHz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    noteHz *= std::pow(2.0, centsOffset / 1200);
    return (float)noteHz;
}

//-------------------------------------
// volume

void Synthesizer::setVolume(float newVolume)
{
    // limit volume to [0, 1]
    newVolume = juce::jmax(0.f, newVolume);
    newVolume = juce::jmin(newVolume, 1.f);

    this->volume = newVolume;
}

//-------------------------------------
// pan

void Synthesizer::setPan(float newPan)
{
    // limit pan to [-1, 1]
    newPan = juce::jmax(-1.f, newPan);
    newPan = juce::jmin(newPan, 1.f);

    this->pan = newPan;
}

//=============================================================================
// OSCILLATORS

void Synthesizer::initializeOscillators()
{
    for (auto &oscillator : oscillators)
    {
        oscillator.setSampleRate(sampleRate);
        oscillator.setFrequency(frequency);
        oscillator.setVolume(volume);
        oscillator.setPan(pan);

        oscillator.setWavetable(getWavetableReadPointer());
        oscillator.setWavetableFrameIndex(0);

        oscillator.setDetuneVoices(detuneVoices);
        oscillator.setDetuneMix(detuneMix);
        oscillator.setDetuneSpread(detuneSpread);
    }
}

void Synthesizer::updateOscillatorParameters(Oscillator& oscillator)
{
    oscillator.setSampleRate(sampleRate);
    oscillator.setVolume(volume);
    oscillator.setPan(pan);

    oscillator.setWavetable(getWavetableReadPointer());
    oscillator.setWavetableFrameIndex(0);

    oscillator.setDetuneVoices(detuneVoices);
    oscillator.setDetuneMix(detuneMix);
    oscillator.setDetuneSpread(detuneSpread);
    oscillator.updateDetuneVoiceConfiguration();
}

//=============================================================================
// WAVETABLE

void Synthesizer::setWavetable(Wavetable &wavetableToCopy)
{
	wavetable = wavetableToCopy;

    for (auto &oscillator : oscillators)
    {
        oscillator.setWavetable(getWavetableReadPointer());
    }

    wavetableSize = wavetable.getNumSamples();
    wavetableNumFrames = wavetable.getNumChannels();
}

void Synthesizer::setWavetableFrameIndex(int newFrameIndex)
{
    newFrameIndex = juce::jmax(0, newFrameIndex);
    newFrameIndex = juce::jmin(wavetableNumFrames, newFrameIndex);

    this->wavetableFrameIndex = newFrameIndex;
}

const Wavetable *Synthesizer::getWavetableReadPointer()
{
	return &wavetable;
}

int Synthesizer::getNumWavetableFrames()
{
	return wavetable.getNumChannels();
}

//=============================================================================
// RENDER

void Synthesizer::render(juce::AudioBuffer<float> &buffer, int startSample, int numSamples)
{ 
    for (auto &oscillator : oscillators)
    {
        if (oscillator.isEnabled())
        {
            oscillator.render(buffer, startSample, numSamples);
        }
    }
}

void Synthesizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiBuffer)
{
    auto currentSample = 0;

    oscillators[0].setVolume(volume);
    oscillators[0].setPan(pan);

    updateOscillatorDetuneIfChanged(0);

    for (const auto midiData : midiBuffer)
    {
        // get next midi message timestamp
        const auto midiMessage = midiData.getMessage();
        const auto midiMessagePosition = static_cast<int>(midiMessage.getTimeStamp());

        // render up to next midi message
        render(buffer, currentSample, midiMessagePosition - currentSample);
        currentSample = midiMessagePosition;
        
        // handle midi message
        handleMidiEvent(midiMessage);
    }

    // render the rest of the block
    render(buffer, currentSample, buffer.getNumSamples() - currentSample);

    
}

void Synthesizer::updateOscillatorDetuneIfChanged(int oscillatorId)
{
    auto oscillator = oscillators[oscillatorId];

    if (oscillator.getDetuneVoices() != detuneVoices ||
        oscillator.getDetuneMix()    != detuneMix    ||
        oscillator.getDetuneSpread() != detuneSpread )
    {
        oscillator.setDetuneVoices(detuneVoices);
        oscillator.setDetuneMix(detuneMix);
        oscillator.setDetuneSpread(detuneSpread);
        oscillator.updateDetuneVoiceConfiguration();
    }
}

//=============================================================================
// MIDI

void Synthesizer::handleMidiEvent(const juce::MidiMessage &midiMessage)
{
    if (midiMessage.isNoteOn())
    {
        auto midiNoteNumber = midiMessage.getNoteNumber();
        auto noteVelocity = midiMessage.getFloatVelocity();
       
        oscillators[0].setFrequency(calculateFrequencyFromOffsetMidiNote(midiNoteNumber, 0));
        oscillators[0].setVelocity(noteVelocity);
        oscillators[0].setEnable(true);
    }
    else if (midiMessage.isNoteOff())
    {
    }
    else if (midiMessage.isAllNotesOff())
    {
        oscillators[0].setEnable(false);
    }
}

void Synthesizer::startNote(int midiNoteNumber, int velocity, int pitchWheelPosition)
{
    oscillators[0].setFrequency(calculateFrequencyFromOffsetMidiNote(midiNoteNumber, 0));
    oscillators[0].setVelocity(velocity);
    oscillators[0].setEnable(true);
}

void Synthesizer::stopNote(int midiNoteNumber)
{
    auto oscillatorId = 0; // todo add findVoicePlayingNote();
    auto oscillator = &(oscillators[oscillatorId]);
    oscillator->setEnable(false);

}

// pitch wheel move callback: store new position and update frequency
void Synthesizer::pitchWheelMoved(int newPitchWheelValue)
{
    setPitchBendPosition(newPitchWheelValue);
}

// set the pitch bend position (normalized to a [-1, 1] float value)
void Synthesizer::setPitchBendPosition(int position)
{
    if (position > 8192)
        pitchBendWheelPosition = (float)(position - 8192.f) / (16383.f - 8192.f);
    else
        pitchBendWheelPosition = (float)(8192.f - position) / -8192.f;
}

// convert pitch bend wheel position to a note offset in cents based on wheel upper or lower bounds
float Synthesizer::getPitchBendOffsetCents()
{
    if (pitchBendWheelPosition >= 0.0f)
    {
        // calculate cents based on position relative to UPPER bound
        return pitchBendWheelPosition * (float)pitchBendUpperBoundSemitones * 100.f;
    }
    else
    {
        // calculate cents based on position relative to LOWER lower
        return pitchBendWheelPosition * (float)pitchBendLowerBoundSemitones * 100.f;
    }
}

// convert pitch bend wheel position to a note offset in cents based on wheel upper or lower bounds
float Synthesizer::getPitchBendOffsetCents(float pitchWheelPosition)
{
    if (pitchWheelPosition >= 0.0f)
    {
        // calculate cents based on position relative to UPPER bound
        return pitchWheelPosition * (float)pitchBendUpperBoundSemitones * 100.f;
    }
    else
    {
        // calculate cents based on position relative to LOWER lower
        return pitchWheelPosition * (float)pitchBendLowerBoundSemitones * 100.f;
    }
}

//-------------------------------------
// detune num voices
void Synthesizer::setDetuneVoices(int newNumVoices)
{
    // limit newNumVoices to [1, MAX_DETUNE_VOICES]
    newNumVoices = std::max(1, newNumVoices);
    newNumVoices = std::min(MAX_DETUNE_VOICES, newNumVoices);

    this->detuneVoices = newNumVoices;
};

//-------------------------------------
// detune frequency offsets

void Synthesizer::setDetuneMix(float newDetuneMix)
{
    // limit newDetuneMix to [0, 1]
    newDetuneMix = std::max(0.f, newDetuneMix);
    newDetuneMix = std::min(1.f, newDetuneMix);

    this->detuneMix = newDetuneMix;
}

//-------------------------------------
// detune frequency offsets

void Synthesizer::setDetuneSpread(float newDetuneSpread)
{
    // limit newDetuneSpread to [0, 1]
    newDetuneSpread = std::max(0.f, newDetuneSpread);
    newDetuneSpread = std::min(1.f, newDetuneSpread);

    this->detuneMix = newDetuneSpread;
}
