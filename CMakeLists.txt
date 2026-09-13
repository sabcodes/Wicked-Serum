cmake_minimum_required(VERSION 3.22)

# ============================================================================
# SerumSynth - A Serum-inspired Wavetable Synthesizer Plugin
# Builds as AU (Audio Unit) for Logic Pro, plus VST3 and Standalone
# ============================================================================

project(SerumSynth VERSION 1.0.0)

# Add JUCE (cloned as a subdirectory)
add_subdirectory(JUCE)

# Define the plugin target
juce_add_plugin(SerumSynth
    # Plugin metadata
    COMPANY_NAME "SerumSynth"
    PLUGIN_MANUFACTURER_CODE Srsn
    PLUGIN_CODE Srs1

    # Plugin formats - AU is what Logic Pro uses
    FORMATS AU VST3 Standalone

    # Plugin type - this is a synthesizer (instrument)
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    # Plugin name as it appears in Logic
    PRODUCT_NAME "SerumSynth"

    # Bundle ID for macOS (important for Logic to find it)
    BUNDLE_ID "com.serumsynth.plugin"

    # Copy plugin to system folder after build (so Logic finds it)
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Generate JuceHeader.h so our source files can #include <JuceHeader.h>
juce_generate_juce_header(SerumSynth)

# Source files - organized by module
target_sources(SerumSynth
    PRIVATE
        # Main plugin entry points
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h

        # DSP Engine - Wavetable Oscillators
        Source/DSP/WavetableOscillator.cpp
        Source/DSP/WavetableOscillator.h
        Source/DSP/SubOscillator.cpp
        Source/DSP/SubOscillator.h
        Source/DSP/NoiseGenerator.cpp
        Source/DSP/NoiseGenerator.h

        # DSP Engine - Filters
        Source/DSP/SynthFilter.cpp
        Source/DSP/SynthFilter.h

        # DSP Engine - Envelopes & LFOs
        Source/DSP/ADSREnvelope.cpp
        Source/DSP/ADSREnvelope.h
        Source/DSP/LFO.cpp
        Source/DSP/LFO.h

        # DSP Engine - Modulation
        Source/DSP/ModulationMatrix.cpp
        Source/DSP/ModulationMatrix.h

        # DSP Engine - Arpeggiator
        Source/DSP/Arpeggiator.cpp
        Source/DSP/Arpeggiator.h

        # DSP Engine - Factory Wavetables
        Source/DSP/FactoryWavetables.cpp
        Source/DSP/FactoryWavetables.h

        # DSP Engine - Effects
        Source/DSP/Effects/EffectsChain.cpp
        Source/DSP/Effects/EffectsChain.h
        Source/DSP/Effects/ReverbEffect.cpp
        Source/DSP/Effects/ReverbEffect.h
        Source/DSP/Effects/DelayEffect.cpp
        Source/DSP/Effects/DelayEffect.h
        Source/DSP/Effects/DistortionEffect.cpp
        Source/DSP/Effects/DistortionEffect.h
        Source/DSP/Effects/ChorusEffect.cpp
        Source/DSP/Effects/ChorusEffect.h
        Source/DSP/Effects/PhaserEffect.cpp
        Source/DSP/Effects/PhaserEffect.h
        Source/DSP/Effects/FlangerEffect.cpp
        Source/DSP/Effects/FlangerEffect.h
        Source/DSP/Effects/EQEffect.cpp
        Source/DSP/Effects/EQEffect.h

        # DSP Engine - Velocity & Oversampling
        Source/DSP/VelocityCurve.cpp
        Source/DSP/VelocityCurve.h
        Source/DSP/OversamplingProcessor.cpp
        Source/DSP/OversamplingProcessor.h

        # DSP Engine - Voice Management
        Source/DSP/SynthVoice.cpp
        Source/DSP/SynthVoice.h
        Source/DSP/SynthEngine.cpp
        Source/DSP/SynthEngine.h

        # GUI Components
        Source/GUI/WaveformDisplay.cpp
        Source/GUI/WaveformDisplay.h
        Source/GUI/KnobLookAndFeel.cpp
        Source/GUI/KnobLookAndFeel.h
        Source/GUI/OscillatorPanel.cpp
        Source/GUI/OscillatorPanel.h
        Source/GUI/FilterPanel.cpp
        Source/GUI/FilterPanel.h
        Source/GUI/EnvelopePanel.cpp
        Source/GUI/EnvelopePanel.h
        Source/GUI/EffectsPanel.cpp
        Source/GUI/EffectsPanel.h
        Source/GUI/ModulationPanel.cpp
        Source/GUI/ModulationPanel.h
        Source/GUI/ModulationOverlay.cpp
        Source/GUI/ModulationOverlay.h
        Source/GUI/LFOEditor.cpp
        Source/GUI/LFOEditor.h
        Source/GUI/PresetPanel.cpp
        Source/GUI/PresetPanel.h
        Source/GUI/WavetableEditor.cpp
        Source/GUI/WavetableEditor.h

        # Preset System
        Source/Presets/PresetManager.cpp
        Source/Presets/PresetManager.h
        Source/Presets/FactoryPresets.cpp
        Source/Presets/FactoryPresets.h
)

# Focused regression executable for modulation, ARP timing, and wavetable editing
juce_add_console_app(SerumSynthFeatureTests PRODUCT_NAME "SerumSynth Feature Tests")
juce_generate_juce_header(SerumSynthFeatureTests)
target_sources(SerumSynthFeatureTests PRIVATE
    Tests/FeatureRegressionTests.cpp
    Source/DSP/Arpeggiator.cpp
    Source/DSP/ADSREnvelope.cpp
    Source/DSP/LFO.cpp
    Source/DSP/ModulationMatrix.cpp
    Source/DSP/NoiseGenerator.cpp
    Source/DSP/SubOscillator.cpp
    Source/DSP/SynthFilter.cpp
    Source/DSP/SynthVoice.cpp
    Source/DSP/VelocityCurve.cpp
    Source/DSP/WavetableOscillator.cpp)
target_link_libraries(SerumSynthFeatureTests PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
target_compile_features(SerumSynthFeatureTests PRIVATE cxx_std_17)

# JUCE modules this plugin needs
target_link_libraries(SerumSynth
    PRIVATE
        juce::juce_audio_utils        # Audio utilities & plugin hosting
        juce::juce_audio_processors   # Plugin processor base classes
        juce::juce_dsp                # DSP module (filters, oscillators, etc.)
        juce::juce_gui_extra          # Extra GUI components
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions
target_compile_definitions(SerumSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Use C++17 (JUCE requirement)
target_compile_features(SerumSynth PRIVATE cxx_std_17)
