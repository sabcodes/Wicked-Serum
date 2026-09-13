/*
  ==============================================================================
    DistortionEffect.cpp
  ==============================================================================
*/

#include "DistortionEffect.h"

void DistortionEffect::prepare(double newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    updateFilter();
    reset();
}

void DistortionEffect::reset()
{
    toneFilterStateL = 0.0f;
    toneFilterStateR = 0.0f;
}

void DistortionEffect::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    if (!enabled)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = leftChannel[i];
        float dryR = rightChannel[i];

        float wetL = processDistortion(dryL);
        float wetR = processDistortion(dryR);

        // Apply tone filter (simple lowpass to tame harsh highs)
        toneFilterStateL += toneFilterCoeff * (wetL - toneFilterStateL);
        toneFilterStateR += toneFilterCoeff * (wetR - toneFilterStateR);

        // Blend between filtered (dark) and unfiltered (bright) based on tone
        wetL = toneFilterStateL * (1.0f - tone) + wetL * tone;
        wetR = toneFilterStateR * (1.0f - tone) + wetR * tone;

        // Apply dry/wet mix
        leftChannel[i]  = dryL * (1.0f - mix) + wetL * mix;
        rightChannel[i] = dryR * (1.0f - mix) + wetR * mix;
    }
}

float DistortionEffect::processDistortion(float input) const
{
    // Apply drive — boost the signal before distortion
    float driveGain = 1.0f + drive * 20.0f;  // 1x to 21x gain
    float driven = input * driveGain;

    switch (type)
    {
        case DistortionType::SoftClip:
        {
            // Tanh soft clipping — warm, tube-like saturation
            // The higher the drive, the more harmonics are added
            return std::tanh(driven) / std::tanh(driveGain);
        }

        case DistortionType::HardClip:
        {
            // Hard digital clipping — aggressive, buzzy
            return juce::jlimit(-1.0f, 1.0f, driven);
        }

        case DistortionType::Foldback:
        {
            // Wavefolding — signal folds back when it exceeds threshold
            // Creates complex, evolving harmonics
            float threshold = 1.0f;
            float folded = driven;

            // Fold multiple times for complex harmonics
            for (int j = 0; j < 4; ++j)
            {
                if (folded > threshold)
                    folded = 2.0f * threshold - folded;
                else if (folded < -threshold)
                    folded = -2.0f * threshold - folded;
            }

            return folded;
        }

        case DistortionType::Bitcrush:
        {
            // Bit depth reduction — quantizes amplitude levels
            float bits = 16.0f - drive * 14.0f; // 16 bits down to 2 bits
            float levels = std::pow(2.0f, bits);
            return std::round(input * levels) / levels;
        }

        case DistortionType::Rectify:
        {
            // Full-wave rectification — flips negative to positive
            // Creates an octave-up effect
            float rectified = std::abs(driven);
            return rectified / std::max(1.0f, driveGain) * 2.0f - 1.0f;
        }

        default:
            return input;
    }
}

void DistortionEffect::updateFilter()
{
    // Tone filter — lowpass at frequency determined by tone knob
    float freq = 500.0f + tone * 19500.0f; // 500 Hz to 20 kHz
    float w = juce::MathConstants<float>::twoPi * freq / static_cast<float>(sampleRate);
    toneFilterCoeff = w / (1.0f + w);
}
