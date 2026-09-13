/*
  ==============================================================================
    NoiseGenerator.cpp
  ==============================================================================
*/

#include "NoiseGenerator.h"

void NoiseGenerator::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    digitalHoldLength = static_cast<int>(sampleRate / 4000.0); // ~4kHz sample rate for digital noise
}

void NoiseGenerator::reset()
{
    std::fill(std::begin(pinkState), std::end(pinkState), 0.0f);
    pinkCounter = 0;
    brownState = 0.0f;
    digitalHoldValue = 0.0f;
    digitalHoldCounter = 0;
}

float NoiseGenerator::processSample()
{
    if (!enabled)
        return 0.0f;

    float sample = 0.0f;

    switch (noiseType)
    {
        case NoiseType::White:
        {
            // Simple white noise — random values between -1 and 1
            sample = random.nextFloat() * 2.0f - 1.0f;
            break;
        }

        case NoiseType::Pink:
        {
            // Voss-McCartney algorithm for pink noise (-3dB/octave)
            // Uses a set of random generators updated at different rates
            float white = random.nextFloat() * 2.0f - 1.0f;

            // Update different octave bands at different rates
            int changed = pinkCounter ^ (pinkCounter + 1);
            pinkCounter++;

            for (int i = 0; i < 7; ++i)
            {
                if (changed & (1 << i))
                    pinkState[i] = random.nextFloat() * 2.0f - 1.0f;
            }

            // Sum all bands plus the white noise
            sample = white;
            for (int i = 0; i < 7; ++i)
                sample += pinkState[i];

            sample *= 0.125f; // Normalize (1/8)
            break;
        }

        case NoiseType::Brown:
        {
            // Brownian noise — integrated white noise (-6dB/octave)
            float white = random.nextFloat() * 2.0f - 1.0f;
            brownState += white * 0.02f;
            brownState = juce::jlimit(-1.0f, 1.0f, brownState);
            sample = brownState;
            break;
        }

        case NoiseType::Crackle:
        {
            // Vinyl crackle — sparse, impulsive noise
            float r = random.nextFloat();
            if (r > 0.997f) // Very sparse crackles
                sample = (random.nextFloat() * 2.0f - 1.0f) * 0.8f;
            else if (r > 0.99f) // Slightly more frequent light crackles
                sample = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
            else
                sample = (random.nextFloat() * 2.0f - 1.0f) * 0.01f; // Very quiet base noise
            break;
        }

        case NoiseType::Digital:
        {
            // Sample-and-hold noise — staircase/retro digital character
            digitalHoldCounter++;
            if (digitalHoldCounter >= digitalHoldLength)
            {
                digitalHoldCounter = 0;
                digitalHoldValue = random.nextFloat() * 2.0f - 1.0f;
            }
            sample = digitalHoldValue;
            break;
        }
    }

    return sample * level;
}
