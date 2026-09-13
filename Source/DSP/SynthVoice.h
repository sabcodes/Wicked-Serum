/*
  ==============================================================================
    SynthVoice.h

    A single polyphonic voice. Each voice contains its own set of oscillators,
    filter, and envelopes so multiple notes can play simultaneously.

    When you press a key in Logic:
    1. The SynthEngine finds a free voice (or steals the oldest one)
    2. That voice's oscillators start playing the note frequency
    3. Envelopes trigger (attack begins)
    4. When you release the key, envelopes enter release stage
    5. When all envelopes finish, the voice becomes free again
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WavetableOscillator.h"
#include "SubOscillator.h"
#include "NoiseGenerator.h"
#include "SynthFilter.h"
#include "ADSREnvelope.h"
#include "LFO.h"
#include "ModulationMatrix.h"
#include "VelocityCurve.h"

class SynthVoice
{
public:
    SynthVoice();
    ~SynthVoice() = default;

    void prepare(double sampleRate);

    // Process a block of audio for this voice
    void renderBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    // Note events
    void noteOn(int midiNote, float velocity);
    void noteOff();
    void reset();

    // Is this voice currently producing sound?
    bool isActive() const { return ampEnvelope.isActive(); }

    // Which note is this voice playing?
    int getCurrentNote() const { return currentNote; }

    // Get the age of this voice (for voice stealing — oldest gets stolen first)
    int getAge() const { return age; }
    void incrementAge() { ++age; }

    // --- Access to components for parameter updates ---
    WavetableOscillator& getOscA() { return oscA; }
    WavetableOscillator& getOscB() { return oscB; }
    SubOscillator& getSubOsc() { return subOsc; }
    NoiseGenerator& getNoise() { return noise; }
    SynthFilter& getFilter() { return filter; }
    ADSREnvelope& getAmpEnvelope() { return ampEnvelope; }
    ADSREnvelope& getFilterEnvelope() { return filterEnvelope; }
    ADSREnvelope& getModEnvelope() { return modEnvelope; }
    ModulationMatrix& getModMatrix() { return modMatrix; }

    // LFO access (LFOs are per-voice when retriggered, shared when free-running)
    LFO& getLFO(int index) { return lfos[juce::jlimit(0, 3, index)]; }

    // Pitch bend (from MIDI, in semitones)
    void setPitchBend(float semitones) { pitchBendSemitones = semitones; }

    // Stable base values used by the modulation engine.
    void setOscAWavetablePositionBase(float value) { baseOscAWTPosition = value; }
    void setOscALevelBase(float value) { baseOscALevel = value; }
    void setOscAPanBase(float value) { baseOscAPan = value; }
    void setOscADetuneBase(float value) { baseOscADetune = value; }
    void setOscAWarpBase(float value) { baseOscAWarp = value; }
    void setOscBWavetablePositionBase(float value) { baseOscBWTPosition = value; }
    void setOscBLevelBase(float value) { baseOscBLevel = value; }
    void setOscBPanBase(float value) { baseOscBPan = value; }
    void setOscBDetuneBase(float value) { baseOscBDetune = value; }
    void setOscBWarpBase(float value) { baseOscBWarp = value; }
    void setFilterCutoffBase(float value) { baseFilterCutoff = value; }
    void setFilterResonanceBase(float value) { baseFilterResonance = value; }
    void setFilterDriveBase(float value) { baseFilterDrive = value; }
    void setFilterMixBase(float value) { baseFilterMix = value; }

    // Velocity curve
    VelocityCurve& getVelocityCurve() { return velocityCurve; }

private:
    // Calculate the frequency for the current note with pitch bend
    float getNoteFrequency() const;

    // Apply modulation matrix to all parameters
    void applyModulation();

    double sampleRate = 44100.0;
    int currentNote = -1;
    float currentVelocity = 0.0f;
    int age = 0;
    float pitchBendSemitones = 0.0f;

    // === Sound sources ===
    WavetableOscillator oscA;   // Wavetable Oscillator A
    WavetableOscillator oscB;   // Wavetable Oscillator B
    SubOscillator subOsc;        // Sub Oscillator
    NoiseGenerator noise;        // Noise Generator

    // === Processing ===
    SynthFilter filter;          // Multi-mode filter

    // === Modulation ===
    ADSREnvelope ampEnvelope;    // ENV 1 — always controls volume
    ADSREnvelope filterEnvelope; // ENV 2 — typically controls filter cutoff
    ADSREnvelope modEnvelope;    // ENV 3 — free modulation envelope
    LFO lfos[4];                 // 4 LFOs for modulation
    ModulationMatrix modMatrix;  // Routes mod sources to destinations

    // Base parameter values (before modulation)
    float baseOscAWTPosition = 0.0f, baseOscALevel = 1.0f, baseOscAPan = 0.0f;
    float baseOscADetune = 0.0f, baseOscAWarp = 0.0f;
    float baseOscBWTPosition = 0.0f, baseOscBLevel = 0.0f, baseOscBPan = 0.0f;
    float baseOscBDetune = 0.0f, baseOscBWarp = 0.0f;
    float baseFilterCutoff = 20000.0f;
    float baseFilterResonance = 0.0f, baseFilterDrive = 0.0f, baseFilterMix = 1.0f;

    // Velocity curve processor
    VelocityCurve velocityCurve;
};
