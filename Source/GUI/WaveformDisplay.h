/*
  ==============================================================================
    WaveformDisplay.h

    Real-time waveform visualization — the signature visual element of Serum.
    Shows the current wavetable frame with a glowing, animated display.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../DSP/WavetableOscillator.h"
#include "KnobLookAndFeel.h"

class WaveformDisplay : public juce::Component, public juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override = default;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    // Update the waveform data to display
    void setWaveformData(const std::array<float, WAVETABLE_SIZE>& data);

    // Set the accent color for this display
    void setAccentColour(juce::Colour colour) { accentColour = colour; }

    // Set label
    void setLabel(const juce::String& text) { label = text; }

private:
    std::array<float, WAVETABLE_SIZE> waveformData{};
    juce::Colour accentColour = KnobLookAndFeel::accentGreen;
    juce::String label = "OSC A";
    float animationPhase = 0.0f;
};
