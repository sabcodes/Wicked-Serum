/*
  ==============================================================================
    FlangerEffect.h

    Stereo flanger — creates a jet-plane-like "whooshing" effect by mixing the
    dry signal with a delayed copy that has a very short, modulated delay time.
    Similar to a phaser but uses delay lines instead of allpass filters.

    Parameters:
    - Rate: Speed of the LFO modulation (Hz)
    - Depth: How much the delay time varies (0-1, affects intensity)
    - Feedback: Amount of output fed back to input (0 = subtle, 0.9 = extreme)
    - Mix: Dry/wet blend

    The delay range is fixed at 0.1ms to 10ms (typical flanger timing).
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

class FlangerEffect
{
public:
    FlangerEffect() = default;
    ~FlangerEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setRate(float hz)       { rate = juce::jlimit(0.1f, 10.0f, hz); }
    void setDepth(float d)       { depth = juce::jlimit(0.0f, 1.0f, d); }
    void setFeedback(float fb)   { feedback = juce::jlimit(0.0f, 0.9f, fb); }
    void setMix(float m)         { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setEnabled(bool e)      { enabled = e; }
    bool isEnabled() const       { return enabled; }

private:
    double sampleRate = 44100.0;
    bool enabled = false;

    float rate = 0.5f;       // Hz
    float depth = 0.5f;      // 0-1, affects modulation depth
    float feedback = 0.5f;   // 0-0.9, feedback amount
    float mix = 0.5f;        // Dry/wet mix

    // LFO phase
    float lfoPhase = 0.0f;

    // Delay buffers (stereo) — for 10ms at 96kHz = ~960 samples
    static constexpr int FLANGER_BUFFER_SIZE = 2048;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    int writeIndex = 0;

    // Min and max delay in samples (0.1ms to 10ms)
    // At 44.1kHz: 0.1ms = 4.4 samples, 10ms = 441 samples
    // We'll compute these based on sample rate
    float minDelaySamples = 0.0f;
    float maxDelaySamples = 0.0f;
};
