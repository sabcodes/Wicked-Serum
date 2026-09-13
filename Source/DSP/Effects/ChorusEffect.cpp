/*
  ==============================================================================
    ChorusEffect.cpp
  ==============================================================================
*/

#include "ChorusEffect.h"

void ChorusEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    bufferL.resize(CHORUS_BUFFER_SIZE, 0.0f);
    bufferR.resize(CHORUS_BUFFER_SIZE, 0.0f);
    reset();
}

void ChorusEffect::reset()
{
    std::fill(bufferL.begin(), bufferL.end(), 0.0f);
    std::fill(bufferR.begin(), bufferR.end(), 0.0f);
    writeIndex = 0;
    lfoPhases[0] = 0.0f;
    lfoPhases[1] = 0.25f;
    lfoPhases[2] = 0.5f;
    lfoPhases[3] = 0.75f;
}

void ChorusEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    float lfoIncrement = rate / static_cast<float>(sampleRate);

    // Base delay in samples (~7ms center)
    float baseDelay = static_cast<float>(sampleRate) * 0.007f;
    // Modulation depth in samples (~3ms max sweep)
    float modDepth = depth * static_cast<float>(sampleRate) * 0.003f;

    for (int i = 0; i < numSamples; ++i)
    {
        // Write dry signal to delay buffer
        bufferL[writeIndex] = leftChannel[i];
        bufferR[writeIndex] = rightChannel[i];

        float chorusL = 0.0f;
        float chorusR = 0.0f;

        // Process each chorus voice
        for (int v = 0; v < voices; ++v)
        {
            // Each voice has its own LFO phase for spread
            float lfoValue = std::sin(lfoPhases[v] * juce::MathConstants<float>::twoPi);

            // Calculate modulated delay time
            float delaySamples = baseDelay + lfoValue * modDepth;

            // Read from delay buffer with linear interpolation
            float readPos = static_cast<float>(writeIndex) - delaySamples;
            if (readPos < 0.0f) readPos += CHORUS_BUFFER_SIZE;

            int readIdx = static_cast<int>(readPos);
            float frac = readPos - readIdx;

            int idx0 = readIdx % CHORUS_BUFFER_SIZE;
            int idx1 = (readIdx + 1) % CHORUS_BUFFER_SIZE;

            float delayedL = bufferL[idx0] * (1.0f - frac) + bufferL[idx1] * frac;
            float delayedR = bufferR[idx0] * (1.0f - frac) + bufferR[idx1] * frac;

            // Pan odd voices left, even voices right for stereo spread
            if (v % 2 == 0)
            {
                chorusL += delayedL;
                chorusR += delayedR * 0.7f;
            }
            else
            {
                chorusL += delayedL * 0.7f;
                chorusR += delayedR;
            }

            // Advance this voice's LFO
            lfoPhases[v] += lfoIncrement;
            if (lfoPhases[v] >= 1.0f) lfoPhases[v] -= 1.0f;
        }

        // Normalize by number of voices
        float norm = 1.0f / static_cast<float>(voices);
        chorusL *= norm;
        chorusR *= norm;

        // Apply dry/wet mix
        leftChannel[i]  = leftChannel[i] * (1.0f - mix) + chorusL * mix;
        rightChannel[i] = rightChannel[i] * (1.0f - mix) + chorusR * mix;

        writeIndex = (writeIndex + 1) % CHORUS_BUFFER_SIZE;
    }
}
