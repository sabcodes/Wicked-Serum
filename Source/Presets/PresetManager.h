/*
  ==============================================================================
    PresetManager.h

    Manages saving, loading, and browsing presets.
    Presets are stored as JSON files containing all synth parameters.

    Preset files are stored in:
    - macOS: ~/Library/Audio/Presets/SerumSynth/
    - Windows: %APPDATA%/SerumSynth/Presets/
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;

    // Save current state to a preset file
    void savePreset(const juce::String& name, juce::AudioProcessorValueTreeState& apvts);

    // Load a preset from file
    bool loadPreset(const juce::String& name, juce::AudioProcessorValueTreeState& apvts);

    // Delete a preset
    void deletePreset(const juce::String& name);

    // Get list of available preset names
    juce::StringArray getPresetNames() const;

    // Navigate presets
    void selectNextPreset();
    void selectPreviousPreset();
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }

    // Get the preset directory
    juce::File getPresetDirectory() const;

private:
    void scanPresets();
    juce::File getPresetFile(const juce::String& name) const;

    juce::StringArray presetNames;
    int currentPresetIndex = -1;
    juce::String currentPresetName = "Init";
};
