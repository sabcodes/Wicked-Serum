/*
  ==============================================================================
    OscillatorPanel.cpp
  ==============================================================================
*/

#include "OscillatorPanel.h"

OscillatorPanel::OscillatorPanel(juce::AudioProcessorValueTreeState& apvts,
                                   const juce::String& oscPrefix,
                                   const juce::String& label)
    : oscLabel(label)
{
    waveformDisplay.setLabel(label);
    if (label.contains("B"))
        waveformDisplay.setAccentColour(KnobLookAndFeel::accentBlue);
    addAndMakeVisible(waveformDisplay);

    // Setup knobs
    setupKnob(wtPosSlider, wtPosLabel, "WT POS");
    setupKnob(levelSlider, levelLabel, "LEVEL");
    setupKnob(panSlider, panLabel, "PAN");
    setupKnob(detuneSlider, detuneLabel, "DETUNE");
    setupKnob(unisonSlider, unisonLabel, "UNISON");
    setupKnob(unisonDetuneSlider, unisonDetuneLabel, "UNI DET");
    setupKnob(warpSlider, warpLabel, "WARP");
    setupKnob(octaveSlider, octaveLabel, "OCTAVE");

    enableButton.setButtonText(label);
    enableButton.setLookAndFeel(&knobLnf);
    addAndMakeVisible(enableButton);

    // Create parameter attachments
    wtPosAtt       = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_wtPos", wtPosSlider);
    levelAtt       = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_level", levelSlider);
    panAtt         = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_pan", panSlider);
    detuneAtt      = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_detune", detuneSlider);
    unisonAtt      = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_unison", unisonSlider);
    unisonDetuneAtt = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_unisonDetune", unisonDetuneSlider);
    warpAtt        = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_warp", warpSlider);
    octaveAtt      = std::make_unique<SliderAttachment>(apvts, oscPrefix + "_octave", octaveSlider);
    enableAtt      = std::make_unique<ButtonAttachment>(apvts, oscPrefix + "_enabled", enableButton);
}

OscillatorPanel::~OscillatorPanel()
{
    wtPosSlider.setLookAndFeel(nullptr);
    levelSlider.setLookAndFeel(nullptr);
    panSlider.setLookAndFeel(nullptr);
    detuneSlider.setLookAndFeel(nullptr);
    unisonSlider.setLookAndFeel(nullptr);
    unisonDetuneSlider.setLookAndFeel(nullptr);
    warpSlider.setLookAndFeel(nullptr);
    octaveSlider.setLookAndFeel(nullptr);
    enableButton.setLookAndFeel(nullptr);
}

void OscillatorPanel::setupKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    slider.setLookAndFeel(&knobLnf);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f));
    label.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(label);
}

void OscillatorPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
}

void OscillatorPanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    // Enable button at top-left
    enableButton.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);

    // Waveform display takes top portion
    waveformDisplay.setBounds(bounds.removeFromTop(100));
    bounds.removeFromTop(6);

    // Knobs in two rows
    int knobSize = 55;
    int labelHeight = 14;

    auto row1 = bounds.removeFromTop(knobSize + labelHeight);
    int knobWidth = row1.getWidth() / 4;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int>& area) {
        auto cell = area.removeFromLeft(knobWidth);
        l.setBounds(cell.removeFromTop(labelHeight));
        s.setBounds(cell);
    };

    placeKnob(wtPosSlider, wtPosLabel, row1);
    placeKnob(levelSlider, levelLabel, row1);
    placeKnob(panSlider, panLabel, row1);
    placeKnob(detuneSlider, detuneLabel, row1);

    bounds.removeFromTop(4);
    auto row2 = bounds.removeFromTop(knobSize + labelHeight);

    placeKnob(unisonSlider, unisonLabel, row2);
    placeKnob(unisonDetuneSlider, unisonDetuneLabel, row2);
    placeKnob(warpSlider, warpLabel, row2);
    placeKnob(octaveSlider, octaveLabel, row2);
}
