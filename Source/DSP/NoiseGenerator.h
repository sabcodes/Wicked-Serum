/*
  ==============================================================================
    NoiseGenerator.h

    Noise oscillator — adds texture and character to sounds.
    Serum ships with 200+ noise samples; we provide algorithmic noise types
    that cover the most useful categories:

    - White noise: Equal energy at all frequencies (hi-hats, risers)
    - Pink noise: -3dB/octave rolloff (warmer, more natural)
    - Brown noise: -6dB/octave rolloff (rumble, wind)
    - Crackle: Vinyl/tape noise texture
    - Digital: Sample-and-hold style noise
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum class NoiseType
{
    White,
    Pink,
    Brown,
    Crackle,
    Digital
};

class NoiseGenerator
{
public:
    NoiseGenerator() = default;
    ~NoiseGenerator() = default;

    void prepare(double sampleRate);
    float processSample();
    void reset();

    void setNoiseType(NoiseType type) { noiseType = type; }
    void setLevel(float newLevel) { level = juce::jlimit(0.0f, 1.0f, newLevel); }
    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

private:
    double sampleRate = 44100.0;
    bool enabled = false;  // Noise is off by default
    juce::Random random;

    NoiseType noiseType = NoiseType::White;
    float level = 0.2f;

    // Pink noise filter state (Voss-McCartney algorithm)
    float pinkState[7] = {};
    int pinkCounter = 0;

    // Brown noise state
    float brownState = 0.0f;

    // Digital noise state
    float digitalHoldValue = 0.0f;
    int digitalHoldCounter = 0;
    int digitalHoldLength = 100;
};
