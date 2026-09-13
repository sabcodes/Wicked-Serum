/*
  ==============================================================================
    WaveformDisplay.cpp
  ==============================================================================
*/

#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
    // Generate a default sine wave for initial display
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        waveformData[i] = std::sin(phase * juce::MathConstants<float>::twoPi);
    }

    startTimerHz(15); // Slower animation timer (glow pulse only)
}

void WaveformDisplay::setWaveformData(const std::array<float, WAVETABLE_SIZE>& data)
{
    waveformData = data;
    repaint();
}

void WaveformDisplay::timerCallback()
{
    animationPhase += 0.02f;
    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Dark background with subtle gradient
    g.setGradientFill(juce::ColourGradient(
        KnobLookAndFeel::backgroundDark, bounds.getX(), bounds.getY(),
        KnobLookAndFeel::backgroundMid, bounds.getX(), bounds.getBottom(),
        false));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Border
    g.setColour(KnobLookAndFeel::backgroundLight);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Draw grid lines
    g.setColour(KnobLookAndFeel::knobTrack.withAlpha(0.3f));
    float midY = bounds.getCentreY();
    g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 4, bounds.getRight() - 4);

    // Draw waveform
    auto waveArea = bounds.reduced(8.0f, 16.0f);
    float centreY = waveArea.getCentreY();
    float amplitude = waveArea.getHeight() * 0.45f;

    // Create the waveform path
    juce::Path waveformPath;
    int displaySamples = 512; // Downsample for display
    int step = WAVETABLE_SIZE / displaySamples;

    for (int i = 0; i < displaySamples; ++i)
    {
        int sampleIdx = i * step;
        float x = waveArea.getX() + (static_cast<float>(i) / displaySamples) * waveArea.getWidth();
        float y = centreY - waveformData[sampleIdx] * amplitude;

        if (i == 0)
            waveformPath.startNewSubPath(x, y);
        else
            waveformPath.lineTo(x, y);
    }

    // Draw glow effect (wider, semi-transparent stroke behind the main line)
    g.setColour(accentColour.withAlpha(0.15f + 0.05f * std::sin(animationPhase)));
    g.strokePath(waveformPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));

    g.setColour(accentColour.withAlpha(0.3f));
    g.strokePath(waveformPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved));

    // Main waveform line
    g.setColour(accentColour);
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved));

    // Fill under the waveform with gradient
    juce::Path fillPath = waveformPath;
    fillPath.lineTo(waveArea.getRight(), centreY);
    fillPath.lineTo(waveArea.getX(), centreY);
    fillPath.closeSubPath();

    g.setGradientFill(juce::ColourGradient(
        accentColour.withAlpha(0.1f), waveArea.getX(), centreY - amplitude,
        accentColour.withAlpha(0.0f), waveArea.getX(), centreY,
        false));
    g.fillPath(fillPath);

    // Label
    g.setColour(KnobLookAndFeel::textSecondary);
    g.setFont(12.0f);
    g.drawText(label, bounds.reduced(8.0f, 4.0f), juce::Justification::topLeft);
}
