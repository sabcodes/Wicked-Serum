/*
  ==============================================================================
    PresetPanel.h — Preset browser bar at the top of the plugin
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Presets/PresetManager.h"
#include "KnobLookAndFeel.h"

class PresetPanel : public juce::Component, public juce::Button::Listener
{
public:
    PresetPanel(PresetManager& pm, juce::AudioProcessorValueTreeState& apvts);
    ~PresetPanel() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

private:
    PresetManager& presetManager;
    juce::AudioProcessorValueTreeState& apvts;

    juce::TextButton prevButton, nextButton, saveButton;
    juce::Label presetNameLabel;
    KnobLookAndFeel knobLnf;
};
