#pragma once

#include <JuceHeader.h>
#include "KnobLookAndFeel.h"
#include "ModulationOverlay.h"
#include "LFOEditor.h"

class ModulationPanel : public juce::Component
{
public:
    ModulationPanel(juce::AudioProcessorValueTreeState&, ModulationOverlay&);
    ~ModulationPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    LFOEditor& getLFOEditor() { return lfoEditor; }
    int getSelectedLfoIndex() const { return selectedLfoIndex; }
    std::function<void(int, const std::array<float, LFO_EDITOR_POINTS>&)> onLfoShapeChanged;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::AudioProcessorValueTreeState& apvts;
    ModulationOverlay& modOverlay;
    KnobLookAndFeel knobLnf;
    LFOEditor lfoEditor;

    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Label lfoRateLabel, lfoDepthLabel, selectedSourceLabel, routeStatusLabel;
    std::unique_ptr<SliderAttachment> lfoRateAtt, lfoDepthAtt;

    juce::Slider macro1Slider, macro2Slider, macro3Slider, macro4Slider;
    juce::Label macro1Label, macro2Label, macro3Label, macro4Label;
    std::unique_ptr<SliderAttachment> macro1Att, macro2Att, macro3Att, macro4Att;

    ModSourceBadge env1Badge, env2Badge, env3Badge;
    ModSourceBadge lfo1Badge, lfo2Badge, lfo3Badge, lfo4Badge;
    ModSourceBadge velBadge, noteBadge, modWheelBadge;
    ModSourceBadge macro1Badge, macro2Badge, macro3Badge, macro4Badge;

    juce::ComboBox destinationBox;
    juce::Slider routeAmountSlider;
    juce::TextButton addRouteButton { "ADD ROUTE" }, clearRoutesButton { "CLEAR ROUTES" };
    juce::Label destinationLabel, amountLabel, routingHelpLabel;

    std::array<std::array<float, LFO_EDITOR_POINTS>, 4> lfoShapes{};
    ModSource selectedSource = ModSource::LFO1;
    int selectedLfoIndex = 0;

    void setupKnob(juce::Slider&, juce::Label&, const juce::String&);
    void setupBadge(ModSourceBadge&);
    void selectSource(ModSource);
    void selectLfo(int);
    ModDestination selectedDestination() const;
    juce::String sourceName(ModSource) const;
    std::array<ModSourceBadge*, 14> badges();
};
