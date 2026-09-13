/*
  ==============================================================================
    LFOEditor.cpp

    Drawable LFO shape with preset shapes and freehand drawing.
  ==============================================================================
*/

#include "LFOEditor.h"

LFOEditor::LFOEditor()
{
    // Initialize with a sine wave
    loadShape(LFOShape::Sine);

    // Shape preset buttons
    auto setupBtn = [this](juce::TextButton& btn, const juce::String& text, int idx)
    {
        btn.setButtonText(text);
        btn.onClick = [this, idx]() { setShapeFromPreset(idx); };
        btn.setColour(juce::TextButton::buttonColourId, KnobLookAndFeel::backgroundLight);
        btn.setColour(juce::TextButton::textColourOffId, KnobLookAndFeel::textSecondary);
        addAndMakeVisible(btn);
    };

    setupBtn(sineBtn, "SIN", 0);
    setupBtn(triBtn, "TRI", 1);
    setupBtn(sawBtn, "SAW", 2);
    setupBtn(sqrBtn, "SQR", 3);
    setupBtn(randBtn, "RND", 4);
}

void LFOEditor::loadShape(LFOShape shape)
{
    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int i = 0; i < LFO_EDITOR_POINTS; ++i)
    {
        float phase = static_cast<float>(i) / LFO_EDITOR_POINTS;

        switch (shape)
        {
            case LFOShape::Sine:
                shapeData[i] = 0.5f + 0.5f * std::sin(twoPi * phase);
                break;
            case LFOShape::Triangle:
                shapeData[i] = 1.0f - std::abs(2.0f * phase - 1.0f);
                break;
            case LFOShape::Saw:
                shapeData[i] = phase;
                break;
            case LFOShape::SawDown:
                shapeData[i] = 1.0f - phase;
                break;
            case LFOShape::Square:
                shapeData[i] = phase < 0.5f ? 1.0f : 0.0f;
                break;
            case LFOShape::SampleAndHold:
            {
                // Random steps
                juce::Random rng(42); // Fixed seed for consistency
                float val = 0.5f;
                for (int j = 0; j <= i; ++j)
                {
                    if (j % (LFO_EDITOR_POINTS / 8) == 0)
                        val = rng.nextFloat();
                }
                shapeData[i] = val;
                break;
            }
            case LFOShape::Smooth:
                shapeData[i] = 0.5f + 0.5f * std::sin(twoPi * phase); // Default to sine
                break;
        }
    }

    if (onShapeChanged) onShapeChanged(shapeData);
    repaint();
}

void LFOEditor::setShapeData(const std::array<float, LFO_EDITOR_POINTS>& data, bool notify)
{
    shapeData = data;
    if (notify && onShapeChanged)
        onShapeChanged(shapeData);
    repaint();
}

void LFOEditor::setShapeFromPreset(int presetIndex)
{
    switch (presetIndex)
    {
        case 0: loadShape(LFOShape::Sine); break;
        case 1: loadShape(LFOShape::Triangle); break;
        case 2: loadShape(LFOShape::Saw); break;
        case 3: loadShape(LFOShape::Square); break;
        case 4: loadShape(LFOShape::SampleAndHold); break;
    }
}

void LFOEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto drawArea = bounds;
    drawArea.removeFromBottom(26.0f); // Space for buttons
    drawArea = drawArea.reduced(2.0f);

    // Background
    g.setColour(KnobLookAndFeel::backgroundDark);
    g.fillRoundedRectangle(drawArea, 4.0f);

    // Grid lines
    g.setColour(KnobLookAndFeel::knobTrack.withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(drawArea.getCentreY()), drawArea.getX(), drawArea.getRight());
    for (int i = 1; i < 4; ++i)
    {
        float x = drawArea.getX() + drawArea.getWidth() * i / 4.0f;
        g.drawVerticalLine(static_cast<int>(x), drawArea.getY(), drawArea.getBottom());
    }

    // Draw the LFO shape
    juce::Path shapePath;
    for (int i = 0; i < LFO_EDITOR_POINTS; ++i)
    {
        float x = drawArea.getX() + (static_cast<float>(i) / (LFO_EDITOR_POINTS - 1)) * drawArea.getWidth();
        float y = drawArea.getBottom() - shapeData[i] * drawArea.getHeight();

        if (i == 0)
            shapePath.startNewSubPath(x, y);
        else
            shapePath.lineTo(x, y);
    }

    // Glow
    g.setColour(accentColour.withAlpha(0.15f));
    g.strokePath(shapePath, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved));

    // Main line
    g.setColour(accentColour);
    g.strokePath(shapePath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));

    // Fill under curve
    juce::Path fillPath = shapePath;
    fillPath.lineTo(drawArea.getRight(), drawArea.getBottom());
    fillPath.lineTo(drawArea.getX(), drawArea.getBottom());
    fillPath.closeSubPath();
    g.setColour(accentColour.withAlpha(0.06f));
    g.fillPath(fillPath);

    // Animated playhead
    if (playheadPos > 0.0f && playheadPos < 1.0f)
    {
        float phX = drawArea.getX() + playheadPos * drawArea.getWidth();
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawVerticalLine(static_cast<int>(phX), drawArea.getY(), drawArea.getBottom());

        // Dot on the curve at playhead position
        float phY = drawArea.getBottom() - getValueAtPhase(playheadPos) * drawArea.getHeight();
        g.setColour(juce::Colours::white);
        g.fillEllipse(phX - 4.0f, phY - 4.0f, 8.0f, 8.0f);
    }

    // Border
    g.setColour(KnobLookAndFeel::backgroundLight);
    g.drawRoundedRectangle(drawArea, 4.0f, 1.0f);
}

void LFOEditor::mouseDown(const juce::MouseEvent& e)
{
    auto drawArea = getLocalBounds().toFloat();
    drawArea.removeFromBottom(26.0f);
    drawArea = drawArea.reduced(2.0f);

    if (drawArea.contains(e.position))
    {
        isDrawing = true;
        updatePointFromMouse(e);
    }
}

void LFOEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (isDrawing)
        updatePointFromMouse(e);
}

void LFOEditor::mouseUp(const juce::MouseEvent& /*e*/)
{
    isDrawing = false;
    if (onShapeChanged) onShapeChanged(shapeData);
}

void LFOEditor::updatePointFromMouse(const juce::MouseEvent& e)
{
    auto drawArea = getLocalBounds().toFloat();
    drawArea.removeFromBottom(26.0f);
    drawArea = drawArea.reduced(2.0f);

    // Convert mouse position to shape data index and value
    float normalizedX = (e.position.x - drawArea.getX()) / drawArea.getWidth();
    float normalizedY = 1.0f - (e.position.y - drawArea.getY()) / drawArea.getHeight();

    normalizedX = juce::jlimit(0.0f, 1.0f, normalizedX);
    normalizedY = juce::jlimit(0.0f, 1.0f, normalizedY);

    int index = static_cast<int>(normalizedX * (LFO_EDITOR_POINTS - 1));
    index = juce::jlimit(0, LFO_EDITOR_POINTS - 1, index);

    // Paint a few neighboring points for smooth drawing
    for (int i = std::max(0, index - 1); i <= std::min(LFO_EDITOR_POINTS - 1, index + 1); ++i)
    {
        float dist = std::abs(static_cast<float>(i - index));
        float blend = 1.0f - dist * 0.5f;
        shapeData[i] = shapeData[i] * (1.0f - blend) + normalizedY * blend;
    }

    repaint();
}

float LFOEditor::getValueAtPhase(float phase) const
{
    float floatIdx = phase * (LFO_EDITOR_POINTS - 1);
    int idx0 = static_cast<int>(floatIdx);
    int idx1 = std::min(idx0 + 1, LFO_EDITOR_POINTS - 1);
    float frac = floatIdx - idx0;
    return shapeData[idx0] * (1.0f - frac) + shapeData[idx1] * frac;
}

void LFOEditor::resized()
{
    auto bounds = getLocalBounds();
    auto btnArea = bounds.removeFromBottom(24);

    int btnWidth = btnArea.getWidth() / 5;
    sineBtn.setBounds(btnArea.removeFromLeft(btnWidth).reduced(1));
    triBtn.setBounds(btnArea.removeFromLeft(btnWidth).reduced(1));
    sawBtn.setBounds(btnArea.removeFromLeft(btnWidth).reduced(1));
    sqrBtn.setBounds(btnArea.removeFromLeft(btnWidth).reduced(1));
    randBtn.setBounds(btnArea.reduced(1));
}
