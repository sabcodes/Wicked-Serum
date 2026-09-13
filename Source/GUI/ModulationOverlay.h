/*
  ==============================================================================
    ModulationOverlay.h

    Drag-and-drop modulation routing system — the signature Serum workflow.

    HOW IT WORKS:
    1. User clicks and drags from a mod source (LFO, ENV, Macro badge)
    2. A colored line follows the mouse cursor
    3. When dropped on a knob/slider, a mod route is created
    4. The target knob shows a colored arc indicating mod depth
    5. Right-click on a mod indicator to adjust amount or remove

    This component sits as an overlay on top of the entire plugin GUI,
    intercepting drag events and painting mod indicators on target knobs.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../DSP/ModulationMatrix.h"
#include "KnobLookAndFeel.h"
#include <map>

// Maps a parameter ID to its ModDestination enum value
struct ParameterModMapping
{
    juce::String parameterId;
    ModDestination destination;
    juce::Component* knobComponent = nullptr;  // Pointer to the slider/knob on screen
};

// Visual representation of an active modulation route on a knob
struct ModIndicator
{
    ModSource source;
    ModDestination destination;
    float amount = 0.0f;       // -1 to 1
    juce::Colour colour;
    int routeIndex = -1;       // Index in the ModulationMatrix
};

/**
 * ModSourceBadge — A small draggable badge representing a mod source.
 * Users drag these onto knobs to create modulation routes.
 */
class ModSourceBadge : public juce::Component
{
public:
    ModSourceBadge(ModSource source, const juce::String& label, juce::Colour colour);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    ModSource getSource() const { return source; }
    juce::Colour getColour() const { return colour; }
    void setSelected(bool shouldBeSelected) { isSelected = shouldBeSelected; repaint(); }

    // Callback when drag ends — set by ModulationOverlay
    std::function<void(ModSource, juce::Point<int>)> onDragEnd;
    std::function<void(ModSource, juce::Point<int>)> onDragging;
    std::function<void()> onDragStart;
    std::function<void(ModSource)> onClick;

private:
    ModSource source;
    juce::String label;
    juce::Colour colour;
    bool isDragging = false;
    bool isSelected = false;
};

/**
 * ModulationOverlay — Transparent overlay that manages all mod routing visuals.
 * Sits on top of the entire plugin and handles drag-drop + mod indicators.
 */
class ModulationOverlay : public juce::Component
{
public:
    ModulationOverlay();
    ~ModulationOverlay() override = default;

    void paint(juce::Graphics& g) override;

    // Register a knob as a modulation target
    void registerTarget(const juce::String& parameterId, ModDestination dest, juce::Component* knob);

    // Called when a mod source badge is being dragged
    void setDragState(bool dragging, ModSource source, juce::Point<int> position);

    // Called when drag ends — create a route if dropped on a valid target
    void completeDrag(ModSource source, juce::Point<int> screenPosition);

    // Update mod indicators with current modulation values (call from timer)
    void updateModValues(const ModulationMatrix& matrix);

    // Add/remove mod routes
    void addModRoute(ModSource source, ModDestination dest, float amount, int routeIndex);
    void removeModRoute(int routeIndex);
    void clearAllRoutes();

    // Get all indicators for a specific knob (for the knob's paint method)
    std::vector<ModIndicator> getIndicatorsForDestination(ModDestination dest) const;

    // Callback to actually create the route in the engine
    std::function<int(ModSource, ModDestination, float)> onCreateRoute;
    std::function<void(int)> onRemoveRoute;
    std::function<void()> onClearRoutes;
    std::function<void(int, float)> onChangeRouteAmount;

private:
    // Find which target knob is under a screen position
    ModDestination findTargetAtPosition(juce::Point<int> screenPos) const;
    juce::Colour getSourceColour(ModSource source) const;

    // Registered mod targets
    std::vector<ParameterModMapping> targets;

    // Active mod indicators
    std::vector<ModIndicator> indicators;

    // Drag state
    bool currentlyDragging = false;
    ModSource dragSource = ModSource::None;
    juce::Point<int> dragPosition;
    juce::Point<int> dragStartPosition;
};
