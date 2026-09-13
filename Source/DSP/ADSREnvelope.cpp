/*
  ==============================================================================
    ADSREnvelope.cpp
  ==============================================================================
*/

#include "ADSREnvelope.h"

void ADSREnvelope::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    recalculate();
}

void ADSREnvelope::noteOn()
{
    stage = Stage::Attack;
    stageProgress = 0.0f;
    // Don't reset currentValue — this allows retriggering without clicks
    // (the attack will smoothly rise from wherever it currently is)
}

void ADSREnvelope::noteOff()
{
    if (stage != Stage::Idle)
    {
        stage = Stage::Release;
        stageProgress = 0.0f;
        releaseStartValue = currentValue; // Release from current level
    }
}

void ADSREnvelope::reset()
{
    stage = Stage::Idle;
    currentValue = 0.0f;
    stageProgress = 0.0f;
}

float ADSREnvelope::processSample()
{
    switch (stage)
    {
        case Stage::Idle:
            currentValue = 0.0f;
            break;

        case Stage::Attack:
        {
            stageProgress += attackIncrement;

            if (stageProgress >= 1.0f)
            {
                // Attack complete — move to decay
                currentValue = 1.0f;
                stage = Stage::Decay;
                stageProgress = 0.0f;
            }
            else
            {
                // Apply curve to the linear ramp
                currentValue = applyCurve(stageProgress, attackCurve);
            }
            break;
        }

        case Stage::Decay:
        {
            stageProgress += decayIncrement;

            if (stageProgress >= 1.0f)
            {
                // Decay complete — hold at sustain
                currentValue = sustainLevel;
                stage = Stage::Sustain;
                stageProgress = 0.0f;
            }
            else
            {
                // Interpolate from 1.0 down to sustain level
                float shaped = applyCurve(stageProgress, decayCurve);
                currentValue = 1.0f - shaped * (1.0f - sustainLevel);
            }
            break;
        }

        case Stage::Sustain:
            // Hold at sustain level until note off
            currentValue = sustainLevel;
            break;

        case Stage::Release:
        {
            stageProgress += releaseIncrement;

            if (stageProgress >= 1.0f)
            {
                // Release complete — envelope is done
                currentValue = 0.0f;
                stage = Stage::Idle;
            }
            else
            {
                // Interpolate from release start value down to 0
                float shaped = applyCurve(stageProgress, releaseCurve);
                currentValue = releaseStartValue * (1.0f - shaped);
            }
            break;
        }
    }

    return currentValue;
}

void ADSREnvelope::recalculate()
{
    // Calculate how much to advance stageProgress per sample
    // stageProgress goes from 0 to 1 over the duration of each stage

    attackIncrement  = 1.0f / static_cast<float>(attackTime * sampleRate);
    decayIncrement   = 1.0f / static_cast<float>(decayTime * sampleRate);
    releaseIncrement = 1.0f / static_cast<float>(releaseTime * sampleRate);
}

float ADSREnvelope::applyCurve(float linearValue, float curve) const
{
    // Curve shaping for natural-sounding envelopes
    // curve = 0: linear (straight line)
    // curve < 0: exponential (fast start, slow end) — natural for decay/release
    // curve > 0: logarithmic (slow start, fast end) — can be nice for attack

    if (std::abs(curve) < 0.01f)
        return linearValue; // Linear — no shaping needed

    if (curve < 0.0f)
    {
        // Exponential curve: y = (e^(x*k) - 1) / (e^k - 1)
        float k = -curve * 5.0f; // Scale factor
        return (std::exp(linearValue * k) - 1.0f) / (std::exp(k) - 1.0f);
    }
    else
    {
        // Logarithmic curve: y = log(1 + x*(e^k - 1)) / k
        float k = curve * 5.0f;
        return std::log(1.0f + linearValue * (std::exp(k) - 1.0f)) / k;
    }
}
