/*
  ==============================================================================
    WavetableOscillator.cpp

    Implementation of the wavetable oscillator — the core sound source.
  ==============================================================================
*/

#include "WavetableOscillator.h"

// ============================================================================
// Wavetable Generation
// ============================================================================

void Wavetable::generateBasicWavetable()
{
    // Creates a wavetable that morphs: Sine -> Triangle -> Saw -> Square
    // This is the most common "Basic Shapes" wavetable in Serum

    name = "Basic Shapes";
    numFrames = 4;
    frames.resize(numFrames);

    const float twoPi = juce::MathConstants<float>::twoPi;

    // Frame 0: Sine wave — the purest tone
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        frames[0][i] = std::sin(twoPi * phase);
    }

    // Frame 1: Triangle wave — softer than saw, brighter than sine
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        frames[1][i] = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
    }

    // Frame 2: Sawtooth wave — rich in harmonics, great for basses and leads
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        frames[2][i] = 2.0f * phase - 1.0f;
    }

    // Frame 3: Square wave — hollow sound, good for pads and plucks
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        frames[3][i] = phase < 0.5f ? 1.0f : -1.0f;
    }
}

void Wavetable::generateAnalogWavetable()
{
    // Simulates analog oscillator drift — slight imperfections that sound "warm"
    // Uses band-limited synthesis (additive) to avoid aliasing

    name = "Analog";
    numFrames = 8;
    frames.resize(numFrames);

    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int frame = 0; frame < numFrames; ++frame)
    {
        // Each frame adds more harmonics, simulating different analog characters
        int maxHarmonics = 4 + frame * 8;  // 4, 12, 20, 28, 36, 44, 52, 60

        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            float phase = static_cast<float>(i) / WAVETABLE_SIZE;
            float sample = 0.0f;

            // Additive synthesis: build up harmonics like a real analog oscillator
            for (int h = 1; h <= maxHarmonics; ++h)
            {
                float amplitude = 1.0f / static_cast<float>(h);  // Saw-like harmonic rolloff

                // Add slight random detuning per harmonic (analog drift)
                float drift = 1.0f + (frame * 0.001f) * std::sin(h * 0.7f);

                sample += amplitude * std::sin(twoPi * phase * h * drift);
            }

            // Normalize
            frames[frame][i] = sample * 0.5f;
        }
    }
}

void Wavetable::generateSpectralWavetable()
{
    // Creates evolving spectral content — great for pads and textures

    name = "Spectral";
    numFrames = 16;
    frames.resize(numFrames);

    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int frame = 0; frame < numFrames; ++frame)
    {
        float morphPosition = static_cast<float>(frame) / (numFrames - 1);

        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            float phase = static_cast<float>(i) / WAVETABLE_SIZE;
            float sample = 0.0f;

            // Evolving harmonic content based on frame position
            for (int h = 1; h <= 32; ++h)
            {
                // Different harmonics fade in and out across frames
                float harmonicPhase = morphPosition * juce::MathConstants<float>::pi * 2.0f;
                float amplitude = std::abs(std::sin(harmonicPhase * h * 0.3f + h * 0.5f));
                amplitude *= 1.0f / std::sqrt(static_cast<float>(h)); // Gentler rolloff than saw

                sample += amplitude * std::sin(twoPi * phase * h);
            }

            frames[frame][i] = sample * 0.4f;
        }
    }
}

void Wavetable::generateFromHarmonics(const std::vector<float>& harmonicAmplitudes)
{
    name = "Custom";
    numFrames = 1;
    frames.resize(1);

    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        float phase = static_cast<float>(i) / WAVETABLE_SIZE;
        float sample = 0.0f;

        for (size_t h = 0; h < harmonicAmplitudes.size(); ++h)
        {
            sample += harmonicAmplitudes[h] * std::sin(twoPi * phase * (h + 1));
        }

        frames[0][i] = sample;
    }
}

// ============================================================================
// WavetableOscillator Implementation
// ============================================================================

bool Wavetable::replaceFrame(int frameIndex, const std::array<float, WAVETABLE_SIZE>& frame)
{
    if (frameIndex < 0 || frameIndex >= numFrames
        || frameIndex >= static_cast<int>(frames.size()))
        return false;

    frames[static_cast<size_t>(frameIndex)] = frame;
    return true;
}

WavetableOscillator::WavetableOscillator()
{
    initFactoryWavetables();
    loadFactoryWavetable(0); // Start with Basic Shapes
    phases.fill(0.0f);
    phaseIncrements.fill(0.0f);
}

void WavetableOscillator::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
}

void WavetableOscillator::reset()
{
    // Randomize starting phases based on phaseRandomization parameter
    // This prevents all unison voices from starting at the same point
    // (which would cause a click and reduce the "width" of unison)

    juce::Random random;
    for (int i = 0; i < MAX_UNISON_VOICES; ++i)
    {
        if (phaseRandomization > 0.0f)
            phases[i] = random.nextFloat() * phaseRandomization;
        else
            phases[i] = 0.0f;
    }
}

std::pair<float, float> WavetableOscillator::processSample(float frequency)
{
    if (!enabled || currentWavetable.numFrames == 0)
        return { 0.0f, 0.0f };

    // Apply octave shift and detune to the base frequency
    // Musical math: each octave doubles the frequency
    // Each semitone multiplies by 2^(1/12) ≈ 1.05946
    float adjustedFreq = frequency
        * std::pow(2.0f, static_cast<float>(octaveShift))           // Octave
        * std::pow(2.0f, detuneSemitones / 12.0f)                   // Semitones
        * std::pow(2.0f, fineTuneCents / 1200.0f);                  // Cents

    float leftSample = 0.0f;
    float rightSample = 0.0f;

    // Process each unison voice
    for (int voice = 0; voice < unisonVoices; ++voice)
    {
        // Calculate this voice's detuned frequency
        // Unison voices are spread evenly around the center pitch
        float voiceDetune = 0.0f;
        if (unisonVoices > 1)
        {
            // Spread from -unisonDetuneCents to +unisonDetuneCents
            float normalizedPos = static_cast<float>(voice) / (unisonVoices - 1) * 2.0f - 1.0f;
            voiceDetune = normalizedPos * unisonDetuneCents;
        }

        float voiceFreq = adjustedFreq * std::pow(2.0f, voiceDetune / 1200.0f);

        // Calculate phase increment (how much to advance through the wavetable per sample)
        // phaseIncrement = frequency / sampleRate
        // This gives us the fraction of the wavetable to advance each sample
        float phaseIncrement = voiceFreq / static_cast<float>(sampleRate);

        // Apply warp to the phase
        float warpedPhase = applyWarp(phases[voice], warpAmount);

        // Read the wavetable at this phase and wavetable position
        float sample = readWavetable(warpedPhase, wavetablePosition);

        // Advance the phase
        phases[voice] += phaseIncrement;

        // Wrap phase to [0, 1) range
        if (phases[voice] >= 1.0f)
            phases[voice] -= 1.0f;

        // Calculate stereo spread for this unison voice
        // Center voice stays centered, outer voices pan left/right
        float voicePan = 0.0f;
        if (unisonVoices > 1)
        {
            float normalizedPos = static_cast<float>(voice) / (unisonVoices - 1) * 2.0f - 1.0f;
            voicePan = normalizedPos * unisonBlend;
        }

        // Apply pan law (constant power panning)
        float combinedPan = juce::jlimit(-1.0f, 1.0f, pan + voicePan);
        float leftGain = std::cos((combinedPan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);
        float rightGain = std::sin((combinedPan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);

        leftSample += sample * leftGain;
        rightSample += sample * rightGain;
    }

    // Normalize by number of unison voices to prevent clipping
    float normFactor = level / std::sqrt(static_cast<float>(unisonVoices));

    return { leftSample * normFactor, rightSample * normFactor };
}

float WavetableOscillator::readWavetable(float phase, float wtPos) const
{
    if (currentWavetable.numFrames == 0)
        return 0.0f;

    // Determine which two frames to interpolate between
    float frameIndexFloat = wtPos * (currentWavetable.numFrames - 1);
    int frameA = static_cast<int>(frameIndexFloat);
    int frameB = std::min(frameA + 1, currentWavetable.numFrames - 1);
    float frameFrac = frameIndexFloat - frameA; // Interpolation amount between frames

    // Determine sample position within the frame
    float samplePos = phase * WAVETABLE_SIZE;
    int sampleA = static_cast<int>(samplePos) % WAVETABLE_SIZE;
    int sampleB = (sampleA + 1) % WAVETABLE_SIZE;
    float sampleFrac = samplePos - std::floor(samplePos);

    // Bilinear interpolation:
    // 1. Interpolate within frame A
    float valueA = currentWavetable.frames[frameA][sampleA] * (1.0f - sampleFrac)
                 + currentWavetable.frames[frameA][sampleB] * sampleFrac;

    // 2. Interpolate within frame B
    float valueB = currentWavetable.frames[frameB][sampleA] * (1.0f - sampleFrac)
                 + currentWavetable.frames[frameB][sampleB] * sampleFrac;

    // 3. Interpolate between the two frames
    return valueA * (1.0f - frameFrac) + valueB * frameFrac;
}

float WavetableOscillator::applyWarp(float phase, float warpAmt) const
{
    if (warpMode == WarpMode::None || warpAmt < 0.001f)
        return phase;

    switch (warpMode)
    {
        case WarpMode::Bend:
        {
            // Bends the phase curve — pushes energy toward start or end of cycle
            float bendPower = 1.0f + warpAmt * 3.0f;
            return std::pow(phase, bendPower);
        }

        case WarpMode::PWM:
        {
            // Pulse Width Modulation — changes the duty cycle
            float pulseWidth = 0.5f + warpAmt * 0.45f; // 0.5 to 0.95
            return phase < pulseWidth ? phase / pulseWidth * 0.5f : 0.5f + (phase - pulseWidth) / (1.0f - pulseWidth) * 0.5f;
        }

        case WarpMode::Sync:
        {
            // Hard sync — multiplies the frequency within one cycle
            float syncRatio = 1.0f + warpAmt * 4.0f; // 1x to 5x
            return std::fmod(phase * syncRatio, 1.0f);
        }

        case WarpMode::Asymmetric:
        {
            // Different warping for first and second half of the waveform
            if (phase < 0.5f)
            {
                float p = phase * 2.0f;
                return std::pow(p, 1.0f + warpAmt * 2.0f) * 0.5f;
            }
            else
            {
                float p = (phase - 0.5f) * 2.0f;
                return 0.5f + std::pow(p, 1.0f / (1.0f + warpAmt * 2.0f)) * 0.5f;
            }
        }

        case WarpMode::Quantize:
        {
            // Bit-crush / reduce resolution of the phase
            float steps = 4.0f + (1.0f - warpAmt) * 252.0f; // 4 to 256 steps
            return std::round(phase * steps) / steps;
        }

        default:
            return phase;
    }
}

void WavetableOscillator::setWavetable(const Wavetable& newWavetable)
{
    currentWavetable = newWavetable;
    frameCacheDirty = true;
}

void WavetableOscillator::loadFactoryWavetable(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryWavetables.size()))
    {
        currentWavetable = factoryWavetables[index];
        frameCacheDirty = true;
    }
}

// --- Parameter Setters ---

void WavetableOscillator::setWavetablePosition(float position)
{
    wavetablePosition = juce::jlimit(0.0f, 1.0f, position);
    frameCacheDirty = true;
}

void WavetableOscillator::setDetune(float semitones)
{
    detuneSemitones = juce::jlimit(-24.0f, 24.0f, semitones);
}

void WavetableOscillator::setFineTune(float cents)
{
    fineTuneCents = juce::jlimit(-100.0f, 100.0f, cents);
}

void WavetableOscillator::setUnisonVoices(int numVoices)
{
    unisonVoices = juce::jlimit(1, MAX_UNISON_VOICES, numVoices);
}

void WavetableOscillator::setUnisonDetune(float detuneCents)
{
    unisonDetuneCents = juce::jlimit(0.0f, 100.0f, detuneCents);
}

void WavetableOscillator::setUnisonBlend(float blend)
{
    unisonBlend = juce::jlimit(0.0f, 1.0f, blend);
}

void WavetableOscillator::setLevel(float newLevel)
{
    level = juce::jlimit(0.0f, 1.0f, newLevel);
}

void WavetableOscillator::setPan(float newPan)
{
    pan = juce::jlimit(-1.0f, 1.0f, newPan);
}

void WavetableOscillator::setOctave(int shift)
{
    octaveShift = juce::jlimit(-3, 3, shift);
}

void WavetableOscillator::setWarpMode(WarpMode mode)
{
    warpMode = mode;
}

void WavetableOscillator::setWarpAmount(float amount)
{
    warpAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void WavetableOscillator::setPhaseRandomization(float amount)
{
    phaseRandomization = juce::jlimit(0.0f, 1.0f, amount);
}

const std::array<float, WAVETABLE_SIZE>& WavetableOscillator::getCurrentFrame() const
{
    if (frameCacheDirty && currentWavetable.numFrames > 0)
    {
        // Generate the interpolated frame for the current wavetable position
        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            float phase = static_cast<float>(i) / WAVETABLE_SIZE;
            currentFrameCache[i] = readWavetable(phase, wavetablePosition);
        }
        frameCacheDirty = false;
    }
    return currentFrameCache;
}

void WavetableOscillator::initFactoryWavetables()
{
    // Create the factory wavetables that ship with the synth
    // These are the bread-and-butter sounds every producer needs

    factoryWavetables.resize(3);

    factoryWavetables[0].generateBasicWavetable();    // Sine/Tri/Saw/Square
    factoryWavetables[1].generateAnalogWavetable();   // Warm analog emulation
    factoryWavetables[2].generateSpectralWavetable(); // Evolving spectral content
}
