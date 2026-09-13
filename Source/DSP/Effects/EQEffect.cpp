/*
  ==============================================================================
    EQEffect.cpp
  ==============================================================================
*/

#include "EQEffect.h"

void EQEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    updateLowFilter();
    updateMidFilter();
    updateHighFilter();
    reset();
}

void EQEffect::reset()
{
    // Reset all filter states
    lowFilterL.x1 = lowFilterL.x2 = lowFilterL.y1 = lowFilterL.y2 = 0.0f;
    lowFilterR.x1 = lowFilterR.x2 = lowFilterR.y1 = lowFilterR.y2 = 0.0f;

    midFilterL.x1 = midFilterL.x2 = midFilterL.y1 = midFilterL.y2 = 0.0f;
    midFilterR.x1 = midFilterR.x2 = midFilterR.y1 = midFilterR.y2 = 0.0f;

    highFilterL.x1 = highFilterL.x2 = highFilterL.y1 = highFilterL.y2 = 0.0f;
    highFilterR.x1 = highFilterR.x2 = highFilterR.y1 = highFilterR.y2 = 0.0f;
}

void EQEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        // Process left channel through all three bands
        float outL = leftChannel[i];
        outL = processBiquad(outL, lowFilterL);
        outL = processBiquad(outL, midFilterL);
        outL = processBiquad(outL, highFilterL);
        leftChannel[i] = outL;

        // Process right channel through all three bands
        float outR = rightChannel[i];
        outR = processBiquad(outR, lowFilterR);
        outR = processBiquad(outR, midFilterR);
        outR = processBiquad(outR, highFilterR);
        rightChannel[i] = outR;
    }
}

float EQEffect::processBiquad(float input, BiquadFilter& filter)
{
    // Direct Form II biquad difference equation:
    // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    // where a1 and a2 are denominator coefficients (normalized by a0)

    float output = filter.b0 * input + filter.b1 * filter.x1 + filter.b2 * filter.x2
                 - filter.a1 * filter.y1 - filter.a2 * filter.y2;

    // Shift input history
    filter.x2 = filter.x1;
    filter.x1 = input;

    // Shift output history
    filter.y2 = filter.y1;
    filter.y1 = output;

    return output;
}

void EQEffect::updateLowFilter()
{
    // Low shelf filter design
    // A shelf filter boosts or cuts frequencies below the cutoff

    float w0 = juce::MathConstants<float>::twoPi * lowFreq / static_cast<float>(sampleRate);
    float sinW0 = std::sin(w0);
    float cosW0 = std::cos(w0);

    // Gain in linear scale (dB to linear: 10^(dB/20))
    float A = std::pow(10.0f, lowGain / 40.0f);  // Divide by 40 for shelf
    float alpha = sinW0 / (2.0f * lowQ);

    // Shelf filter coefficients
    float a0 = (A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * std::sqrt(A) * alpha;

    lowFilterL.b0 = A * ((A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * std::sqrt(A) * alpha) / a0;
    lowFilterL.b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW0) / a0;
    lowFilterL.b2 = A * ((A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * std::sqrt(A) * alpha) / a0;
    lowFilterL.a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosW0) / a0;
    lowFilterL.a2 = ((A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * std::sqrt(A) * alpha) / a0;

    // Copy to right channel
    lowFilterR = lowFilterL;
}

void EQEffect::updateMidFilter()
{
    // Mid peaking filter design
    // A peaking filter boosts or cuts frequencies around the center frequency

    float w0 = juce::MathConstants<float>::twoPi * midFreq / static_cast<float>(sampleRate);
    float sinW0 = std::sin(w0);
    float cosW0 = std::cos(w0);

    // Gain in linear scale (dB to linear: 10^(dB/20))
    float A = std::pow(10.0f, midGain / 40.0f);
    float alpha = sinW0 / (2.0f * midQ);

    // Peak filter coefficients
    float a0 = 1.0f + alpha / A;

    midFilterL.b0 = (1.0f + alpha * A) / a0;
    midFilterL.b1 = -2.0f * cosW0 / a0;
    midFilterL.b2 = (1.0f - alpha * A) / a0;
    midFilterL.a1 = -2.0f * cosW0 / a0;
    midFilterL.a2 = (1.0f - alpha / A) / a0;

    // Copy to right channel
    midFilterR = midFilterL;
}

void EQEffect::updateHighFilter()
{
    // High shelf filter design
    // A shelf filter boosts or cuts frequencies above the cutoff

    float w0 = juce::MathConstants<float>::twoPi * highFreq / static_cast<float>(sampleRate);
    float sinW0 = std::sin(w0);
    float cosW0 = std::cos(w0);

    // Gain in linear scale (dB to linear: 10^(dB/20))
    float A = std::pow(10.0f, highGain / 40.0f);
    float alpha = sinW0 / (2.0f * highQ);

    // Shelf filter coefficients (high shelf)
    float a0 = (A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * std::sqrt(A) * alpha;

    highFilterL.b0 = A * ((A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * std::sqrt(A) * alpha) / a0;
    highFilterL.b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0) / a0;
    highFilterL.b2 = A * ((A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * std::sqrt(A) * alpha) / a0;
    highFilterL.a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0) / a0;
    highFilterL.a2 = ((A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * std::sqrt(A) * alpha) / a0;

    // Copy to right channel
    highFilterR = highFilterL;
}
