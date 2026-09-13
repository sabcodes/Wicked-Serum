/*
  ==============================================================================
    Arpeggiator.cpp

    Implementation of the Arpeggiator class.
    See Arpeggiator.h for detailed documentation and usage examples.

  ==============================================================================
*/

#include "Arpeggiator.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

void Arpeggiator::prepare(double inSampleRate)
{
    // Store the sample rate for later calculations
    sampleRate = inSampleRate;

    // Reset state
    heldNotes.clear();
    currentStepIndex = 0;
    stepDirection = 1;
    stepSampleCounter = 0.0;
    currentNote = -1;
    noteIsActive = false;
    gateEndSampleCounter = 0.0;
}

// ============================================================================
// MAIN PROCESS METHOD
// ============================================================================

void Arpeggiator::processBlock(const juce::MidiBuffer& midiMessages,
                               juce::MidiBuffer& midiOut,
                               int numSamples,
                               double bpm,
                               double sampleRate)
{
    // If arpeggiator is disabled, don't do anything
    if (!enabled)
    {
        midiOut = midiMessages;
        return;
    }

    // Update step duration based on current BPM
    updateStepDuration(bpm);

    // Process incoming MIDI to update which notes are held
    processMidiInput(midiMessages);

    // If no notes are held, we have nothing to arpeggiate
    if (heldNotes.empty())
    {
        // Send note off if a note was playing
        if (noteIsActive && currentNote >= 0)
        {
            addNoteOff(midiOut, 0, currentNote);
            noteIsActive = false;
        }
        return;
    }

    // ========================================================================
    // MAIN STEP LOOP - Process each sample
    // ========================================================================

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // ----- Handle note gate off (note is too old, turn it off) -----
        if (noteIsActive && gateEndSampleCounter <= 0.0)
        {
            addNoteOff(midiOut, sampleIndex, currentNote);
            noteIsActive = false;
        }

        // Advance the gate timer
        if (noteIsActive)
            gateEndSampleCounter -= 1.0;

        // ----- Check if it's time for the next step -----
        stepSampleCounter -= 1.0;

        if (stepSampleCounter <= 0.0)
        {
            // ===== TRIGGER NEW STEP =====

            // First, turn off the previous note (if still playing)
            if (noteIsActive && currentNote >= 0)
            {
                addNoteOff(midiOut, sampleIndex, currentNote);
            }

            // Get the next note to play
            currentNote = getNextNote();

            // Send MIDI Note On
            addNoteOn(midiOut, sampleIndex, currentNote, 100);
            noteIsActive = true;

            // Calculate when to turn this note off (based on gate length)
            gateEndSampleCounter = stepDurationInSamples * gateLength;

            // ===== ADVANCE TO NEXT STEP =====

            // Apply swing: if it's an even step, add a small delay
            double swingAmount = (currentStepIndex % 2 == 1) ?
                                (swing * stepDurationInSamples * 0.5) : 0.0;

            // Reset counter for next step
            stepSampleCounter = stepDurationInSamples + swingAmount;

            // Move to next step based on pattern
            currentStepIndex++;

            // Handle pattern direction (for UpDown and DownUp patterns)
            if (pattern == Pattern::UpDown || pattern == Pattern::DownUp)
            {
                int totalNotes = heldNotes.size() * octaveRange;

                // Check if we've reached the end and need to reverse direction
                if (currentStepIndex >= totalNotes - 1)
                    stepDirection = -1;
                else if (currentStepIndex <= 0)
                    stepDirection = 1;
            }
        }
    }
}

// ============================================================================
// CONTROL METHODS
// ============================================================================

void Arpeggiator::setOctaveRange(int newRange)
{
    // Clamp to valid range
    octaveRange = juce::jlimit(1, 4, newRange);
    // Reset step index since note range changed
    currentStepIndex = 0;
}

void Arpeggiator::setGateLength(float newLength)
{
    // Clamp to 10% - 100%
    gateLength = juce::jlimit(0.1f, 1.0f, newLength);
}

void Arpeggiator::setSwing(float newSwing)
{
    // Clamp to 0% - 100%
    swing = juce::jlimit(0.0f, 1.0f, newSwing);
}

void Arpeggiator::allNotesOff()
{
    heldNotes.clear();
    currentStepIndex = 0;
    stepDirection = 1;
    noteIsActive = false;
}

// ============================================================================
// INTERNAL HELPER METHODS
// ============================================================================

void Arpeggiator::updateStepDuration(double bpm)
{
    // Calculate how many samples are in one step
    // Formula: 60 seconds / BPM = duration of quarter note in seconds
    //          duration_in_seconds * sampleRate = duration_in_samples
    //          multiply by rate multiplier to get actual step duration

    double quarterNoteDurationSeconds = 60.0 / bpm;
    float rateMultiplier = getRateMultiplier();

    stepDurationInSamples = quarterNoteDurationSeconds * sampleRate * rateMultiplier;
}

float Arpeggiator::getRateMultiplier() const
{
    // How many quarter notes per step?
    // 1/4 note = 1 quarter note = 1.0 multiplier
    // 1/8 note = half quarter = 0.5 multiplier
    // 1/16 note = quarter of a quarter = 0.25 multiplier
    // etc.

    switch (rate)
    {
        case Rate::OneQuarter:
            return 1.0f;

        case Rate::OneEighth:
            return 0.5f;

        case Rate::OneEighthTriplet:
            // 3 triplets fit in the time of 2 normal notes
            return (0.5f * 2.0f) / 3.0f;  // 0.333...

        case Rate::OneSixteenth:
            return 0.25f;

        case Rate::OneSixteenthTriplet:
            // 3 triplets fit in the time of 2 normal notes
            return (0.25f * 2.0f) / 3.0f;  // 0.1666...

        case Rate::OneThirtysecond:
            return 0.125f;

        default:
            return 0.5f;  // Default to 1/8
    }
}

int Arpeggiator::getNextNote()
{
    // Handle empty held notes (shouldn't happen, but be safe)
    if (heldNotes.empty())
        return 60;  // Middle C fallback

    int noteIndex = 0;  // Which note in our held notes list

    switch (pattern)
    {
        case Pattern::Up:
        {
            // Simple ascending: wrap around when reaching the end
            noteIndex = currentStepIndex % (heldNotes.size() * octaveRange);
            break;
        }

        case Pattern::Down:
        {
            // Descending: count backwards
            int totalNotes = heldNotes.size() * octaveRange;
            noteIndex = totalNotes - 1 - (currentStepIndex % totalNotes);
            break;
        }

        case Pattern::UpDown:
        {
            // Ascending then descending (bounce)
            // Imagine the notes laid out: 0, 1, 2, 1, 0, 1, 2, 1, 0...
            int totalNotes = heldNotes.size() * octaveRange;

            // Protect with one held note: just loop through the single note
            if (totalNotes == 1)
            {
                noteIndex = 0;
            }
            else
            {
                int cycle = (totalNotes - 1) * 2;  // Full cycle length
                int posInCycle = currentStepIndex % cycle;

                if (posInCycle < totalNotes)
                {
                    noteIndex = posInCycle;  // Going up
                }
                else
                {
                    noteIndex = cycle - posInCycle;  // Going down
                }
            }
            break;
        }

        case Pattern::DownUp:
        {
            // Descending then ascending (bounce)
            int totalNotes = heldNotes.size() * octaveRange;
            int cycle = (totalNotes - 1) * 2;
            int posInCycle = currentStepIndex % cycle;

            if (posInCycle < totalNotes)
            {
                noteIndex = totalNotes - 1 - posInCycle;  // Going down
            }
            else
            {
                noteIndex = posInCycle - totalNotes + 1;  // Going up
            }
            break;
        }

        case Pattern::Random:
        {
            // Pick a random note from held notes
            // (Note: this is slightly biased if octaveRange > 1, but that's okay for a synth)
            int totalNotes = heldNotes.size() * octaveRange;
            noteIndex = rand() % totalNotes;
            break;
        }

        case Pattern::Order:
        {
            // Play in order held (just wrap around, no octaves)
            noteIndex = currentStepIndex % heldNotes.size();
            // For "Order" pattern, ignore octave range
            return heldNotes[noteIndex];
        }

        default:
            noteIndex = 0;
    }

    // Convert index (0 to totalNotes-1) to actual MIDI note with octaves
    return getNoteWithOctave(noteIndex);
}

int Arpeggiator::getNoteWithOctave(int index) const
{
    // index: 0 to (heldNotes.size() * octaveRange - 1)
    // We want to spread the notes across multiple octaves

    if (heldNotes.empty())
        return 60;  // Middle C fallback

    int notesPerOctave = static_cast<int>(heldNotes.size());
    int octaveNumber = index / notesPerOctave;  // Which octave (0, 1, 2, etc.)
    int noteInOctave = index % notesPerOctave;  // Which note within octave

    // Each octave is 12 semitones
    int midiNote = heldNotes[noteInOctave] + (octaveNumber * 12);

    // Clamp to valid MIDI range (0-127)
    return juce::jlimit(0, 127, midiNote);
}

void Arpeggiator::addNoteOn(juce::MidiBuffer& midiOut,
                            int samplePosition,
                            int midiNote,
                            int velocity)
{
    // Create a MIDI Note On message
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)velocity);
    midiOut.addEvent(noteOn, samplePosition);
}

void Arpeggiator::addNoteOff(juce::MidiBuffer& midiOut,
                             int samplePosition,
                             int midiNote)
{
    // Create a MIDI Note Off message
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, midiNote);
    midiOut.addEvent(noteOff, samplePosition);
}

void Arpeggiator::processMidiInput(const juce::MidiBuffer& midiMessages)
{
    // Go through all incoming MIDI messages and track which notes are held

    for (const auto metadata : midiMessages)
    {
        const juce::MidiMessage& msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            int noteNumber = msg.getNoteNumber();

            // Add note if not already in list (avoid duplicates)
            if (std::find(heldNotes.begin(), heldNotes.end(), noteNumber) == heldNotes.end())
            {
                heldNotes.push_back(noteNumber);
                // Keep the list sorted so Up/Down patterns work correctly
                std::sort(heldNotes.begin(), heldNotes.end());
            }
        }

        if (msg.isNoteOff())
        {
            int noteNumber = msg.getNoteNumber();

            // Remove note from held list
            auto it = std::find(heldNotes.begin(), heldNotes.end(), noteNumber);
            if (it != heldNotes.end())
            {
                heldNotes.erase(it);
            }
        }
    }
}
