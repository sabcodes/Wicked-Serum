/*
  ==============================================================================
    ReverbEffect.cpp
  ==============================================================================
*/

#include "ReverbEffect.h"

void ReverbEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    reverb.setSampleRate(sampleRate);
    updateParams();
    reverb.reset();
}

void ReverbEffect::reset()
{
    reverb.reset();
}

void ReverbEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    // Store dry signal for mix
    std::vector<float> dryLeft(numSamples), dryRight(numSamples);
    std::copy(leftChannel, leftChannel + numSamples, dryLeft.begin());
    std::copy(rightChannel, rightChannel + numSamples, dryRight.begin());

    // Process reverb (in-place)
    reverb.processStereo(leftChannel, rightChannel, numSamples);

    // Apply dry/wet mix
    for (int i = 0; i < numSamples; ++i)
    {
        leftChannel[i]  = dryLeft[i] * (1.0f - mix) + leftChannel[i] * mix;
        rightChannel[i] = dryRight[i] * (1.0f - mix) + rightChannel[i] * mix;
    }
}

void ReverbEffect::updateParams()
{
    reverbParams.roomSize   = size;
    reverbParams.damping    = damping;
    reverbParams.width      = width;
    reverbParams.wetLevel   = 1.0f;  // We handle mix ourselves
    reverbParams.dryLevel   = 0.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);
}
