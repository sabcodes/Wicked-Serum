/*
  ==============================================================================
    FactoryWavetables.cpp

    Implementation of factory wavetable generation using additive synthesis.
    Each wavetable is created algorithmically by summing harmonics with
    specific amplitude envelopes across frames.
  ==============================================================================
*/

#include "FactoryWavetables.h"
#include <cmath>
#include <algorithm>

namespace FactoryWavetables
{
    // Utility function to generate a sine wave at a specific frequency
    static void addHarmonic(std::array<float, WAVETABLE_SIZE>& frame,
                           int harmonic, float amplitude)
    {
        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            float phase = 2.0f * M_PI * harmonic * i / WAVETABLE_SIZE;
            frame[i] += amplitude * std::sin(phase);
        }
    }

    // Utility function to normalize a frame to prevent clipping
    static void normalizeFrame(std::array<float, WAVETABLE_SIZE>& frame)
    {
        float maxVal = 0.0f;
        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            maxVal = std::max(maxVal, std::abs(frame[i]));
        }

        if (maxVal > 0.0f)
        {
            for (int i = 0; i < WAVETABLE_SIZE; ++i)
            {
                frame[i] /= maxVal;
            }
        }
    }

    // ========== DIGITAL ==========
    // Harsh digital harmonics with odd partials emphasized
    Wavetable createDigital()
    {
        Wavetable wt;
        wt.name = "Digital";
        wt.frames.resize(64);

        for (int frameIdx = 0; frameIdx < 64; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 63.0f; // 0 to 1

            // Emphasize odd harmonics with varying intensity
            for (int harmonic = 1; harmonic <= 32; harmonic += 2)
            {
                float amp = 1.0f / harmonic;
                // Vary emphasis across frames
                amp *= (1.0f - framePos * 0.3f);
                addHarmonic(frame, harmonic, amp);
            }

            // Add some even harmonics for harshness
            for (int harmonic = 2; harmonic <= 16; harmonic += 2)
            {
                float amp = (1.0f / harmonic) * framePos * 0.5f;
                addHarmonic(frame, harmonic, amp);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 64;
        return wt;
    }

    // ========== VOWEL ==========
    // Morphs through vowel formant shapes (A-E-I-O-U)
    Wavetable createVowel()
    {
        Wavetable wt;
        wt.name = "Vowel";
        wt.frames.resize(60);

        // Simplified vowel formants (approximations)
        struct FormantSet { int f1, f2, f3; };
        FormantSet vowels[] = {
            {700, 1220, 2600},   // A
            {400, 1600, 2200},   // E
            {300, 2300, 2600},   // I
            {600, 860, 2600},    // O
            {250, 595, 2400}     // U
        };

        for (int frameIdx = 0; frameIdx < 60; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 59.0f;

            // Interpolate between vowels
            int vowelIdx = static_cast<int>(framePos * 4.999f);
            float vowelBlend = (framePos * 5.0f) - vowelIdx;

            FormantSet current = vowels[vowelIdx];
            FormantSet next = vowels[(vowelIdx + 1) % 5];

            FormantSet blend;
            blend.f1 = static_cast<int>(current.f1 * (1 - vowelBlend) + next.f1 * vowelBlend);
            blend.f2 = static_cast<int>(current.f2 * (1 - vowelBlend) + next.f2 * vowelBlend);
            blend.f3 = static_cast<int>(current.f3 * (1 - vowelBlend) + next.f3 * vowelBlend);

            // Create harmonics with formant emphasis
            for (int h = 1; h <= 16; ++h)
            {
                float freq = h * 100.0f; // Base frequency for harmonic
                float dist1 = std::abs(freq - blend.f1);
                float dist2 = std::abs(freq - blend.f2);
                float dist3 = std::abs(freq - blend.f3);

                float amp = 1.0f / h;
                amp *= (1.0f / (1.0f + dist1 / 100.0f));
                amp *= (0.5f + 0.5f / (1.0f + dist2 / 150.0f));
                amp *= (0.3f + 0.7f / (1.0f + dist3 / 200.0f));

                addHarmonic(frame, h, amp);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 60;
        return wt;
    }

    // ========== PWM ==========
    // Pulse width modulation sweep from 50% to 5%
    Wavetable createPWM()
    {
        Wavetable wt;
        wt.name = "PWM";
        wt.frames.resize(48);

        for (int frameIdx = 0; frameIdx < 48; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 47.0f; // 0 to 1

            // PWM: duty cycle from 0.5 to 0.05
            float dutyCycle = 0.5f - framePos * 0.45f;

            // Generate PWM waveform using Fourier series
            for (int harmonic = 1; harmonic <= 32; harmonic += 2)
            {
                float amp = 2.0f / (M_PI * harmonic) * std::sin(M_PI * harmonic * dutyCycle);
                addHarmonic(frame, harmonic, amp);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 48;
        return wt;
    }

    // ========== FM ==========
    // Frequency modulation with increasing modulation index
    Wavetable createFM()
    {
        Wavetable wt;
        wt.name = "FM";
        wt.frames.resize(56);

        for (int frameIdx = 0; frameIdx < 56; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 55.0f; // 0 to 1

            // FM modulation index increases from 0 to ~8
            float modIndex = framePos * 8.0f;

            // Carrier frequency normalized
            int carrier = 1;
            int modulator = 2;

            // FM generates sidebands at carrier +/- n*modulator
            for (int n = -16; n <= 16; ++n)
            {
                int harmonic = carrier + n * modulator;
                if (harmonic > 0)
                {
                    // Bessel function approximation for FM sidebands
                    float amp = std::abs(std::sin(modIndex * n / (1.0f + std::abs(n) * 0.1f)));
                    addHarmonic(frame, harmonic, amp * 0.5f);
                }
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 56;
        return wt;
    }

    // ========== SUPERSAW ==========
    // Detuned saw stacks with increasing detune
    Wavetable createSupersaw()
    {
        Wavetable wt;
        wt.name = "Supersaw";
        wt.frames.resize(64);

        for (int frameIdx = 0; frameIdx < 64; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 63.0f; // 0 to 1

            // Increase detuning from unison to wide detune
            float maxDetune = framePos * 0.12f; // 0 to 12% pitch spread

            // Create 7 detuned sawtooth voices
            for (int voice = 0; voice < 7; ++voice)
            {
                float detuneRatio = 1.0f + (voice - 3) * maxDetune * 0.05f;

                // Add sawtooth harmonics
                for (int harmonic = 1; harmonic <= 16; ++harmonic)
                {
                    float amp = 1.0f / harmonic;
                    float detunedHarmonic = harmonic * detuneRatio;

                    // Linear interpolation for detuned harmonic
                    int baseHarm = static_cast<int>(detunedHarmonic);
                    float fracHarm = detunedHarmonic - baseHarm;

                    addHarmonic(frame, baseHarm, amp * (1.0f - fracHarm) / 7.0f);
                    if (baseHarm < 32)
                        addHarmonic(frame, baseHarm + 1, amp * fracHarm / 7.0f);
                }
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 64;
        return wt;
    }

    // ========== METALLIC ==========
    // Inharmonic partials for bell/metallic sounds
    Wavetable createMetallic()
    {
        Wavetable wt;
        wt.name = "Metallic";
        wt.frames.resize(52);

        // Inharmonic ratios (inspired by bell modes)
        float ratios[] = { 1.0f, 2.6f, 4.4f, 6.8f, 10.1f, 13.8f, 18.2f, 23.8f };

        for (int frameIdx = 0; frameIdx < 52; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 51.0f; // 0 to 1

            // Evolve the spectral decay
            for (size_t i = 0; i < 8; ++i)
            {
                float partialFreq = ratios[i];
                float amp = 1.0f / (1.0f + i * 0.3f);

                // Decay across frames (brighter at start, darker at end)
                amp *= (1.0f - framePos * 0.4f);

                // Create partial using sub-harmonic summation
                int baseHarm = static_cast<int>(partialFreq);
                float fracHarm = partialFreq - baseHarm;

                addHarmonic(frame, baseHarm, amp * (1.0f - fracHarm));
                if (baseHarm < 32)
                    addHarmonic(frame, baseHarm + 1, amp * fracHarm);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 52;
        return wt;
    }

    // ========== WARM PAD ==========
    // Slowly evolving even harmonics
    Wavetable createWarmPad()
    {
        Wavetable wt;
        wt.name = "Warm Pad";
        wt.frames.resize(64);

        for (int frameIdx = 0; frameIdx < 64; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 63.0f; // 0 to 1

            // Use only even harmonics for smooth, warm sound
            for (int harmonic = 2; harmonic <= 32; harmonic += 2)
            {
                float amp = 1.0f / harmonic;

                // Slow evolution: amplitude fluctuation
                float fluctuation = 0.7f + 0.3f * std::sin(2.0f * M_PI * framePos + harmonic * 0.5f);
                amp *= fluctuation;

                addHarmonic(frame, harmonic, amp);
            }

            // Add fundamental for richness
            addHarmonic(frame, 1, 1.2f * (0.8f + 0.2f * std::sin(2.0f * M_PI * framePos)));

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 64;
        return wt;
    }

    // ========== ACID ==========
    // TB-303 style resonant saw/square morphing
    Wavetable createAcid()
    {
        Wavetable wt;
        wt.name = "Acid";
        wt.frames.resize(48);

        for (int frameIdx = 0; frameIdx < 48; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 47.0f; // 0 to 1

            // Morph between sawtooth and square
            // Sawtooth: all harmonics with 1/n amplitude
            // Square: odd harmonics only with 1/n amplitude

            for (int harmonic = 1; harmonic <= 24; ++harmonic)
            {
                float amp = 1.0f / harmonic;

                // Blend between saw (even present) and square (even absent)
                if (harmonic % 2 == 0)
                {
                    amp *= (1.0f - framePos); // Even harmonics fade out
                }

                addHarmonic(frame, harmonic, amp);
            }

            // Add some resonance-like peak
            for (int harmonic = 5; harmonic <= 12; ++harmonic)
            {
                float resonancePeak = framePos * 0.8f;
                addHarmonic(frame, harmonic, resonancePeak * 0.1f / harmonic);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 48;
        return wt;
    }

    // ========== PLUCK ==========
    // Sharp attack decaying harmonics (bright to dark)
    Wavetable createPluck()
    {
        Wavetable wt;
        wt.name = "Pluck";
        wt.frames.resize(60);

        for (int frameIdx = 0; frameIdx < 60; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 59.0f; // 0 to 1

            // Pluck: starts bright, decays to dark
            // Exponential decay of high harmonics
            for (int harmonic = 1; harmonic <= 32; ++harmonic)
            {
                float amp = 1.0f / harmonic;

                // High harmonics decay faster
                float decayFactor = std::exp(-framePos * 2.0f * harmonic / 8.0f);
                amp *= decayFactor;

                addHarmonic(frame, harmonic, amp);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 60;
        return wt;
    }

    // ========== AMBIENT ==========
    // Slowly evolving spectral content with gaps
    Wavetable createAmbient()
    {
        Wavetable wt;
        wt.name = "Ambient";
        wt.frames.resize(72);

        for (int frameIdx = 0; frameIdx < 72; ++frameIdx)
        {
            std::array<float, WAVETABLE_SIZE> frame{};
            float framePos = frameIdx / 71.0f; // 0 to 1

            // Sparse, evolving harmonics
            for (int harmonic = 1; harmonic <= 28; ++harmonic)
            {
                float amp = 0.0f;

                // Create "holes" in the spectrum that move
                float holePos = std::fmod(framePos * 5.0f + harmonic * 0.3f, 1.0f);
                if (std::abs(holePos - 0.5f) > 0.15f) // Avoid certain harmonics
                {
                    amp = 1.0f / (harmonic * (1.0f + framePos));
                    // Slow amplitude modulation
                    amp *= 0.5f + 0.5f * std::sin(2.0f * M_PI * framePos + harmonic);
                }

                addHarmonic(frame, harmonic, amp);
            }

            normalizeFrame(frame);
            wt.frames[frameIdx] = frame;
        }

        wt.numFrames = 72;
        return wt;
    }

    // ========== SINE ==========
    // Pure sine wave (single frame)
    Wavetable createSine()
    {
        Wavetable wt;
        wt.name = "Sine";
        wt.frames.resize(1);

        std::array<float, WAVETABLE_SIZE> frame{};
        addHarmonic(frame, 1, 1.0f);

        wt.frames[0] = frame;
        wt.numFrames = 1;
        return wt;
    }

    // ========== SQUARE ==========
    // Perfect square wave (single frame)
    Wavetable createSquare()
    {
        Wavetable wt;
        wt.name = "Square";
        wt.frames.resize(1);

        std::array<float, WAVETABLE_SIZE> frame{};

        // Square wave: odd harmonics only, amplitude 4/(pi*n)
        for (int harmonic = 1; harmonic <= 32; harmonic += 2)
        {
            float amp = 4.0f / (M_PI * harmonic);
            addHarmonic(frame, harmonic, amp);
        }

        normalizeFrame(frame);
        wt.frames[0] = frame;
        wt.numFrames = 1;
        return wt;
    }

    // ========== GET ALL FACTORY WAVETABLES ==========
    std::vector<Wavetable> getAllFactoryWavetables()
    {
        std::vector<Wavetable> wavetables;

        wavetables.push_back(createSine());
        wavetables.push_back(createSquare());
        wavetables.push_back(createDigital());
        wavetables.push_back(createVowel());
        wavetables.push_back(createPWM());
        wavetables.push_back(createFM());
        wavetables.push_back(createSupersaw());
        wavetables.push_back(createMetallic());
        wavetables.push_back(createWarmPad());
        wavetables.push_back(createAcid());
        wavetables.push_back(createPluck());
        wavetables.push_back(createAmbient());

        return wavetables;
    }

} // namespace FactoryWavetables
