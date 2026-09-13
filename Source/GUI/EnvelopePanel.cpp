/*
  ==============================================================================
    EnvelopePanel.cpp
  ==============================================================================
*/

#include "EnvelopePanel.h"

EnvelopePanel::EnvelopePanel(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& envPrefix,
                               const juce::String& label)
    : envLabel(label)
{
    setupKnob(attackSlider, attackLabel, "ATK");
    setupKnob(decaySlider, decayLabel, "DEC");
    setupKnob(sustainSlider, sustainLabel, "SUS");
    setupKnob(releaseSlider, releaseLabel, "REL");

    attackAtt  = std::make_unique<SliderAttachment>(apvts, envPrefix + "_attack", attackSlider);
    decayAtt   = std::make_unique<SliderAttachment>(apvts, envPrefix + "_decay", decaySlider);
    sustainAtt = std::make_unique<SliderAttachment>(apvts, envPrefix + "_sustain", sustainSlider);
    releaseAtt = std::make_unique<SliderAttachment>(apvts, envPrefix + "_release", releaseSlider);
}

EnvelopePanel::~EnvelopePanel()
{
    attackSlider.setLookAndFeel(nullptr);
    decaySlider.setLookAndFeel(nullptr);
    sustainSlider.setLookAndFeel(nullptr);
    releaseSlider.setLookAndFeel(nullptr);
}

void EnvelopePanel::setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    slider.setLookAndFeel(&knobLnf);
    slider.setName("env_" + text);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f));
    label.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(label);
}

void EnvelopePanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Title
    g.setColour(KnobLookAndFeel::accentPink);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(envLabel, bounds.reduced(8, 4), juce::Justification::topLeft);
}

void EnvelopePanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);
    bounds.removeFromTop(18); // Title

    int knobSize = 50;
    int labelHeight = 14;
    auto row = bounds.removeFromTop(knobSize + labelHeight);
    int knobWidth = row.getWidth() / 4;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l) {
        auto cell = row.removeFromLeft(knobWidth);
        l.setBounds(cell.removeFromTop(labelHeight));
        s.setBounds(cell);
    };

    placeKnob(attackSlider, attackLabel);
    placeKnob(decaySlider, decayLabel);
    placeKnob(sustainSlider, sustainLabel);
    placeKnob(releaseSlider, releaseLabel);
}
