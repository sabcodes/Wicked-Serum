/*
  ==============================================================================
    PluginEditor.cpp

    Tabbed GUI — larger window, oscillators on top, tabs below.
  ==============================================================================
*/

#include "PluginEditor.h"

// ============================================================================
// Arpeggiator Panel — accepts AudioProcessorValueTreeState for parameter binding
// ============================================================================
class ArpPanel : public juce::Component
{
public:
    ArpPanel(juce::AudioProcessorValueTreeState& apvts)
        : apvts(apvts)
    {
        // Pattern selector
        patternBox.addItem("Up", 1);
        patternBox.addItem("Down", 2);
        patternBox.addItem("Up/Down", 3);
        patternBox.addItem("Down/Up", 4);
        patternBox.addItem("Random", 5);
        patternBox.addItem("Order", 6);
        patternBox.setSelectedId(1);
        patternBox.setColour(juce::ComboBox::backgroundColourId, KnobLookAndFeel::backgroundMid);
        patternBox.setColour(juce::ComboBox::textColourId, KnobLookAndFeel::textPrimary);
        patternBox.setColour(juce::ComboBox::outlineColourId, KnobLookAndFeel::backgroundLight);
        addAndMakeVisible(patternBox);

        // Rate selector
        rateBox.addItem("1/4", 1);
        rateBox.addItem("1/8", 2);
        rateBox.addItem("1/8T", 3);
        rateBox.addItem("1/16", 4);
        rateBox.addItem("1/16T", 5);
        rateBox.addItem("1/32", 6);
        rateBox.setSelectedId(2);
        rateBox.setColour(juce::ComboBox::backgroundColourId, KnobLookAndFeel::backgroundMid);
        rateBox.setColour(juce::ComboBox::textColourId, KnobLookAndFeel::textPrimary);
        rateBox.setColour(juce::ComboBox::outlineColourId, KnobLookAndFeel::backgroundLight);
        addAndMakeVisible(rateBox);

        // Labels
        auto makeLabel = [this](juce::Label& l, const juce::String& text)
        {
            l.setText(text, juce::dontSendNotification);
            l.setFont(juce::Font(11.0f));
            l.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
            addAndMakeVisible(l);
        };

        makeLabel(patternLabel, "PATTERN");
        makeLabel(rateLabel, "RATE");
        makeLabel(octaveLabel, "OCTAVES");
        makeLabel(gateLabel, "GATE");
        makeLabel(swingLabel, "SWING");

        // Help text
        helpText.setText("Drag notes to arpeggiate. Hold multiple keys for patterns.", juce::dontSendNotification);
        helpText.setFont(juce::Font(9.0f, juce::Font::italic));
        helpText.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary.withAlpha(0.6f));
        addAndMakeVisible(helpText);

        // Knobs
        auto makeKnob = [this](juce::Slider& s, float min, float max, float def)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
            s.setRange(min, max, (max - min > 10) ? 1.0 : 0.01);
            s.setValue(def);
            s.setLookAndFeel(&lnf);
            addAndMakeVisible(s);
        };

        makeKnob(octaveSlider, 1, 4, 1);
        makeKnob(gateSlider, 10, 100, 80);
        makeKnob(swingSlider, 0, 100, 0);

        // Enable button
        enableBtn.setButtonText("ARP ON");
        enableBtn.setClickingTogglesState(true);
        enableBtn.setColour(juce::TextButton::buttonColourId, KnobLookAndFeel::backgroundLight);
        enableBtn.setColour(juce::TextButton::buttonOnColourId, KnobLookAndFeel::accentGreen.withAlpha(0.3f));
        enableBtn.setColour(juce::TextButton::textColourOffId, KnobLookAndFeel::textSecondary);
        enableBtn.setColour(juce::TextButton::textColourOnId, KnobLookAndFeel::accentGreen);
        addAndMakeVisible(enableBtn);

        // Create parameter attachments
        arpEnabledAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "arp_enabled", enableBtn);
        arpPatternAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "arp_pattern", patternBox);
        arpRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "arp_rate", rateBox);
        arpOctavesAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "arp_octaves", octaveSlider);
        arpGateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "arp_gate", gateSlider);
        arpSwingAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "arp_swing", swingSlider);
    }

    void setCurrentStep(int stepIndex)
    {
        currentStep = stepIndex;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(KnobLookAndFeel::backgroundMid.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds, 8.0f);

        g.setColour(KnobLookAndFeel::accentGreen);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("ARPEGGIATOR", bounds.reduced(12, 8), juce::Justification::topLeft);

        // Draw a visual keyboard/step indicator
        auto stepArea = bounds.reduced(12).withTop(bounds.getBottom() - 60);
        g.setColour(KnobLookAndFeel::backgroundLight.withAlpha(0.4f));
        g.fillRoundedRectangle(stepArea, 4.0f);

        // Draw 16 step indicators, highlight current step
        float stepW = stepArea.getWidth() / 16.0f;
        for (int i = 0; i < 16; ++i)
        {
            auto stepRect = juce::Rectangle<float>(stepArea.getX() + i * stepW + 2,
                                                     stepArea.getY() + 4, stepW - 4, stepArea.getHeight() - 8);

            // Highlight current step
            if (i == currentStep)
                g.setColour(KnobLookAndFeel::accentGreen);
            else if (i % 4 == 0)
                g.setColour(KnobLookAndFeel::accentGreen.withAlpha(0.3f));
            else
                g.setColour(KnobLookAndFeel::knobTrack);

            g.fillRoundedRectangle(stepRect, 3.0f);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(24); // Title
        bounds.removeFromBottom(65); // Step display

        // Top row: Enable button + Pattern + Rate
        auto topRow = bounds.removeFromTop(50);
        enableBtn.setBounds(topRow.removeFromLeft(80).reduced(2, 10));
        topRow.removeFromLeft(10);

        auto patCol = topRow.removeFromLeft(120);
        patternLabel.setBounds(patCol.removeFromTop(16));
        patternBox.setBounds(patCol.removeFromTop(26).reduced(0, 2));

        topRow.removeFromLeft(10);
        auto rateCol = topRow.removeFromLeft(100);
        rateLabel.setBounds(rateCol.removeFromTop(16));
        rateBox.setBounds(rateCol.removeFromTop(26).reduced(0, 2));

        bounds.removeFromTop(8);

        // Help text
        helpText.setBounds(bounds.removeFromTop(18));
        bounds.removeFromTop(4);

        // Knob row: Octaves, Gate, Swing
        auto knobRow = bounds.removeFromTop(70);
        int knobW = knobRow.getWidth() / 3;

        auto placeKnob = [&](juce::Slider& s, juce::Label& l) {
            auto col = knobRow.removeFromLeft(knobW);
            l.setBounds(col.removeFromTop(14));
            s.setBounds(col);
        };

        placeKnob(octaveSlider, octaveLabel);
        placeKnob(gateSlider, gateLabel);
        placeKnob(swingSlider, swingLabel);
    }

    ~ArpPanel() override
    {
        octaveSlider.setLookAndFeel(nullptr);
        gateSlider.setLookAndFeel(nullptr);
        swingSlider.setLookAndFeel(nullptr);
    }

    juce::ComboBox patternBox, rateBox;
    juce::Slider octaveSlider, gateSlider, swingSlider;
    juce::TextButton enableBtn;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::Label patternLabel, rateLabel, octaveLabel, gateLabel, swingLabel, helpText;
    KnobLookAndFeel lnf;
    int currentStep = 0;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpEnabledAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpPatternAtt, arpRateAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpOctavesAtt, arpGateAtt, arpSwingAtt;
};

// ============================================================================
// SerumSynthEditor
// ============================================================================

SerumSynthEditor::SerumSynthEditor(SerumSynthProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      presetPanel(p.getPresetManager(), p.getAPVTS()),
      oscAPanel(p.getAPVTS(), "oscA", "OSC A"),
      oscBPanel(p.getAPVTS(), "oscB", "OSC B"),
      filterPanel(p.getAPVTS()),
      env1Panel(p.getAPVTS(), "env1", "ENV 1 — AMP"),
      env2Panel(p.getAPVTS(), "env2", "ENV 2 — FILTER"),
      effectsPanel(p.getAPVTS()),
      modulationPanel(p.getAPVTS(), modOverlay)
{
    // Always-visible panels
    addAndMakeVisible(presetPanel);
    addAndMakeVisible(oscAPanel);
    addAndMakeVisible(oscBPanel);
    addAndMakeVisible(filterPanel);
    addAndMakeVisible(env1Panel);
    addAndMakeVisible(env2Panel);

    // Master volume
    masterVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    masterVolumeSlider.setLookAndFeel(&knobLnf);
    masterVolumeSlider.setName("master");
    addAndMakeVisible(masterVolumeSlider);

    masterVolumeLabel.setText("MASTER", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    masterVolumeLabel.setFont(juce::Font(10.0f));
    masterVolumeLabel.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(masterVolumeLabel);

    masterVolAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.getAPVTS(), "masterVolume", masterVolumeSlider);

    voiceCountLabel.setText("Voices: 0", juce::dontSendNotification);
    voiceCountLabel.setJustificationType(juce::Justification::centred);
    voiceCountLabel.setFont(juce::Font(10.0f));
    voiceCountLabel.setColour(juce::Label::textColourId, KnobLookAndFeel::textSecondary);
    addAndMakeVisible(voiceCountLabel);

    // --- Tab Buttons ---
    auto setupTab = [this](juce::TextButton& btn, const juce::String& text, int idx)
    {
        btn.setButtonText(text);
        btn.setClickingTogglesState(false);
        btn.setColour(juce::TextButton::buttonColourId, KnobLookAndFeel::backgroundDark);
        btn.setColour(juce::TextButton::textColourOffId, KnobLookAndFeel::textSecondary);
        btn.onClick = [this, idx]() { switchTab(idx); };
        addAndMakeVisible(btn);
    };

    setupTab(fxTabBtn, "FX", 0);
    setupTab(modTabBtn, "MOD", 1);
    setupTab(arpTabBtn, "ARP", 2);
    setupTab(editorTabBtn, "EDITOR", 3);

    // --- Tabbed Panels ---
    addChildComponent(effectsPanel);     // Hidden by default
    addChildComponent(modulationPanel);
    addChildComponent(wavetableEditor);

    // Arp panel — pass APVTS for parameter binding
    arpPanel = std::make_unique<ArpPanel>(p.getAPVTS());
    addChildComponent(*arpPanel);

    // Wire wavetable editor callback to engine — new signature with full Wavetable
    wavetableEditor.setOnWavetableChanged([this](const Wavetable& wt)
    {
        // Load full table into Osc A on all voices
        processor.getSynthEngine().forAllVoices([&wt](SynthVoice& voice)
        {
            voice.getOscA().setWavetable(wt);
        });

        // Update display oscillator (GUI-thread only, no race condition)
        displayOscA.setWavetable(wt);
    });

    // Show FX tab by default
    switchTab(0);

    // Modulation overlay — permanently visible with all mod routes
    // Stays on top but transparent when no active drag
    addAndMakeVisible(modOverlay);
    modOverlay.toFront(false);
    wireModulationCallbacks();

    startTimerHz(30);

    // Set size LAST — this triggers resized() which needs all components to exist
    setSize(1050, 820);
}

SerumSynthEditor::~SerumSynthEditor()
{
    stopTimer();
    masterVolumeSlider.setLookAndFeel(nullptr);
}

void SerumSynthEditor::switchTab(int tabIndex)
{
    currentTab = tabIndex;

    // Hide all tabbed panels
    effectsPanel.setVisible(false);
    modulationPanel.setVisible(false);
    arpPanel->setVisible(false);
    wavetableEditor.setVisible(false);

    // Show the selected one
    switch (tabIndex)
    {
        case 0: effectsPanel.setVisible(true); break;
        case 1: modulationPanel.setVisible(true); break;
        case 2: arpPanel->setVisible(true); break;
        case 3: wavetableEditor.setVisible(true); break;
    }

    // Update tab button colors
    auto updateBtn = [](juce::TextButton& btn, bool active)
    {
        btn.setColour(juce::TextButton::buttonColourId,
            active ? KnobLookAndFeel::backgroundLight : KnobLookAndFeel::backgroundDark);
        btn.setColour(juce::TextButton::textColourOffId,
            active ? KnobLookAndFeel::accentBlue : KnobLookAndFeel::textSecondary);
    };

    updateBtn(fxTabBtn, tabIndex == 0);
    updateBtn(modTabBtn, tabIndex == 1);
    updateBtn(arpTabBtn, tabIndex == 2);
    updateBtn(editorTabBtn, tabIndex == 3);

    repaint();
}

void SerumSynthEditor::wireModulationCallbacks()
{
    modOverlay.onCreateRoute = [this](ModSource source, ModDestination dest, float amount) -> int
    {
        auto& engine = processor.getSynthEngine();
        int routeIdx = -1;
        engine.forAllVoices([&](SynthVoice& voice)
        {
            routeIdx = voice.getModMatrix().addRoute(source, dest, amount);
        });
        return routeIdx;
    };

    modOverlay.onRemoveRoute = [this](int routeIndex)
    {
        processor.getSynthEngine().forAllVoices([routeIndex](SynthVoice& voice)
        {
            voice.getModMatrix().removeRoute(routeIndex);
        });
    };

    modOverlay.onChangeRouteAmount = [this](int routeIndex, float amount)
    {
        processor.getSynthEngine().forAllVoices([routeIndex, amount](SynthVoice& voice)
        {
            voice.getModMatrix().setRouteAmount(routeIndex, amount);
        });
    };

    // Wire clear routes to clear all routes on every voice
    modOverlay.onClearRoutes = [this]()
    {
        processor.getSynthEngine().forAllVoices([](SynthVoice& voice)
        {
            voice.getModMatrix().clearAllRoutes();
        });
    };

    // Wire LFO shape changes to apply to matching voice LFO
    modulationPanel.onLfoShapeChanged = [this](int lfoIndex, const std::array<float, LFO_EDITOR_POINTS>& shape)
    {
        processor.getSynthEngine().forAllVoices([lfoIndex, &shape](SynthVoice& voice)
        {
            voice.getLFO(lfoIndex).setCustomShape(shape);
        });
    };

    // Register all knobs as mod targets so drag-and-drop can find them
    // Oscillator A
    modOverlay.registerTarget("oscA_wtPos",   ModDestination::OscA_WTPosition,  &oscAPanel.getWTPosSlider());
    modOverlay.registerTarget("oscA_level",   ModDestination::OscA_Level,       &oscAPanel.getLevelSlider());
    modOverlay.registerTarget("oscA_pan",     ModDestination::OscA_Pan,         &oscAPanel.getPanSlider());
    modOverlay.registerTarget("oscA_detune",  ModDestination::OscA_Detune,      &oscAPanel.getDetuneSlider());
    modOverlay.registerTarget("oscA_warp",    ModDestination::OscA_WarpAmount,  &oscAPanel.getWarpSlider());

    // Oscillator B
    modOverlay.registerTarget("oscB_wtPos",   ModDestination::OscB_WTPosition,  &oscBPanel.getWTPosSlider());
    modOverlay.registerTarget("oscB_level",   ModDestination::OscB_Level,       &oscBPanel.getLevelSlider());
    modOverlay.registerTarget("oscB_pan",     ModDestination::OscB_Pan,         &oscBPanel.getPanSlider());
    modOverlay.registerTarget("oscB_detune",  ModDestination::OscB_Detune,      &oscBPanel.getDetuneSlider());
    modOverlay.registerTarget("oscB_warp",    ModDestination::OscB_WarpAmount,  &oscBPanel.getWarpSlider());

    // Filter
    modOverlay.registerTarget("filter_cutoff",    ModDestination::Filter_Cutoff,    &filterPanel.getCutoffSlider());
    modOverlay.registerTarget("filter_resonance", ModDestination::Filter_Resonance, &filterPanel.getResonanceSlider());
    modOverlay.registerTarget("filter_drive",     ModDestination::Filter_Drive,     &filterPanel.getDriveSlider());
    modOverlay.registerTarget("filter_mix",       ModDestination::Filter_Mix,       &filterPanel.getMixSlider());
}

void SerumSynthEditor::timerCallback()
{
    int activeVoices = processor.getSynthEngine().getActiveVoiceCount();
    voiceCountLabel.setText("Voices: " + juce::String(activeVoices), juce::dontSendNotification);

    // Use display-only oscillators (not shared with audio thread) for waveform display
    float wtPosA = processor.getAPVTS().getRawParameterValue("oscA_wtPos")->load();
    displayOscA.setWavetablePosition(wtPosA);
    oscAPanel.getWaveformDisplay().setWaveformData(displayOscA.getCurrentFrame());

    float wtPosB = processor.getAPVTS().getRawParameterValue("oscB_wtPos")->load();
    displayOscB.setWavetablePosition(wtPosB);
    oscBPanel.getWaveformDisplay().setWaveformData(displayOscB.getCurrentFrame());

    auto& engine = processor.getSynthEngine();
    auto& voice0 = engine.getVoice(0);

    // Update ArpPanel step indicator from arpeggiator
    currentArpStep = processor.getArpeggiator().getCurrentStepIndex();
    if (auto arp = dynamic_cast<ArpPanel*>(arpPanel.get()))
        arp->setCurrentStep(currentArpStep);

    // Update LFO playhead from selected LFO index
    int selectedLfoIndex = modulationPanel.getSelectedLfoIndex();
    modulationPanel.getLFOEditor().setPlayheadPosition(voice0.getLFO(selectedLfoIndex).getPhase());

    modOverlay.updateModValues(voice0.getModMatrix());
}

void SerumSynthEditor::paint(juce::Graphics& g)
{
    // Dark gradient background
    g.setGradientFill(juce::ColourGradient(
        KnobLookAndFeel::backgroundDark, 0.0f, 0.0f,
        juce::Colour(0xFF0D0D1A), 0.0f, static_cast<float>(getHeight()),
        false));
    g.fillAll();

    // Subtle grid
    g.setColour(juce::Colour(0x06FFFFFF));
    for (int x = 0; x < getWidth(); x += 20)
        g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
    for (int y = 0; y < getHeight(); y += 20)
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));

    // Tab bar background
    auto bounds = getLocalBounds().reduced(6);
    int tabBarY = 36 + 6 + 260 + 6 + 100 + 6; // preset + osc + filter row
    auto tabBarBg = juce::Rectangle<float>(6.0f, static_cast<float>(tabBarY + 6),
                                            static_cast<float>(getWidth() - 12), 32.0f);
    g.setColour(KnobLookAndFeel::backgroundDark.withAlpha(0.8f));
    g.fillRoundedRectangle(tabBarBg, 4.0f);
}

void SerumSynthEditor::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    // Preset bar
    presetPanel.setBounds(bounds.removeFromTop(36));
    bounds.removeFromTop(6);

    // Oscillators side by side
    auto oscArea = bounds.removeFromTop(260);
    int oscWidth = oscArea.getWidth() / 2 - 3;
    oscAPanel.setBounds(oscArea.removeFromLeft(oscWidth));
    oscArea.removeFromLeft(6);
    oscBPanel.setBounds(oscArea);
    bounds.removeFromTop(6);

    // Filter + Envelopes + Master row
    auto midRow = bounds.removeFromTop(100);
    int filterW = midRow.getWidth() * 3 / 10;
    int envW = midRow.getWidth() * 5 / 20;
    int masterW = midRow.getWidth() - filterW - envW * 2 - 18;

    filterPanel.setBounds(midRow.removeFromLeft(filterW));
    midRow.removeFromLeft(6);
    env1Panel.setBounds(midRow.removeFromLeft(envW));
    midRow.removeFromLeft(6);
    env2Panel.setBounds(midRow.removeFromLeft(envW));
    midRow.removeFromLeft(6);

    // Master in remaining space
    auto masterArea = midRow;
    masterVolumeLabel.setBounds(masterArea.removeFromTop(14));
    masterVolumeSlider.setBounds(masterArea.removeFromTop(60));
    voiceCountLabel.setBounds(masterArea.removeFromTop(16));

    bounds.removeFromTop(6);

    // Tab buttons row
    auto tabBar = bounds.removeFromTop(30);
    int tabBtnW = 100;
    int tabGap = 4;
    int totalTabW = tabBtnW * 4 + tabGap * 3;
    int tabStartX = (tabBar.getWidth() - totalTabW) / 2;

    fxTabBtn.setBounds(tabBar.getX() + tabStartX, tabBar.getY(), tabBtnW, 28);
    modTabBtn.setBounds(tabBar.getX() + tabStartX + (tabBtnW + tabGap), tabBar.getY(), tabBtnW, 28);
    arpTabBtn.setBounds(tabBar.getX() + tabStartX + (tabBtnW + tabGap) * 2, tabBar.getY(), tabBtnW, 28);
    editorTabBtn.setBounds(tabBar.getX() + tabStartX + (tabBtnW + tabGap) * 3, tabBar.getY(), tabBtnW, 28);

    bounds.removeFromTop(4);

    // Tabbed content area — all panels share the same bounds
    auto contentArea = bounds;
    effectsPanel.setBounds(contentArea);
    modulationPanel.setBounds(contentArea);
    if (arpPanel != nullptr)
        arpPanel->setBounds(contentArea);
    wavetableEditor.setBounds(contentArea);

    // Modulation overlay covers everything
    modOverlay.setBounds(getLocalBounds());
    modOverlay.toFront(false);
}
