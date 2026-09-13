/*
  ==============================================================================
    SynthEngine.h

    The top-level DSP engine that ties everything together.

    Responsibilities:
    - Manages polyphonic voices (16 voices = 16 simultaneous notes)
    - Routes MIDI input to voices (note on/off, pitch bend, mod wheel, etc.)
    - Applies global effects chain (reverb, delay, distortion, chorus)
    - Manages master volume and output

    Signal flow:
    MIDI → Voice Allocation → [Per-voice: Oscillators → Filter → Envelopes]
         → Mix all voices → Effects Chain → Master Output → DAW
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthVoice.h"
#include "Effects/EffectsChain.h"
#include <array>

// Maximum polyphony (number of simultaneous notes)
static constexpr int MAX_POLYPHONY = 16;

class SynthEngine
{
public:
    SynthEngine();
    ~SynthEngine() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void reset();

    // --- Access to components ---
    EffectsChain& getEffectsChain() { return effectsChain; }
    SynthVoice& getVoice(int index) { return voices[juce::jlimit(0, MAX_POLYPHONY - 1, index)]; }

    // Apply a parameter change to ALL voices (for global controls)
    // This is used by the plugin processor to sync parameter changes
    template<typename Func>
    void forAllVoices(Func&& func)
    {
        for (auto& voice : voices)
            func(voice);
    }

    // Master controls
    void setMasterVolume(float vol) { masterVolume = juce::jlimit(0.0f, 1.0f, vol); }
    void setPitchBendRange(int semitones) { pitchBendRange = semitones; }

    // Macro controls (for mod matrix)
    void setMacro(int index, float value);

    float getMasterVolume() const { return masterVolume; }
    int getActiveVoiceCount() const;

private:
    void handleMidiEvent(const juce::MidiMessage& msg);
    void handleNoteOn(int note, float velocity);
    void handleNoteOff(int note);
    void handlePitchBend(float bendValue);  // -1 to +1
    void handleModWheel(float value);       // 0 to 1
    void handleAftertouch(float value);     // 0 to 1
    void handlePerNotePitchBend(int channel, float bendValue);
    void handlePerNotePressure(int channel, float pressure);
    void handlePerNoteSlide(int channel, float slideValue);

    // Find a free voice, or steal the oldest one
    int findFreeVoice() const;
    int findVoicePlayingNote(int note) const;

    double sampleRate = 44100.0;

    // Polyphonic voices
    std::array<SynthVoice, MAX_POLYPHONY> voices;

    // Global effects (applied after mixing all voices)
    EffectsChain effectsChain;

    // Master parameters
    float masterVolume = 0.8f;
    int pitchBendRange = 2;  // Semitones (default: whole step)

    // Macro values
    float macros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
