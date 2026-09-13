/*
  ==============================================================================
    DelayEffect.h

    Stereo delay with tempo sync, ping-pong mode, and feedback filtering.

    Parameters:
    - Time: Delay time in ms (or synced to tempo)
    - Feedback: How much signal feeds back (0 = single echo, 0.9 = long trail)
    - Mix: Dry/wet blend
    - PingPong: Alternates echoes between left and right channels
    - HighCut: Darkens the echoes over time (like tape delay)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class DelayEffect
{
public:
    DelayEffect() = default;
    ~DelayEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    void setDelayTime(float ms)       { delayTimeMs = juce::jlimit(1.0f, 2000.0f, ms); updateDelay(); }
    void setFeedback(float fb)        { feedback = juce::jlimit(0.0f, 0.95f, fb); }
    void setMix(float m)              { mix = juce::jlimit(0.0f, 1.0f, m); }
    void setPingPong(bool pp)         { pingPong = pp; }
    void setHighCut(float freqHz)     { highCutFreq = juce::jlimit(200.0f, 20000.0f, freqHz); updateFilter(); }
    void setTempo(double bpm)         { tempoBPM = bpm; }
    void setEnabled(bool e)           { enabled = e; }
    bool isEnabled() const            { return enabled; }

private:
    void updateDelay();
    void updateFilter();

    double sampleRate = 44100.0;
    double tempoBPM = 120.0;
    bool enabled = false;
    bool pingPong = false;

    float delayTimeMs = 375.0f;  // ~1/8 note at 120bpm
    float feedback = 0.4f;
    float mix = 0.3f;
    float highCutFreq = 8000.0f;

    // Delay buffers (stereo)
    static constexpr int MAX_DELAY_SAMPLES = 192000; // ~2 seconds at 96kHz
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int writeIndex = 0;
    int delaySamples = 0;

    // Feedback filter state (simple one-pole lowpass)
    float filterStateL = 0.0f;
    float filterStateR = 0.0f;
    float filterCoeff = 0.5f;
};
