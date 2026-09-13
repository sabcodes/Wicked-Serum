/*
  ==============================================================================
    FactoryWavetables.h

    Pre-generated factory wavetables for SerumSynth using additive synthesis.

    Each wavetable contains multiple frames (waveform snapshots) that morph
    together to create dynamic, evolving timbres. This file defines functions
    to generate these wavetables using algorithmic harmonic synthesis.

    Frame structure: Each frame is 2048 samples representing one complete
    waveform cycle at normalized amplitude (-1.0 to 1.0).
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WavetableOscillator.h"

namespace FactoryWavetables
{
    /**
     * DIGITAL - Harsh digital harmonics with odd partials emphasized.
     * Creates a buzzy, synthetic lead sound with emphasized odd harmonics.
     */
    Wavetable createDigital();

    /**
     * VOWEL - Morphs through vowel formant shapes (A-E-I-O-U).
     * Creates vocal-like textures by sweeping through formant resonances.
     */
    Wavetable createVowel();

    /**
     * PWM - Pulse width modulation sweep from 50% to 5%.
     * Classic PWM movement from a square wave to a thin pulse.
     */
    Wavetable createPWM();

    /**
     * FM - Frequency modulation with increasing modulation index.
     * Evolves from simple sine to complex FM textures.
     */
    Wavetable createFM();

    /**
     * SUPERSAW - Detuned saw stacks with increasing detune.
     * Lush, wide sawtooth progression from unison to heavily detuned.
     */
    Wavetable createSupersaw();

    /**
     * METALLIC - Inharmonic partials for bell/metallic sounds.
     * Uses non-integer partials to create bell-like, inharmonic textures.
     */
    Wavetable createMetallic();

    /**
     * WARM PAD - Slowly evolving even harmonics.
     * Smooth, warm, pad-like evolution using even harmonics only.
     */
    Wavetable createWarmPad();

    /**
     * ACID - TB-303 style resonant saw/square morphing.
     * Classic acid synth morphing between sawtooth and square waves.
     */
    Wavetable createAcid();

    /**
     * PLUCK - Sharp attack decaying harmonics (bright to dark).
     * Plucked string-like evolution from bright harmonics to dark.
     */
    Wavetable createPluck();

    /**
     * AMBIENT - Slowly evolving spectral content with gaps.
     * Ethereal evolution with sparse, evolving harmonic content.
     */
    Wavetable createAmbient();

    /**
     * SINE - Pure sine wave (single frame).
     * Basic fundamental for additive synthesis starting point.
     */
    Wavetable createSine();

    /**
     * SQUARE - Perfect square wave (single frame).
     * Classic pulse wave with odd harmonics only.
     */
    Wavetable createSquare();

    /**
     * Get all factory wavetables as a vector.
     */
    std::vector<Wavetable> getAllFactoryWavetables();

} // namespace FactoryWavetables
