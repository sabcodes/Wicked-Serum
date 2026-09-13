/*
  ==============================================================================
    WavetableOscillator.h

    The heart of the synth — a wavetable oscillator inspired by Serum.

    HOW WAVETABLE SYNTHESIS WORKS:
    - A "wavetable" is a collection of single-cycle waveforms (called "frames")
    - The oscillator reads through one frame at audio rate to produce sound
    - You can smoothly morph between frames using the "wavetable position" knob
    - This creates evolving, dynamic timbres that static waveforms can't achieve

    Think of it like a flipbook of waveforms — each page is a slightly different
    shape, and you can smoothly animate between them.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>
#include <cmath>

// Number of samples in each wavetable frame (single-cycle waveform)
static constexpr int WAVETABLE_SIZE = 2048;

// Maximum number of frames in a wavetable
static constexpr int MAX_WAVETABLE_FRAMES = 256;

// Maximum unison voices per oscillator (like Serum's up to 16)
static constexpr int MAX_UNISON_VOICES = 16;

/**
 * Warp modes — different ways to distort/transform the wavetable playback.
 * These are inspired by Serum's warp modes.
 */
enum class WarpMode
{
    None,       // No warping — play wavetable as-is
    Sync,       // Hard sync effect — creates harmonics
    Bend,       // Bend the phase — pushes waveform left/right
    PWM,        // Pulse Width Modulation
    Asymmetric, // Different warp for positive/negative halves
    FM,         // Frequency Modulation from other oscillator
    RM,         // Ring Modulation with other oscillator
    Quantize    // Bit-crush / sample rate reduction
};

/**
 * A single wavetable — a collection of waveform frames that can be morphed between.
 */
struct Wavetable
{
    juce::String name;
    std::vector<std::array<float, WAVETABLE_SIZE>> frames;  // Each frame is one waveform cycle
    int numFrames = 0;

    // Generate a basic wavetable from standard waveforms
    void generateBasicWavetable();

    // Generate a wavetable that morphs between standard shapes
    void generateAnalogWavetable();

    // Generate spectral wavetable (additive harmonics)
    void generateSpectralWavetable();

    // Generate a wavetable from user-drawn waveform
    void generateFromHarmonics(const std::vector<float>& harmonicAmplitudes);

    // Replace one frame without collapsing the rest of the wavetable.
    bool replaceFrame(int frameIndex, const std::array<float, WAVETABLE_SIZE>& frame);
};

/**
 * WavetableOscillator — produces audio by reading through wavetable frames.
 *
 * KEY PARAMETERS (like Serum's oscillator controls):
 *   - wavetablePosition: Which frame to read (0.0 = first, 1.0 = last). Morph between frames.
 *   - detune: Pitch offset in semitones
 *   - fineTune: Fine pitch offset in cents (1/100th of a semitone)
 *   - unisonVoices: Stack multiple detuned copies (1-16)
 *   - unisonDetune: How far apart unison voices are spread
 *   - unisonBlend: Mix between center voice and spread voices
 *   - level: Output volume (0.0 to 1.0)
 *   - pan: Stereo position (-1.0 left, 0.0 center, 1.0 right)
 *   - octave: Octave shift (-3 to +3)
 *   - warpMode: Type of waveform warping
 *   - warpAmount: Intensity of the warp effect
 */
class WavetableOscillator
{
public:
    WavetableOscillator();
    ~WavetableOscillator() = default;

    // Call this when the sample rate changes (e.g., when plugin loads)
    void prepare(double sampleRate);

    // Generate one sample of audio for a given MIDI note frequency
    // Returns a stereo pair [left, right]
    std::pair<float, float> processSample(float frequency);

    // Reset the oscillator (e.g., when a new note starts)
    void reset();

    // --- Wavetable Management ---
    void setWavetable(const Wavetable& newWavetable);
    void loadFactoryWavetable(int index);

    // --- Parameter Setters ---
    void setWavetablePosition(float position);   // 0.0 to 1.0
    void setDetune(float semitones);             // -24 to +24
    void setFineTune(float cents);               // -100 to +100
    void setUnisonVoices(int numVoices);          // 1 to 16
    void setUnisonDetune(float detuneCents);     // 0 to 100
    void setUnisonBlend(float blend);            // 0.0 to 1.0
    void setLevel(float newLevel);               // 0.0 to 1.0
    void setPan(float newPan);                   // -1.0 to 1.0
    void setOctave(int octaveShift);             // -3 to +3
    void setWarpMode(WarpMode mode);
    void setWarpAmount(float amount);            // 0.0 to 1.0
    void setPhaseRandomization(float amount);    // 0.0 to 1.0

    // Get current wavetable frame data for GUI visualization
    const std::array<float, WAVETABLE_SIZE>& getCurrentFrame() const;
    float getWavetablePosition() const { return wavetablePosition; }

    // Enable/disable this oscillator
    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

private:
    // Read a sample from the wavetable with interpolation between frames
    float readWavetable(float phase, float wtPos) const;

    // Apply warp effect to the phase
    float applyWarp(float phase, float warpAmt) const;

    // Generate factory wavetables
    void initFactoryWavetables();

    // --- State ---
    double sampleRate = 44100.0;
    bool enabled = true;

    // Current wavetable
    Wavetable currentWavetable;

    // Phase accumulators for each unison voice
    std::array<float, MAX_UNISON_VOICES> phases{};
    std::array<float, MAX_UNISON_VOICES> phaseIncrements{};

    // --- Parameters ---
    float wavetablePosition = 0.0f;  // Which frame in the wavetable (0-1)
    float detuneSemitones = 0.0f;
    float fineTuneCents = 0.0f;
    int unisonVoices = 1;
    float unisonDetuneCents = 20.0f;
    float unisonBlend = 1.0f;
    float level = 1.0f;
    float pan = 0.0f;
    int octaveShift = 0;
    WarpMode warpMode = WarpMode::None;
    float warpAmount = 0.0f;
    float phaseRandomization = 0.0f;

    // Factory wavetables (basic shapes everyone needs)
    std::vector<Wavetable> factoryWavetables;

    // Interpolated frame cache for visualization
    mutable std::array<float, WAVETABLE_SIZE> currentFrameCache{};
    mutable bool frameCacheDirty = true;
};
