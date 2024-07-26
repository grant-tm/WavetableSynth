#include "Synthesizer.h"

//=============================================================================
// PUBLIC CONSTRUCTORS / DESTRUCTORS

Synthesizer::Synthesizer()
{
    // initialize voices
    int voiceId = 0;
    for (auto &voice : voices)
    {
        voice.id = voiceId++;
    }

    // initialize oscillators
    updateOscillators();
}

//=============================================================================
// RENDER PARAMETERS GETTERS & SETTERS

float Synthesizer::getSampleRate() const
{
    return this->sampleRate;
}

void Synthesizer::setSampleRate(float newSampleRate)
{
    newSampleRate = juce::jmax(0.f, newSampleRate);
    newSampleRate = juce::jmin(192000.f, newSampleRate);

    this->sampleRate = newSampleRate;
}

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

void Synthesizer::setVolume(float newVolume)
{
    // limit volume to [0, 1]
    newVolume = juce::jmax(0.f, newVolume);
    newVolume = juce::jmin(newVolume, 1.f);

    this->volume = newVolume;
}

void Synthesizer::setPan(float newPan)
{
    // limit pan to [-1, 1]
    newPan = juce::jmax(-1.f, newPan);
    newPan = juce::jmin(newPan, 1.f);

    this->pan = newPan;
}

//=============================================================================
// DETUNE PARAMETERS GETTERS & SETTERS

void Synthesizer::setDetuneVoices(int newNumVoices)
{
    // limit newNumVoices to [1, MAX_DETUNE_VOICES]
    newNumVoices = std::max(1, newNumVoices);
    newNumVoices = std::min(MAX_DETUNE_VOICES, newNumVoices);

    this->detuneVoices = newNumVoices;
};

void Synthesizer::setDetuneMix(float newDetuneMix)
{
    // limit newDetuneMix to [0, 1]
    newDetuneMix = std::max(0.f, newDetuneMix);
    newDetuneMix = std::min(1.f, newDetuneMix);

    this->detuneMix = newDetuneMix;
}

void Synthesizer::setDetuneSpread(float newDetuneSpread)
{
    // limit newDetuneSpread to [0, 1]
    newDetuneSpread = std::max(0.f, newDetuneSpread);
    newDetuneSpread = std::min(1.f, newDetuneSpread);

    this->detuneMix = newDetuneSpread;
}

//=============================================================================
// WAVETABLE GETTERS & SETTERS

const Wavetable *Synthesizer::getWavetableReadPointer() const
{
    return &wavetable;
}

int Synthesizer::getNumWavetableFrames() const
{
    return wavetable.getNumChannels();
}

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

//=============================================================================
// OSCILLATORS

void Synthesizer::updateOscillators()
{
    for (auto &oscillator : oscillators)
    {
        updateOscillator(oscillator);
    }
}

void Synthesizer::updateOscillator(Oscillator &oscillator)
{
    oscillator.setSampleRate(sampleRate);
    oscillator.setVolume(volume);
    oscillator.setPan(pan);

    oscillator.setWavetable(getWavetableReadPointer());
    oscillator.setWavetableFrameIndex(wavetableFrameIndex);

    updateOscillatorDetuneParameters(oscillator);
}

// sets oscillator detune parameters and triggers voice config recalculation
// if and only if synthesizer detune parameters changed
void Synthesizer::updateOscillatorDetuneParameters(Oscillator &oscillator)
{
    if (oscillator.getDetuneVoices() != detuneVoices ||
        oscillator.getDetuneMix() != detuneMix ||
        oscillator.getDetuneSpread() != detuneSpread)
    {
        oscillator.setDetuneVoices(detuneVoices);
        oscillator.setDetuneMix(detuneMix);
        oscillator.setDetuneSpread(detuneSpread);
        oscillator.updateDetuneVoiceConfiguration();
    }
}

//=============================================================================
// RENDERING

void Synthesizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiBuffer)
{
    auto currentSample = 0;

    updateOscillators();

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

void Synthesizer::render(juce::AudioBuffer<float> &buffer, int startSample, int numSamples)
{ 
    buffer.clear(startSample, numSamples);
    for (auto &oscillator : this->oscillators)
    {
        if (oscillator.adsrEnvelopeIsActive())
        {
            oscillator.render(buffer, startSample, numSamples);
        }
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
        startNote(midiNoteNumber, noteVelocity, 0);
    }
    else if (midiMessage.isNoteOff())
    {
        stopNote(midiMessage.getNoteNumber());
    }
}

void Synthesizer::startNote(int midiNoteNumber, float velocity, int pitchWheelPosition)
{
    auto voiceIndex = findVoice(midiNoteNumber);
    if (voiceIndex < 0 || voiceIndex > MAX_POLYPHONY)
    {
        // reject input
        return;
    }

    updateVoiceAges();
    auto &voice = voices[voiceIndex];
    voice.noteNumber = midiNoteNumber;
    voice.age = 0;

    auto &oscillator = oscillators[voiceIndex];
    oscillator.setFrequency(calculateFrequencyFromOffsetMidiNote(midiNoteNumber, 0));
    oscillator.setVelocity(velocity);
    oscillator.startAdsrEnvelope();

}

void Synthesizer::updateVoiceAges()
{
    for (auto &voice : voices)
    {
        // skip idle voices
        if (voice.age < 0) {
            continue;
        }

        // increment or reset voice age based on osc adsr envelope status
        auto oscIsActive = oscillators[voice.id].adsrEnvelopeIsActive();
        voice.age = oscIsActive ? ++voice.age : -1;
    }
}

void Synthesizer::stopNote(int midiNoteNumber)
{
    auto voiceIndex = findVoicePlayingNote(midiNoteNumber);
    if (voiceIndex < 0 || voiceIndex > MAX_POLYPHONY)
    {
        return;
    }

    auto &voice = voices[voiceIndex];
    auto &oscillator = oscillators[voiceIndex];
    oscillator.releaseAdsrEnvelope();
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

//=============================================================================
// VOICE MANAGEMENT

int Synthesizer::findVoice(int midiNoteNumber) const
{
    int voidId = -1;
    
    // 1st choice: find voice playing the same note to retrigger
    if ((voidId = findVoicePlayingNote(midiNoteNumber)) >= 0) {
        return voidId;
    }

    // 2nd choice: trigger an inactive voice
    if ((voidId = findFreeVoice()) >= 0) {
        return voidId;
    }

    // all voices active!
    // report no voice availble if voice stealing is disabled
    if (!voiceStealingEnabled)
    {
        return -1;
    }

    // 3rd choice: steal oldest voice (if allowed)
    if ((voidId = findOldestVoice()) >= 0) {
        return voidId;
    }

    // there is a bug in the voice management system
    jassert(false);
    return voidId;
}

int Synthesizer::findVoicePlayingNote(int midiNoteNumber) const
{
    // serach for voice playing note
    for (const auto &voice : voices)
    {
        if (voice.noteNumber == midiNoteNumber)
        {
            return voice.id;
        }
    }

    // failed to find voice playing the given note
    return -1;
}

int Synthesizer::findFreeVoice() const
{
    // serach for voice playing note
    for (const auto &voice : voices)
    {
        if (voice.age == -1)
        {
            return voice.id;
        }
    }

    // failed to find voice playing the given note
    return -1;
}

int Synthesizer::findOldestVoice() const
{
    int greatestAgeSeen = -1;
    int oldestVoiceIndex = 0;

    for (auto &voice : voices)
    {
        if (voice.age > greatestAgeSeen)
        {
            greatestAgeSeen = voice.age;
            oldestVoiceIndex = voice.id;
        }
    }

    return oldestVoiceIndex;
}

//=============================================================================
// PITCH WHEEL

void Synthesizer::pitchWheelMoved(int newPitchWheelValue)
{
    setPitchBendPosition(newPitchWheelValue);
}

void Synthesizer::setPitchBendPosition(int position)
{
    if (position > 8192)
        pitchBendWheelPosition = (float)(position - 8192.f) / (16383.f - 8192.f);
    else
        pitchBendWheelPosition = (float)(8192.f - position) / -8192.f;
}

float Synthesizer::getPitchBendOffsetCents() const
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

float Synthesizer::getPitchBendOffsetCents(float pitchWheelPosition) const
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