/*
  ==============================================================================
    Arpeggiator.h

    ARPEGGIATOR & STEP SEQUENCER

    The arpeggiator takes MIDI notes you hold down and plays them in a sequence.
    Think of it like a musical phrase player triggered by your held notes.

    HOW IT WORKS:

    1. You hold down multiple MIDI notes (e.g., C, E, G for a C major chord)
    2. The arpeggiator stores those notes in a sorted list
    3. On each step (tempo-synced), it picks a note and sends a MIDI Note On
    4. After gate_length (e.g., 80% of step duration), it sends a MIDI Note Off
    5. Then it moves to the next note based on the pattern (Up, Down, Random, etc.)
    6. Process repeats as long as you hold notes

    PATTERNS:
    - Up:      C, E, G, C, E, G... (lowest to highest, loop back)
    - Down:    G, E, C, G, E, C... (highest to lowest, loop back)
    - UpDown:  C, E, G, E, C, E, G, E... (goes up, then down, bounces)
    - DownUp:  G, E, C, E, G, E, C, E... (goes down, then up, bounces)
    - Random:  E, C, G, E, C, G... (random picks from held notes)
    - Order:   Plays in the order you pressed the keys (first to last)

    TEMPO SYNC:
    - 1/4, 1/8, 1/8T (triplet), 1/16, 1/16T, 1/32 note divisions
    - Automatically syncs to the DAW's BPM

    OCTAVE RANGE:
    - Play the pattern across multiple octaves (1-4)
    - 1 octave: just the notes you hold
    - 2 octaves: C, E, G, C+12, E+12, G+12...
    - 4 octaves: expands further

    GATE LENGTH:
    - How long each note sustains (10-100% of step duration)
    - 50% = staccato (short, punchy)
    - 100% = legato (no gaps between notes)

    SWING:
    - Delays every second step by a small amount (0-100%)
    - Creates a "human" groovy feel

    USAGE:

    Arpeggiator arp;
    arp.prepare(sampleRate);
    arp.setEnabled(true);
    arp.setPattern(Arpeggiator::Pattern::Up);
    arp.setRate(Arpeggiator::Rate::OneEighth);
    arp.setOctaveRange(2);
    arp.setGateLength(0.8f);
    arp.setSwing(0.3f);

    // In your processBlock:
    juce::MidiBuffer midiOut;
    arp.processBlock(midiIn, midiOut, numSamples, bpm, sampleRate);
    midiOut is now filled with arpeggiated notes

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <cstdlib>

class Arpeggiator
{
public:
    // ========================================================================
    // ENUMS - Define the different modes and settings
    // ========================================================================

    /// Pattern: how the arpeggiator steps through held notes
    enum class Pattern
    {
        Up,      // C, E, G, C, E, G... (ascending, loop)
        Down,    // G, E, C, G, E, C... (descending, loop)
        UpDown,  // C, E, G, E, C, E, G, E... (bounce up then down)
        DownUp,  // G, E, C, E, G, E, C, E... (bounce down then up)
        Random,  // Random picks from held notes
        Order    // In the order you pressed them
    };

    /// Rate: tempo-synced step duration (note divisions)
    enum class Rate
    {
        OneQuarter,    // 1/4 note (slowest)
        OneEighth,     // 1/8 note
        OneEighthTriplet, // 1/8 triplet (3 notes in the time of 2)
        OneSixteenth,  // 1/16 note
        OneSixteenthTriplet,
        OneThirtysecond // 1/32 note (fastest)
    };

    // ========================================================================
    // CONSTRUCTOR / DESTRUCTOR
    // ========================================================================

    Arpeggiator() = default;
    ~Arpeggiator() = default;

    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    /// Call this once at startup or when sample rate changes
    void prepare(double sampleRate);

    // ========================================================================
    // MAIN PROCESS METHOD - Do the arpeggiating!
    // ========================================================================

    /// Process a block of audio with MIDI
    /// midiMessages: input MIDI buffer (contains note on/off for held keys)
    /// midiOut: output MIDI buffer (will be filled with arpeggiated notes)
    /// numSamples: how many audio samples to process
    /// bpm: current BPM from the DAW
    /// sampleRate: audio sample rate
    void processBlock(const juce::MidiBuffer& midiMessages,
                     juce::MidiBuffer& midiOut,
                     int numSamples,
                     double bpm,
                     double sampleRate);

    // ========================================================================
    // CONTROL METHODS - Change settings
    // ========================================================================

    /// Turn the arpeggiator on/off
    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

    /// Choose the pattern (Up, Down, UpDown, etc.)
    void setPattern(Pattern newPattern) { pattern = newPattern; }
    Pattern getPattern() const { return pattern; }

    /// Set the step speed (1/4, 1/8, 1/16, etc.)
    void setRate(Rate newRate) { rate = newRate; }
    Rate getRate() const { return rate; }

    /// How many octaves to span (1 = just the notes, 4 = spread across 4 octaves)
    void setOctaveRange(int newRange);
    int getOctaveRange() const { return octaveRange; }

    /// Gate length: 0.1 (10%, staccato) to 1.0 (100%, legato)
    void setGateLength(float newLength);
    float getGateLength() const { return gateLength; }

    /// Swing: 0.0 (no swing) to 1.0 (maximum swing)
    /// Adds a groove by delaying every other step
    void setSwing(float newSwing);
    float getSwing() const { return swing; }

    // ========================================================================
    // UTILITY
    // ========================================================================

    /// How many notes are currently held?
    int getHeldNoteCount() const { return static_cast<int>(heldNotes.size()); }

    /// Get the current step index
    int getCurrentStepIndex() const { return currentStepIndex; }

    /// Clear all held notes
    void allNotesOff();

private:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    bool enabled = true;
    Pattern pattern = Pattern::Up;
    Rate rate = Rate::OneEighth;
    int octaveRange = 1;           // 1-4 octaves
    float gateLength = 0.8f;       // 10-100% of step duration
    float swing = 0.0f;            // 0-100% swing

    double sampleRate = 44100.0;

    // Held notes storage: sorted vector of MIDI note numbers (0-127)
    // We keep them sorted so "Up" and "Down" patterns work correctly
    std::vector<int> heldNotes;

    // Step sequencer state
    int currentStepIndex = 0;       // Which note in the pattern are we on?
    int stepDirection = 1;          // For UpDown/DownUp: 1 = ascending, -1 = descending

    double stepSampleCounter = 0.0; // Counts samples to know when to trigger next step
    double stepDurationInSamples = 0.0; // How many samples per step (based on BPM + Rate)

    double gateEndSampleCounter = 0.0; // When to send note off for current note
    int currentNote = -1;           // Which MIDI note is currently playing
    bool noteIsActive = false;      // Is a note currently sounding?

    // ========================================================================
    // INTERNAL HELPER METHODS
    // ========================================================================

    /// Calculate step duration based on BPM and rate setting
    void updateStepDuration(double bpm);

    /// Get the MIDI note number to play at the current step
    /// Takes into account: pattern, octave range, held notes
    int getNextNote();

    /// Add a MIDI note on event to the output buffer
    void addNoteOn(juce::MidiBuffer& midiOut, int samplePosition, int midiNote, int velocity = 100);

    /// Add a MIDI note off event to the output buffer
    void addNoteOff(juce::MidiBuffer& midiOut, int samplePosition, int midiNote);

    /// Convert a rate enum to a multiplier (how many 1/16ths per step)
    float getRateMultiplier() const;

    /// Get MIDI note at index, accounting for octave range
    /// index: 0 to (heldNotes.size() * octaveRange - 1)
    int getNoteWithOctave(int index) const;

    /// Process incoming MIDI to update heldNotes list
    void processMidiInput(const juce::MidiBuffer& midiMessages);
};
