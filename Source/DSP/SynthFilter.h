/*
  ==============================================================================
    SynthFilter.h

    Multi-mode filter inspired by Serum's 75+ filter types.
    We implement the most essential and musical filter types:

    - Low Pass (LP): Removes high frequencies. The most used filter in synthesis.
      Turn down the cutoff for warm, muffled sounds.
    - High Pass (HP): Removes low frequencies. Great for thinning out sounds.
    - Band Pass (BP): Keeps only a band of frequencies. Good for vocal/wah sounds.
    - Notch: Removes a band of frequencies. Opposite of bandpass.
    - Low Pass Ladder: Emulates the classic Moog ladder filter — warm and musical.
    - Comb: Creates metallic, resonant peaks — like a short delay with feedback.
    - Formant: Emulates vowel sounds (A, E, I, O, U).

    KEY PARAMETERS:
    - Cutoff: Which frequency the filter acts at (20 Hz to 20 kHz)
    - Resonance: How much the filter "rings" at the cutoff frequency (0 to 1)
    - Drive: Saturation/distortion before the filter — adds harmonics and grit
    - Mix: Dry/wet blend (0 = bypass, 1 = fully filtered)
    - KeyTrack: How much the filter cutoff follows the note pitch (0 to 1)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>

enum class FilterType
{
    LowPass12,      // 12 dB/octave — gentle, transparent
    LowPass24,      // 24 dB/octave — aggressive, classic synth sound
    HighPass12,
    HighPass24,
    BandPass,
    Notch,
    LadderLP,       // Moog-style ladder — warm and fat
    Comb,           // Comb filter — metallic/flanging
    FormantA,       // Vowel "A" (ah)
    FormantE,       // Vowel "E" (eh)
    FormantI,       // Vowel "I" (ee)
    FormantO,       // Vowel "O" (oh)
    FormantU        // Vowel "U" (oo)
};

class SynthFilter
{
public:
    SynthFilter() = default;
    ~SynthFilter() = default;

    void prepare(double sampleRate);
    float processSample(float input);
    float processSampleStereo(float inputL, float inputR, float& outL, float& outR);
    void reset();

    // --- Parameter Setters ---
    void setFilterType(FilterType type) { filterType = type; updateCoefficients(); }
    void setCutoff(float freqHz)
    {
        float newCutoff = juce::jlimit(20.0f, 20000.0f, freqHz);
        if (std::abs(newCutoff - cutoff) > 0.1f) { cutoff = newCutoff; updateCoefficients(); }
    }
    void setResonance(float res)
    {
        float newRes = juce::jlimit(0.0f, 1.0f, res);
        if (std::abs(newRes - resonance) > 0.001f) { resonance = newRes; updateCoefficients(); }
    }
    void setDrive(float driveAmount) { drive = juce::jlimit(0.0f, 1.0f, driveAmount); }
    void setMix(float wetDry) { mix = juce::jlimit(0.0f, 1.0f, wetDry); }
    void setKeyTrack(float amount) { keyTrack = juce::jlimit(0.0f, 1.0f, amount); }
    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

    // Call this each time a new note plays so the filter can track the pitch
    void setNoteFrequency(float noteFreq) { noteFrequency = noteFreq; updateCoefficients(); }

    // Get cutoff for GUI visualization
    float getCutoff() const { return cutoff; }
    float getResonance() const { return resonance; }

private:
    void updateCoefficients();
    float processSVF(float input);       // State Variable Filter (LP, HP, BP, Notch)
    void processSVFStereo(float inputL, float inputR, float& outL, float& outR);  // Stereo SVF
    float processLadder(float input);    // Moog ladder emulation
    void processLadderStereo(float inputL, float inputR, float& outL, float& outR);  // Stereo Ladder
    float processComb(float input);      // Comb filter
    void processCombStereo(float inputL, float inputR, float& outL, float& outR);  // Stereo Comb
    float processFormant(float input);   // Formant filter
    void processFormantStereo(float inputL, float inputR, float& outL, float& outR);  // Stereo Formant

    // Apply soft saturation (drive)
    float saturate(float input, float amount) const;

    double sampleRate = 44100.0;
    bool enabled = true;
    FilterType filterType = FilterType::LowPass24;

    // Parameters
    float cutoff = 20000.0f;     // Hz
    float resonance = 0.0f;      // 0 to 1
    float drive = 0.0f;          // 0 to 1
    float mix = 1.0f;            // 0 = dry, 1 = wet
    float keyTrack = 0.0f;       // 0 to 1
    float noteFrequency = 440.0f;

    // SVF (State Variable Filter) state — MONO
    float svfIc1eq = 0.0f;
    float svfIc2eq = 0.0f;

    // SVF (State Variable Filter) state — STEREO
    float svfIc1eqL = 0.0f;
    float svfIc2eqL = 0.0f;
    float svfIc1eqR = 0.0f;
    float svfIc2eqR = 0.0f;

    // SVF coefficients
    float svfG = 0.0f;   // frequency coefficient
    float svfR = 0.0f;   // damping (resonance)
    float svfA1 = 0.0f;
    float svfA2 = 0.0f;
    float svfA3 = 0.0f;

    // Ladder filter state (4 cascaded one-pole filters) — MONO
    float ladderState[4] = {};
    float ladderCutoffCoeff = 0.0f;

    // Ladder filter state — STEREO
    float ladderStateL[4] = {};
    float ladderStateR[4] = {};

    // Comb filter state — MONO
    static constexpr int COMB_BUFFER_SIZE = 4096;
    std::array<float, COMB_BUFFER_SIZE> combBuffer{};
    int combWriteIndex = 0;
    float combDelaySamples = 100.0f;

    // Comb filter state — STEREO
    std::array<float, COMB_BUFFER_SIZE> combBufferL{};
    std::array<float, COMB_BUFFER_SIZE> combBufferR{};
    int combWriteIndexL = 0;
    int combWriteIndexR = 0;

    // Formant filter state (3 bandpass filters for formant peaks)
    struct FormantBand
    {
        float s1 = 0.0f, s2 = 0.0f;
        float freq = 1000.0f;
        float bw = 100.0f;
        float gain = 1.0f;
    };
    std::array<FormantBand, 3> formantBands;
};
