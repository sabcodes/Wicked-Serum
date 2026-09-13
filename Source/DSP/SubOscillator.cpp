/*
  ==============================================================================
    SubOscillator.cpp
  ==============================================================================
*/

#include "SubOscillator.h"

void SubOscillator::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
}

void SubOscillator::reset()
{
    phase = 0.0f;
}

float SubOscillator::processSample(float frequency)
{
    if (!enabled)
        return 0.0f;

    // Apply octave shift (negative = lower)
    float freq = frequency * std::pow(2.0f, static_cast<float>(octaveShift));

    // Advance phase
    float phaseIncrement = freq / static_cast<float>(sampleRate);
    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;

    const float twoPi = juce::MathConstants<float>::twoPi;
    float sample = 0.0f;
    float polyBLEPCorrection = 0.0f;

    switch (shape)
    {
        case SubOscShape::Sine:
            sample = std::sin(twoPi * phase);
            break;

        case SubOscShape::Triangle:
            sample = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
            // PolyBLEP correction at discontinuities (at 0 and 0.5)
            if (phase < phaseIncrement)
                polyBLEPCorrection += polyBLEP(phase / phaseIncrement);
            if (phase < (phaseIncrement + 0.5f) && lastPhase >= 0.5f)
                polyBLEPCorrection += polyBLEP((phase - 0.5f) / phaseIncrement);
            sample += polyBLEPCorrection;
            break;

        case SubOscShape::Saw:
            sample = 2.0f * phase - 1.0f;
            // PolyBLEP correction at phase wraparound (discontinuity at 0/1)
            polyBLEPCorrection = polyBLEP(phase / phaseIncrement);
            sample += polyBLEPCorrection;
            break;

        case SubOscShape::Square:
            sample = phase < 0.5f ? 1.0f : -1.0f;
            // PolyBLEP correction at 0 and 0.5
            if (phase < phaseIncrement)
                polyBLEPCorrection += polyBLEP(phase / phaseIncrement);
            if (phase < (phaseIncrement + 0.5f) && lastPhase >= 0.5f)
                polyBLEPCorrection += polyBLEP((phase - 0.5f) / phaseIncrement);
            sample += polyBLEPCorrection * 2.0f;
            break;

        case SubOscShape::Pulse25:
            sample = phase < 0.25f ? 1.0f : -1.0f;
            // PolyBLEP correction at 0 and 0.25
            if (phase < phaseIncrement)
                polyBLEPCorrection += polyBLEP(phase / phaseIncrement);
            if (phase < (phaseIncrement + 0.25f) && lastPhase >= 0.25f)
                polyBLEPCorrection += polyBLEP((phase - 0.25f) / phaseIncrement);
            sample += polyBLEPCorrection * 2.0f;
            break;

        case SubOscShape::Pulse12:
            sample = phase < 0.125f ? 1.0f : -1.0f;
            // PolyBLEP correction at 0 and 0.125
            if (phase < phaseIncrement)
                polyBLEPCorrection += polyBLEP(phase / phaseIncrement);
            if (phase < (phaseIncrement + 0.125f) && lastPhase >= 0.125f)
                polyBLEPCorrection += polyBLEP((phase - 0.125f) / phaseIncrement);
            sample += polyBLEPCorrection * 2.0f;
            break;
    }

    lastPhase = phase;
    return sample * level;
}

float SubOscillator::polyBLEP(float phase)
{
    // PolyBLEP (polynomial band-limited step) correction function
    // Reduces aliasing at discontinuities in waveforms
    // phase: phase distance from discontinuity normalized by phase increment

    if (phase < -1.0f || phase > 1.0f)
        return 0.0f;

    if (phase > 0.0f)
    {
        // Right edge of discontinuity
        float t = phase;
        return 0.5f * t * t;
    }
    else
    {
        // Left edge of discontinuity
        float t = phase + 1.0f;
        return -0.5f * t * t;
    }
}
