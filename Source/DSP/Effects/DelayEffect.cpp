/*
  ==============================================================================
    DelayEffect.cpp
  ==============================================================================
*/

#include "DelayEffect.h"

void DelayEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    delayBufferL.resize(MAX_DELAY_SAMPLES, 0.0f);
    delayBufferR.resize(MAX_DELAY_SAMPLES, 0.0f);
    updateDelay();
    updateFilter();
    reset();
}

void DelayEffect::reset()
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);
    writeIndex = 0;
    filterStateL = 0.0f;
    filterStateR = 0.0f;
}

void DelayEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        // Read from delay buffer
        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0)
            readIndex += MAX_DELAY_SAMPLES;

        float delayedL = delayBufferL[readIndex];
        float delayedR = delayBufferR[readIndex];

        // Apply feedback filter (darkens echoes over time like tape delay)
        filterStateL += filterCoeff * (delayedL - filterStateL);
        filterStateR += filterCoeff * (delayedR - filterStateR);

        float filteredL = filterStateL;
        float filteredR = filterStateR;

        // Write to delay buffer with feedback
        if (pingPong)
        {
            // Ping-pong: left feeds into right's delay, right feeds into left's
            delayBufferL[writeIndex] = leftChannel[i] + filteredR * feedback;
            delayBufferR[writeIndex] = rightChannel[i] + filteredL * feedback;
        }
        else
        {
            // Normal stereo delay
            delayBufferL[writeIndex] = leftChannel[i] + filteredL * feedback;
            delayBufferR[writeIndex] = rightChannel[i] + filteredR * feedback;
        }

        // Apply dry/wet mix
        leftChannel[i]  = leftChannel[i] * (1.0f - mix) + filteredL * mix;
        rightChannel[i] = rightChannel[i] * (1.0f - mix) + filteredR * mix;

        // Advance write position
        writeIndex = (writeIndex + 1) % MAX_DELAY_SAMPLES;
    }
}

void DelayEffect::updateDelay()
{
    delaySamples = static_cast<int>(delayTimeMs * sampleRate / 1000.0);
    delaySamples = juce::jlimit(1, MAX_DELAY_SAMPLES - 1, delaySamples);
}

void DelayEffect::updateFilter()
{
    // One-pole lowpass coefficient
    float w = juce::MathConstants<float>::twoPi * highCutFreq / static_cast<float>(sampleRate);
    filterCoeff = w / (1.0f + w);
}
