/*
  ==============================================================================
    SynthEngine.cpp
  ==============================================================================
*/

#include "SynthEngine.h"

SynthEngine::SynthEngine()
{
}

void SynthEngine::prepare(double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;

    for (auto& voice : voices)
        voice.prepare(sampleRate);

    effectsChain.prepare(sampleRate, maxBlockSize);
}

void SynthEngine::reset()
{
    for (auto& voice : voices)
        voice.reset();

    effectsChain.reset();
}

void SynthEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear the buffer (we'll add voice output to it)
    buffer.clear();

    // Process MIDI messages and render audio sample-accurately
    // This means MIDI events are handled at the exact sample they occur

    int currentSample = 0;

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        const int msgSample = metadata.samplePosition;

        // Render audio up to this MIDI event
        if (msgSample > currentSample)
        {
            int samplesToRender = msgSample - currentSample;

            for (auto& voice : voices)
            {
                if (voice.isActive())
                {
                    voice.renderBlock(buffer, currentSample, samplesToRender);
                    voice.incrementAge();
                }
            }

            currentSample = msgSample;
        }

        // Handle the MIDI event
        handleMidiEvent(msg);
    }

    // Render remaining samples after the last MIDI event
    int remainingSamples = buffer.getNumSamples() - currentSample;
    if (remainingSamples > 0)
    {
        for (auto& voice : voices)
        {
            if (voice.isActive())
            {
                voice.renderBlock(buffer, currentSample, remainingSamples);
                voice.incrementAge();
            }
        }
    }

    // Apply global effects chain (reverb, delay, distortion, chorus)
    if (buffer.getNumChannels() >= 2)
    {
        effectsChain.processStereo(
            buffer.getWritePointer(0),
            buffer.getWritePointer(1),
            buffer.getNumSamples()
        );
    }

    // Apply master volume
    buffer.applyGain(masterVolume);
}

void SynthEngine::handleMidiEvent(const juce::MidiMessage& msg)
{
    if (msg.isNoteOn())
    {
        handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());
    }
    else if (msg.isNoteOff())
    {
        handleNoteOff(msg.getNoteNumber());
    }
    else if (msg.isPitchWheel())
    {
        // Convert 14-bit pitch bend (0-16383) to -1..+1
        float bendValue = (msg.getPitchWheelValue() - 8192) / 8192.0f;

        // MPE: per-note pitch bend if on channel 2-16 (MPE member channels)
        int channel = msg.getChannel();
        if (channel >= 2 && channel <= 16)
        {
            handlePerNotePitchBend(channel, bendValue);
        }
        else
        {
            // Master channel (1) pitch bend applies to all voices
            handlePitchBend(bendValue);
        }
    }
    else if (msg.isController())
    {
        int cc = msg.getControllerNumber();
        float value = msg.getControllerValue() / 127.0f;

        if (cc == 1) // Mod wheel
        {
            handleModWheel(value);
        }
        else if (cc == 74) // MPE: per-note slide
        {
            int channel = msg.getChannel();
            if (channel >= 2 && channel <= 16)
            {
                handlePerNoteSlide(channel, value);
            }
        }
    }
    else if (msg.isChannelPressure())
    {
        int channel = msg.getChannel();
        float pressure = msg.getChannelPressureValue() / 127.0f;

        // MPE: per-note pressure if on channel 2-16
        if (channel >= 2 && channel <= 16)
        {
            handlePerNotePressure(channel, pressure);
        }
        else
        {
            // Master channel (1) aftertouch applies to all voices
            handleAftertouch(pressure);
        }
    }
}

void SynthEngine::handleNoteOn(int note, float velocity)
{
    // First, check if this note is already playing (retrigger)
    int existingVoice = findVoicePlayingNote(note);
    if (existingVoice >= 0)
    {
        voices[existingVoice].noteOn(note, velocity);
        return;
    }

    // Find a free voice or steal the oldest
    int voiceIndex = findFreeVoice();
    voices[voiceIndex].noteOn(note, velocity);
}

void SynthEngine::handleNoteOff(int note)
{
    // Release all voices playing this note
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() == note)
        {
            voice.noteOff();
        }
    }
}

void SynthEngine::handlePitchBend(float bendValue)
{
    float bendSemitones = bendValue * static_cast<float>(pitchBendRange);

    for (auto& voice : voices)
    {
        voice.setPitchBend(bendSemitones);
        voice.getModMatrix().setSourceValue(ModSource::PitchBend, bendValue);
    }
}

void SynthEngine::handleModWheel(float value)
{
    for (auto& voice : voices)
    {
        voice.getModMatrix().setSourceValue(ModSource::ModWheel, value);
    }
}

void SynthEngine::handleAftertouch(float value)
{
    for (auto& voice : voices)
    {
        voice.getModMatrix().setSourceValue(ModSource::Aftertouch, value);
    }
}

void SynthEngine::setMacro(int index, float value)
{
    if (index >= 0 && index < 4)
    {
        macros[index] = juce::jlimit(0.0f, 1.0f, value);

        ModSource macroSource = static_cast<ModSource>(
            static_cast<int>(ModSource::Macro1) + index
        );

        for (auto& voice : voices)
        {
            voice.getModMatrix().setSourceValue(macroSource, value);
        }
    }
}

int SynthEngine::findFreeVoice() const
{
    // First pass: find an idle voice
    for (int i = 0; i < MAX_POLYPHONY; ++i)
    {
        if (!voices[i].isActive())
            return i;
    }

    // No free voices — steal the oldest one (voice stealing)
    int oldestVoice = 0;
    int oldestAge = 0;

    for (int i = 0; i < MAX_POLYPHONY; ++i)
    {
        if (voices[i].getAge() > oldestAge)
        {
            oldestAge = voices[i].getAge();
            oldestVoice = i;
        }
    }

    return oldestVoice;
}

int SynthEngine::findVoicePlayingNote(int note) const
{
    for (int i = 0; i < MAX_POLYPHONY; ++i)
    {
        if (voices[i].isActive() && voices[i].getCurrentNote() == note)
            return i;
    }
    return -1;
}

int SynthEngine::getActiveVoiceCount() const
{
    int count = 0;
    for (const auto& voice : voices)
    {
        if (voice.isActive())
            ++count;
    }
    return count;
}

void SynthEngine::handlePerNotePitchBend(int channel, float bendValue)
{
    // MPE per-note pitch bend — apply only to voices on this channel
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() != -1)
        {
            // In MPE, each channel corresponds to a single voice
            // Map channel (2-16) to voice index
            int voiceChannel = (voice.getCurrentNote() + channel) % 16; // Simple mapping
            if ((voiceChannel % 16) == (channel % 16))
            {
                voice.setPitchBend(bendValue * static_cast<float>(pitchBendRange));
                voice.getModMatrix().setSourceValue(ModSource::PerNotePitchBend, bendValue);
            }
        }
    }
}

void SynthEngine::handlePerNotePressure(int channel, float pressure)
{
    // MPE per-note pressure — apply only to voices on this channel
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() != -1)
        {
            int voiceChannel = (voice.getCurrentNote() + channel) % 16;
            if ((voiceChannel % 16) == (channel % 16))
            {
                voice.getModMatrix().setSourceValue(ModSource::PerNotePressure, pressure);
            }
        }
    }
}

void SynthEngine::handlePerNoteSlide(int channel, float slideValue)
{
    // MPE per-note slide (CC74) — apply only to voices on this channel
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() != -1)
        {
            int voiceChannel = (voice.getCurrentNote() + channel) % 16;
            if ((voiceChannel % 16) == (channel % 16))
            {
                voice.getModMatrix().setSourceValue(ModSource::PerNoteSlide, slideValue);
            }
        }
    }
}
