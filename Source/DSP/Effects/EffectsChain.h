/*
  ==============================================================================
    EffectsChain.h

    Manages the effects processing chain — like Serum's FX tab.
    Effects are processed in order: Distortion → EQ → Phaser → Flanger → Chorus → Delay → Reverb
    (you can reorder these, but this is a good default order)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReverbEffect.h"
#include "DelayEffect.h"
#include "DistortionEffect.h"
#include "EQEffect.h"
#include "PhaserEffect.h"
#include "FlangerEffect.h"
#include "ChorusEffect.h"

class EffectsChain
{
public:
    EffectsChain() = default;
    ~EffectsChain() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processStereo(float* leftChannel, float* rightChannel, int numSamples);
    void reset();

    // Direct access to each effect for parameter control
    DistortionEffect& getDistortion() { return distortion; }
    EQEffect& getEQ()                 { return eq; }
    PhaserEffect& getPhaser()         { return phaser; }
    FlangerEffect& getFlanger()       { return flanger; }
    ChorusEffect& getChorus()         { return chorus; }
    DelayEffect& getDelay()           { return delay; }
    ReverbEffect& getReverb()         { return reverb; }

private:
    DistortionEffect distortion;
    EQEffect eq;
    PhaserEffect phaser;
    FlangerEffect flanger;
    ChorusEffect chorus;
    DelayEffect delay;
    ReverbEffect reverb;
};
