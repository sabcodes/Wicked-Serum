/*
  ==============================================================================
    ADSREnvelope.h

    ADSR Envelope — controls how a parameter changes over time when a note plays.

    THE 4 STAGES OF ADSR:

    1. ATTACK (A):  How quickly the sound reaches full volume after you press a key
                    Short = percussive (pluck, hit). Long = gradual fade-in (pad, strings)

    2. DECAY (D):   How quickly the sound drops from peak to sustain level
                    Short = snappy. Long = slow fade to sustain.

    3. SUSTAIN (S): The level the sound holds while you keep the key pressed
                    This is a LEVEL (0-1), not a time! High = organ-like. Low = plucky.

    4. RELEASE (R): How quickly the sound fades out after you release the key
                    Short = stops immediately. Long = fades out (reverb-like tail)

    Serum typically has 3 envelopes:
    - ENV 1: Controls volume (amplitude) — always routed to VCA
    - ENV 2: Free envelope — often routed to filter cutoff
    - ENV 3: Free envelope — route to anything via mod matrix
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

class ADSREnvelope
{
public:
    ADSREnvelope() = default;
    ~ADSREnvelope() = default;

    void prepare(double sampleRate);

    // Get the next envelope value (call once per sample)
    float processSample();

    // Trigger the envelope (note on)
    void noteOn();

    // Release the envelope (note off)
    void noteOff();

    // Force envelope to idle state
    void reset();

    // Is the envelope completely finished? (Used to know when voice can be freed)
    bool isActive() const { return stage != Stage::Idle; }

    // Get current envelope value (0.0 to 1.0)
    float getCurrentValue() const { return currentValue; }

    // --- Parameter Setters (all times in seconds) ---
    void setAttack(float seconds)   { attackTime = juce::jlimit(0.001f, 10.0f, seconds); recalculate(); }
    void setDecay(float seconds)    { decayTime = juce::jlimit(0.001f, 10.0f, seconds); recalculate(); }
    void setSustain(float level)    { sustainLevel = juce::jlimit(0.0f, 1.0f, level); }
    void setRelease(float seconds)  { releaseTime = juce::jlimit(0.001f, 30.0f, seconds); recalculate(); }

    // Envelope curve shape: 0 = linear, negative = exponential (natural), positive = logarithmic
    void setAttackCurve(float curve)  { attackCurve = juce::jlimit(-1.0f, 1.0f, curve); }
    void setDecayCurve(float curve)   { decayCurve = juce::jlimit(-1.0f, 1.0f, curve); }
    void setReleaseCurve(float curve) { releaseCurve = juce::jlimit(-1.0f, 1.0f, curve); }

    // Getters for GUI
    float getAttack() const  { return attackTime; }
    float getDecay() const   { return decayTime; }
    float getSustain() const { return sustainLevel; }
    float getRelease() const { return releaseTime; }

private:
    enum class Stage
    {
        Idle,       // Envelope is not running
        Attack,     // Rising from 0 to 1
        Decay,      // Falling from 1 to sustain level
        Sustain,    // Holding at sustain level
        Release     // Falling from current level to 0
    };

    void recalculate();

    // Apply curve shaping to a linear ramp
    float applyCurve(float linearValue, float curve) const;

    double sampleRate = 44100.0;

    Stage stage = Stage::Idle;
    float currentValue = 0.0f;
    float stageProgress = 0.0f;     // 0.0 to 1.0 within current stage
    float releaseStartValue = 0.0f; // Level when release was triggered

    // Parameters
    float attackTime = 0.01f;    // seconds
    float decayTime = 0.1f;      // seconds
    float sustainLevel = 0.7f;   // 0 to 1
    float releaseTime = 0.3f;    // seconds

    // Curve shapes (0 = linear, -1 = exponential, 1 = logarithmic)
    float attackCurve = 0.0f;
    float decayCurve = -0.5f;    // Slightly exponential by default (natural)
    float releaseCurve = -0.5f;

    // Precomputed increments (how much stageProgress advances per sample)
    float attackIncrement = 0.0f;
    float decayIncrement = 0.0f;
    float releaseIncrement = 0.0f;
};
