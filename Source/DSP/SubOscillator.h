/*
  ==============================================================================
    SubOscillator.h

    A simple sub-oscillator that plays one or two octaves below the main pitch.
    Serum's sub oscillator provides low-end weight to patches — essential for
    bass sounds, leads, and adding body to pads.

    Unlike the wavetable oscillators, the sub uses simple waveforms (sine,
    triangle, saw, square) for clean, predictable low-end.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

enum class SubOscShape
{
    Sine,
    Triangle,
    Saw,
    Square,
    Pulse25,    // 25% pulse width — classic sub sound
    Pulse12     // 12.5% pulse width — thinner sub
};

class SubOscillator
{
public:
    SubOscillator() = default;
    ~SubOscillator() = default;

    void prepare(double sampleRate);
    float processSample(float frequency);
    void reset();

    // --- Parameter Setters ---
    void setShape(SubOscShape newShape) { shape = newShape; }
    void setOctave(int oct) { octaveShift = juce::jlimit(-2, 0, oct); } // -2, -1, or 0
    void setLevel(float newLevel) { level = juce::jlimit(0.0f, 1.0f, newLevel); }
    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

private:
    // PolyBLEP anti-aliasing
    float polyBLEP(float phase);
    float lastPhase = 0.0f;

    double sampleRate = 44100.0;
    bool enabled = true;
    float phase = 0.0f;

    SubOscShape shape = SubOscShape::Sine;
    int octaveShift = -1;  // Default: one octave below
    float level = 0.5f;
};
