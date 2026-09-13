/*
  ==============================================================================
    FilterPanel.cpp
  ==============================================================================
*/

#include "FilterPanel.h"

FilterPanel::FilterPanel(juce::AudioProcessorValueTreeState& apvts)
{
    setupKnob(cutoffSlider, cutoffLabel, "CUTOFF");
    setupKnob(resonanceSlider, resonanceLabel, "RES");
    setupKnob(driveSlider, driveLabel, "DRIVE");
    setupKnob(mixSlider, mixLabel, "MIX");
    setupKnob(keyTrackSlider, keyTrackLabel, "KEY TRK");

    enableButton.setButtonText("FILTER");
    enableButton.setLookAndFeel(&knobLnf);
    addAndMakeVisible(enableButton);

    cutoffAtt   = std::make_unique<SliderAttachment>(apvts, "filter_cutoff", cutoffSlider);
    resonanceAtt = std::make_unique<SliderAttachment>(apvts, "filter_resonance", resonanceSlider);
    driveAtt    = std::make_unique<SliderAttachment>(apvts, "filter_drive", driveSlider);
    mixAtt      = std::make_unique<SliderAttachment>(apvts, "filter_mix", mixSlider);
    keyTrackAtt = std::make_unique<SliderAttachment>(apvts, "filter_keyTrack", keyTrackSlider);
    enableAtt   = std::make_unique<ButtonAttachment>(apvts, "filter_enabled", enableButton);
}

FilterPanel::~FilterPanel()
{
    cutoffSlider.setLookAndFeel(nullptr);
    resonanceSlider.setLookAndFeel(nullptr);
    driveSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
    keyTrackSlider.setLookAndFeel(nullptr);
    enableButton.setLookAndFeel(nullptr);
}

void FilterPanel::setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    slider.setLookAndFeel(&knobLnf);
    slider.setName("filter_" + text);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f));
    label.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(label);
}

void FilterPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
}

void FilterPanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);
    enableButton.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);

    int knobSize = 55;
    int labelHeight = 14;
    auto row = bounds.removeFromTop(knobSize + labelHeight);
    int knobWidth = row.getWidth() / 5;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l) {
        auto cell = row.removeFromLeft(knobWidth);
        l.setBounds(cell.removeFromTop(labelHeight));
        s.setBounds(cell);
    };

    placeKnob(cutoffSlider, cutoffLabel);
    placeKnob(resonanceSlider, resonanceLabel);
    placeKnob(driveSlider, driveLabel);
    placeKnob(mixSlider, mixLabel);
    placeKnob(keyTrackSlider, keyTrackLabel);
}
