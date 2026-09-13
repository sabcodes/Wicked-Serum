/*
  ==============================================================================
    FlangerEffect.cpp
  ==============================================================================
*/

#include "FlangerEffect.h"

void FlangerEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;

    // Initialize delay buffers
    bufferL.resize(FLANGER_BUFFER_SIZE, 0.0f);
    bufferR.resize(FLANGER_BUFFER_SIZE, 0.0f);

    // Calculate min and max delay times in samples
    // Min: 0.1ms, Max: 10ms
    minDelaySamples = static_cast<float>(sampleRate) * 0.0001f;   // 0.1ms
    maxDelaySamples = static_cast<float>(sampleRate) * 0.01f;     // 10ms

    reset();
}

void FlangerEffect::reset()
{
    lfoPhase = 0.0f;
    writeIndex = 0;
    std::fill(bufferL.begin(), bufferL.end(), 0.0f);
    std::fill(bufferR.begin(), bufferR.end(), 0.0f);
}

void FlangerEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    // LFO increment per sample
    float lfoIncrement = rate / static_cast<float>(sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        // Generate LFO — sine wave from 0 to 1
        float lfoValue = (std::sin(lfoPhase * juce::MathConstants<float>::twoPi) + 1.0f) * 0.5f;

        // Modulate delay time: ranges from minDelay to maxDelay
        float modDepthRange = (maxDelaySamples - minDelaySamples) * depth;
        float delaySamples = minDelaySamples + lfoValue * modDepthRange;

        // ===== Left channel =====
        // Write to buffer
        bufferL[writeIndex] = leftChannel[i];

        // Calculate read position (subtract delay from write position)
        float readPos = static_cast<float>(writeIndex) - delaySamples;
        if (readPos < 0.0f)
            readPos += FLANGER_BUFFER_SIZE;

        // Read with linear interpolation
        int idx0 = static_cast<int>(readPos) % FLANGER_BUFFER_SIZE;
        int idx1 = (idx0 + 1) % FLANGER_BUFFER_SIZE;
        float frac = readPos - std::floor(readPos);

        float delayedL = bufferL[idx0] * (1.0f - frac) + bufferL[idx1] * frac;

        // Apply feedback and mix
        float outL = leftChannel[i] + delayedL * feedback;
        leftChannel[i] = leftChannel[i] * (1.0f - mix) + delayedL * mix;

        // ===== Right channel =====
        // Write to buffer
        bufferR[writeIndex] = rightChannel[i];

        // Calculate read position (same modulation for stereo link)
        readPos = static_cast<float>(writeIndex) - delaySamples;
        if (readPos < 0.0f)
            readPos += FLANGER_BUFFER_SIZE;

        // Read with linear interpolation
        idx0 = static_cast<int>(readPos) % FLANGER_BUFFER_SIZE;
        idx1 = (idx0 + 1) % FLANGER_BUFFER_SIZE;
        frac = readPos - std::floor(readPos);

        float delayedR = bufferR[idx0] * (1.0f - frac) + bufferR[idx1] * frac;

        // Apply feedback and mix
        float outR = rightChannel[i] + delayedR * feedback;
        rightChannel[i] = rightChannel[i] * (1.0f - mix) + delayedR * mix;

        // Advance write index
        writeIndex = (writeIndex + 1) % FLANGER_BUFFER_SIZE;

        // Advance LFO phase
        lfoPhase += lfoIncrement;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
    }
}
