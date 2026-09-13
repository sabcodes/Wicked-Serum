/*
  ==============================================================================
    ChorusEffect.h

    Stereo chorus — creates a lush, wide, "doubled" sound by mixing the dry
    signal with slightly detuned and delayed copies modulated by LFOs.

    Parameters:
    - Rate: Speed of the modulation LFO
    - Depth: How much the delay time varies (more = more detuning)
    - Mix: Dry/wet blend
    - Voices: Number of chorus voices (1-4)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

class ChorusEffect
{
public:
    ChorusEffect() = default;
    ~ChorusEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setRate(float hz)    { rate = juce::jlimit(0.1f, 10.0f, hz); }
    void setDepth(float d)    { depth = juce::jlimit(0.0f, 1.0f, d); }
    void setMix(float m)      { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setVoices(int v)     { voices = juce::jlimit(1, 4, v); }
    void setEnabled(bool e)   { enabled = e; }
    bool isEnabled() const    { return enabled; }

private:
    double sampleRate = 44100.0;
    bool enabled = false;

    float rate = 0.8f;    // Hz
    float depth = 0.5f;
    float mix = 0.5f;
    int voices = 2;

    // Delay buffer for chorus (short delays ~1-30ms)
    static constexpr int CHORUS_BUFFER_SIZE = 4096;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    int writeIndex = 0;

    // LFO phases for each chorus voice
    float lfoPhases[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
};
