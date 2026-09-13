/*
  ==============================================================================
    EffectsPanel.cpp
  ==============================================================================
*/

#include "EffectsPanel.h"

EffectsPanel::EffectsPanel(juce::AudioProcessorValueTreeState& apvts)
{
    auto setupGroup = [&](const juce::String& prefix, const juce::String& name,
                          juce::ToggleButton& enableBtn, std::unique_ptr<ButtonAttachment>& enableAtt)
    {
        enableBtn.setButtonText(name);
        enableBtn.setLookAndFeel(&knobLnf);
        addAndMakeVisible(enableBtn);
        enableAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "Enabled", enableBtn);
    };

    // Setup all knobs
    setupKnob(distDriveSlider, distDriveLabel, "DRIVE");
    setupKnob(distMixSlider, distMixLabel, "MIX");
    setupGroup("fx_dist", "DISTORTION", distEnableButton, distEnableAtt);
    distDriveAtt = std::make_unique<SliderAttachment>(apvts, "fx_distDrive", distDriveSlider);
    distMixAtt = std::make_unique<SliderAttachment>(apvts, "fx_distMix", distMixSlider);

    setupKnob(chorusRateSlider, chorusRateLabel, "RATE");
    setupKnob(chorusDepthSlider, chorusDepthLabel, "DEPTH");
    setupKnob(chorusMixSlider, chorusMixLabel, "MIX");
    setupGroup("fx_chorus", "CHORUS", chorusEnableButton, chorusEnableAtt);
    chorusRateAtt = std::make_unique<SliderAttachment>(apvts, "fx_chorusRate", chorusRateSlider);
    chorusDepthAtt = std::make_unique<SliderAttachment>(apvts, "fx_chorusDepth", chorusDepthSlider);
    chorusMixAtt = std::make_unique<SliderAttachment>(apvts, "fx_chorusMix", chorusMixSlider);

    setupKnob(delayTimeSlider, delayTimeLabel, "TIME");
    setupKnob(delayFbSlider, delayFbLabel, "FDBK");
    setupKnob(delayMixSlider, delayMixLabel, "MIX");
    setupGroup("fx_delay", "DELAY", delayEnableButton, delayEnableAtt);
    delayTimeAtt = std::make_unique<SliderAttachment>(apvts, "fx_delayTime", delayTimeSlider);
    delayFbAtt = std::make_unique<SliderAttachment>(apvts, "fx_delayFeedback", delayFbSlider);
    delayMixAtt = std::make_unique<SliderAttachment>(apvts, "fx_delayMix", delayMixSlider);

    setupKnob(reverbSizeSlider, reverbSizeLabel, "SIZE");
    setupKnob(reverbDampSlider, reverbDampLabel, "DAMP");
    setupKnob(reverbMixSlider, reverbMixLabel, "MIX");
    setupGroup("fx_reverb", "REVERB", reverbEnableButton, reverbEnableAtt);
    reverbSizeAtt = std::make_unique<SliderAttachment>(apvts, "fx_reverbSize", reverbSizeSlider);
    reverbDampAtt = std::make_unique<SliderAttachment>(apvts, "fx_reverbDamping", reverbDampSlider);
    reverbMixAtt = std::make_unique<SliderAttachment>(apvts, "fx_reverbMix", reverbMixSlider);
}

EffectsPanel::~EffectsPanel()
{
    distDriveSlider.setLookAndFeel(nullptr); distMixSlider.setLookAndFeel(nullptr);
    chorusRateSlider.setLookAndFeel(nullptr); chorusDepthSlider.setLookAndFeel(nullptr); chorusMixSlider.setLookAndFeel(nullptr);
    delayTimeSlider.setLookAndFeel(nullptr); delayFbSlider.setLookAndFeel(nullptr); delayMixSlider.setLookAndFeel(nullptr);
    reverbSizeSlider.setLookAndFeel(nullptr); reverbDampSlider.setLookAndFeel(nullptr); reverbMixSlider.setLookAndFeel(nullptr);
    distEnableButton.setLookAndFeel(nullptr); chorusEnableButton.setLookAndFeel(nullptr);
    delayEnableButton.setLookAndFeel(nullptr); reverbEnableButton.setLookAndFeel(nullptr);
}

void EffectsPanel::setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    slider.setLookAndFeel(&knobLnf);
    slider.setName("fx_" + text);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(9.0f));
    label.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(label);
}

void EffectsPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    g.setColour(KnobLookAndFeel::accentBlue);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("FX", bounds.reduced(8, 4), juce::Justification::topLeft);
}

void EffectsPanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);
    bounds.removeFromTop(18);

    int knobSize = 45;
    int labelHeight = 12;
    int rowHeight = 20 + knobSize + labelHeight + 4;

    auto layoutFxRow = [&](juce::ToggleButton& enableBtn,
                            std::initializer_list<std::pair<juce::Slider*, juce::Label*>> knobs)
    {
        auto row = bounds.removeFromTop(rowHeight);
        enableBtn.setBounds(row.removeFromTop(18));
        row.removeFromTop(2);

        int knobWidth = row.getWidth() / static_cast<int>(knobs.size());
        for (auto& [s, l] : knobs)
        {
            auto cell = row.removeFromLeft(knobWidth);
            l->setBounds(cell.removeFromTop(labelHeight));
            s->setBounds(cell.removeFromTop(knobSize));
        }
        bounds.removeFromTop(4);
    };

    layoutFxRow(distEnableButton, {{&distDriveSlider, &distDriveLabel}, {&distMixSlider, &distMixLabel}});
    layoutFxRow(chorusEnableButton, {{&chorusRateSlider, &chorusRateLabel}, {&chorusDepthSlider, &chorusDepthLabel}, {&chorusMixSlider, &chorusMixLabel}});
    layoutFxRow(delayEnableButton, {{&delayTimeSlider, &delayTimeLabel}, {&delayFbSlider, &delayFbLabel}, {&delayMixSlider, &delayMixLabel}});
    layoutFxRow(reverbEnableButton, {{&reverbSizeSlider, &reverbSizeLabel}, {&reverbDampSlider, &reverbDampLabel}, {&reverbMixSlider, &reverbMixLabel}});
}
