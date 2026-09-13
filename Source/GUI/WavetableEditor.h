/*
  ==============================================================================
    WavetableEditor.h

    Visual wavetable editor GUI component — the core editing interface for
    crafting custom wavetables in SerumSynth, inspired by Serum's wavetable editor.

    FEATURES:
    - Waveform drawing mode: Click and drag to draw a single-cycle waveform
    - Harmonic editor mode: Adjust the amplitude of the first 32 harmonics
    - Factory wavetable selector: Load preset wavetables (Basic, Analog, Spectral)
    - WAV import: Load a .wav file to use as a custom wavetable frame
    - Mode toggle: Switch between Drawing and Harmonic editor modes
    - Dark theme styling matching the rest of the plugin

    BEGINNER NOTES:
    - Wavetables are arrays of 2048 floating-point samples representing one
      complete cycle of a waveform.
    - The \"harmonic editor\" uses additive synthesis: summing sine waves at
      different frequencies and amplitudes to create complex tones.
    - The waveform data is stored as floats ranging from -1.0 to +1.0.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../DSP/WavetableOscillator.h"
#include "KnobLookAndFeel.h"
#include <functional>

/**
 * WavetableEditor — a complete GUI for creating and editing wavetables.
 *
 * This component provides an intuitive interface for users to design custom
 * waveforms either by drawing them directly or by adjusting harmonic content.
 * The editor owns a full Wavetable (initialized to Basic) and allows editing
 * of a single selected frame at a time while preserving all other frames.
 */
class WavetableEditor : public juce::Component
{
public:
    WavetableEditor();
    ~WavetableEditor() override = default;

    // ========== JUCE COMPONENT OVERRIDES ==========
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // ========== CONFIGURATION ==========
    /**
     * Set the callback function that fires when the wavetable is edited.
     * The callback receives a const reference to the full working wavetable.
     */
    void setOnWavetableChanged(std::function<void(const Wavetable&)> callback)
    {
        onWavetableChanged = callback;
    }

    // ========== PUBLIC GETTERS ==========
    /**
     * Get the current working wavetable (all frames).
     */
    const Wavetable& getWorkingWavetable() const { return workingWavetable; }

    /**
     * Get the currently displayed waveform (selected frame).
     * Each sample is in the range [-1.0, +1.0].
     */
    const std::array<float, 2048>& getCurrentWaveform() const { return currentWaveform; }

    /**
     * Returns true if we're currently in drawing mode, false for harmonic mode.
     */
    bool isInDrawMode() const { return drawMode; }

private:
    // ========== INTERNAL HELPERS ==========

    /**
     * Generate the current waveform from the harmonic amplitudes.
     * Uses additive synthesis to combine sine waves.
     */
    void regenerateWaveformFromHarmonics();

    /**
     * Load a .wav file as a wavetable frame.
     * Reads the first 2048 samples (or pads with zeros if shorter).
     */
    void loadWavFile();

    /**
     * Load one of the factory wavetables.
     * index: 0 = Basic, 1 = Analog, 2 = Spectral
     */
    void loadFactoryWavetable(int index);

    /**
     * Get the pixel X position corresponding to a sample index.
     * Used for drawing the waveform display and handling mouse input.
     */
    int sampleIndexToPixelX(int sampleIndex) const;

    /**
     * Get the sample index (0-2047) corresponding to a pixel X position.
     * Used for handling mouse input in draw mode.
     */
    int pixelXToSampleIndex(int pixelX) const;

    /**
     * Get the pixel Y position corresponding to a waveform value (-1.0 to +1.0).
     * Positive values go toward the top, negative toward the bottom.
     */
    int waveformValueToPixelY(float value) const;

    /**
     * Get the waveform value (-1.0 to +1.0) corresponding to a pixel Y position.
     */
    float pixelYToWaveformValue(int pixelY) const;

    /**
     * Paint the waveform drawing area (the main grid/background).
     */
    void paintWaveformArea(juce::Graphics& g);

    /**
     * Paint the harmonic editor (bar graph of harmonics).
     */
    void paintHarmonicEditor(juce::Graphics& g);

    /**
     * Paint the control panel (buttons, labels, mode selector).
     */
    void paintControlPanel(juce::Graphics& g);

    /**
     * Get the bounding rectangle for the waveform display area.
     */
    juce::Rectangle<int> getWaveformArea() const;

    /**
     * Get the bounding rectangle for the harmonic editor area.
     */
    juce::Rectangle<int> getHarmonicArea() const;

    /**
     * Get the bounding rectangle for the control panel area.
     */
    juce::Rectangle<int> getControlArea() const;

    /**
     * Get the bounding rectangle for a specific harmonic bar (by index 0-31).
     * Used for both drawing and mouse hit-testing.
     */
    juce::Rectangle<int> getHarmonicBarBounds(int harmonicIndex) const;

    /**
     * Snap waveform values to a grid for cleaner drawing (optional).
     * Helps make the waveform look more intentional.
     */
    float snapToGrid(float value, float gridSize = 0.1f) const;

    // ========== STATE ==========

    // The working wavetable: owns all frames, initialized to Basic
    Wavetable workingWavetable;

    // Index of currently selected frame (0-based, maps to 1..numFrames in UI)
    int selectedFrameIndex = 0;

    // The currently displayed waveform (selected frame): 2048 samples, each in range [-1.0, +1.0]
    std::array<float, 2048> currentWaveform;

    // Harmonic amplitudes for the first 32 harmonics (for harmonic mode editing)
    // Index 0 is fundamental, 1 is 1st harmonic, etc.
    std::array<float, 32> harmonicAmplitudes;

    // UI state
    bool drawMode = true;  // true = drawing mode, false = harmonic mode
    bool isDrawing = false;  // true while user is actively drawing

    // Currently selected harmonic (for harmonic mode mouse interaction)
    int selectedHarmonic = -1;

    // Callback function fired when wavetable changes (receives full Wavetable)
    std::function<void(const Wavetable&)> onWavetableChanged;

    // Factory wavetable names for display
    const juce::StringArray factoryWavetableNames = { "Basic", "Analog", "Spectral" };
    int currentFactoryIndex = 0;

    // Look and feel colors (caching to avoid repeated lookups)
    juce::Colour colBackgroundDark;
    juce::Colour colBackgroundMid;
    juce::Colour colBackgroundLight;
    juce::Colour colAccentGreen;
    juce::Colour colAccentBlue;
    juce::Colour colTextPrimary;
    juce::Colour colTextSecondary;

    // Initialize color scheme from KnobLookAndFeel
    void initializeColors();

    // File chooser must be a member for async usage
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Helper to sync currentWaveform from working wavetable selected frame
    void syncCurrentWaveformFromFrame();

    // Helper to commit edits to selected frame in working wavetable
    void commitEditedFrameToWavetable();
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableEditor)
};
