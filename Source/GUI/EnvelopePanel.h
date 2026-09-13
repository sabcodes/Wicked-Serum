/*
  ==============================================================================
    EnvelopePanel.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KnobLookAndFeel.h"

class EnvelopePanel : public juce::Component
{
public:
    EnvelopePanel(juce::AudioProcessorValueTreeState& apvts, const juce::String& envPrefix, const juce::String& label);
    ~EnvelopePanel() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    juce::String envLabel;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    std::unique_ptr<SliderAttachment> attackAtt, decayAtt, sustainAtt, releaseAtt;
    KnobLookAndFeel knobLnf;
    void setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
};
