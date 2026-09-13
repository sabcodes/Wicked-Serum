/*
  ==============================================================================
    PluginProcessor.cpp

    The main plugin processor — connects Logic Pro to our synth engine.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

SerumSynthProcessor::SerumSynthProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

SerumSynthProcessor::~SerumSynthProcessor()
{
}

void SerumSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synthEngine.prepare(sampleRate, samplesPerBlock);
    arpeggiator.prepare(sampleRate);
}

void SerumSynthProcessor::releaseResources()
{
    synthEngine.reset();
}

void SerumSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Sync parameter values from the APVTS to the synth engine
    updateSynthFromParameters();

    // Process MIDI through the arpeggiator
    juce::MidiBuffer arpeggiatedMidi;
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
        if (const auto position = playHead->getPosition())
            if (const auto hostBpm = position->getBpm())
                bpm = *hostBpm;

    arpeggiator.processBlock(midiMessages, arpeggiatedMidi,
                             buffer.getNumSamples(), bpm, getSampleRate());

    // Process audio with arpeggiated MIDI
    synthEngine.processBlock(buffer, arpeggiatedMidi);
}

juce::AudioProcessorEditor* SerumSynthProcessor::createEditor()
{
    return new SerumSynthEditor(*this);
}

void SerumSynthProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save all parameter values when Logic saves the project
    auto state = apvts.copyState();
    auto xml = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void SerumSynthProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore parameter values when Logic opens a saved project
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr)
    {
        auto newState = juce::ValueTree::fromXml(*xml);
        if (newState.isValid())
            apvts.replaceState(newState);
    }
}

void SerumSynthProcessor::updateSynthFromParameters()
{
    // Read parameter values from the APVTS and apply them to the synth engine.
    // This is called every audio block to keep the engine in sync.

    // --- Master ---
    synthEngine.setMasterVolume(apvts.getRawParameterValue("masterVolume")->load());

    // --- Oscillator A ---
    synthEngine.forAllVoices([this](SynthVoice& voice) {
        auto& oscA = voice.getOscA();
        voice.setOscAWavetablePositionBase(apvts.getRawParameterValue("oscA_wtPos")->load());
        voice.setOscALevelBase(apvts.getRawParameterValue("oscA_level")->load());
        voice.setOscAPanBase(apvts.getRawParameterValue("oscA_pan")->load());
        voice.setOscADetuneBase(apvts.getRawParameterValue("oscA_detune")->load());
        oscA.setUnisonVoices(static_cast<int>(apvts.getRawParameterValue("oscA_unison")->load()));
        oscA.setUnisonDetune(apvts.getRawParameterValue("oscA_unisonDetune")->load());
        voice.setOscAWarpBase(apvts.getRawParameterValue("oscA_warp")->load());
        oscA.setOctave(static_cast<int>(apvts.getRawParameterValue("oscA_octave")->load()));
        oscA.setEnabled(apvts.getRawParameterValue("oscA_enabled")->load() > 0.5f);

        // Oscillator B
        auto& oscB = voice.getOscB();
        voice.setOscBWavetablePositionBase(apvts.getRawParameterValue("oscB_wtPos")->load());
        voice.setOscBLevelBase(apvts.getRawParameterValue("oscB_level")->load());
        voice.setOscBPanBase(apvts.getRawParameterValue("oscB_pan")->load());
        voice.setOscBDetuneBase(apvts.getRawParameterValue("oscB_detune")->load());
        oscB.setUnisonVoices(static_cast<int>(apvts.getRawParameterValue("oscB_unison")->load()));
        oscB.setUnisonDetune(apvts.getRawParameterValue("oscB_unisonDetune")->load());
        voice.setOscBWarpBase(apvts.getRawParameterValue("oscB_warp")->load());
        oscB.setOctave(static_cast<int>(apvts.getRawParameterValue("oscB_octave")->load()));
        oscB.setEnabled(apvts.getRawParameterValue("oscB_enabled")->load() > 0.5f);

        // Sub oscillator
        auto& sub = voice.getSubOsc();
        sub.setLevel(apvts.getRawParameterValue("sub_level")->load());
        sub.setOctave(static_cast<int>(apvts.getRawParameterValue("sub_octave")->load()));
        sub.setEnabled(apvts.getRawParameterValue("sub_enabled")->load() > 0.5f);

        // Noise
        auto& noise = voice.getNoise();
        noise.setLevel(apvts.getRawParameterValue("noise_level")->load());
        noise.setEnabled(apvts.getRawParameterValue("noise_enabled")->load() > 0.5f);

        // Filter
        auto& filter = voice.getFilter();
        voice.setFilterCutoffBase(apvts.getRawParameterValue("filter_cutoff")->load());
        voice.setFilterResonanceBase(apvts.getRawParameterValue("filter_resonance")->load());
        voice.setFilterDriveBase(apvts.getRawParameterValue("filter_drive")->load());
        voice.setFilterMixBase(apvts.getRawParameterValue("filter_mix")->load());
        filter.setKeyTrack(apvts.getRawParameterValue("filter_keyTrack")->load());
        filter.setEnabled(apvts.getRawParameterValue("filter_enabled")->load() > 0.5f);

        // Amp Envelope (ENV 1)
        auto& ampEnv = voice.getAmpEnvelope();
        ampEnv.setAttack(apvts.getRawParameterValue("env1_attack")->load());
        ampEnv.setDecay(apvts.getRawParameterValue("env1_decay")->load());
        ampEnv.setSustain(apvts.getRawParameterValue("env1_sustain")->load());
        ampEnv.setRelease(apvts.getRawParameterValue("env1_release")->load());

        // Filter Envelope (ENV 2)
        auto& filtEnv = voice.getFilterEnvelope();
        filtEnv.setAttack(apvts.getRawParameterValue("env2_attack")->load());
        filtEnv.setDecay(apvts.getRawParameterValue("env2_decay")->load());
        filtEnv.setSustain(apvts.getRawParameterValue("env2_sustain")->load());
        filtEnv.setRelease(apvts.getRawParameterValue("env2_release")->load());

        // Four independent LFOs. The MOD tab rebinds its controls to the selected one.
        for (int lfoIndex = 0; lfoIndex < 4; ++lfoIndex)
        {
            const auto prefix = "lfo" + juce::String(lfoIndex + 1);
            auto& lfo = voice.getLFO(lfoIndex);
            lfo.setRate(apvts.getRawParameterValue(prefix + "_rate")->load());
            lfo.setDepth(apvts.getRawParameterValue(prefix + "_depth")->load());
        }
    });

    // --- Effects ---
    auto& fx = synthEngine.getEffectsChain();

    fx.getDistortion().setDrive(apvts.getRawParameterValue("fx_distDrive")->load());
    fx.getDistortion().setMix(apvts.getRawParameterValue("fx_distMix")->load());
    fx.getDistortion().setEnabled(apvts.getRawParameterValue("fx_distEnabled")->load() > 0.5f);

    fx.getChorus().setRate(apvts.getRawParameterValue("fx_chorusRate")->load());
    fx.getChorus().setDepth(apvts.getRawParameterValue("fx_chorusDepth")->load());
    fx.getChorus().setMix(apvts.getRawParameterValue("fx_chorusMix")->load());
    fx.getChorus().setEnabled(apvts.getRawParameterValue("fx_chorusEnabled")->load() > 0.5f);

    fx.getDelay().setDelayTime(apvts.getRawParameterValue("fx_delayTime")->load());
    fx.getDelay().setFeedback(apvts.getRawParameterValue("fx_delayFeedback")->load());
    fx.getDelay().setMix(apvts.getRawParameterValue("fx_delayMix")->load());
    fx.getDelay().setEnabled(apvts.getRawParameterValue("fx_delayEnabled")->load() > 0.5f);

    fx.getReverb().setSize(apvts.getRawParameterValue("fx_reverbSize")->load());
    fx.getReverb().setDamping(apvts.getRawParameterValue("fx_reverbDamping")->load());
    fx.getReverb().setMix(apvts.getRawParameterValue("fx_reverbMix")->load());
    fx.getReverb().setEnabled(apvts.getRawParameterValue("fx_reverbEnabled")->load() > 0.5f);

    // --- Arpeggiator ---
    arpeggiator.setEnabled(apvts.getRawParameterValue("arp_enabled")->load() > 0.5f);

    int patternIdx = static_cast<int>(apvts.getRawParameterValue("arp_pattern")->load());
    switch (patternIdx)
    {
        case 0: arpeggiator.setPattern(Arpeggiator::Pattern::Up); break;
        case 1: arpeggiator.setPattern(Arpeggiator::Pattern::Down); break;
        case 2: arpeggiator.setPattern(Arpeggiator::Pattern::UpDown); break;
        case 3: arpeggiator.setPattern(Arpeggiator::Pattern::DownUp); break;
        case 4: arpeggiator.setPattern(Arpeggiator::Pattern::Random); break;
        case 5: arpeggiator.setPattern(Arpeggiator::Pattern::Order); break;
        default: arpeggiator.setPattern(Arpeggiator::Pattern::Up); break;
    }

    int rateIdx = static_cast<int>(apvts.getRawParameterValue("arp_rate")->load());
    switch (rateIdx)
    {
        case 0: arpeggiator.setRate(Arpeggiator::Rate::OneQuarter); break;      // 1/1
        case 1: arpeggiator.setRate(Arpeggiator::Rate::OneEighth); break;        // 1/2
        case 2: arpeggiator.setRate(Arpeggiator::Rate::OneEighth); break;        // 1/4
        case 3: arpeggiator.setRate(Arpeggiator::Rate::OneEighth); break;        // 1/8
        case 4: arpeggiator.setRate(Arpeggiator::Rate::OneSixteenth); break;     // 1/16
        case 5: arpeggiator.setRate(Arpeggiator::Rate::OneThirtysecond); break;  // 1/32
        case 6: arpeggiator.setRate(Arpeggiator::Rate::OneEighthTriplet); break; // 1/8T
        case 7: arpeggiator.setRate(Arpeggiator::Rate::OneSixteenthTriplet); break; // 1/16T
        default: arpeggiator.setRate(Arpeggiator::Rate::OneEighth); break;
    }

    arpeggiator.setOctaveRange(static_cast<int>(apvts.getRawParameterValue("arp_octaves")->load()));
    arpeggiator.setGateLength(apvts.getRawParameterValue("arp_gate")->load());
    arpeggiator.setSwing(apvts.getRawParameterValue("arp_swing")->load());

    // --- Macros ---
    for (int i = 0; i < 4; ++i)
    {
        juce::String paramId = "macro" + juce::String(i + 1);
        synthEngine.setMacro(i, apvts.getRawParameterValue(paramId)->load());
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SerumSynthProcessor::createParameterLayout()
{
    // Define ALL automatable parameters here.
    // These show up in Logic's automation lanes and can be mapped to MIDI controllers.

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === MASTER ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterVolume", "Master Volume", 0.0f, 1.0f, 0.8f));

    // === OSCILLATOR A ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_wtPos", "Osc A WT Position", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_level", "Osc A Level", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_pan", "Osc A Pan", -1.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_detune", "Osc A Detune", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_unison", "Osc A Unison", 1.0f, 16.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_unisonDetune", "Osc A Unison Detune", 0.0f, 100.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_warp", "Osc A Warp", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscA_octave", "Osc A Octave", -3.0f, 3.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "oscA_enabled", "Osc A Enabled", true));

    // === OSCILLATOR B ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_wtPos", "Osc B WT Position", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_level", "Osc B Level", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_pan", "Osc B Pan", -1.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_detune", "Osc B Detune", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_unison", "Osc B Unison", 1.0f, 16.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_unisonDetune", "Osc B Unison Detune", 0.0f, 100.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_warp", "Osc B Warp", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscB_octave", "Osc B Octave", -3.0f, 3.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "oscB_enabled", "Osc B Enabled", false));

    // === SUB OSCILLATOR ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sub_level", "Sub Level", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sub_octave", "Sub Octave", -2.0f, 0.0f, -1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "sub_enabled", "Sub Enabled", false));

    // === NOISE ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "noise_level", "Noise Level", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "noise_enabled", "Noise Enabled", false));

    // === FILTER ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter_cutoff", "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f), // Skewed for musical response
        20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter_resonance", "Filter Resonance", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter_drive", "Filter Drive", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter_mix", "Filter Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter_keyTrack", "Filter Key Track", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "filter_enabled", "Filter Enabled", true));

    // === ENVELOPE 1 (Amp) ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env1_attack", "Env 1 Attack",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env1_decay", "Env 1 Decay",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env1_sustain", "Env 1 Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env1_release", "Env 1 Release",
        juce::NormalisableRange<float>(0.001f, 30.0f, 0.001f, 0.3f), 0.3f));

    // === ENVELOPE 2 (Filter) ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env2_attack", "Env 2 Attack",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env2_decay", "Env 2 Decay",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env2_sustain", "Env 2 Sustain", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "env2_release", "Env 2 Release",
        juce::NormalisableRange<float>(0.001f, 30.0f, 0.001f, 0.3f), 0.5f));

    // === LFOs ===
    for (int i = 1; i <= 4; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "lfo" + juce::String(i) + "_rate", "LFO " + juce::String(i) + " Rate",
            juce::NormalisableRange<float>(0.01f, 50.0f, 0.01f, 0.3f), 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "lfo" + juce::String(i) + "_depth", "LFO " + juce::String(i) + " Depth",
            0.0f, 1.0f, 1.0f));
    }

    // === EFFECTS ===
    // Distortion
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_distDrive", "Distortion Drive", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_distMix", "Distortion Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "fx_distEnabled", "Distortion Enabled", false));

    // Chorus
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_chorusRate", "Chorus Rate", 0.1f, 10.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_chorusDepth", "Chorus Depth", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_chorusMix", "Chorus Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "fx_chorusEnabled", "Chorus Enabled", false));

    // Delay
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_delayTime", "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f, 0.3f), 375.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_delayFeedback", "Delay Feedback", 0.0f, 0.95f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_delayMix", "Delay Mix", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "fx_delayEnabled", "Delay Enabled", false));

    // Reverb
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_reverbSize", "Reverb Size", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_reverbDamping", "Reverb Damping", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fx_reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "fx_reverbEnabled", "Reverb Enabled", false));

    // === ARPEGGIATOR ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "arp_enabled", "Arpeggiator Enabled", false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "arp_pattern", "Arpeggiator Pattern",
        juce::StringArray("Up", "Down", "UpDown", "DownUp", "Random", "Order"), 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "arp_rate", "Arpeggiator Rate",
        juce::StringArray("1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/8T", "1/16T"), 3));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "arp_octaves", "Arpeggiator Octaves", 1, 4, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "arp_gate", "Arpeggiator Gate",
        juce::NormalisableRange<float>(0.05f, 1.0f, 0.01f, 1.0f), 0.8f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "arp_swing", "Arpeggiator Swing",
        juce::NormalisableRange<float>(0.0f, 0.75f, 0.01f, 1.0f), 0.0f));

    // === MACROS ===
    for (int i = 1; i <= 4; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "macro" + juce::String(i), "Macro " + juce::String(i), 0.0f, 1.0f, 0.0f));
    }

    return { params.begin(), params.end() };
}

// This creates the plugin instance — required by JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SerumSynthProcessor();
}
