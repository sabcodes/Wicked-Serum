/*
  ==============================================================================
    EffectsChain.cpp
  ==============================================================================
*/

#include "EffectsChain.h"

void EffectsChain::prepare(double sampleRate, int maxBlockSize)
{
    distortion.prepare(sampleRate, maxBlockSize);
    eq.prepare(sampleRate, maxBlockSize);
    phaser.prepare(sampleRate, maxBlockSize);
    flanger.prepare(sampleRate, maxBlockSize);
    chorus.prepare(sampleRate, maxBlockSize);
    delay.prepare(sampleRate, maxBlockSize);
    reverb.prepare(sampleRate, maxBlockSize);
}

void EffectsChain::reset()
{
    distortion.reset();
    eq.reset();
    phaser.reset();
    flanger.reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
}

void EffectsChain::processStereo(float* leftChannel, float* rightChannel, int numSamples)
{
    // Process effects in order: Distortion → EQ → Phaser → Flanger → Chorus → Delay → Reverb
    // This order is intentional:
    // 1. Distortion — shapes the raw tone
    // 2. EQ — tone shaping (refine after distortion)
    // 3. Phaser — sweeping allpass effect (modulation)
    // 4. Flanger — sweeping delay effect (more modulation)
    // 5. Chorus — adds width and detuning
    // 6. Delay — echoes the processed signal
    // 7. Reverb — places everything in a space

    distortion.processStereo(leftChannel, rightChannel, numSamples);
    eq.processStereo(leftChannel, rightChannel, numSamples);
    phaser.processStereo(leftChannel, rightChannel, numSamples);
    flanger.processStereo(leftChannel, rightChannel, numSamples);
    chorus.processStereo(leftChannel, rightChannel, numSamples);
    delay.processStereo(leftChannel, rightChannel, numSamples);
    reverb.processStereo(leftChannel, rightChannel, numSamples);
}
