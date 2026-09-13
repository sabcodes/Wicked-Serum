/*
  ==============================================================================
    PluginProcessor.h

    The "brain" of the plugin — this is what Logic Pro talks to.

    JUCE's AudioProcessor class handles:
    - Receiving MIDI from the DAW
    - Sending audio back to the DAW
    - Saving/loading plugin state (when you save a Logic project)
    - Exposing automatable parameters to Logic

    This class connects the DAW to our SynthEngine.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/SynthEngine.h"
#include "DSP/Arpeggiator.h"
#include "Presets/PresetManager.h"

class SerumSynthProcessor : public juce::AudioProcessor
{
public:
    SerumSynthProcessor();
    ~SerumSynthProcessor() override;

    // === AudioProcessor overrides (required by JUCE) ===

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Editor (GUI)
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Plugin info
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Programs (we use our own preset system instead)
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    // State save/load (called when Logic saves/opens a project)
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // === Our synth-specific stuff ===

    // Access to the parameter tree (for GUI and automation)
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Access to the synth engine
    SynthEngine& getSynthEngine() { return synthEngine; }

    // Access to the arpeggiator
    Arpeggiator& getArpeggiator() { return arpeggiator; }

    // Access to preset manager
    PresetManager& getPresetManager() { return presetManager; }

private:
    // Create the parameter layout — defines all automatable parameters
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Sync APVTS parameter values to the synth engine
    void updateSynthFromParameters();

    // The DSP engine
    SynthEngine synthEngine;

    // The arpeggiator
    Arpeggiator arpeggiator;

    // Parameter state tree (connects GUI ↔ DSP ↔ DAW automation)
    juce::AudioProcessorValueTreeState apvts;

    // Preset system
    PresetManager presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerumSynthProcessor)
};
