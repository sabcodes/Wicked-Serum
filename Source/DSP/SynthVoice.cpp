/*
  ==============================================================================
    SynthVoice.cpp
  ==============================================================================
*/

#include "SynthVoice.h"

SynthVoice::SynthVoice()
{
    // Set up default modulation routes that make the synth immediately playable:

    // ENV 2 → Filter Cutoff (amount: 0.5) — classic filter envelope
    modMatrix.addRoute(ModSource::Env2, ModDestination::Filter_Cutoff, 0.5f);

    // Default envelope settings for a versatile "init" patch
    ampEnvelope.setAttack(0.01f);    // 10ms — fast but click-free
    ampEnvelope.setDecay(0.3f);
    ampEnvelope.setSustain(0.7f);
    ampEnvelope.setRelease(0.3f);

    filterEnvelope.setAttack(0.01f);
    filterEnvelope.setDecay(0.5f);
    filterEnvelope.setSustain(0.3f);
    filterEnvelope.setRelease(0.5f);

    modEnvelope.setAttack(0.1f);
    modEnvelope.setDecay(0.5f);
    modEnvelope.setSustain(0.5f);
    modEnvelope.setRelease(0.5f);

    // Default LFO settings
    for (int i = 0; i < 4; ++i)
    {
        lfos[i].setRate(1.0f + i * 0.5f);  // Slightly different rates
        lfos[i].setDepth(1.0f);
        lfos[i].setShape(LFOShape::Sine);
    }
}

void SynthVoice::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;

    oscA.prepare(sampleRate);
    oscB.prepare(sampleRate);
    subOsc.prepare(sampleRate);
    noise.prepare(sampleRate);
    filter.prepare(sampleRate);
    ampEnvelope.prepare(sampleRate);
    filterEnvelope.prepare(sampleRate);
    modEnvelope.prepare(sampleRate);

    for (int i = 0; i < 4; ++i)
        lfos[i].prepare(sampleRate);
}

void SynthVoice::noteOn(int midiNote, float velocity)
{
    currentNote = midiNote;
    currentVelocity = velocity;
    age = 0;

    float freq = getNoteFrequency();

    // Reset oscillator phases for clean note starts
    oscA.reset();
    oscB.reset();
    subOsc.reset();

    // Tell filter what note is playing (for key tracking)
    filter.setNoteFrequency(freq);

    // Trigger all envelopes
    ampEnvelope.noteOn();
    filterEnvelope.noteOn();
    modEnvelope.noteOn();

    // Trigger LFOs
    for (int i = 0; i < 4; ++i)
        lfos[i].noteOn();

    // Apply velocity curve and set in mod matrix
    float curvedVelocity = velocityCurve.processCurve(velocity);
    modMatrix.setSourceValue(ModSource::Velocity, curvedVelocity);

    // Set note number in mod matrix (normalized 0-1 across MIDI range)
    modMatrix.setSourceValue(ModSource::NoteNumber, static_cast<float>(midiNote) / 127.0f);
}

void SynthVoice::noteOff()
{
    ampEnvelope.noteOff();
    filterEnvelope.noteOff();
    modEnvelope.noteOff();

    for (int i = 0; i < 4; ++i)
        lfos[i].noteOff();
}

void SynthVoice::reset()
{
    currentNote = -1;
    currentVelocity = 0.0f;
    age = 0;

    oscA.reset();
    oscB.reset();
    subOsc.reset();
    noise.reset();
    filter.reset();
    ampEnvelope.reset();
    filterEnvelope.reset();
    modEnvelope.reset();

    for (int i = 0; i < 4; ++i)
        lfos[i].reset();
}

void SynthVoice::renderBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (!isActive())
        return;

    float frequency = getNoteFrequency();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // === 1. Process modulation sources ===
        float envValue1 = ampEnvelope.processSample();
        float envValue2 = filterEnvelope.processSample();
        float envValue3 = modEnvelope.processSample();

        float lfoValues[4];
        for (int i = 0; i < 4; ++i)
            lfoValues[i] = lfos[i].processSample();

        // Update mod matrix source values
        modMatrix.setSourceValue(ModSource::Env1, envValue1);
        modMatrix.setSourceValue(ModSource::Env2, envValue2);
        modMatrix.setSourceValue(ModSource::Env3, envValue3);
        modMatrix.setSourceValue(ModSource::LFO1, lfoValues[0]);
        modMatrix.setSourceValue(ModSource::LFO2, lfoValues[1]);
        modMatrix.setSourceValue(ModSource::LFO3, lfoValues[2]);
        modMatrix.setSourceValue(ModSource::LFO4, lfoValues[3]);

        // === 2. Apply modulation to parameters ===
        applyModulation();

        // === 3. Generate oscillator audio ===
        // Get pitch modulation
        float pitchMod = modMatrix.getModulationAmount(ModDestination::MasterPitch);
        float modulatedFreq = frequency * std::pow(2.0f, pitchMod / 12.0f);

        auto [oscALeft, oscARight] = oscA.processSample(modulatedFreq);
        auto [oscBLeft, oscBRight] = oscB.processSample(modulatedFreq);
        float subSample = subOsc.processSample(modulatedFreq);
        float noiseSample = noise.processSample();

        // === 4. Mix all sound sources ===
        float mixedLeft  = oscALeft + oscBLeft + subSample + noiseSample;
        float mixedRight = oscARight + oscBRight + subSample + noiseSample;

        // === 5. Apply stereo filter ===
        float filteredLeft  = 0.0f;
        float filteredRight = 0.0f;
        filter.processSampleStereo(mixedLeft, mixedRight, filteredLeft, filteredRight);

        // === 6. Apply amplitude envelope (VCA) ===
        float ampMod = modMatrix.getModulationAmount(ModDestination::MasterVolume);
        float finalAmp = juce::jlimit(0.0f, 1.0f, envValue1 + ampMod) * currentVelocity;

        filteredLeft  *= finalAmp;
        filteredRight *= finalAmp;

        // === 7. Add to output buffer ===
        int bufferSample = startSample + sample;
        if (buffer.getNumChannels() >= 2)
        {
            buffer.addSample(0, bufferSample, filteredLeft);
            buffer.addSample(1, bufferSample, filteredRight);
        }
        else if (buffer.getNumChannels() >= 1)
        {
            buffer.addSample(0, bufferSample, (filteredLeft + filteredRight) * 0.5f);
        }
    }
}

void SynthVoice::applyModulation()
{
    auto amount = [this](ModDestination destination)
    {
        return modMatrix.getModulationAmount(destination);
    };

    // Always derive the instantaneous value from a stable base. This prevents
    // modulation from accumulating every sample and getting stuck at a limit.
    oscA.setWavetablePosition(juce::jlimit(0.0f, 1.0f,
        baseOscAWTPosition + amount(ModDestination::OscA_WTPosition)));
    oscA.setLevel(juce::jlimit(0.0f, 1.0f,
        baseOscALevel + amount(ModDestination::OscA_Level)));
    oscA.setPan(juce::jlimit(-1.0f, 1.0f,
        baseOscAPan + amount(ModDestination::OscA_Pan)));
    oscA.setDetune(juce::jlimit(-24.0f, 24.0f,
        baseOscADetune + amount(ModDestination::OscA_Detune) * 12.0f));
    oscA.setWarpAmount(juce::jlimit(0.0f, 1.0f,
        baseOscAWarp + amount(ModDestination::OscA_WarpAmount)));

    oscB.setWavetablePosition(juce::jlimit(0.0f, 1.0f,
        baseOscBWTPosition + amount(ModDestination::OscB_WTPosition)));
    oscB.setLevel(juce::jlimit(0.0f, 1.0f,
        baseOscBLevel + amount(ModDestination::OscB_Level)));
    oscB.setPan(juce::jlimit(-1.0f, 1.0f,
        baseOscBPan + amount(ModDestination::OscB_Pan)));
    oscB.setDetune(juce::jlimit(-24.0f, 24.0f,
        baseOscBDetune + amount(ModDestination::OscB_Detune) * 12.0f));
    oscB.setWarpAmount(juce::jlimit(0.0f, 1.0f,
        baseOscBWarp + amount(ModDestination::OscB_WarpAmount)));

    const float modulatedCutoff = baseFilterCutoff
        * std::pow(2.0f, amount(ModDestination::Filter_Cutoff) * 5.0f);
    filter.setCutoff(juce::jlimit(20.0f, 20000.0f, modulatedCutoff));
    filter.setResonance(juce::jlimit(0.0f, 1.0f,
        baseFilterResonance + amount(ModDestination::Filter_Resonance)));
    filter.setDrive(juce::jlimit(0.0f, 1.0f,
        baseFilterDrive + amount(ModDestination::Filter_Drive)));
    filter.setMix(juce::jlimit(0.0f, 1.0f,
        baseFilterMix + amount(ModDestination::Filter_Mix)));
}

float SynthVoice::getNoteFrequency() const
{
    if (currentNote < 0)
        return 440.0f;

    // MIDI note to frequency: f = 440 * 2^((note - 69) / 12)
    // Note 69 = A4 = 440 Hz
    float noteWithBend = static_cast<float>(currentNote) + pitchBendSemitones;
    return 440.0f * std::pow(2.0f, (noteWithBend - 69.0f) / 12.0f);
}
