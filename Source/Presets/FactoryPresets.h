/*
  ==============================================================================
    FactoryPresets.h

    Factory preset definitions for SerumSynth.

    Each preset is a snapshot of all synthesizer parameters that defines a
    complete sound. Presets are stored as vectors of parameter ID/value pairs
    that can be loaded into the APVTS (AudioProcessorValueTreeState).

    Parameter IDs match those defined in PluginProcessor::createParameterLayout()
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>

namespace FactoryPresets
{
    // A single parameter snapshot (parameterID -> value)
    struct ParameterSnapshot
    {
        juce::String parameterId;
        float value;
    };

    // A complete preset is a vector of parameter snapshots
    using PresetData = std::vector<ParameterSnapshot>;

    /**
     * A complete preset with metadata
     */
    struct FactoryPreset
    {
        juce::String name;
        juce::String description;
        PresetData parameters;
    };

    /**
     * Get all factory presets
     */
    std::vector<FactoryPreset> getFactoryPresets();

    /**
     * Get a specific preset by name
     */
    FactoryPreset getPresetByName(const juce::String& name);

    /**
     * Apply a preset to the APVTS
     */
    void applyPreset(juce::AudioProcessorValueTreeState& apvts, const FactoryPreset& preset);

} // namespace FactoryPresets
