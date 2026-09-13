/*
  ==============================================================================
    PhaserEffect.cpp
  ==============================================================================
*/

#include "PhaserEffect.h"

void PhaserEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    reset();
}

void PhaserEffect::reset()
{
    lfoPhase = 0.0f;
    feedbackL = 0.0f;
    feedbackR = 0.0f;

    // Reset all allpass filter states
    for (int i = 0; i < MAX_STAGES; ++i)
    {
        stagesL[i].state = 0.0f;
        stagesR[i].state = 0.0f;
    }
}

void PhaserEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    // LFO increment per sample
    float lfoIncrement = rate / static_cast<float>(sampleRate);

    // Allpass coefficient range — controls how much the filters sweep
    // Min: ~200 Hz, Max: ~2000 Hz (approximate allpass center frequencies)
    float minCoeff = 0.2f;
    float maxCoeff = 0.8f;

    for (int i = 0; i < numSamples; ++i)
    {
        // Generate LFO — triangular wave from 0 to 1
        float lfoValue = std::abs(lfoPhase * 2.0f - 1.0f);

        // Sweep the allpass coefficient based on depth and LFO
        float coeff = minCoeff + lfoValue * depth * (maxCoeff - minCoeff);

        // ===== Left channel =====
        float inL = leftChannel[i] + feedbackL * feedback;

        // Process through cascaded allpass filters
        float outL = inL;
        for (int stage = 0; stage < stages; ++stage)
        {
            outL = processAllpass(outL, stagesL[stage], coeff);
        }

        // Store feedback for next sample
        feedbackL = outL;

        // Apply dry/wet mix
        leftChannel[i] = inL * (1.0f - mix) + outL * mix;

        // ===== Right channel =====
        float inR = rightChannel[i] + feedbackR * feedback;

        // Process through cascaded allpass filters (with same coefficient)
        float outR = inR;
        for (int stage = 0; stage < stages; ++stage)
        {
            outR = processAllpass(outR, stagesR[stage], coeff);
        }

        // Store feedback for next sample
        feedbackR = outR;

        // Apply dry/wet mix
        rightChannel[i] = inR * (1.0f - mix) + outR * mix;

        // Advance LFO phase
        lfoPhase += lfoIncrement;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
    }
}

float PhaserEffect::processAllpass(float input, AllpassStage& stage, float coeff)
{
    // Allpass filter formula:
    // y[n] = -x[n] + coeff * (x[n] + y[n-1])
    // where coeff is typically 0.0 to 0.99 (higher = narrower center frequency)

    float output = -input + coeff * (input + stage.state);
    stage.state = output;
    return output;
}
