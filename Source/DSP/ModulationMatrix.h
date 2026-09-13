/*
  ==============================================================================
    ModulationMatrix.h

    The Modulation Matrix — the routing system that connects modulation sources
    to destination parameters. This is what makes Serum so powerful and flexible.

    HOW IT WORKS:
    In Serum, you drag a modulation source (like an LFO or envelope) onto any
    knob to create a "modulation route." The source then continuously adjusts
    that parameter by the specified amount.

    SOURCES (things that generate modulation signals):
    - Envelopes (ENV 1, 2, 3) — one-shot shapes triggered by notes
    - LFOs (LFO 1, 2, 3, 4) — repeating cyclic shapes
    - Velocity — how hard you press the key (0-1)
    - Note — which note you play (MIDI note number mapped to 0-1)
    - Mod Wheel — MIDI CC1 (0-1)
    - Aftertouch — pressure after key press (0-1)
    - Macro 1-4 — user-defined macro knobs

    DESTINATIONS (things that can be modulated):
    - Any oscillator parameter (wavetable position, level, pan, detune, warp...)
    - Filter cutoff, resonance, drive
    - LFO rate, depth
    - Effect parameters
    - Basically anything with a knob!

    Each route has:
    - Source: what generates the modulation
    - Destination: what parameter gets modulated
    - Amount: how much modulation is applied (-1 to +1, bipolar)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <functional>

// Maximum number of simultaneous modulation routes
static constexpr int MAX_MOD_ROUTES = 32;

// ============================================================================
// Modulation Source IDs
// ============================================================================
enum class ModSource
{
    None,
    Env1, Env2, Env3,           // ADSR Envelopes
    LFO1, LFO2, LFO3, LFO4,   // Low Frequency Oscillators
    Velocity,                    // Note velocity (how hard key was pressed)
    NoteNumber,                  // MIDI note (which key was pressed)
    ModWheel,                    // MIDI Mod Wheel (CC1)
    Aftertouch,                  // Channel aftertouch
    PitchBend,                   // Pitch bend wheel
    Macro1, Macro2, Macro3, Macro4,  // User macro knobs
    PerNotePressure,             // MPE: per-note pressure
    PerNoteSlide,                // MPE: per-note slide (CC74)
    PerNotePitchBend,            // MPE: per-note pitch bend
    NumSources
};

// ============================================================================
// Modulation Destination IDs
// ============================================================================
enum class ModDestination
{
    None,

    // Oscillator A
    OscA_WTPosition, OscA_Level, OscA_Pan, OscA_Detune,
    OscA_UnisonDetune, OscA_WarpAmount, OscA_Octave,

    // Oscillator B
    OscB_WTPosition, OscB_Level, OscB_Pan, OscB_Detune,
    OscB_UnisonDetune, OscB_WarpAmount, OscB_Octave,

    // Sub Oscillator
    Sub_Level,

    // Noise
    Noise_Level,

    // Filter
    Filter_Cutoff, Filter_Resonance, Filter_Drive, Filter_Mix,

    // Global
    MasterVolume, MasterPan, MasterPitch,

    // LFO rates
    LFO1_Rate, LFO2_Rate, LFO3_Rate, LFO4_Rate,

    // Effects
    FX_ReverbMix, FX_DelayMix, FX_DistortionDrive, FX_ChorusMix,

    NumDestinations
};

// ============================================================================
// A single modulation route
// ============================================================================
struct ModRoute
{
    ModSource source = ModSource::None;
    ModDestination destination = ModDestination::None;
    float amount = 0.0f;    // -1.0 to 1.0 (bipolar modulation amount)
    bool enabled = true;
};

// ============================================================================
// The Modulation Matrix
// ============================================================================
class ModulationMatrix
{
public:
    ModulationMatrix() = default;
    ~ModulationMatrix() = default;

    // --- Route Management ---

    // Add a new modulation route. Returns the index, or -1 if matrix is full.
    int addRoute(ModSource source, ModDestination destination, float amount);

    // Remove a route by index
    void removeRoute(int index);

    // Clear all routes
    void clearAllRoutes();

    // Modify an existing route
    void setRouteAmount(int index, float amount);
    void setRouteEnabled(int index, bool enabled);

    // --- Source Value Updates ---
    // Call these each sample (or each block) to update source values
    void setSourceValue(ModSource source, float value);

    // --- Destination Queries ---
    // Get the total modulation amount for a destination parameter
    // This sums all active routes targeting this destination
    float getModulatedValue(ModDestination destination, float baseValue) const;

    // Get just the modulation offset (without the base value)
    float getModulationAmount(ModDestination destination) const;

    // --- Getters for GUI ---
    const std::vector<ModRoute>& getRoutes() const { return routes; }
    int getNumRoutes() const { return static_cast<int>(routes.size()); }
    float getSourceValue(ModSource source) const;

    // Get human-readable names
    static juce::String getSourceName(ModSource source);
    static juce::String getDestinationName(ModDestination destination);

private:
    std::vector<ModRoute> routes;

    // Current values of all modulation sources
    float sourceValues[static_cast<int>(ModSource::NumSources)] = {};
};
