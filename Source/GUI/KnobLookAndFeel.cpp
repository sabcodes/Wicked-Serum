/*
  ==============================================================================
    KnobLookAndFeel.cpp
  ==============================================================================
*/

#include "KnobLookAndFeel.h"

// Serum-inspired dark color palette
const juce::Colour KnobLookAndFeel::backgroundDark  = juce::Colour(0xFF1A1A2E);
const juce::Colour KnobLookAndFeel::backgroundMid   = juce::Colour(0xFF16213E);
const juce::Colour KnobLookAndFeel::backgroundLight  = juce::Colour(0xFF0F3460);
const juce::Colour KnobLookAndFeel::accentBlue      = juce::Colour(0xFF00D4FF);
const juce::Colour KnobLookAndFeel::accentGreen     = juce::Colour(0xFF00FF88);
const juce::Colour KnobLookAndFeel::accentOrange     = juce::Colour(0xFFFF6B35);
const juce::Colour KnobLookAndFeel::accentPink      = juce::Colour(0xFFFF2E63);
const juce::Colour KnobLookAndFeel::textPrimary     = juce::Colour(0xFFE0E0E0);
const juce::Colour KnobLookAndFeel::textSecondary   = juce::Colour(0xFF888888);
const juce::Colour KnobLookAndFeel::knobTrack       = juce::Colour(0xFF333355);

KnobLookAndFeel::KnobLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, textPrimary);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, textPrimary);
    setColour(juce::ToggleButton::textColourId, textPrimary);
    setColour(juce::ToggleButton::tickColourId, accentBlue);
    setColour(juce::ComboBox::backgroundColourId, backgroundMid);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::ComboBox::outlineColourId, backgroundLight);
    setColour(juce::TextButton::buttonColourId, backgroundLight);
    setColour(juce::TextButton::textColourOffId, textPrimary);
}

void KnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Determine accent color based on slider name
    juce::Colour accent = accentBlue;
    auto name = slider.getName().toLowerCase();
    if (name.contains("osc") || name.contains("wt"))
        accent = accentGreen;
    else if (name.contains("filter") || name.contains("cutoff") || name.contains("res"))
        accent = accentOrange;
    else if (name.contains("env") || name.contains("attack") || name.contains("decay"))
        accent = accentPink;
    else if (name.contains("lfo") || name.contains("fx") || name.contains("effect"))
        accent = accentBlue;

    // Draw background track (full arc)
    auto trackWidth = 3.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - trackWidth, radius - trackWidth,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(knobTrack);
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved));

    // Draw value arc (filled portion)
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius - trackWidth, radius - trackWidth,
                            0.0f, rotaryStartAngle, angle, true);
    g.setColour(accent);
    g.strokePath(valueArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved));

    // Draw knob body
    auto knobRadius = radius * 0.65f;
    g.setColour(backgroundMid);
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Draw knob border
    g.setColour(backgroundLight);
    g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

    // Draw pointer line
    auto pointerLength = knobRadius * 0.7f;
    auto pointerThickness = 2.5f;
    juce::Path pointer;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius + 4.0f, pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(accent);
    g.fillPath(pointer);

    // Glow effect around the knob at higher values
    if (sliderPos > 0.01f)
    {
        g.setColour(accent.withAlpha(0.1f * sliderPos));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    }
}

void KnobLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));

    bool isVertical = (style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical);

    // Background track
    g.setColour(knobTrack);
    if (isVertical)
    {
        auto trackX = bounds.getCentreX() - 2.0f;
        g.fillRoundedRectangle(trackX, bounds.getY(), 4.0f, bounds.getHeight(), 2.0f);

        // Value fill
        g.setColour(accentBlue);
        float fillHeight = bounds.getBottom() - sliderPos;
        g.fillRoundedRectangle(trackX, sliderPos, 4.0f, fillHeight, 2.0f);
    }
    else
    {
        auto trackY = bounds.getCentreY() - 2.0f;
        g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), 4.0f, 2.0f);

        // Value fill
        g.setColour(accentBlue);
        g.fillRoundedRectangle(bounds.getX(), trackY, sliderPos - x, 4.0f, 2.0f);
    }

    // Thumb
    g.setColour(accentBlue);
    if (isVertical)
        g.fillEllipse(bounds.getCentreX() - 6.0f, sliderPos - 6.0f, 12.0f, 12.0f);
    else
        g.fillEllipse(sliderPos - 6.0f, bounds.getCentreY() - 6.0f, 12.0f, 12.0f);
}
