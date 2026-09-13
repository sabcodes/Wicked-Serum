/*
  ==============================================================================
    FactoryPresets.cpp

    Factory preset implementations for SerumSynth.

    Defines ~10 carefully crafted presets covering common synthesis use cases:
    - Init: Basic starting point
    - Supersaw Lead: Classic wide, detuned lead
    - Deep Bass: Sub-bass with resonant filter
    - Warm Pad: Smooth, evolving pad
    - Pluck: Percussive plucked string
    - Acid Bass: Classic acid synth line sound
    - Ambient Texture: Ethereal, evolving texture
    - Wobble Bass: Modulated, dynamic bass
    - Bright Lead: Sharp, cutting lead
    - Soft Keys: Piano/keys-like sound
  ==============================================================================
*/

#include "FactoryPresets.h"

namespace FactoryPresets
{
    // Helper function to create a parameter snapshot
    static ParameterSnapshot param(const juce::String& id, float value)
    {
        return { id, value };
    }

    // ========== INIT ==========
    // Basic starting point - minimal configuration
    static FactoryPreset createInit()
    {
        FactoryPreset preset;
        preset.name = "Init";
        preset.description = "Basic starting point with Osc A square wave";
        preset.parameters = {
            // Master
            param("masterVolume", 0.8f),

            // Oscillator A - Square wave
            param("oscA_wtPos", 0.0f),
            param("oscA_level", 1.0f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", 0.0f),
            param("oscA_unison", 1.0f),
            param("oscA_unisonDetune", 20.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            // Oscillator B - Off
            param("oscB_wtPos", 0.0f),
            param("oscB_level", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),
            param("oscB_enabled", 0.0f),

            // Sub - Off
            param("sub_level", 0.0f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 0.0f),

            // Noise - Off
            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter
            param("filter_cutoff", 20000.0f),
            param("filter_resonance", 0.0f),
            param("filter_drive", 0.0f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.0f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp)
            param("env1_attack", 0.01f),
            param("env1_decay", 0.3f),
            param("env1_sustain", 0.7f),
            param("env1_release", 0.3f),

            // Envelope 2 (Filter)
            param("env2_attack", 0.01f),
            param("env2_decay", 0.5f),
            param("env2_sustain", 0.3f),
            param("env2_release", 0.5f),

            // LFO 1
            param("lfo1_rate", 1.0f),
            param("lfo1_depth", 1.0f),

            // Effects - All off
            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.0f),
            param("fx_chorusEnabled", 0.0f),
            param("fx_delayTime", 375.0f),
            param("fx_delayFeedback", 0.4f),
            param("fx_delayMix", 0.0f),
            param("fx_delayEnabled", 0.0f),
            param("fx_reverbSize", 0.5f),
            param("fx_reverbDamping", 0.5f),
            param("fx_reverbMix", 0.0f),
            param("fx_reverbEnabled", 0.0f),

            // Macros
            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== SUPERSAW LEAD ==========
    // Classic wide, detuned lead with dynamic filter
    static FactoryPreset createSupersawLead()
    {
        FactoryPreset preset;
        preset.name = "Supersaw Lead";
        preset.description = "Wide, lush detuned sawtooth lead with filter sweep";
        preset.parameters = {
            param("masterVolume", 0.85f),

            // Oscillator A - Supersaw
            param("oscA_wtPos", 0.5f),
            param("oscA_level", 1.0f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", 0.0f),
            param("oscA_unison", 7.0f),
            param("oscA_unisonDetune", 45.0f),
            param("oscA_warp", 0.3f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            param("sub_level", 0.25f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Cutoff modulated by Env2
            param("filter_cutoff", 12000.0f),
            param("filter_resonance", 0.45f),
            param("filter_drive", 0.2f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.3f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Fast attack
            param("env1_attack", 0.005f),
            param("env1_decay", 0.4f),
            param("env1_sustain", 0.8f),
            param("env1_release", 0.2f),

            // Envelope 2 (Filter) - Modulate cutoff
            param("env2_attack", 0.01f),
            param("env2_decay", 0.8f),
            param("env2_sustain", 0.4f),
            param("env2_release", 0.6f),

            param("lfo1_rate", 4.5f),
            param("lfo1_depth", 0.3f),

            // Chorus for width
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.6f),
            param("fx_chorusMix", 0.35f),
            param("fx_chorusEnabled", 1.0f),

            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_delayTime", 375.0f),
            param("fx_delayFeedback", 0.4f),
            param("fx_delayMix", 0.0f),
            param("fx_delayEnabled", 0.0f),
            param("fx_reverbSize", 0.5f),
            param("fx_reverbDamping", 0.5f),
            param("fx_reverbMix", 0.15f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== DEEP BASS ==========
    // Sub-bass with resonant filter sweep
    static FactoryPreset createDeepBass()
    {
        FactoryPreset preset;
        preset.name = "Deep Bass";
        preset.description = "Deep sub-bass with resonant filter modulation";
        preset.parameters = {
            param("masterVolume", 0.8f),

            // Oscillator A - Sawtooth
            param("oscA_wtPos", 0.3f),
            param("oscA_level", 0.7f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", -3.0f),
            param("oscA_unison", 3.0f),
            param("oscA_unisonDetune", 15.0f),
            param("oscA_warp", 0.1f),
            param("oscA_octave", -1.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            // Strong sub
            param("sub_level", 0.85f),
            param("sub_octave", -2.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - High resonance, low cutoff
            param("filter_cutoff", 800.0f),
            param("filter_resonance", 0.75f),
            param("filter_drive", 0.3f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.6f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Slightly longer attack
            param("env1_attack", 0.02f),
            param("env1_decay", 0.2f),
            param("env1_sustain", 1.0f),
            param("env1_release", 0.5f),

            // Envelope 2 (Filter) - Sweep up
            param("env2_attack", 0.05f),
            param("env2_decay", 1.2f),
            param("env2_sustain", 0.2f),
            param("env2_release", 0.8f),

            param("lfo1_rate", 0.5f),
            param("lfo1_depth", 0.2f),

            param("fx_distDrive", 0.6f),
            param("fx_distMix", 0.15f),
            param("fx_distEnabled", 1.0f),
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.0f),
            param("fx_chorusEnabled", 0.0f),
            param("fx_delayTime", 750.0f),
            param("fx_delayFeedback", 0.35f),
            param("fx_delayMix", 0.2f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.6f),
            param("fx_reverbDamping", 0.7f),
            param("fx_reverbMix", 0.2f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== WARM PAD ==========
    // Smooth, evolving pad
    static FactoryPreset createWarmPad()
    {
        FactoryPreset preset;
        preset.name = "Warm Pad";
        preset.description = "Smooth, evolving pad with slow wavetable morphing";
        preset.parameters = {
            param("masterVolume", 0.75f),

            // Oscillator A - Warm Pad wavetable
            param("oscA_wtPos", 0.6f),
            param("oscA_level", 0.95f),
            param("oscA_pan", -0.2f),
            param("oscA_detune", -2.0f),
            param("oscA_unison", 4.0f),
            param("oscA_unisonDetune", 25.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            // Oscillator B - Different wavetable, detuned
            param("oscB_wtPos", 0.4f),
            param("oscB_level", 0.6f),
            param("oscB_pan", 0.2f),
            param("oscB_detune", 3.0f),
            param("oscB_unison", 3.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),
            param("oscB_enabled", 1.0f),

            param("sub_level", 0.0f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 0.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Wide open, gentle modulation
            param("filter_cutoff", 18000.0f),
            param("filter_resonance", 0.15f),
            param("filter_drive", 0.0f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.1f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Long attack
            param("env1_attack", 0.8f),
            param("env1_decay", 0.5f),
            param("env1_sustain", 0.9f),
            param("env1_release", 1.5f),

            // Envelope 2 (Filter) - Gentle
            param("env2_attack", 0.2f),
            param("env2_decay", 0.3f),
            param("env2_sustain", 1.0f),
            param("env2_release", 1.0f),

            // LFO - Very slow modulation
            param("lfo1_rate", 0.3f),
            param("lfo1_depth", 0.15f),

            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_chorusRate", 0.6f),
            param("fx_chorusDepth", 0.7f),
            param("fx_chorusMix", 0.4f),
            param("fx_chorusEnabled", 1.0f),
            param("fx_delayTime", 600.0f),
            param("fx_delayFeedback", 0.3f),
            param("fx_delayMix", 0.25f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.75f),
            param("fx_reverbDamping", 0.4f),
            param("fx_reverbMix", 0.4f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== PLUCK ==========
    // Percussive plucked string
    static FactoryPreset createPluck()
    {
        FactoryPreset preset;
        preset.name = "Pluck";
        preset.description = "Bright, percussive plucked string with fast decay";
        preset.parameters = {
            param("masterVolume", 0.8f),

            // Oscillator A - Pluck wavetable
            param("oscA_wtPos", 0.5f),
            param("oscA_level", 1.0f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", 0.0f),
            param("oscA_unison", 2.0f),
            param("oscA_unisonDetune", 10.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            param("sub_level", 0.15f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Bright attack, quick rolloff
            param("filter_cutoff", 14000.0f),
            param("filter_resonance", 0.3f),
            param("filter_drive", 0.1f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.4f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Sharp attack, quick release
            param("env1_attack", 0.001f),
            param("env1_decay", 0.15f),
            param("env1_sustain", 0.0f),
            param("env1_release", 0.3f),

            // Envelope 2 (Filter) - Bright then rolls off
            param("env2_attack", 0.005f),
            param("env2_decay", 0.5f),
            param("env2_sustain", 0.0f),
            param("env2_release", 0.3f),

            param("lfo1_rate", 2.0f),
            param("lfo1_depth", 0.0f),

            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.1f),
            param("fx_chorusEnabled", 1.0f),
            param("fx_delayTime", 300.0f),
            param("fx_delayFeedback", 0.25f),
            param("fx_delayMix", 0.2f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.5f),
            param("fx_reverbDamping", 0.8f),
            param("fx_reverbMix", 0.25f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== ACID BASS ==========
    // Classic acid synth line sound
    static FactoryPreset createAcidBass()
    {
        FactoryPreset preset;
        preset.name = "Acid Bass";
        preset.description = "Classic TB-303 acid synth bass with resonant sweep";
        preset.parameters = {
            param("masterVolume", 0.85f),

            // Oscillator A - Acid wavetable
            param("oscA_wtPos", 0.5f),
            param("oscA_level", 0.9f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", 0.0f),
            param("oscA_unison", 1.0f),
            param("oscA_unisonDetune", 20.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", -1.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            param("sub_level", 0.3f),
            param("sub_octave", -2.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Tight sweep
            param("filter_cutoff", 400.0f),
            param("filter_resonance", 0.85f),
            param("filter_drive", 0.5f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.8f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp)
            param("env1_attack", 0.005f),
            param("env1_decay", 0.1f),
            param("env1_sustain", 0.85f),
            param("env1_release", 0.2f),

            // Envelope 2 (Filter) - Resonance sweep
            param("env2_attack", 0.01f),
            param("env2_decay", 1.0f),
            param("env2_sustain", 0.1f),
            param("env2_release", 0.4f),

            // LFO - Modulate cutoff
            param("lfo1_rate", 2.0f),
            param("lfo1_depth", 0.4f),

            param("fx_distDrive", 0.7f),
            param("fx_distMix", 0.3f),
            param("fx_distEnabled", 1.0f),
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.0f),
            param("fx_chorusEnabled", 0.0f),
            param("fx_delayTime", 500.0f),
            param("fx_delayFeedback", 0.4f),
            param("fx_delayMix", 0.1f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.4f),
            param("fx_reverbDamping", 0.6f),
            param("fx_reverbMix", 0.1f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== AMBIENT TEXTURE ==========
    // Ethereal, evolving texture
    static FactoryPreset createAmbientTexture()
    {
        FactoryPreset preset;
        preset.name = "Ambient Texture";
        preset.description = "Ethereal, slowly evolving ambient texture";
        preset.parameters = {
            param("masterVolume", 0.7f),

            // Oscillator A - Ambient
            param("oscA_wtPos", 0.7f),
            param("oscA_level", 0.8f),
            param("oscA_pan", -0.3f),
            param("oscA_detune", -4.0f),
            param("oscA_unison", 5.0f),
            param("oscA_unisonDetune", 35.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", 1.0f),
            param("oscA_enabled", 1.0f),

            // Oscillator B - Different partial
            param("oscB_wtPos", 0.3f),
            param("oscB_level", 0.5f),
            param("oscB_pan", 0.25f),
            param("oscB_detune", 5.0f),
            param("oscB_unison", 4.0f),
            param("oscB_unisonDetune", 30.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),
            param("oscB_enabled", 1.0f),

            param("sub_level", 0.2f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.15f),
            param("noise_enabled", 1.0f),

            // Filter - Open, gentle modulation
            param("filter_cutoff", 16000.0f),
            param("filter_resonance", 0.1f),
            param("filter_drive", 0.0f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.0f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Very long, pad-like
            param("env1_attack", 1.5f),
            param("env1_decay", 0.5f),
            param("env1_sustain", 0.85f),
            param("env1_release", 2.0f),

            // Envelope 2 (Filter) - Barely active
            param("env2_attack", 0.5f),
            param("env2_decay", 1.0f),
            param("env2_sustain", 1.0f),
            param("env2_release", 1.5f),

            // LFO - Very slow, subtle
            param("lfo1_rate", 0.15f),
            param("lfo1_depth", 0.2f),

            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_chorusRate", 0.5f),
            param("fx_chorusDepth", 0.8f),
            param("fx_chorusMix", 0.5f),
            param("fx_chorusEnabled", 1.0f),
            param("fx_delayTime", 1000.0f),
            param("fx_delayFeedback", 0.5f),
            param("fx_delayMix", 0.35f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.9f),
            param("fx_reverbDamping", 0.3f),
            param("fx_reverbMix", 0.55f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== WOBBLE BASS ==========
    // Modulated, dynamic bass
    static FactoryPreset createWobbleBass()
    {
        FactoryPreset preset;
        preset.name = "Wobble Bass";
        preset.description = "Dynamic, LFO-modulated wobble bass with distortion";
        preset.parameters = {
            param("masterVolume", 0.8f),

            // Oscillator A - Sawtooth
            param("oscA_wtPos", 0.2f),
            param("oscA_level", 0.75f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", -2.0f),
            param("oscA_unison", 2.0f),
            param("oscA_unisonDetune", 20.0f),
            param("oscA_warp", 0.2f),
            param("oscA_octave", -1.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            // Strong sub
            param("sub_level", 0.8f),
            param("sub_octave", -2.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Dynamic cutoff
            param("filter_cutoff", 1200.0f),
            param("filter_resonance", 0.65f),
            param("filter_drive", 0.4f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.5f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp)
            param("env1_attack", 0.015f),
            param("env1_decay", 0.2f),
            param("env1_sustain", 1.0f),
            param("env1_release", 0.3f),

            // Envelope 2 (Filter)
            param("env2_attack", 0.02f),
            param("env2_decay", 0.6f),
            param("env2_sustain", 0.3f),
            param("env2_release", 0.5f),

            // LFO - Fast wobble
            param("lfo1_rate", 6.0f),
            param("lfo1_depth", 0.65f),

            param("fx_distDrive", 0.8f),
            param("fx_distMix", 0.4f),
            param("fx_distEnabled", 1.0f),
            param("fx_chorusRate", 0.8f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.0f),
            param("fx_chorusEnabled", 0.0f),
            param("fx_delayTime", 400.0f),
            param("fx_delayFeedback", 0.3f),
            param("fx_delayMix", 0.15f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.5f),
            param("fx_reverbDamping", 0.6f),
            param("fx_reverbMix", 0.15f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== BRIGHT LEAD ==========
    // Sharp, cutting lead
    static FactoryPreset createBrightLead()
    {
        FactoryPreset preset;
        preset.name = "Bright Lead";
        preset.description = "Sharp, cutting lead with high resonance and drive";
        preset.parameters = {
            param("masterVolume", 0.85f),

            // Oscillator A - Digital
            param("oscA_wtPos", 0.4f),
            param("oscA_level", 1.0f),
            param("oscA_pan", 0.0f),
            param("oscA_detune", 1.0f),
            param("oscA_unison", 3.0f),
            param("oscA_unisonDetune", 12.0f),
            param("oscA_warp", 0.4f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            param("oscB_level", 0.0f),
            param("oscB_enabled", 0.0f),
            param("oscB_wtPos", 0.0f),
            param("oscB_pan", 0.0f),
            param("oscB_detune", 0.0f),
            param("oscB_unison", 1.0f),
            param("oscB_unisonDetune", 20.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 0.0f),

            param("sub_level", 0.0f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 0.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Bright with high resonance
            param("filter_cutoff", 15000.0f),
            param("filter_resonance", 0.8f),
            param("filter_drive", 0.6f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.5f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Very fast
            param("env1_attack", 0.001f),
            param("env1_decay", 0.2f),
            param("env1_sustain", 0.75f),
            param("env1_release", 0.15f),

            // Envelope 2 (Filter) - Bright onset
            param("env2_attack", 0.002f),
            param("env2_decay", 0.4f),
            param("env2_sustain", 0.5f),
            param("env2_release", 0.3f),

            // LFO - Subtle modulation
            param("lfo1_rate", 5.0f),
            param("lfo1_depth", 0.2f),

            param("fx_distDrive", 0.7f),
            param("fx_distMix", 0.25f),
            param("fx_distEnabled", 1.0f),
            param("fx_chorusRate", 0.9f),
            param("fx_chorusDepth", 0.5f),
            param("fx_chorusMix", 0.2f),
            param("fx_chorusEnabled", 1.0f),
            param("fx_delayTime", 350.0f),
            param("fx_delayFeedback", 0.3f),
            param("fx_delayMix", 0.1f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.4f),
            param("fx_reverbDamping", 0.7f),
            param("fx_reverbMix", 0.1f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== SOFT KEYS ==========
    // Piano/keys-like sound
    static FactoryPreset createSoftKeys()
    {
        FactoryPreset preset;
        preset.name = "Soft Keys";
        preset.description = "Warm, piano-like keys with smooth envelope and light reverb";
        preset.parameters = {
            param("masterVolume", 0.8f),

            // Oscillator A - Warm sine/fundamental
            param("oscA_wtPos", 0.1f),
            param("oscA_level", 0.85f),
            param("oscA_pan", -0.15f),
            param("oscA_detune", -1.5f),
            param("oscA_unison", 2.0f),
            param("oscA_unisonDetune", 8.0f),
            param("oscA_warp", 0.0f),
            param("oscA_octave", 0.0f),
            param("oscA_enabled", 1.0f),

            // Oscillator B - Harmonics
            param("oscB_wtPos", 0.3f),
            param("oscB_level", 0.4f),
            param("oscB_pan", 0.1f),
            param("oscB_detune", 1.5f),
            param("oscB_unison", 2.0f),
            param("oscB_unisonDetune", 8.0f),
            param("oscB_warp", 0.0f),
            param("oscB_octave", 1.0f),
            param("oscB_enabled", 1.0f),

            param("sub_level", 0.1f),
            param("sub_octave", -1.0f),
            param("sub_enabled", 1.0f),

            param("noise_level", 0.0f),
            param("noise_enabled", 0.0f),

            // Filter - Open, minimal coloration
            param("filter_cutoff", 19000.0f),
            param("filter_resonance", 0.1f),
            param("filter_drive", 0.0f),
            param("filter_mix", 1.0f),
            param("filter_keyTrack", 0.0f),
            param("filter_enabled", 1.0f),

            // Envelope 1 (Amp) - Piano-like attack/release
            param("env1_attack", 0.03f),
            param("env1_decay", 0.4f),
            param("env1_sustain", 0.6f),
            param("env1_release", 1.0f),

            // Envelope 2 (Filter) - Minimal movement
            param("env2_attack", 0.05f),
            param("env2_decay", 0.3f),
            param("env2_sustain", 0.9f),
            param("env2_release", 0.8f),

            // LFO - Off
            param("lfo1_rate", 1.0f),
            param("lfo1_depth", 0.0f),

            param("fx_distDrive", 0.5f),
            param("fx_distMix", 0.0f),
            param("fx_distEnabled", 0.0f),
            param("fx_chorusRate", 0.7f),
            param("fx_chorusDepth", 0.4f),
            param("fx_chorusMix", 0.15f),
            param("fx_chorusEnabled", 1.0f),
            param("fx_delayTime", 500.0f),
            param("fx_delayFeedback", 0.2f),
            param("fx_delayMix", 0.1f),
            param("fx_delayEnabled", 1.0f),
            param("fx_reverbSize", 0.65f),
            param("fx_reverbDamping", 0.5f),
            param("fx_reverbMix", 0.3f),
            param("fx_reverbEnabled", 1.0f),

            param("macro1", 0.0f),
            param("macro2", 0.0f),
            param("macro3", 0.0f),
            param("macro4", 0.0f),
        };
        return preset;
    }

    // ========== PUBLIC API ==========

    std::vector<FactoryPreset> getFactoryPresets()
    {
        std::vector<FactoryPreset> presets;

        presets.push_back(createInit());
        presets.push_back(createSupersawLead());
        presets.push_back(createDeepBass());
        presets.push_back(createWarmPad());
        presets.push_back(createPluck());
        presets.push_back(createAcidBass());
        presets.push_back(createAmbientTexture());
        presets.push_back(createWobbleBass());
        presets.push_back(createBrightLead());
        presets.push_back(createSoftKeys());

        return presets;
    }

    FactoryPreset getPresetByName(const juce::String& name)
    {
        auto presets = getFactoryPresets();
        for (const auto& preset : presets)
        {
            if (preset.name == name)
                return preset;
        }

        // Return Init if not found
        return presets[0];
    }

    void applyPreset(juce::AudioProcessorValueTreeState& apvts, const FactoryPreset& preset)
    {
        for (const auto& param : preset.parameters)
        {
            auto* parameter = apvts.getParameter(param.parameterId);
            if (parameter != nullptr)
            {
                parameter->setValueNotifyingHost(parameter->convertTo0to1(param.value));
            }
        }
    }

} // namespace FactoryPresets
