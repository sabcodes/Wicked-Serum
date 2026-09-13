/*
  ==============================================================================
    OscillatorPanel.h

    GUI panel for one oscillator — shows waveform display and all knobs.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WaveformDisplay.h"
#include "KnobLookAndFeel.h"

class OscillatorPanel : public juce::Component
{
public:
    OscillatorPanel(juce::AudioProcessorValueTreeState& apvts, const juce::String& oscPrefix, const juce::String& label);
    ~OscillatorPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    WaveformDisplay& getWaveformDisplay() { return waveformDisplay; }

public:
    juce::Slider& getWTPosSlider() { return wtPosSlider; }
    juce::Slider& getLevelSlider() { return levelSlider; }
    juce::Slider& getPanSlider() { return panSlider; }
    juce::Slider& getDetuneSlider() { return detuneSlider; }
    juce::Slider& getWarpSlider() { return warpSlider; }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::String oscLabel;
    WaveformDisplay waveformDisplay;

    juce::Slider wtPosSlider, levelSlider, panSlider, detuneSlider;
    juce::Slider unisonSlider, unisonDetuneSlider, warpSlider, octaveSlider;
    juce::ToggleButton enableButton;

    juce::Label wtPosLabel, levelLabel, panLabel, detuneLabel;
    juce::Label unisonLabel, unisonDetuneLabel, warpLabel, octaveLabel;

    std::unique_ptr<SliderAttachment> wtPosAtt, levelAtt, panAtt, detuneAtt;
    std::unique_ptr<SliderAttachment> unisonAtt, unisonDetuneAtt, warpAtt, octaveAtt;
    std::unique_ptr<ButtonAttachment> enableAtt;

    KnobLookAndFeel knobLnf;

    void setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
};
