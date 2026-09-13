/*
  ==============================================================================
    SynthFilter.cpp

    Multi-mode filter implementation with SVF, ladder, comb, and formant types.
  ==============================================================================
*/

#include "SynthFilter.h"

void SynthFilter::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
    updateCoefficients();
}

void SynthFilter::reset()
{
    // Mono state
    svfIc1eq = 0.0f;
    svfIc2eq = 0.0f;
    std::fill(std::begin(ladderState), std::end(ladderState), 0.0f);
    combBuffer.fill(0.0f);
    combWriteIndex = 0;

    // Stereo state
    svfIc1eqL = 0.0f;
    svfIc2eqL = 0.0f;
    svfIc1eqR = 0.0f;
    svfIc2eqR = 0.0f;
    std::fill(std::begin(ladderStateL), std::end(ladderStateL), 0.0f);
    std::fill(std::begin(ladderStateR), std::end(ladderStateR), 0.0f);
    combBufferL.fill(0.0f);
    combBufferR.fill(0.0f);
    combWriteIndexL = 0;
    combWriteIndexR = 0;

    for (auto& band : formantBands)
    {
        band.s1 = 0.0f;
        band.s2 = 0.0f;
    }
}

float SynthFilter::processSample(float input)
{
    if (!enabled)
        return input;

    // Apply drive (pre-filter saturation)
    float driven = input;
    if (drive > 0.001f)
        driven = saturate(input, drive);

    float filtered = 0.0f;

    switch (filterType)
    {
        case FilterType::LowPass12:
        case FilterType::LowPass24:
        case FilterType::HighPass12:
        case FilterType::HighPass24:
        case FilterType::BandPass:
        case FilterType::Notch:
            filtered = processSVF(driven);
            break;

        case FilterType::LadderLP:
            filtered = processLadder(driven);
            break;

        case FilterType::Comb:
            filtered = processComb(driven);
            break;

        case FilterType::FormantA:
        case FilterType::FormantE:
        case FilterType::FormantI:
        case FilterType::FormantO:
        case FilterType::FormantU:
            filtered = processFormant(driven);
            break;
    }

    // Apply dry/wet mix
    return input * (1.0f - mix) + filtered * mix;
}

float SynthFilter::processSampleStereo(float inputL, float inputR, float& outL, float& outR)
{
    if (!enabled)
    {
        outL = inputL;
        outR = inputR;
        return 0.0f;  // Return value unused in stereo mode
    }

    // Apply drive (pre-filter saturation)
    float drivenL = inputL;
    float drivenR = inputR;
    if (drive > 0.001f)
    {
        drivenL = saturate(inputL, drive);
        drivenR = saturate(inputR, drive);
    }

    float filteredL = 0.0f;
    float filteredR = 0.0f;

    switch (filterType)
    {
        case FilterType::LowPass12:
        case FilterType::LowPass24:
        case FilterType::HighPass12:
        case FilterType::HighPass24:
        case FilterType::BandPass:
        case FilterType::Notch:
            processSVFStereo(drivenL, drivenR, filteredL, filteredR);
            break;

        case FilterType::LadderLP:
            processLadderStereo(drivenL, drivenR, filteredL, filteredR);
            break;

        case FilterType::Comb:
            processCombStereo(drivenL, drivenR, filteredL, filteredR);
            break;

        case FilterType::FormantA:
        case FilterType::FormantE:
        case FilterType::FormantI:
        case FilterType::FormantO:
        case FilterType::FormantU:
            processFormantStereo(drivenL, drivenR, filteredL, filteredR);
            break;
    }

    // Apply dry/wet mix
    outL = inputL * (1.0f - mix) + filteredL * mix;
    outR = inputR * (1.0f - mix) + filteredR * mix;

    return 0.0f;  // Return value unused in stereo mode
}

void SynthFilter::updateCoefficients()
{
    // Apply key tracking: blend between fixed cutoff and note-following cutoff
    float effectiveCutoff = cutoff;
    if (keyTrack > 0.0f)
    {
        // Key tracking: cutoff follows the note frequency
        float trackedCutoff = noteFrequency * (cutoff / 440.0f);
        effectiveCutoff = cutoff * (1.0f - keyTrack) + trackedCutoff * keyTrack;
    }
    effectiveCutoff = juce::jlimit(20.0f, 20000.0f, effectiveCutoff);

    // --- SVF Coefficients (Andrew Simper / Cytomic) ---
    // This is a high-quality digital filter topology that doesn't suffer from
    // the frequency warping issues of simpler filter implementations
    float w = juce::MathConstants<float>::pi * effectiveCutoff / static_cast<float>(sampleRate);
    svfG = std::tan(w);

    // Resonance mapping: 0-1 parameter to damping coefficient
    // Lower R = more resonance. At R=0, filter self-oscillates
    float Q = 0.5f + resonance * 9.5f; // Q from 0.5 to 10
    svfR = 1.0f / Q;

    svfA1 = 1.0f / (1.0f + svfG * (svfG + svfR));
    svfA2 = svfG * svfA1;
    svfA3 = svfG * svfA2;

    // --- Ladder filter coefficient ---
    ladderCutoffCoeff = juce::MathConstants<float>::pi * effectiveCutoff / static_cast<float>(sampleRate);
    ladderCutoffCoeff = juce::jlimit(0.0f, 0.99f, ladderCutoffCoeff);

    // --- Comb filter delay ---
    if (filterType == FilterType::Comb)
    {
        combDelaySamples = static_cast<float>(sampleRate) / effectiveCutoff;
        combDelaySamples = juce::jlimit(1.0f, static_cast<float>(COMB_BUFFER_SIZE - 1), combDelaySamples);
    }

    // --- Formant frequencies ---
    // Each vowel sound is characterized by 3 resonant peaks (formants)
    if (filterType >= FilterType::FormantA && filterType <= FilterType::FormantU)
    {
        // Formant frequencies for male voice (Hz) — F1, F2, F3
        struct VowelFormants { float f1, f2, f3; float b1, b2, b3; };

        VowelFormants vowels[5] = {
            { 800, 1150, 2800, 80, 90, 120 },  // A (ah)
            { 400, 1600, 2700, 60, 80, 120 },   // E (eh)
            { 350, 2300, 3000, 50, 100, 120 },   // I (ee)
            { 450,  800, 2830, 70, 80, 100 },    // O (oh)
            { 325,  700, 2530, 50, 60, 100 }     // U (oo)
        };

        int vowelIdx = static_cast<int>(filterType) - static_cast<int>(FilterType::FormantA);
        auto& v = vowels[vowelIdx];

        // Scale formant frequencies by cutoff position
        float scale = effectiveCutoff / 1000.0f;

        formantBands[0] = { 0, 0, v.f1 * scale, v.b1 * scale, 1.0f };
        formantBands[1] = { 0, 0, v.f2 * scale, v.b2 * scale, 0.7f };
        formantBands[2] = { 0, 0, v.f3 * scale, v.b3 * scale, 0.5f };
    }
}

float SynthFilter::processSVF(float input)
{
    // State Variable Filter (SVF) — Cytomic/Simper topology
    // Simultaneously computes low-pass, high-pass, band-pass, and notch

    float v3 = input - svfIc2eq;
    float v1 = svfA1 * svfIc1eq + svfA2 * v3;
    float v2 = svfIc2eq + svfA2 * svfIc1eq + svfA3 * v3;

    svfIc1eq = 2.0f * v1 - svfIc1eq;
    svfIc2eq = 2.0f * v2 - svfIc2eq;

    // Select output based on filter type
    float lp = v2;                    // Low-pass
    float hp = input - svfR * v1 - v2; // High-pass
    float bp = v1;                    // Band-pass
    float notch = lp + hp;            // Notch (band-reject)

    switch (filterType)
    {
        case FilterType::LowPass12:   return lp;
        case FilterType::LowPass24:
        {
            // Approximate 24dB/oct by squaring the LP response
            // (true cascaded SVF would need separate state variables)
            return lp * lp / (std::abs(lp) + 0.0001f);
        }
        case FilterType::HighPass12:  return hp;
        case FilterType::HighPass24:  return hp * hp / (std::abs(hp) + 0.0001f); // Approximation
        case FilterType::BandPass:    return bp;
        case FilterType::Notch:       return notch;
        default: return lp;
    }
}

float SynthFilter::processLadder(float input)
{
    // Moog Ladder Filter emulation
    // 4 cascaded one-pole filters with feedback = classic Moog sound
    // The feedback creates the characteristic resonant peak

    float feedback = resonance * 4.0f; // Resonance amount (0 to 4 for self-oscillation)
    float cutCoeff = ladderCutoffCoeff;

    // Feedback path with delay compensation
    float feedbackSample = ladderState[3];
    float input_with_feedback = input - feedback * feedbackSample;

    // Soft clip the feedback to prevent blowup and add warmth
    input_with_feedback = std::tanh(input_with_feedback);

    // 4 cascaded one-pole lowpass filters
    for (int stage = 0; stage < 4; ++stage)
    {
        float stageInput = (stage == 0) ? input_with_feedback : ladderState[stage - 1];
        ladderState[stage] += cutCoeff * (stageInput - ladderState[stage]);
    }

    return ladderState[3];
}

float SynthFilter::processComb(float input)
{
    // Comb filter — creates metallic, resonant sounds
    // Works like a very short delay with feedback

    // Read from delay buffer with linear interpolation
    float readPos = combWriteIndex - combDelaySamples;
    if (readPos < 0) readPos += COMB_BUFFER_SIZE;

    int readIdx = static_cast<int>(readPos);
    float frac = readPos - readIdx;

    int idx0 = readIdx % COMB_BUFFER_SIZE;
    int idx1 = (readIdx + 1) % COMB_BUFFER_SIZE;

    float delayed = combBuffer[idx0] * (1.0f - frac) + combBuffer[idx1] * frac;

    // Write to buffer with feedback
    float feedback = resonance * 0.95f; // Cap at 0.95 to prevent infinite buildup
    combBuffer[combWriteIndex] = input + delayed * feedback;
    combWriteIndex = (combWriteIndex + 1) % COMB_BUFFER_SIZE;

    return input + delayed;
}

float SynthFilter::processFormant(float input)
{
    // Formant filter — 3 parallel bandpass filters tuned to vowel frequencies
    float output = 0.0f;

    for (auto& band : formantBands)
    {
        // Simple resonant bandpass using biquad-style processing
        float w0 = juce::MathConstants<float>::twoPi * band.freq / static_cast<float>(sampleRate);
        float alpha = std::sin(w0) / (2.0f * (band.freq / band.bw));

        float a0 = 1.0f + alpha;
        float b1 = std::sin(w0) / a0;
        float a1 = -2.0f * std::cos(w0) / a0;
        float a2 = (1.0f - alpha) / a0;

        float y = b1 * input + band.s1;
        band.s1 = -a1 * y + band.s2;
        band.s2 = b1 * input - a2 * y;

        output += y * band.gain;
    }

    return output;
}

void SynthFilter::processSVFStereo(float inputL, float inputR, float& outL, float& outR)
{
    // Left channel
    float v1L = svfIc1eqL;
    float v2L = svfIc2eqL;
    float v3L = inputL;
    v3L -= v2L;
    float v1newL = v1L + svfA1 * v3L;
    float v3_2L = v3L + v1newL;
    float v2newL = v2L + svfA2 * v3_2L;
    svfIc1eqL = 2.0f * v1newL - v1L;
    svfIc2eqL = 2.0f * v2newL - v2L;

    // Right channel
    float v1R = svfIc1eqR;
    float v2R = svfIc2eqR;
    float v3R = inputR;
    v3R -= v2R;
    float v1newR = v1R + svfA1 * v3R;
    float v3_2R = v3R + v1newR;
    float v2newR = v2R + svfA2 * v3_2R;
    svfIc1eqR = 2.0f * v1newR - v1R;
    svfIc2eqR = 2.0f * v2newR - v2R;

    // Compute outputs based on filter type
    float lpL = v2newL;
    float hpL = inputL - v1newL - v2newL;
    float bpL = v1newL;
    float notchL = inputL - v1newL * svfR - v2newL;

    float lpR = v2newR;
    float hpR = inputR - v1newR - v2newR;
    float bpR = v1newR;
    float notchR = inputR - v1newR * svfR - v2newR;

    switch (filterType)
    {
        case FilterType::LowPass12:
            outL = lpL;
            outR = lpR;
            break;
        case FilterType::LowPass24:
            outL = lpL * lpL / (std::abs(lpL) + 0.0001f);
            outR = lpR * lpR / (std::abs(lpR) + 0.0001f);
            break;
        case FilterType::HighPass12:
            outL = hpL;
            outR = hpR;
            break;
        case FilterType::HighPass24:
            outL = hpL * hpL / (std::abs(hpL) + 0.0001f);
            outR = hpR * hpR / (std::abs(hpR) + 0.0001f);
            break;
        case FilterType::BandPass:
            outL = bpL;
            outR = bpR;
            break;
        case FilterType::Notch:
            outL = notchL;
            outR = notchR;
            break;
        default:
            outL = lpL;
            outR = lpR;
            break;
    }
}

void SynthFilter::processLadderStereo(float inputL, float inputR, float& outL, float& outR)
{
    // Moog Ladder Filter emulation — stereo version
    float feedback = resonance * 4.0f;
    float cutCoeff = ladderCutoffCoeff;

    // Left channel
    float feedbackSampleL = ladderStateL[3];
    float input_with_feedbackL = inputL - feedback * feedbackSampleL;
    input_with_feedbackL = std::tanh(input_with_feedbackL);

    for (int stage = 0; stage < 4; ++stage)
    {
        float stageInput = (stage == 0) ? input_with_feedbackL : ladderStateL[stage - 1];
        ladderStateL[stage] += cutCoeff * (stageInput - ladderStateL[stage]);
    }

    // Right channel
    float feedbackSampleR = ladderStateR[3];
    float input_with_feedbackR = inputR - feedback * feedbackSampleR;
    input_with_feedbackR = std::tanh(input_with_feedbackR);

    for (int stage = 0; stage < 4; ++stage)
    {
        float stageInput = (stage == 0) ? input_with_feedbackR : ladderStateR[stage - 1];
        ladderStateR[stage] += cutCoeff * (stageInput - ladderStateR[stage]);
    }

    outL = ladderStateL[3];
    outR = ladderStateR[3];
}

void SynthFilter::processCombStereo(float inputL, float inputR, float& outL, float& outR)
{
    // Comb filter — stereo version
    float feedback = resonance * 0.95f;

    // Left channel
    float readPosL = combWriteIndexL - combDelaySamples;
    if (readPosL < 0) readPosL += COMB_BUFFER_SIZE;

    int readIdxL = static_cast<int>(readPosL);
    float fracL = readPosL - readIdxL;

    int idx0L = readIdxL % COMB_BUFFER_SIZE;
    int idx1L = (readIdxL + 1) % COMB_BUFFER_SIZE;

    float delayedL = combBufferL[idx0L] * (1.0f - fracL) + combBufferL[idx1L] * fracL;
    combBufferL[combWriteIndexL] = inputL + delayedL * feedback;
    combWriteIndexL = (combWriteIndexL + 1) % COMB_BUFFER_SIZE;

    // Right channel
    float readPosR = combWriteIndexR - combDelaySamples;
    if (readPosR < 0) readPosR += COMB_BUFFER_SIZE;

    int readIdxR = static_cast<int>(readPosR);
    float fracR = readPosR - readIdxR;

    int idx0R = readIdxR % COMB_BUFFER_SIZE;
    int idx1R = (readIdxR + 1) % COMB_BUFFER_SIZE;

    float delayedR = combBufferR[idx0R] * (1.0f - fracR) + combBufferR[idx1R] * fracR;
    combBufferR[combWriteIndexR] = inputR + delayedR * feedback;
    combWriteIndexR = (combWriteIndexR + 1) % COMB_BUFFER_SIZE;

    outL = inputL + delayedL;
    outR = inputR + delayedR;
}

void SynthFilter::processFormantStereo(float inputL, float inputR, float& outL, float& outR)
{
    // Formant filter is typically mono or applies the same to both channels
    // since formant frequencies don't need per-channel state separation
    // We just apply the same processing to L and R independently
    float outputL = 0.0f;
    float outputR = 0.0f;

    // Create temporary formant state for L and R to avoid cross-contamination
    std::array<float, 3> s1L_temp, s2L_temp, s1R_temp, s2R_temp;
    for (int i = 0; i < 3; ++i)
    {
        s1L_temp[i] = formantBands[i].s1;
        s2L_temp[i] = formantBands[i].s2;
        s1R_temp[i] = formantBands[i].s1;
        s2R_temp[i] = formantBands[i].s2;
    }

    for (int i = 0; i < 3; ++i)
    {
        auto& band = formantBands[i];
        float w0 = juce::MathConstants<float>::twoPi * band.freq / static_cast<float>(sampleRate);
        float alpha = std::sin(w0) / (2.0f * (band.freq / band.bw));

        float a0 = 1.0f + alpha;
        float b1 = std::sin(w0) / a0;
        float a1 = -2.0f * std::cos(w0) / a0;
        float a2 = (1.0f - alpha) / a0;

        // Left channel
        float yL = b1 * inputL + s1L_temp[i];
        s1L_temp[i] = -a1 * yL + s2L_temp[i];
        s2L_temp[i] = b1 * inputL - a2 * yL;
        outputL += yL * band.gain;

        // Right channel
        float yR = b1 * inputR + s1R_temp[i];
        s1R_temp[i] = -a1 * yR + s2R_temp[i];
        s2R_temp[i] = b1 * inputR - a2 * yR;
        outputR += yR * band.gain;
    }

    outL = outputL;
    outR = outputR;
}

float SynthFilter::saturate(float input, float amount) const
{
    // Soft saturation using tanh — adds harmonics without harsh clipping
    // This is the "Drive" control — makes the filter sound more aggressive
    float gainFactor = 1.0f + amount * 10.0f; // Drive gain: 1x to 11x
    return std::tanh(input * gainFactor) / std::tanh(gainFactor);
}
