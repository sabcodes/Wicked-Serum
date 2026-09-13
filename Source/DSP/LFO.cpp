/*
  ==============================================================================
    LFO.cpp
  ==============================================================================
*/

#include "LFO.h"

LFO::LFO()
{
    for (int i = 0; i < LFO_CUSTOM_POINTS; ++i)
    {
        const float phase = static_cast<float>(i) / static_cast<float>(LFO_CUSTOM_POINTS);
        customShape[static_cast<size_t>(i)].store(0.5f + 0.5f * std::sin(
            phase * juce::MathConstants<float>::twoPi));
    }
}

void LFO::setCustomShape(const std::array<float, LFO_CUSTOM_POINTS>& points)
{
    for (int i = 0; i < LFO_CUSTOM_POINTS; ++i)
        customShape[static_cast<size_t>(i)].store(juce::jlimit(0.0f, 1.0f,
            points[static_cast<size_t>(i)]), std::memory_order_relaxed);
    customShapeEnabled.store(true, std::memory_order_release);
}

void LFO::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
}

void LFO::noteOn()
{
    if (triggerMode == LFOTriggerMode::Retrigger || triggerMode == LFOTriggerMode::OneShot)
    {
        phase = 0.0f;
        oneShotComplete = false;
    }
}

void LFO::noteOff()
{
    // LFOs typically don't respond to note off
    // (they keep running during release stage)
}

void LFO::reset()
{
    phase = 0.0f;
    currentValue = 0.0f;
    smoothedValue = 0.0f;
    oneShotComplete = false;
}

float LFO::processSample()
{
    if (!enabled)
        return 0.0f;

    // One-shot mode: hold at final value once complete
    if (triggerMode == LFOTriggerMode::OneShot && oneShotComplete)
        return currentValue * depth;

    // Calculate effective rate (free-running or tempo-synced)
    float effectiveRate = (syncRate == LFOSyncRate::Off) ? rateHz : getSyncedRateHz();

    // Advance phase
    float phaseIncrement = effectiveRate / static_cast<float>(sampleRate);
    phase += phaseIncrement;

    // Handle phase wrapping
    if (phase >= 1.0f)
    {
        if (triggerMode == LFOTriggerMode::OneShot)
        {
            phase = 1.0f;
            oneShotComplete = true;
        }
        else
        {
            phase -= 1.0f;
        }

        // Update random values at phase wrap (for S&H)
        prevRandomValue = randomValue;
        randomValue = random.nextFloat() * 2.0f - 1.0f;
    }

    // Apply phase offset
    float effectivePhase = std::fmod(phase + phaseOffset, 1.0f);

    // Generate shape
    float rawValue = generateShape(effectivePhase);

    // Apply smoothing (low-pass filter on the LFO output)
    if (smoothing > 0.001f)
    {
        float smoothCoeff = std::exp(-1.0f / (smoothing * 1000.0f * static_cast<float>(sampleRate) / effectiveRate));
        smoothedValue = smoothedValue * smoothCoeff + rawValue * (1.0f - smoothCoeff);
        currentValue = smoothedValue;
    }
    else
    {
        currentValue = rawValue;
    }

    return currentValue * depth;
}

float LFO::generateShape(float p) const
{
    // Custom editor points are stored as 0..1 and converted to bipolar output.
    if (customShapeEnabled.load(std::memory_order_acquire))
    {
        const float position = p * static_cast<float>(LFO_CUSTOM_POINTS);
        const int indexA = static_cast<int>(position) % LFO_CUSTOM_POINTS;
        const int indexB = (indexA + 1) % LFO_CUSTOM_POINTS;
        const float fraction = position - std::floor(position);
        const float a = customShape[static_cast<size_t>(indexA)].load(std::memory_order_relaxed);
        const float b = customShape[static_cast<size_t>(indexB)].load(std::memory_order_relaxed);
        return ((a + (b - a) * fraction) * 2.0f) - 1.0f;
    }

    // All built-in shapes output in range [-1.0, 1.0]
    switch (shape)
    {
        case LFOShape::Sine:
            return std::sin(p * juce::MathConstants<float>::twoPi);

        case LFOShape::Triangle:
            return 2.0f * std::abs(2.0f * p - 1.0f) - 1.0f;

        case LFOShape::Saw:
            return 2.0f * p - 1.0f;

        case LFOShape::SawDown:
            return 1.0f - 2.0f * p;

        case LFOShape::Square:
            return p < 0.5f ? 1.0f : -1.0f;

        case LFOShape::SampleAndHold:
            return randomValue;

        case LFOShape::Smooth:
        {
            // Smoothly interpolate between random values using cosine interpolation
            float t = (1.0f - std::cos(p * juce::MathConstants<float>::pi)) * 0.5f;
            return prevRandomValue * (1.0f - t) + randomValue * t;
        }

        default:
            return 0.0f;
    }
}

float LFO::getSyncedRateHz() const
{
    // Convert tempo-synced note divisions to Hz
    // Formula: Hz = BPM / (60 * beats_per_cycle)

    float beatsPerCycle = 4.0f; // Default: 1 bar in 4/4

    switch (syncRate)
    {
        case LFOSyncRate::Bars4:             beatsPerCycle = 16.0f; break;
        case LFOSyncRate::Bars2:             beatsPerCycle = 8.0f;  break;
        case LFOSyncRate::Bar1:              beatsPerCycle = 4.0f;  break;
        case LFOSyncRate::Half:              beatsPerCycle = 2.0f;  break;
        case LFOSyncRate::HalfDotted:        beatsPerCycle = 3.0f;  break;
        case LFOSyncRate::HalfTriplet:       beatsPerCycle = 4.0f / 3.0f; break;
        case LFOSyncRate::Quarter:           beatsPerCycle = 1.0f;  break;
        case LFOSyncRate::QuarterDotted:     beatsPerCycle = 1.5f;  break;
        case LFOSyncRate::QuarterTriplet:    beatsPerCycle = 2.0f / 3.0f; break;
        case LFOSyncRate::Eighth:            beatsPerCycle = 0.5f;  break;
        case LFOSyncRate::EighthDotted:      beatsPerCycle = 0.75f; break;
        case LFOSyncRate::EighthTriplet:     beatsPerCycle = 1.0f / 3.0f; break;
        case LFOSyncRate::Sixteenth:         beatsPerCycle = 0.25f; break;
        case LFOSyncRate::SixteenthDotted:   beatsPerCycle = 0.375f; break;
        case LFOSyncRate::SixteenthTriplet:  beatsPerCycle = 1.0f / 6.0f; break;
        case LFOSyncRate::ThirtySecond:      beatsPerCycle = 0.125f; break;
        default: break;
    }

    return static_cast<float>(tempoBPM) / (60.0f * beatsPerCycle);
}
