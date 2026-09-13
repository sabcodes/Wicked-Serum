/*
  ==============================================================================
    PhaserEffect.h

    Stereo phaser — creates a sweeping comb-filter effect by mixing the dry
    signal with a copy that's been processed through cascaded allpass filters.
    The allpass filter frequencies are modulated by an LFO.

    Parameters:
    - Rate: Speed of the LFO modulation (Hz)
    - Depth: How much the allpass filters sweep (0 = subtle, 1 = extreme)
    - Feedback: Amount of output fed back to input (creates more intense effect)
    - Stages: Number of cascaded allpass filters (4-12, more = richer effect)
    - Mix: Dry/wet blend
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>

class PhaserEffect
{
public:
    PhaserEffect() = default;
    ~PhaserEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setRate(float hz)       { rate = juce::jlimit(0.1f, 10.0f, hz); }
    void setDepth(float d)       { depth = juce::jlimit(0.0f, 1.0f, d); }
    void setFeedback(float fb)   { feedback = juce::jlimit(0.0f, 0.9f, fb); }
    void setStages(int s)        { stages = juce::jlimit(4, 12, s); reset(); }
    void setMix(float m)         { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setEnabled(bool e)      { enabled = e; }
    bool isEnabled() const       { return enabled; }

private:
    // Single allpass filter state for a channel
    struct AllpassStage
    {
        float state = 0.0f;
    };

    // Process one sample through an allpass filter
    float processAllpass(float input, AllpassStage& stage, float coeff);

    double sampleRate = 44100.0;
    bool enabled = false;

    float rate = 0.5f;       // Hz
    float depth = 0.5f;      // 0-1
    float feedback = 0.5f;   // 0-0.9
    int stages = 6;          // 4-12 cascaded allpass filters
    float mix = 0.5f;        // Dry/wet mix

    // LFO phase
    float lfoPhase = 0.0f;

    // Allpass filter stages for left and right channels
    static constexpr int MAX_STAGES = 12;
    std::array<AllpassStage, MAX_STAGES> stagesL;
    std::array<AllpassStage, MAX_STAGES> stagesR;

    // Feedback state (one sample delay)
    float feedbackL = 0.0f;
    float feedbackR = 0.0f;
};
