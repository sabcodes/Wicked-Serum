/*
  ==============================================================================
    EffectsPanel.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KnobLookAndFeel.h"

class EffectsPanel : public juce::Component
{
public:
    EffectsPanel(juce::AudioProcessorValueTreeState& apvts);
    ~EffectsPanel() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

public:
    juce::Slider& getDistDriveSlider() { return distDriveSlider; }
    juce::Slider& getDistMixSlider() { return distMixSlider; }
    juce::Slider& getChorusMixSlider() { return chorusMixSlider; }
    juce::Slider& getDelayMixSlider() { return delayMixSlider; }
    juce::Slider& getReverbMixSlider() { return reverbMixSlider; }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Distortion
    juce::Slider distDriveSlider, distMixSlider;
    juce::Label distDriveLabel, distMixLabel;
    juce::ToggleButton distEnableButton;
    std::unique_ptr<SliderAttachment> distDriveAtt, distMixAtt;
    std::unique_ptr<ButtonAttachment> distEnableAtt;

    // Chorus
    juce::Slider chorusRateSlider, chorusDepthSlider, chorusMixSlider;
    juce::Label chorusRateLabel, chorusDepthLabel, chorusMixLabel;
    juce::ToggleButton chorusEnableButton;
    std::unique_ptr<SliderAttachment> chorusRateAtt, chorusDepthAtt, chorusMixAtt;
    std::unique_ptr<ButtonAttachment> chorusEnableAtt;

    // Delay
    juce::Slider delayTimeSlider, delayFbSlider, delayMixSlider;
    juce::Label delayTimeLabel, delayFbLabel, delayMixLabel;
    juce::ToggleButton delayEnableButton;
    std::unique_ptr<SliderAttachment> delayTimeAtt, delayFbAtt, delayMixAtt;
    std::unique_ptr<ButtonAttachment> delayEnableAtt;

    // Reverb
    juce::Slider reverbSizeSlider, reverbDampSlider, reverbMixSlider;
    juce::Label reverbSizeLabel, reverbDampLabel, reverbMixLabel;
    juce::ToggleButton reverbEnableButton;
    std::unique_ptr<SliderAttachment> reverbSizeAtt, reverbDampAtt, reverbMixAtt;
    std::unique_ptr<ButtonAttachment> reverbEnableAtt;

    KnobLookAndFeel knobLnf;
    void setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
};
