/*
  ==============================================================================
    PluginEditor.h

    Tabbed GUI — Oscillators always visible on top, tabbed bottom section:

    ┌──────────────────────────────────────────────────────┐
    │  SERUM SYNTH  [< preset name >]  [SAVE]  Voices: 0  │
    ├────────────────────────┬─────────────────────────────┤
    │    OSCILLATOR A        │      OSCILLATOR B           │
    │    [waveform]          │      [waveform]             │
    │    knobs...            │      knobs...               │
    ├────────────────────────┴─────────────────────────────┤
    │  FILTER │ ENV 1 (AMP) │ ENV 2 (FILTER) │  MASTER    │
    ├──────────────────────────────────────────────────────┤
    │ [FX] [MOD] [ARP] [EDITOR]  ← tab buttons            │
    │                                                      │
    │  (tabbed content area — shows one panel at a time)   │
    │                                                      │
    └──────────────────────────────────────────────────────┘
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/OscillatorPanel.h"
#include "GUI/FilterPanel.h"
#include "GUI/EnvelopePanel.h"
#include "GUI/EffectsPanel.h"
#include "GUI/ModulationPanel.h"
#include "GUI/PresetPanel.h"
#include "GUI/ModulationOverlay.h"
#include "GUI/WavetableEditor.h"
#include "GUI/KnobLookAndFeel.h"

class SerumSynthEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit SerumSynthEditor(SerumSynthProcessor&);
    ~SerumSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void wireModulationCallbacks();
    void switchTab(int tabIndex);

    SerumSynthProcessor& processor;

    // Modulation overlay
    ModulationOverlay modOverlay;

    // Separate display oscillators — not shared with audio thread
    WavetableOscillator displayOscA, displayOscB;

    // Always-visible panels
    PresetPanel presetPanel;
    OscillatorPanel oscAPanel;
    OscillatorPanel oscBPanel;
    FilterPanel filterPanel;
    EnvelopePanel env1Panel;
    EnvelopePanel env2Panel;

    // Master volume
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;
    juce::Label voiceCountLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAtt;

    // Tab buttons
    juce::TextButton fxTabBtn, modTabBtn, arpTabBtn, editorTabBtn;
    int currentTab = 0; // 0=FX, 1=MOD, 2=ARP, 3=EDITOR

    // Tabbed panels
    EffectsPanel effectsPanel;
    ModulationPanel modulationPanel;
    WavetableEditor wavetableEditor;

    // Arpeggiator controls (simple panel built inline)
    std::unique_ptr<juce::Component> arpPanel;
    int currentArpStep = 0;  // Track for ArpPanel step highlighting

    KnobLookAndFeel knobLnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerumSynthEditor)
};
