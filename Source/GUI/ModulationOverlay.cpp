/*
  ==============================================================================
    ModulationOverlay.cpp

    Drag-and-drop modulation routing and visual indicators.
  ==============================================================================
*/

#include "ModulationOverlay.h"

// ============================================================================
// ModSourceBadge — Draggable source icons
// ============================================================================

ModSourceBadge::ModSourceBadge(ModSource src, const juce::String& lbl, juce::Colour col)
    : source(src), label(lbl), colour(col)
{
    setSize(50, 22);
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void ModSourceBadge::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Background pill shape
    g.setColour((isDragging || isSelected) ? colour.withAlpha(0.32f)
                                             : KnobLookAndFeel::backgroundLight);
    g.fillRoundedRectangle(bounds, 10.0f);

    // Border
    g.setColour(colour.withAlpha((isDragging || isSelected) ? 1.0f : 0.6f));
    g.drawRoundedRectangle(bounds, 10.0f, isSelected ? 2.5f : 1.5f);

    // Label text
    g.setColour(isDragging ? juce::Colours::white : colour);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(label, bounds, juce::Justification::centred);
}

void ModSourceBadge::mouseDown(const juce::MouseEvent& /*e*/)
{
    isDragging = true;
    if (onDragStart) onDragStart();
    repaint();
}

void ModSourceBadge::mouseDrag(const juce::MouseEvent& e)
{
    if (isDragging && onDragging)
    {
        // Convert to screen coordinates
        auto screenPos = localPointToGlobal(e.getPosition());
        onDragging(source, screenPos);
    }
}

void ModSourceBadge::mouseUp(const juce::MouseEvent& e)
{
    const bool wasClick = !e.mouseWasDraggedSinceMouseDown();
    if (wasClick && onClick)
        onClick(source);

    if (!wasClick && isDragging && onDragEnd)
    {
        auto screenPos = localPointToGlobal(e.getPosition());
        onDragEnd(source, screenPos);
    }
    else if (wasClick && onDragEnd)
    {
        // End the overlay drag state without creating a route.
        auto screenPos = localPointToGlobal(e.getPosition());
        onDragEnd(ModSource::None, screenPos);
    }

    isDragging = false;
    repaint();
}

// ============================================================================
// ModulationOverlay
// ============================================================================

ModulationOverlay::ModulationOverlay()
{
    setInterceptsMouseClicks(false, false);  // Let clicks pass through to knobs
    setPaintingIsUnclipped(true);
}

void ModulationOverlay::paint(juce::Graphics& g)
{
    // Draw drag line while dragging
    if (currentlyDragging)
    {
        auto startLocal = getLocalPoint(nullptr, dragStartPosition);
        auto endLocal = getLocalPoint(nullptr, dragPosition);

        auto sourceCol = getSourceColour(dragSource);

        // Glowing drag line
        g.setColour(sourceCol.withAlpha(0.3f));
        g.drawLine(juce::Line<float>(startLocal.toFloat(), endLocal.toFloat()), 4.0f);
        g.setColour(sourceCol.withAlpha(0.8f));
        g.drawLine(juce::Line<float>(startLocal.toFloat(), endLocal.toFloat()), 2.0f);

        // Pulsing circle at cursor
        g.setColour(sourceCol.withAlpha(0.5f));
        g.fillEllipse(endLocal.getX() - 8.0f, endLocal.getY() - 8.0f, 16.0f, 16.0f);
        g.setColour(sourceCol);
        g.fillEllipse(endLocal.getX() - 4.0f, endLocal.getY() - 4.0f, 8.0f, 8.0f);

        // Highlight valid targets
        for (const auto& target : targets)
        {
            if (target.knobComponent != nullptr && target.knobComponent->isVisible())
            {
                auto targetBounds = getLocalArea(target.knobComponent, target.knobComponent->getLocalBounds());

                // Check if cursor is near this target
                if (targetBounds.expanded(10).contains(endLocal))
                {
                    g.setColour(sourceCol.withAlpha(0.3f));
                    g.fillRoundedRectangle(targetBounds.toFloat().expanded(4.0f), 6.0f);
                    g.setColour(sourceCol);
                    g.drawRoundedRectangle(targetBounds.toFloat().expanded(4.0f), 6.0f, 2.0f);
                }
            }
        }
    }

    // Draw mod indicators on knobs
    for (const auto& indicator : indicators)
    {
        // Find the knob for this destination
        for (const auto& target : targets)
        {
            if (target.destination == indicator.destination && target.knobComponent != nullptr)
            {
                auto knobBounds = getLocalArea(target.knobComponent, target.knobComponent->getLocalBounds());
                auto centre = knobBounds.getCentre().toFloat();
                float radius = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) * 0.5f + 4.0f;

                // Draw modulation arc around the knob
                float startAngle = juce::MathConstants<float>::pi * 1.25f; // 7 o'clock
                float endAngle = startAngle + juce::MathConstants<float>::twoPi * 0.75f; // 5 o'clock
                float modArc = indicator.amount * (endAngle - startAngle) * 0.5f;

                // Current position angle (approximate center)
                float midAngle = (startAngle + endAngle) * 0.5f;

                juce::Path modPath;
                modPath.addCentredArc(centre.x, centre.y, radius, radius, 0.0f,
                                       midAngle, midAngle + modArc, true);

                // Glow
                g.setColour(indicator.colour.withAlpha(0.2f));
                g.strokePath(modPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));

                // Main arc
                g.setColour(indicator.colour.withAlpha(0.7f));
                g.strokePath(modPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved));

                // Small source badge next to the knob
                g.setColour(indicator.colour);
                g.fillEllipse(centre.x + radius * 0.7f - 4.0f, centre.y - radius * 0.7f - 4.0f, 8.0f, 8.0f);

                break;
            }
        }
    }
}

void ModulationOverlay::registerTarget(const juce::String& parameterId, ModDestination dest, juce::Component* knob)
{
    targets.push_back({ parameterId, dest, knob });
}

void ModulationOverlay::setDragState(bool dragging, ModSource source, juce::Point<int> position)
{
    if (dragging && !currentlyDragging)
        dragStartPosition = position;

    currentlyDragging = dragging;
    dragSource = source;
    dragPosition = position;
    repaint();
}

void ModulationOverlay::completeDrag(ModSource source, juce::Point<int> screenPosition)
{
    currentlyDragging = false;

    auto dest = findTargetAtPosition(screenPosition);
    if (dest != ModDestination::None && onCreateRoute)
    {
        float defaultAmount = 0.5f;
        int routeIdx = onCreateRoute(source, dest, defaultAmount);

        if (routeIdx >= 0)
        {
            addModRoute(source, dest, defaultAmount, routeIdx);
        }
    }

    repaint();
}

void ModulationOverlay::updateModValues(const ModulationMatrix& matrix)
{
    // Update indicator amounts based on current source values
    for (auto& indicator : indicators)
    {
        float sourceVal = matrix.getSourceValue(indicator.source);
        // The visual amount combines the route amount with the current source value
        // This makes the indicator "pulse" with the LFO/envelope
        indicator.amount = indicator.amount; // Keep base amount, animation done in paint
    }
}

void ModulationOverlay::addModRoute(ModSource source, ModDestination dest, float amount, int routeIndex)
{
    ModIndicator indicator;
    indicator.source = source;
    indicator.destination = dest;
    indicator.amount = amount;
    indicator.colour = getSourceColour(source);
    indicator.routeIndex = routeIndex;
    indicators.push_back(indicator);
    repaint();
}

void ModulationOverlay::removeModRoute(int routeIndex)
{
    indicators.erase(
        std::remove_if(indicators.begin(), indicators.end(),
            [routeIndex](const ModIndicator& ind) { return ind.routeIndex == routeIndex; }),
        indicators.end()
    );
    repaint();
}

void ModulationOverlay::clearAllRoutes()
{
    indicators.clear();
    repaint();
}

std::vector<ModIndicator> ModulationOverlay::getIndicatorsForDestination(ModDestination dest) const
{
    std::vector<ModIndicator> result;
    for (const auto& ind : indicators)
    {
        if (ind.destination == dest)
            result.push_back(ind);
    }
    return result;
}

ModDestination ModulationOverlay::findTargetAtPosition(juce::Point<int> screenPos) const
{
    auto localPos = getLocalPoint(nullptr, screenPos);

    for (const auto& target : targets)
    {
        if (target.knobComponent != nullptr && target.knobComponent->isVisible())
        {
            auto targetBounds = getLocalArea(target.knobComponent, target.knobComponent->getLocalBounds());

            if (targetBounds.expanded(10).contains(localPos))
                return target.destination;
        }
    }

    return ModDestination::None;
}

juce::Colour ModulationOverlay::getSourceColour(ModSource source) const
{
    switch (source)
    {
        case ModSource::Env1:
        case ModSource::Env2:
        case ModSource::Env3:
            return KnobLookAndFeel::accentPink;

        case ModSource::LFO1:
        case ModSource::LFO2:
        case ModSource::LFO3:
        case ModSource::LFO4:
            return KnobLookAndFeel::accentBlue;

        case ModSource::Velocity:
        case ModSource::NoteNumber:
        case ModSource::ModWheel:
        case ModSource::Aftertouch:
        case ModSource::PitchBend:
            return KnobLookAndFeel::accentOrange;

        case ModSource::Macro1:
        case ModSource::Macro2:
        case ModSource::Macro3:
        case ModSource::Macro4:
            return KnobLookAndFeel::accentGreen;

        default:
            return KnobLookAndFeel::textSecondary;
    }
}
