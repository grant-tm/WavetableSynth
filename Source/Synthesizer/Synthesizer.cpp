#include "Synthesizer.h"

inline int clampInt (int input, int lowerBound, int upperBound)
{
    input = juce::jmax(lowerBound, input);
    return juce::jmin(upperBound, input);
}

inline float clampFloat (float input, float lowerBound, float upperBound)
{
    input = juce::jmax(lowerBound, input);
    return juce::jmin(upperBound, input);
}

//=============================================================================
// PUBLIC CONSTRUCTORS / DESTRUCTORS

Synthesizer::Synthesizer()
{
    wavetableSize = 0;
    wavetableNumFrames = 0;
    wavetableFrameIndex = 0;

    // initialize oscillators
    updateOscillators();

    // initialize adsr
    adsrParameters = juce::ADSR::Parameters(0.10f, 1.f, 0.8f, 1.f);

    sampleRate = 22100.f;
    frequency = 200.f;
    volume = 1.f;
    pan = 0.f;

    detuneVoices = 5;
    detuneMix = 1.f;
    detuneSpread = 1.f;

    voiceStealingEnabled = true;

    pitchBendWheelPosition = 0;
    pitchBendUpperBoundSemitones = 2;
    pitchBendLowerBoundSemitones = -2;

    // initialize voices
    int voiceId = 0;
    for (auto &voice : voices)
    {
        voice.id = voiceId++;
    }
}

//=============================================================================
// RENDER PARAMETERS GETTERS & SETTERS

float Synthesizer::getSampleRate() const
{
    return this->sampleRate;
}

// [0, MAX FLOAT]
void Synthesizer::setSampleRate(float newSampleRate)
{
    this->sampleRate = clampFloat(newSampleRate, 0.f, FLT_MAX);
}

// [0, 20k]
void Synthesizer::setFrequency(float newFrequency)
{
    this->frequency = clampFloat(newFrequency, 0.f, 20000.f);
}

void Synthesizer::setFrequencyByMidiNote(int midiNoteNumber, float pitchWheelPosition)
{
    auto newFrequency = calculateFrequencyFromOffsetMidiNote(midiNoteNumber, pitchWheelPosition);
    setFrequency(newFrequency);
}

// [0, 1]
void Synthesizer::setVolume(float newVolume)
{
    this->volume = clampFloat(newVolume, 0.f, 1.f);
}

// [-1, 1]
void Synthesizer::setPan(float newPan)
{
    this->pan = clampFloat(newPan, -1.f, 1.f);
}

//=============================================================================
// ADSR PARAMETERS

void Synthesizer::setAdsrParameters(float attack, float decay, float sustain, float release)
{
    adsrParameters.attack = clampFloat(attack, 0.f, 15.f);
    adsrParameters.decay = clampFloat(decay, 0.f, 15.f);
    adsrParameters.sustain = clampFloat(sustain, 0.f, 1.f);
    adsrParameters.release = clampFloat(release, 0.f, 15.f);
    
    for (auto &osc : oscillators) {
        osc.setAdsrParameters(adsrParameters);
    }
}

//=============================================================================
// DETUNE PARAMETERS GETTERS & SETTERS

// [1, MAX_DETUNE_VOICES]
void Synthesizer::setDetuneVoices(int newNumVoices)
{
    this->detuneVoices = clampInt(newNumVoices, 1, MAX_DETUNE_VOICES);
};

// [0, 1]
void Synthesizer::setDetuneMix(float newDetuneMix)
{
    this->detuneMix = clampFloat(newDetuneMix, 0.f, 1.f);
}

// [0, 1]
void Synthesizer::setDetuneSpread(float newDetuneSpread)
{
    this->detuneMix = clampFloat(newDetuneSpread, 0.f, 1.f);
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
    this->wavetableFrameIndex = clampInt(newFrameIndex, 0, newFrameIndex);
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
        oscillator.setDetuneVoices(detuneVoices);
        oscillator.setDetuneSpread(detuneSpread);
        oscillator.setDetuneMix(detuneMix);
        oscillator.updateDetuneVoiceConfiguration();
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
        startNote(midiNoteNumber, noteVelocity);
    }
    else if (midiMessage.isNoteOff())
    {
        stopNote(midiMessage.getNoteNumber());
    }
    else if (midiMessage.isPitchWheel())
    {
        pitchWheelMoved(midiMessage.getPitchWheelValue());
    }
}

void Synthesizer::startNote(int midiNoteNumber, float velocity)
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
    oscillator.setFrequency(calculateFrequencyFromOffsetMidiNote(midiNoteNumber, getPitchBendOffsetCents()));
    oscillator.setVelocity(velocity);
    oscillator.randomizePhases();
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

    auto &oscillator = oscillators[voiceIndex];
    oscillator.releaseAdsrEnvelope();
}

void Synthesizer::pitchWheelMoved(int newPitchWheelValue)
{
    setPitchBendPosition(newPitchWheelValue);
    for (auto &voice : voices)
    {
        if (voice.age > -1) {
            oscillators[voice.id].setFrequency(calculateFrequencyFromOffsetMidiNote(voice.noteNumber, getPitchBendOffsetCents()));
        }
    }
}

float Synthesizer::calculateFrequencyFromMidiInput(int midiNoteNumber, float pitchWheelPosition) const
{
    auto pitchBendOffsetCents = getPitchBendOffsetCents(pitchWheelPosition);
    auto calculatedFrequency = calculateFrequencyFromOffsetMidiNote(midiNoteNumber, pitchBendOffsetCents);
    return calculatedFrequency;
}

float Synthesizer::calculateFrequencyFromOffsetMidiNote(int midiNoteNumber, float centsOffset) const
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

    // all voices are active; reject input if voice stealing is disabled
    if (!voiceStealingEnabled)
        return -1;

    // 3rd choice: steal oldest voice
    if ((voidId = findOldestVoice()) >= 0) {
        return voidId;
    }

    // doomsday
    jassert(false);
    return 0;
}

int Synthesizer::findVoicePlayingNote(int midiNoteNumber) const
{
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
    for (const auto &voice : voices)
    {
        if (voice.age == -1)
        {
            return voice.id;
        }
    }

    // failed to find free voice
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

void Synthesizer::setPitchBendPosition(int position)
{
    if (position > 8192)
        pitchBendWheelPosition = (float) (position - 8192.f) / (16383.f - 8192.f);
    else
        pitchBendWheelPosition = (float) (8192.f - position) / -8192.f;
}

float Synthesizer::getPitchBendOffsetCents() const
{
    if (pitchBendWheelPosition >= 0.0f)
        return pitchBendWheelPosition * (float) pitchBendUpperBoundSemitones * 100.f;
    else
        return pitchBendWheelPosition * (float) pitchBendLowerBoundSemitones * 100.f;
}

float Synthesizer::getPitchBendOffsetCents(float pitchWheelPosition) const
{
    if (pitchWheelPosition >= 0.0f)
        return pitchWheelPosition * (float) pitchBendUpperBoundSemitones * 100.f;
    else
        return pitchWheelPosition * (float) pitchBendLowerBoundSemitones * 100.f;
}