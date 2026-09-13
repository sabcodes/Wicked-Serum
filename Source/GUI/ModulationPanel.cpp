#include "ModulationPanel.h"

ModulationPanel::ModulationPanel(juce::AudioProcessorValueTreeState& state,
                                 ModulationOverlay& overlay)
    : apvts(state), modOverlay(overlay),
      env1Badge(ModSource::Env1, "ENV1", KnobLookAndFeel::accentPink),
      env2Badge(ModSource::Env2, "ENV2", KnobLookAndFeel::accentPink),
      env3Badge(ModSource::Env3, "ENV3", KnobLookAndFeel::accentPink),
      lfo1Badge(ModSource::LFO1, "LFO1", KnobLookAndFeel::accentBlue),
      lfo2Badge(ModSource::LFO2, "LFO2", KnobLookAndFeel::accentBlue),
      lfo3Badge(ModSource::LFO3, "LFO3", KnobLookAndFeel::accentBlue),
      lfo4Badge(ModSource::LFO4, "LFO4", KnobLookAndFeel::accentBlue),
      velBadge(ModSource::Velocity, "VEL", KnobLookAndFeel::accentOrange),
      noteBadge(ModSource::NoteNumber, "NOTE", KnobLookAndFeel::accentOrange),
      modWheelBadge(ModSource::ModWheel, "MW", KnobLookAndFeel::accentOrange),
      macro1Badge(ModSource::Macro1, "M1", KnobLookAndFeel::accentGreen),
      macro2Badge(ModSource::Macro2, "M2", KnobLookAndFeel::accentGreen),
      macro3Badge(ModSource::Macro3, "M3", KnobLookAndFeel::accentGreen),
      macro4Badge(ModSource::Macro4, "M4", KnobLookAndFeel::accentGreen)
{
    setupKnob(lfoRateSlider, lfoRateLabel, "LFO 1 RATE");
    setupKnob(lfoDepthSlider, lfoDepthLabel, "LFO 1 DEPTH");
    setupKnob(macro1Slider, macro1Label, "MACRO 1");
    setupKnob(macro2Slider, macro2Label, "MACRO 2");
    setupKnob(macro3Slider, macro3Label, "MACRO 3");
    setupKnob(macro4Slider, macro4Label, "MACRO 4");

    macro1Att = std::make_unique<SliderAttachment>(apvts, "macro1", macro1Slider);
    macro2Att = std::make_unique<SliderAttachment>(apvts, "macro2", macro2Slider);
    macro3Att = std::make_unique<SliderAttachment>(apvts, "macro3", macro3Slider);
    macro4Att = std::make_unique<SliderAttachment>(apvts, "macro4", macro4Slider);

    for (auto* badge : badges()) setupBadge(*badge);
    addAndMakeVisible(lfoEditor);

    for (auto& shape : lfoShapes) shape = lfoEditor.getShapeData();
    lfoEditor.onShapeChanged = [this](const auto& shape)
    {
        lfoShapes[static_cast<size_t>(selectedLfoIndex)] = shape;
        if (onLfoShapeChanged) onLfoShapeChanged(selectedLfoIndex, shape);
    };

    const juce::StringArray destinations {
        "Osc A WT Position", "Osc A Level", "Osc A Pan", "Osc A Detune", "Osc A Warp",
        "Osc B WT Position", "Osc B Level", "Osc B Pan", "Osc B Detune", "Osc B Warp",
        "Filter Cutoff", "Filter Resonance", "Filter Drive", "Filter Mix" };
    destinationBox.addItemList(destinations, 1);
    destinationBox.setSelectedId(11);
    addAndMakeVisible(destinationBox);

    routeAmountSlider.setRange(-1.0, 1.0, 0.01);
    routeAmountSlider.setValue(0.5);
    routeAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    routeAmountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    addAndMakeVisible(routeAmountSlider);

    for (auto* label : { &selectedSourceLabel, &routeStatusLabel, &destinationLabel,
                         &amountLabel, &routingHelpLabel })
    {
        label->setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
        label->setFont(juce::Font(11.0f));
        addAndMakeVisible(*label);
    }
    destinationLabel.setText("DESTINATION", juce::dontSendNotification);
    amountLabel.setText("AMOUNT", juce::dontSendNotification);
    routingHelpLabel.setText("1. Click a source  2. Pick a destination  3. Add route\nOr drag a source badge directly onto a knob.", juce::dontSendNotification);
    routingHelpLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(addRouteButton);
    addAndMakeVisible(clearRoutesButton);
    addRouteButton.onClick = [this]
    {
        const auto destination = selectedDestination();
        if (destination == ModDestination::None || !modOverlay.onCreateRoute) return;
        const float amount = static_cast<float>(routeAmountSlider.getValue());
        const int route = modOverlay.onCreateRoute(selectedSource, destination, amount);
        if (route >= 0)
        {
            modOverlay.addModRoute(selectedSource, destination, amount, route);
            routeStatusLabel.setText("ACTIVE: " + sourceName(selectedSource) + " -> "
                                     + destinationBox.getText(), juce::dontSendNotification);
        }
    };
    clearRoutesButton.onClick = [this]
    {
        if (modOverlay.onClearRoutes) modOverlay.onClearRoutes();
        modOverlay.clearAllRoutes();
        routeStatusLabel.setText("All routes cleared", juce::dontSendNotification);
    };

    selectSource(ModSource::LFO1);
    selectLfo(0);
}

ModulationPanel::~ModulationPanel()
{
    for (auto* slider : { &lfoRateSlider, &lfoDepthSlider, &macro1Slider,
                          &macro2Slider, &macro3Slider, &macro4Slider })
        slider->setLookAndFeel(nullptr);
}

void ModulationPanel::setupKnob(juce::Slider& slider, juce::Label& label,
                                const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 14);
    slider.setLookAndFeel(&knobLnf);
    addAndMakeVisible(slider);
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    label.setFont(juce::Font(9.0f));
    addAndMakeVisible(label);
}

std::array<ModSourceBadge*, 14> ModulationPanel::badges()
{
    return { &env1Badge, &env2Badge, &env3Badge, &lfo1Badge, &lfo2Badge,
             &lfo3Badge, &lfo4Badge, &velBadge, &noteBadge, &modWheelBadge,
             &macro1Badge, &macro2Badge, &macro3Badge, &macro4Badge };
}

void ModulationPanel::setupBadge(ModSourceBadge& badge)
{
    addAndMakeVisible(badge);
    badge.onClick = [this](ModSource source) { selectSource(source); };
    badge.onDragStart = [this, &badge]
    {
        selectSource(badge.getSource());
        const auto position = badge.localPointToGlobal(badge.getLocalBounds().getCentre());
        modOverlay.setDragState(true, badge.getSource(), position);
        modOverlay.toFront(false);
    };
    badge.onDragging = [this](ModSource source, juce::Point<int> position)
    {
        modOverlay.setDragState(true, source, position);
    };
    badge.onDragEnd = [this](ModSource source, juce::Point<int> position)
    {
        if (source == ModSource::None)
            modOverlay.setDragState(false, source, position);
        else
            modOverlay.completeDrag(source, position);
    };
}

void ModulationPanel::selectSource(ModSource source)
{
    selectedSource = source;
    for (auto* badge : badges()) badge->setSelected(badge->getSource() == source);
    selectedSourceLabel.setText("SOURCE: " + sourceName(source), juce::dontSendNotification);

    const int value = static_cast<int>(source);
    if (value >= static_cast<int>(ModSource::LFO1)
        && value <= static_cast<int>(ModSource::LFO4))
        selectLfo(value - static_cast<int>(ModSource::LFO1));
}

void ModulationPanel::selectLfo(int index)
{
    selectedLfoIndex = juce::jlimit(0, 3, index);
    const auto prefix = "lfo" + juce::String(selectedLfoIndex + 1);
    lfoRateAtt.reset();
    lfoDepthAtt.reset();
    lfoRateAtt = std::make_unique<SliderAttachment>(apvts, prefix + "_rate", lfoRateSlider);
    lfoDepthAtt = std::make_unique<SliderAttachment>(apvts, prefix + "_depth", lfoDepthSlider);
    lfoRateLabel.setText("LFO " + juce::String(selectedLfoIndex + 1) + " RATE", juce::dontSendNotification);
    lfoDepthLabel.setText("LFO " + juce::String(selectedLfoIndex + 1) + " DEPTH", juce::dontSendNotification);
    lfoEditor.setShapeData(lfoShapes[static_cast<size_t>(selectedLfoIndex)]);
}

ModDestination ModulationPanel::selectedDestination() const
{
    static constexpr ModDestination destinations[] = {
        ModDestination::OscA_WTPosition, ModDestination::OscA_Level,
        ModDestination::OscA_Pan, ModDestination::OscA_Detune,
        ModDestination::OscA_WarpAmount, ModDestination::OscB_WTPosition,
        ModDestination::OscB_Level, ModDestination::OscB_Pan,
        ModDestination::OscB_Detune, ModDestination::OscB_WarpAmount,
        ModDestination::Filter_Cutoff, ModDestination::Filter_Resonance,
        ModDestination::Filter_Drive, ModDestination::Filter_Mix };
    const int index = destinationBox.getSelectedId() - 1;
    return index >= 0 && index < 14 ? destinations[index] : ModDestination::None;
}

juce::String ModulationPanel::sourceName(ModSource source) const
{
    switch (source)
    {
        case ModSource::Env1: return "ENV 1"; case ModSource::Env2: return "ENV 2";
        case ModSource::Env3: return "ENV 3"; case ModSource::LFO1: return "LFO 1";
        case ModSource::LFO2: return "LFO 2"; case ModSource::LFO3: return "LFO 3";
        case ModSource::LFO4: return "LFO 4"; case ModSource::Velocity: return "VELOCITY";
        case ModSource::NoteNumber: return "NOTE"; case ModSource::ModWheel: return "MOD WHEEL";
        case ModSource::Macro1: return "MACRO 1"; case ModSource::Macro2: return "MACRO 2";
        case ModSource::Macro3: return "MACRO 3"; case ModSource::Macro4: return "MACRO 4";
        default: return "NONE";
    }
}

void ModulationPanel::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    g.setColour(KnobLookAndFeel::accentBlue);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("MODULATION ROUTING", getLocalBounds().reduced(8, 4), juce::Justification::topLeft);
}

void ModulationPanel::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    bounds.removeFromTop(18);
    auto left = bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.56f));
    bounds.removeFromLeft(10);
    auto right = bounds;

    selectedSourceLabel.setBounds(left.removeFromTop(20));
    lfoEditor.setBounds(left.removeFromTop(105));
    auto lfoKnobs = left.removeFromTop(62);
    auto placeKnob = [](juce::Rectangle<int> area, juce::Label& label, juce::Slider& slider)
    {
        label.setBounds(area.removeFromTop(13)); slider.setBounds(area);
    };
    placeKnob(lfoKnobs.removeFromLeft(lfoKnobs.getWidth() / 2), lfoRateLabel, lfoRateSlider);
    placeKnob(lfoKnobs, lfoDepthLabel, lfoDepthSlider);

    auto badgeList = badges();
    for (int row = 0; row < 2; ++row)
    {
        auto badgeRow = left.removeFromTop(26);
        for (int column = 0; column < 7; ++column)
        {
            badgeList[static_cast<size_t>(row * 7 + column)]->setBounds(badgeRow.removeFromLeft(50));
            badgeRow.removeFromLeft(3);
        }
    }

    routingHelpLabel.setBounds(right.removeFromTop(42));
    selectedSourceLabel.setColour(juce::Label::textColourId, KnobLookAndFeel::accentBlue);
    destinationLabel.setBounds(right.removeFromTop(17));
    destinationBox.setBounds(right.removeFromTop(24));
    right.removeFromTop(5);
    amountLabel.setBounds(right.removeFromTop(17));
    routeAmountSlider.setBounds(right.removeFromTop(25));
    auto buttons = right.removeFromTop(28);
    addRouteButton.setBounds(buttons.removeFromLeft(buttons.getWidth() / 2).reduced(2));
    clearRoutesButton.setBounds(buttons.reduced(2));
    routeStatusLabel.setBounds(right.removeFromTop(30));

    auto macros = right.removeFromBottom(66);
    juce::Slider* sliders[] { &macro1Slider, &macro2Slider, &macro3Slider, &macro4Slider };
    juce::Label* labels[] { &macro1Label, &macro2Label, &macro3Label, &macro4Label };
    for (int i = 0; i < 4; ++i)
    {
        auto cell = macros.removeFromLeft(macros.getWidth() / (4 - i));
        labels[i]->setBounds(cell.removeFromTop(13)); sliders[i]->setBounds(cell);
    }
}
