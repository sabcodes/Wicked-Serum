/*
  ==============================================================================
    EQEffect.h

    3-band parametric equalizer using biquad filters.
    - Low band:  Shelf filter (boosts/cuts lows below cutoff frequency)
    - Mid band:  Peak filter (boosts/cuts mids around center frequency)
    - High band: Shelf filter (boosts/cuts highs above cutoff frequency)

    Each band has:
    - Frequency: Center/cutoff frequency (Hz)
    - Gain: Boost/cut amount (dB, typically ±12dB)
    - Q: Resonance / bandwidth (higher Q = narrower/sharper)

    Uses standard biquad filter design for efficient, stable processing.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

class EQEffect
{
public:
    EQEffect() = default;
    ~EQEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    // Low band (shelf)
    void setLowFreq(float hz)    { lowFreq = juce::jlimit(20.0f, 500.0f, hz); updateLowFilter(); }
    void setLowGain(float db)    { lowGain = juce::jlimit(-24.0f, 24.0f, db); updateLowFilter(); }
    void setLowQ(float q)        { lowQ = juce::jlimit(0.1f, 10.0f, q); updateLowFilter(); }

    // Mid band (peak)
    void setMidFreq(float hz)    { midFreq = juce::jlimit(200.0f, 5000.0f, hz); updateMidFilter(); }
    void setMidGain(float db)    { midGain = juce::jlimit(-24.0f, 24.0f, db); updateMidFilter(); }
    void setMidQ(float q)        { midQ = juce::jlimit(0.1f, 10.0f, q); updateMidFilter(); }

    // High band (shelf)
    void setHighFreq(float hz)   { highFreq = juce::jlimit(2000.0f, 20000.0f, hz); updateHighFilter(); }
    void setHighGain(float db)   { highGain = juce::jlimit(-24.0f, 24.0f, db); updateHighFilter(); }
    void setHighQ(float q)       { highQ = juce::jlimit(0.1f, 10.0f, q); updateHighFilter(); }

    void setEnabled(bool e)      { enabled = e; }
    bool isEnabled() const       { return enabled; }

private:
    // Biquad filter state and coefficients
    struct BiquadFilter
    {
        // Coefficients (time-invariant part)
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;

        // State variables (time-varying part)
        float x1 = 0.0f, x2 = 0.0f;  // Input history
        float y1 = 0.0f, y2 = 0.0f;  // Output history
    };

    // Process one sample through a biquad filter
    float processBiquad(float input, BiquadFilter& filter);

    void updateLowFilter();
    void updateMidFilter();
    void updateHighFilter();

    double sampleRate = 44100.0;
    bool enabled = false;

    // Low band (shelf)
    float lowFreq = 100.0f;
    float lowGain = 0.0f;  // dB
    float lowQ = 0.707f;
    BiquadFilter lowFilterL, lowFilterR;

    // Mid band (peak)
    float midFreq = 1000.0f;
    float midGain = 0.0f;  // dB
    float midQ = 0.707f;
    BiquadFilter midFilterL, midFilterR;

    // High band (shelf)
    float highFreq = 10000.0f;
    float highGain = 0.0f;  // dB
    float highQ = 0.707f;
    BiquadFilter highFilterL, highFilterR;
};
