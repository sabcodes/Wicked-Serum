/*
  ==============================================================================
    KnobLookAndFeel.h

    Custom look and feel for knobs/sliders — Serum-inspired dark theme
    with glowing accent colors and clean, modern aesthetics.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // Color scheme — Serum-inspired dark theme
    static const juce::Colour backgroundDark;
    static const juce::Colour backgroundMid;
    static const juce::Colour backgroundLight;
    static const juce::Colour accentBlue;
    static const juce::Colour accentGreen;
    static const juce::Colour accentOrange;
    static const juce::Colour accentPink;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour knobTrack;
};
