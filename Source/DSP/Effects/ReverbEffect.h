/*
  ==============================================================================
    ReverbEffect.h

    Algorithmic reverb — simulates the sound of a physical space.
    Uses a Schroeder/Moorer reverb topology with comb and allpass filters.

    Parameters:
    - Size: How large the virtual room is (affects reverb tail length)
    - Damping: How quickly high frequencies decay (bright vs dark reverb)
    - Width: Stereo spread of the reverb (mono to wide)
    - Mix: Dry/wet blend
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

class ReverbEffect
{
public:
    ReverbEffect() = default;
    ~ReverbEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setSize(float s)    { size = juce::jlimit(0.0f, 1.0f, s); updateParams(); }
    void setDamping(float d) { damping = juce::jlimit(0.0f, 1.0f, d); updateParams(); }
    void setWidth(float w)   { width = juce::jlimit(0.0f, 1.0f, w); }
    void setMix(float m)     { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setEnabled(bool e)  { enabled = e; }
    bool isEnabled() const   { return enabled; }

private:
    void updateParams();

    // We use JUCE's built-in reverb which implements Freeverb (Schroeder topology)
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    bool enabled = false;
    float size = 0.5f;
    float damping = 0.5f;
    float width = 1.0f;
    float mix = 0.3f;

    double sampleRate = 44100.0;
};
