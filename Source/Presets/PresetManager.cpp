/*
  ==============================================================================
    PresetManager.cpp
  ==============================================================================
*/

#include "PresetManager.h"

PresetManager::PresetManager()
{
    // Create preset directory if it doesn't exist
    auto dir = getPresetDirectory();
    if (!dir.exists())
        dir.createDirectory();

    scanPresets();
}

juce::File PresetManager::getPresetDirectory() const
{
    // Store presets in the standard location for audio presets
    #if JUCE_MAC
        return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
            .getChildFile("Library/Audio/Presets/SerumSynth");
    #else
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("SerumSynth/Presets");
    #endif
}

juce::File PresetManager::getPresetFile(const juce::String& name) const
{
    return getPresetDirectory().getChildFile(name + ".preset");
}

void PresetManager::scanPresets()
{
    presetNames.clear();

    auto dir = getPresetDirectory();
    if (!dir.exists())
        return;

    // Find all .preset files
    for (auto& file : dir.findChildFiles(juce::File::findFiles, false, "*.preset"))
    {
        presetNames.add(file.getFileNameWithoutExtension());
    }

    presetNames.sort(true); // Alphabetical order
}

void PresetManager::savePreset(const juce::String& name, juce::AudioProcessorValueTreeState& apvts)
{
    auto file = getPresetFile(name);

    // Serialize the entire parameter state to XML
    auto state = apvts.copyState();
    auto xml = state.createXml();

    if (xml != nullptr)
    {
        xml->writeTo(file);
        currentPresetName = name;
        scanPresets(); // Refresh the list

        // Update current index
        currentPresetIndex = presetNames.indexOf(name);
    }
}

bool PresetManager::loadPreset(const juce::String& name, juce::AudioProcessorValueTreeState& apvts)
{
    auto file = getPresetFile(name);

    if (!file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse(file);

    if (xml != nullptr)
    {
        auto newState = juce::ValueTree::fromXml(*xml);

        if (newState.isValid())
        {
            apvts.replaceState(newState);
            currentPresetName = name;
            currentPresetIndex = presetNames.indexOf(name);
            return true;
        }
    }

    return false;
}

void PresetManager::deletePreset(const juce::String& name)
{
    auto file = getPresetFile(name);

    if (file.existsAsFile())
    {
        file.deleteFile();
        scanPresets();

        if (currentPresetIndex >= presetNames.size())
            currentPresetIndex = presetNames.size() - 1;
    }
}

juce::StringArray PresetManager::getPresetNames() const
{
    return presetNames;
}

void PresetManager::selectNextPreset()
{
    if (presetNames.isEmpty())
        return;

    currentPresetIndex = (currentPresetIndex + 1) % presetNames.size();
    currentPresetName = presetNames[currentPresetIndex];
}

void PresetManager::selectPreviousPreset()
{
    if (presetNames.isEmpty())
        return;

    currentPresetIndex--;
    if (currentPresetIndex < 0)
        currentPresetIndex = presetNames.size() - 1;

    currentPresetName = presetNames[currentPresetIndex];
}
