/*
  ==============================================================================
    DistortionEffect.h

    Multi-mode distortion / waveshaping effect.

    Distortion Types:
    - Soft Clip (Tube): Warm, musical saturation like a tube amp
    - Hard Clip: Aggressive digital clipping
    - Foldback: Wavefolding — creates complex harmonics
    - Bitcrush: Reduces bit depth for lo-fi / retro sounds
    - Rectify: Full-wave rectification — octave-up effect
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

enum class DistortionType
{
    SoftClip,   // Tube-like warmth (tanh saturation)
    HardClip,   // Aggressive digital clipping
    Foldback,   // Wavefolding — wraps signal back on itself
    Bitcrush,   // Bit depth reduction
    Rectify     // Full-wave rectification
};

class DistortionEffect
{
public:
    DistortionEffect() = default;
    ~DistortionEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setType(DistortionType t) { type = t; }
    void setDrive(float d)   { drive = juce::jlimit(0.0f, 1.0f, d); }
    void setMix(float m)     { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setTone(float t)    { tone = juce::jlimit(0.0f, 1.0f, t); updateFilter(); }
    void setEnabled(bool e)  { enabled = e; }
    bool isEnabled() const   { return enabled; }

private:
    float processDistortion(float input) const;
    void updateFilter();

    double sampleRate = 44100.0;
    bool enabled = false;

    DistortionType type = DistortionType::SoftClip;
    float drive = 0.5f;
    float mix = 1.0f;
    float tone = 0.5f;

    // Post-distortion tone filter state
    float toneFilterStateL = 0.0f;
    float toneFilterStateR = 0.0f;
    float toneFilterCoeff = 0.5f;
};
