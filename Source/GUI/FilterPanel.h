/*
  ==============================================================================
    FilterPanel.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KnobLookAndFeel.h"

class FilterPanel : public juce::Component
{
public:
    FilterPanel(juce::AudioProcessorValueTreeState& apvts);
    ~FilterPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

public:
    juce::Slider& getCutoffSlider() { return cutoffSlider; }
    juce::Slider& getResonanceSlider() { return resonanceSlider; }
    juce::Slider& getDriveSlider() { return driveSlider; }
    juce::Slider& getMixSlider() { return mixSlider; }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::Slider cutoffSlider, resonanceSlider, driveSlider, mixSlider, keyTrackSlider;
    juce::Label cutoffLabel, resonanceLabel, driveLabel, mixLabel, keyTrackLabel;
    juce::ToggleButton enableButton;

    std::unique_ptr<SliderAttachment> cutoffAtt, resonanceAtt, driveAtt, mixAtt, keyTrackAtt;
    std::unique_ptr<ButtonAttachment> enableAtt;

    KnobLookAndFeel knobLnf;
    void setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
};
