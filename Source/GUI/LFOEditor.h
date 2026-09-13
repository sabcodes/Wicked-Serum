/*
  ==============================================================================
    LFOEditor.h

    Drawable LFO shape editor — like Serum's LFO tab.
    Users can draw custom LFO shapes by clicking and dragging points,
    or select from preset shapes.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../DSP/LFO.h"
#include "KnobLookAndFeel.h"
#include <array>

// Number of editable points in the LFO shape
static constexpr int LFO_EDITOR_POINTS = 64;

class LFOEditor : public juce::Component
{
public:
    LFOEditor();
    ~LFOEditor() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void resized() override;

    // Get the custom LFO shape data (normalized 0-1 for each point)
    const std::array<float, LFO_EDITOR_POINTS>& getShapeData() const { return shapeData; }

    // Load a preset shape into the editor
    void loadShape(LFOShape shape);
    void setShapeData(const std::array<float, LFO_EDITOR_POINTS>& data, bool notify = false);

    // Set accent color
    void setAccentColour(juce::Colour c) { accentColour = c; }

    // Set the current LFO value for the animated playhead
    void setPlayheadPosition(float pos) { playheadPos = pos; repaint(); }

    // Callback when shape is edited
    std::function<void(const std::array<float, LFO_EDITOR_POINTS>&)> onShapeChanged;

    // Preset shape buttons
    void setShapeFromPreset(int presetIndex);

private:
    void updatePointFromMouse(const juce::MouseEvent& e);
    float getValueAtPhase(float phase) const;

    std::array<float, LFO_EDITOR_POINTS> shapeData{};
    juce::Colour accentColour = KnobLookAndFeel::accentBlue;
    float playheadPos = 0.0f;
    bool isDrawing = false;

    // Shape preset buttons
    juce::TextButton sineBtn, triBtn, sawBtn, sqrBtn, randBtn;
};
