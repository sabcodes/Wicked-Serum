/*
  ==============================================================================
    LFO.h

    Low Frequency Oscillator — generates slow, cyclic modulation signals.

    LFOs are the "movement" in synth sounds. They don't produce audible sound
    themselves — instead, they modulate (wiggle) other parameters over time:

    COMMON LFO USES:
    - LFO → Filter Cutoff = "wah-wah" / filter sweep effect
    - LFO → Wavetable Position = evolving, morphing timbre
    - LFO → Volume = tremolo effect
    - LFO → Pitch = vibrato effect
    - LFO → Pan = auto-panning

    Serum's LFOs are particularly powerful because:
    - They can be drawn freehand (custom shapes)
    - They can sync to DAW tempo (1/4 note, 1/8 note, etc.)
    - They support multiple trigger modes (free-running, retrigger, one-shot)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <atomic>

static constexpr int LFO_CUSTOM_POINTS = 64;

enum class LFOShape
{
    Sine,           // Smooth, natural modulation
    Triangle,       // Linear up and down
    Saw,            // Ramp up, instant reset — good for risers
    SawDown,        // Ramp down, instant reset
    Square,         // Instant on/off — choppy effect
    SampleAndHold,  // Random steps — classic "random" modulation
    Smooth          // Smoothed random — organic movement
};

enum class LFOTriggerMode
{
    FreeRunning,    // LFO runs continuously, not affected by note events
    Retrigger,      // LFO resets phase when a note is pressed
    OneShot         // LFO runs once (like an envelope), then holds at end
};

// Tempo-synced rate options (as note divisions)
enum class LFOSyncRate
{
    Off,            // Use free-running Hz rate
    Bars4,          // 4 bars
    Bars2,          // 2 bars
    Bar1,           // 1 bar
    Half,           // 1/2 note
    HalfDotted,     // Dotted 1/2
    HalfTriplet,    // 1/2 triplet
    Quarter,        // 1/4 note
    QuarterDotted,
    QuarterTriplet,
    Eighth,         // 1/8 note
    EighthDotted,
    EighthTriplet,
    Sixteenth,      // 1/16 note
    SixteenthDotted,
    SixteenthTriplet,
    ThirtySecond    // 1/32 note
};

class LFO
{
public:
    LFO();
    ~LFO() = default;

    void prepare(double sampleRate);

    // Get the next LFO value (call once per sample)
    // Returns a value between -1.0 and 1.0 (bipolar)
    float processSample();

    // Trigger/retrigger the LFO
    void noteOn();
    void noteOff();
    void reset();

    // --- Parameter Setters ---
    void setRate(float hz) { rateHz = juce::jlimit(0.01f, 50.0f, hz); }
    void setShape(LFOShape newShape) { shape = newShape; customShapeEnabled.store(false); }
    void setCustomShape(const std::array<float, LFO_CUSTOM_POINTS>& points);
    void setTriggerMode(LFOTriggerMode mode) { triggerMode = mode; }
    void setSyncRate(LFOSyncRate rate) { syncRate = rate; }
    void setDepth(float d) { depth = juce::jlimit(0.0f, 1.0f, d); }
    void setPhaseOffset(float offset) { phaseOffset = juce::jlimit(0.0f, 1.0f, offset); }
    void setSmooth(float s) { smoothing = juce::jlimit(0.0f, 1.0f, s); }

    // Set DAW tempo for synced LFO rates
    void setTempo(double bpm) { tempoBPM = bpm; }

    // Get current value for GUI visualization
    float getCurrentValue() const { return currentValue; }
    float getPhase() const { return phase; }

    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

private:
    float generateShape(float p) const;
    float getSyncedRateHz() const;

    double sampleRate = 44100.0;
    double tempoBPM = 120.0;
    bool enabled = true;

    float phase = 0.0f;
    float currentValue = 0.0f;
    float smoothedValue = 0.0f;
    bool oneShotComplete = false;

    // Random state for S&H and Smooth shapes
    float randomValue = 0.0f;
    float prevRandomValue = 0.0f;
    juce::Random random;

    // Parameters
    LFOShape shape = LFOShape::Sine;
    LFOTriggerMode triggerMode = LFOTriggerMode::Retrigger;
    LFOSyncRate syncRate = LFOSyncRate::Off;
    float rateHz = 1.0f;
    float depth = 1.0f;
    float phaseOffset = 0.0f;
    float smoothing = 0.0f;
    std::array<std::atomic<float>, LFO_CUSTOM_POINTS> customShape{};
    std::atomic<bool> customShapeEnabled { false };
};
