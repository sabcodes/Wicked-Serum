/*
  ==============================================================================
    PresetPanel.cpp
  ==============================================================================
*/

#include "PresetPanel.h"

PresetPanel::PresetPanel(PresetManager& pm, juce::AudioProcessorValueTreeState& a)
    : presetManager(pm), apvts(a)
{
    prevButton.setButtonText("<");
    prevButton.setLookAndFeel(&knobLnf);
    prevButton.addListener(this);
    addAndMakeVisible(prevButton);

    nextButton.setButtonText(">");
    nextButton.setLookAndFeel(&knobLnf);
    nextButton.addListener(this);
    addAndMakeVisible(nextButton);

    saveButton.setButtonText("SAVE");
    saveButton.setLookAndFeel(&knobLnf);
    saveButton.addListener(this);
    addAndMakeVisible(saveButton);

    presetNameLabel.setText(presetManager.getCurrentPresetName(), juce::dontSendNotification);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    presetNameLabel.setColour(juce::Label::textColourId, KnobLookAndFeel::textPrimary);
    presetNameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(presetNameLabel);
}

PresetPanel::~PresetPanel()
{
    prevButton.setLookAndFeel(nullptr);
    nextButton.setLookAndFeel(nullptr);
    saveButton.setLookAndFeel(nullptr);
}

void PresetPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundDark);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(KnobLookAndFeel::backgroundLight);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Plugin title
    g.setColour(KnobLookAndFeel::accentBlue);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("SERUM SYNTH", bounds.reduced(10, 0), juce::Justification::centredLeft);
}

void PresetPanel::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    // Title takes left side (already drawn in paint)
    bounds.removeFromLeft(120);

    // Save button on the right
    saveButton.setBounds(bounds.removeFromRight(60));
    bounds.removeFromRight(4);

    // Prev/Next buttons and preset name in the center
    prevButton.setBounds(bounds.removeFromLeft(30));
    nextButton.setBounds(bounds.removeFromRight(30));
    presetNameLabel.setBounds(bounds);
}

void PresetPanel::buttonClicked(juce::Button* button)
{
    if (button == &prevButton)
    {
        presetManager.selectPreviousPreset();
        presetManager.loadPreset(presetManager.getCurrentPresetName(), apvts);
        presetNameLabel.setText(presetManager.getCurrentPresetName(), juce::dontSendNotification);
    }
    else if (button == &nextButton)
    {
        presetManager.selectNextPreset();
        presetManager.loadPreset(presetManager.getCurrentPresetName(), apvts);
        presetNameLabel.setText(presetManager.getCurrentPresetName(), juce::dontSendNotification);
    }
    else if (button == &saveButton)
    {
        // Save with a simple incremented name (no modal dialog needed)
        auto name = presetManager.getCurrentPresetName();
        if (name == "Init" || name.isEmpty())
            name = "My Preset 1";

        presetManager.savePreset(name, apvts);
        presetNameLabel.setText(name, juce::dontSendNotification);
    }
}
