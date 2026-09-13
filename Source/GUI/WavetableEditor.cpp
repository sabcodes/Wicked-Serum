/*
  ==============================================================================
    WavetableEditor.cpp

    Implementation of the visual wavetable editor GUI component.

    This file contains all the drawing, interaction, and processing logic for
    creating and editing custom wavetables.
  ==============================================================================
*/

#include "WavetableEditor.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

WavetableEditor::WavetableEditor()
{
    // Initialize the working wavetable to Basic
    workingWavetable.generateBasicWavetable();
    selectedFrameIndex = 0;  // Start at first frame

    // Sync currentWaveform from the first frame
    syncCurrentWaveformFromFrame();

    // Initialize harmonic amplitudes (fundamental at full strength, others at 0)
    harmonicAmplitudes.fill(0.0f);
    harmonicAmplitudes[0] = 1.0f;  // Fundamental (first harmonic)

    // Set up colors from the theme
    initializeColors();
}

void WavetableEditor::initializeColors()
{
    // Load colors from KnobLookAndFeel static constants
    colBackgroundDark = KnobLookAndFeel::backgroundDark;
    colBackgroundMid = KnobLookAndFeel::backgroundMid;
    colBackgroundLight = KnobLookAndFeel::backgroundLight;
    colAccentGreen = KnobLookAndFeel::accentGreen;
    colAccentBlue = KnobLookAndFeel::accentBlue;
    colTextPrimary = KnobLookAndFeel::textPrimary;
    colTextSecondary = KnobLookAndFeel::textSecondary;
}

// ============================================================================
// JUCE COMPONENT OVERRIDES
// ============================================================================

void WavetableEditor::paint(juce::Graphics& g)
{
    // Fill the entire component with the background color
    g.fillAll(colBackgroundDark);

    // Paint the different sections
    paintControlPanel(g);

    if (drawMode)
    {
        paintWaveformArea(g);
    }
    else
    {
        paintHarmonicEditor(g);
    }
}

void WavetableEditor::resized()
{
    // No child components to resize in this case, but this is where you'd
    // call setBounds() on any buttons, sliders, etc. if we added them.
}

void WavetableEditor::mouseDown(const juce::MouseEvent& event)
{
    juce::Rectangle<int> controlArea = getControlArea();
    int padding = 10;
    int buttonWidth = 75;
    int buttonHeight = 16;

    // ===== Check frame slider click =====
    int frameSliderX = controlArea.getX() + padding + 70;
    int frameSliderWidth = 120;
    int frameSliderY = controlArea.getY() + 8;
    juce::Rectangle<int> frameSliderBounds(frameSliderX, frameSliderY, frameSliderWidth, 20);
    if (frameSliderBounds.contains(event.getPosition()))
    {
        // Update frame selection based on click position
        float relativeX = (event.x - frameSliderX) / (float)frameSliderWidth;
        relativeX = juce::jlimit(0.0f, 1.0f, relativeX);
        int newFrame = (int)(relativeX * (workingWavetable.numFrames - 1));
        if (newFrame != selectedFrameIndex)
        {
            // Commit any pending edits before switching frames
            commitEditedFrameToWavetable();
            selectedFrameIndex = newFrame;
            syncCurrentWaveformFromFrame();
            repaint();
        }
        return;
    }

    // ===== Check mode buttons =====
    int modeButtonX = controlArea.getX() + padding + 70;
    int modeButtonY = controlArea.getY() + 38;
    juce::Rectangle<int> drawButton(modeButtonX, modeButtonY, buttonWidth, buttonHeight);
    juce::Rectangle<int> harmonicButton(modeButtonX + buttonWidth + 5, modeButtonY, buttonWidth, buttonHeight);

    if (drawButton.contains(event.getPosition()))
    {
        drawMode = true;
        repaint();
        return;
    }
    if (harmonicButton.contains(event.getPosition()))
    {
        drawMode = false;
        repaint();
        return;
    }

    // ===== Check factory buttons =====
    int factoryButtonX = controlArea.getX() + padding + 70;
    int factoryButtonY = controlArea.getY() + 68;
    for (int i = 0; i < 3; ++i)
    {
        juce::Rectangle<int> factoryButton(factoryButtonX + (i * (buttonWidth + 5)), factoryButtonY, buttonWidth, buttonHeight);
        if (factoryButton.contains(event.getPosition()))
        {
            loadFactoryWavetable(i);
            repaint();
            return;
        }
    }

    // ===== Check CLEAR EDITS button =====
    int clearButtonWidth = 110;
    int clearButtonX = controlArea.getRight() - clearButtonWidth - padding;
    int clearButtonY = factoryButtonY;
    juce::Rectangle<int> clearButton(clearButtonX, clearButtonY, clearButtonWidth, buttonHeight);
    if (clearButton.contains(event.getPosition()))
    {
        // Reload the current factory wavetable to restore the original
        loadFactoryWavetable(currentFactoryIndex);
        repaint();
        return;
    }

    // ===== Waveform/Harmonic editing =====
    if (drawMode)
    {
        // Start drawing on the waveform
        juce::Rectangle<int> waveArea = getWaveformArea();
        if (waveArea.contains(event.getPosition()))
        {
            isDrawing = true;
            // Update the waveform at this point
            int sampleIndex = pixelXToSampleIndex(event.x);
            if (sampleIndex >= 0 && sampleIndex < 2048)
            {
                currentWaveform[sampleIndex] = pixelYToWaveformValue(event.y);
                repaint();
            }
        }
    }
    else
    {
        // Harmonic mode: detect which harmonic was clicked
        for (int i = 0; i < 32; ++i)
        {
            juce::Rectangle<int> barBounds = getHarmonicBarBounds(i);
            if (barBounds.contains(event.getPosition()))
            {
                selectedHarmonic = i;
                break;
            }
        }
    }
}

void WavetableEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (drawMode && isDrawing)
    {
        // Draw a smooth line as the user drags
        juce::Rectangle<int> waveArea = getWaveformArea();
        if (waveArea.contains(event.getPosition()))
        {
            int sampleIndex = pixelXToSampleIndex(event.x);
            if (sampleIndex >= 0 && sampleIndex < 2048)
            {
                currentWaveform[sampleIndex] = pixelYToWaveformValue(event.y);
                repaint();
            }
        }
    }
    else if (selectedHarmonic >= 0)
    {
        // Harmonic mode: adjust the selected harmonic amplitude based on Y position
        juce::Rectangle<int> harmonicArea = getHarmonicArea();

        // Calculate relative Y position within the harmonic area
        float relativeY = static_cast<float>(event.y - harmonicArea.getY()) /
                         static_cast<float>(harmonicArea.getHeight());

        // Invert so dragging up increases amplitude
        float newAmplitude = juce::jlimit(0.0f, 1.0f, 1.0f - relativeY);

        harmonicAmplitudes[selectedHarmonic] = newAmplitude;
        regenerateWaveformFromHarmonics();

        repaint();
    }
}

void WavetableEditor::mouseUp(const juce::MouseEvent& event)
{
    if (drawMode && isDrawing)
    {
        isDrawing = false;
        // Commit drawn changes to the selected frame in the wavetable
        commitEditedFrameToWavetable();
        repaint();
    }
    else if (selectedHarmonic >= 0)
    {
        // Commit harmonic changes to the selected frame in the wavetable
        commitEditedFrameToWavetable();
        selectedHarmonic = -1;
    }
}

// ============================================================================
// PAINTING HELPERS
// ============================================================================

void WavetableEditor::paintWaveformArea(juce::Graphics& g)
{
    juce::Rectangle<int> waveArea = getWaveformArea();

    // Draw the waveform background (grid area)
    g.setColour(colBackgroundMid);
    g.fillRect(waveArea);

    // Draw grid lines for reference
    g.setColour(colBackgroundLight.withAlpha(0.2f));

    // Vertical center line (zero crossing)
    int centerY = waveArea.getCentreY();
    g.drawLine(waveArea.getX(), centerY, waveArea.getRight(), centerY, 2.0f);

    // Horizontal grid lines (every 256 samples, showing 8 divisions)
    for (int i = 1; i < 8; ++i)
    {
        float normalizedPos = i / 8.0f;
        int pixelX = waveArea.getX() + (int)(normalizedPos * waveArea.getWidth());
        g.drawLine(pixelX, waveArea.getY(), pixelX, waveArea.getBottom(), 1.0f);
    }

    // Vertical grid lines (at ±0.5, showing 4 divisions)
    int quarterHeight = waveArea.getHeight() / 4;
    for (int i = 1; i < 4; ++i)
    {
        int pixelY = waveArea.getY() + (i * quarterHeight);
        g.drawLine(waveArea.getX(), pixelY, waveArea.getRight(), pixelY, 1.0f);
    }

    // Draw the waveform as a connected line
    juce::Path waveformPath;
    bool firstPoint = true;

    for (int i = 0; i < 2048; ++i)
    {
        int pixelX = sampleIndexToPixelX(i);
        int pixelY = waveformValueToPixelY(currentWaveform[i]);

        if (firstPoint)
        {
            waveformPath.startNewSubPath(pixelX, pixelY);
            firstPoint = false;
        }
        else
        {
            waveformPath.lineTo(pixelX, pixelY);
        }
    }

    // Draw the waveform with a glowing green accent
    g.setColour(colAccentGreen);
    g.strokePath(waveformPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));

    // Add a subtle glow effect
    g.setColour(colAccentGreen.withAlpha(0.2f));
    g.strokePath(waveformPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));

    // Draw axis labels
    g.setColour(colTextSecondary);
    g.setFont(12.0f);
    g.drawText("0", waveArea.getX() - 20, centerY - 6, 15, 12, juce::Justification::right);
    g.drawText("+1", waveArea.getX() - 20, waveArea.getY(), 15, 12, juce::Justification::right);
    g.drawText("-1", waveArea.getX() - 20, waveArea.getBottom() - 12, 15, 12, juce::Justification::right);
}

void WavetableEditor::paintHarmonicEditor(juce::Graphics& g)
{
    juce::Rectangle<int> harmonicArea = getHarmonicArea();

    // Draw the harmonic editor background
    g.setColour(colBackgroundMid);
    g.fillRect(harmonicArea);

    // Draw the baseline
    g.setColour(colTextSecondary);
    int baselineY = harmonicArea.getBottom() - 10;
    g.drawLine(harmonicArea.getX(), baselineY, harmonicArea.getRight(), baselineY, 1.0f);

    // Draw each harmonic as a vertical bar
    for (int i = 0; i < 32; ++i)
    {
        juce::Rectangle<int> barBounds = getHarmonicBarBounds(i);

        // Calculate bar height based on amplitude
        float amplitude = harmonicAmplitudes[i];
        int barHeight = (int)(amplitude * (barBounds.getHeight() - 10));
        int barY = baselineY - barHeight;

        // Draw the bar
        bool isSelected = (i == selectedHarmonic);
        juce::Colour barColor = isSelected ? colAccentBlue : colAccentGreen;

        g.setColour(barColor);
        g.fillRect(barBounds.getX(), barY, barBounds.getWidth(), barHeight);

        // Draw bar outline
        g.setColour(barColor.withAlpha(0.5f));
        g.drawRect(barBounds.getX(), barY, barBounds.getWidth(), barHeight);

        // Draw harmonic number label (every 4th harmonic to avoid clutter)
        if (i % 4 == 0)
        {
            g.setColour(colTextSecondary);
            g.setFont(10.0f);
            juce::String label = juce::String(i + 1);  // Display 1-indexed
            g.drawText(label, barBounds, juce::Justification::centredBottom);
        }
    }

    // Draw help text at the top
    g.setColour(colTextSecondary);
    g.setFont(11.0f);
    g.drawText("Click and drag bars to adjust harmonic amplitudes",
               harmonicArea.getX() + 5, harmonicArea.getY() + 5,
               harmonicArea.getWidth() - 10, 20, juce::Justification::topLeft);
}

void WavetableEditor::paintControlPanel(juce::Graphics& g)
{
    juce::Rectangle<int> controlArea = getControlArea();

    // Draw control panel background
    g.setColour(colBackgroundLight);
    g.fillRect(controlArea);

    g.setColour(colTextPrimary);
    g.setFont(13.0f);

    int padding = 10;
    int xPos = controlArea.getX() + padding;
    int yPos = controlArea.getY() + 8;
    int buttonWidth = 75;
    int buttonHeight = 16;

    // ===== ROW 1: Frame Selector =====
    g.drawText("Frame:", xPos, yPos, 60, 20, juce::Justification::centredLeft);

    int frameSliderX = xPos + 70;
    int frameSliderWidth = 120;
    g.setColour(colAccentGreen);

    // Normalize selectedFrameIndex to 0-1 range for display
    float frameNormalized = workingWavetable.numFrames > 1
        ? selectedFrameIndex / (float)(workingWavetable.numFrames - 1)
        : 0.0f;
    int sliderPosX = frameSliderX + (int)(frameNormalized * frameSliderWidth);

    // Draw slider track
    g.setColour(colBackgroundMid);
    g.fillRect(frameSliderX, yPos + 6, frameSliderWidth, 8);
    g.setColour(colAccentGreen);
    g.drawRect(frameSliderX, yPos + 6, frameSliderWidth, 8, 1);

    // Draw slider thumb
    g.setColour(colAccentGreen);
    g.fillEllipse(sliderPosX - 4, yPos + 2, 8, 16);

    // Frame number label
    g.setColour(colTextSecondary);
    g.setFont(10.0f);
    juce::String frameLabel = juce::String(selectedFrameIndex + 1) + "/" + juce::String(workingWavetable.numFrames);
    g.drawText(frameLabel, frameSliderX + frameSliderWidth + 10, yPos, 50, 20, juce::Justification::centredLeft);

    // ===== ROW 2: Mode Buttons =====
    xPos = controlArea.getX() + padding;
    yPos += 30;

    g.setColour(colTextPrimary);
    g.setFont(12.0f);
    g.drawText("Mode:", xPos, yPos, 60, 20, juce::Justification::centredLeft);

    int modeButtonX = xPos + 70;
    juce::Colour drawButtonColor = drawMode ? colAccentGreen : colTextSecondary;
    juce::Colour harmonicButtonColor = !drawMode ? colAccentGreen : colTextSecondary;

    // Draw button (clickable area with visual feedback)
    g.setColour(drawButtonColor);
    g.drawRect(juce::Rectangle<int>(modeButtonX, yPos + 2, buttonWidth, buttonHeight), 1);
    g.setFont(11.0f);
    g.drawText("Draw", modeButtonX, yPos + 2, buttonWidth, buttonHeight, juce::Justification::centred);

    // Harmonics button
    g.setColour(harmonicButtonColor);
    g.drawRect(juce::Rectangle<int>(modeButtonX + buttonWidth + 5, yPos + 2, buttonWidth, buttonHeight), 1);
    g.drawText("Harmonics", modeButtonX + buttonWidth + 5, yPos + 2, buttonWidth, buttonHeight, juce::Justification::centred);

    // ===== ROW 3: Factory Wavetable Buttons =====
    xPos = controlArea.getX() + padding;
    yPos += 30;

    g.setColour(colTextPrimary);
    g.setFont(12.0f);
    g.drawText("Factory:", xPos, yPos, 65, 20, juce::Justification::centredLeft);

    int factoryButtonX = xPos + 70;
    for (int i = 0; i < 3; ++i)
    {
        juce::Colour factoryColor = (i == currentFactoryIndex) ? colAccentGreen : colTextSecondary;
        g.setColour(factoryColor);
        int btnX = factoryButtonX + (i * (buttonWidth + 5));
        g.drawRect(juce::Rectangle<int>(btnX, yPos + 2, buttonWidth, buttonHeight), 1);
        g.drawText(factoryWavetableNames[i], btnX, yPos + 2, buttonWidth, buttonHeight, juce::Justification::centred);
    }

    // ===== CLEAR EDITS BUTTON =====
    int clearButtonWidth = 110;
    int clearButtonX = controlArea.getRight() - clearButtonWidth - padding;
    g.setColour(colAccentBlue);
    g.drawRect(juce::Rectangle<int>(clearButtonX, yPos + 2, clearButtonWidth, buttonHeight), 2);
    g.fillRect(juce::Rectangle<int>(clearButtonX, yPos + 2, clearButtonWidth, buttonHeight).reduced(1));
    g.setColour(colBackgroundDark);
    g.setFont(11.0f);
    g.drawText("CLEAR EDITS", clearButtonX, yPos + 2, clearButtonWidth, buttonHeight, juce::Justification::centred);
}

// ============================================================================
// GEOMETRY HELPERS
// ============================================================================

juce::Rectangle<int> WavetableEditor::getWaveformArea() const
{
    // The waveform drawing area takes up most of the space (excluding control panel at top)
    int controlHeight = 110;
    return juce::Rectangle<int>(40, controlHeight, getWidth() - 80, getHeight() - controlHeight - 20);
}

juce::Rectangle<int> WavetableEditor::getHarmonicArea() const
{
    // Same area as waveform for harmonic editor
    int controlHeight = 110;
    return juce::Rectangle<int>(40, controlHeight, getWidth() - 80, getHeight() - controlHeight - 20);
}

juce::Rectangle<int> WavetableEditor::getControlArea() const
{
    // Top section with buttons and mode selector (expanded for frame slider, mode buttons, factory buttons, clear button)
    return juce::Rectangle<int>(0, 0, getWidth(), 110);
}

juce::Rectangle<int> WavetableEditor::getHarmonicBarBounds(int harmonicIndex) const
{
    // Each harmonic gets an equal portion of the width
    juce::Rectangle<int> harmonicArea = getHarmonicArea();

    int barsPerRow = 32;
    int barWidth = (harmonicArea.getWidth() - 5) / barsPerRow;
    int barX = harmonicArea.getX() + (harmonicIndex * barWidth);

    return juce::Rectangle<int>(barX, harmonicArea.getY() + 25, barWidth - 1, harmonicArea.getHeight() - 35);
}

// ============================================================================
// COORDINATE CONVERSION HELPERS
// ============================================================================

int WavetableEditor::sampleIndexToPixelX(int sampleIndex) const
{
    juce::Rectangle<int> waveArea = getWaveformArea();
    float normalizedPos = sampleIndex / 2048.0f;
    return waveArea.getX() + (int)(normalizedPos * waveArea.getWidth());
}

int WavetableEditor::pixelXToSampleIndex(int pixelX) const
{
    juce::Rectangle<int> waveArea = getWaveformArea();
    float normalizedPos = (pixelX - waveArea.getX()) / (float)waveArea.getWidth();
    normalizedPos = juce::jlimit(0.0f, 1.0f, normalizedPos);
    return (int)(normalizedPos * 2048.0f);
}

int WavetableEditor::waveformValueToPixelY(float value) const
{
    juce::Rectangle<int> waveArea = getWaveformArea();
    // Clamp value to [-1, +1]
    value = juce::jlimit(-1.0f, 1.0f, value);

    // Map: -1.0 -> bottom, 0.0 -> center, +1.0 -> top
    float normalizedValue = (1.0f - value) * 0.5f;  // Now 0.0 at top, 1.0 at bottom
    return waveArea.getY() + (int)(normalizedValue * waveArea.getHeight());
}

float WavetableEditor::pixelYToWaveformValue(int pixelY) const
{
    juce::Rectangle<int> waveArea = getWaveformArea();

    // Map pixel Y back to waveform value
    float normalizedY = (pixelY - waveArea.getY()) / (float)waveArea.getHeight();
    normalizedY = juce::jlimit(0.0f, 1.0f, normalizedY);

    // Reverse: 0.0 at top -> value +1.0, 1.0 at bottom -> value -1.0
    float value = 1.0f - (2.0f * normalizedY);
    return snapToGrid(value, 0.05f);
}

float WavetableEditor::snapToGrid(float value, float gridSize) const
{
    return std::round(value / gridSize) * gridSize;
}

// ============================================================================
// WAVEFORM GENERATION
// ============================================================================

void WavetableEditor::regenerateWaveformFromHarmonics()
{
    // Clear the waveform
    currentWaveform.fill(0.0f);

    // Generate using additive synthesis: sum sine waves at harmonic frequencies
    // with amplitudes specified in harmonicAmplitudes array
    for (int h = 0; h < 32; ++h)
    {
        float amplitude = harmonicAmplitudes[h];

        // Skip if this harmonic has no amplitude
        if (amplitude < 0.001f)
            continue;

        // Generate this harmonic: sine wave at (h+1) * fundamental frequency
        int harmonicNumber = h + 1;

        for (int i = 0; i < 2048; ++i)
        {
            float phase = (2.0f * juce::MathConstants<float>::pi * harmonicNumber * i) / 2048.0f;
            currentWaveform[i] += amplitude * std::sin(phase);
        }
    }

    // Normalize to prevent clipping (scale to fit in [-1, +1])
    float maxValue = 0.0f;
    for (int i = 0; i < 2048; ++i)
    {
        maxValue = std::max(maxValue, std::abs(currentWaveform[i]));
    }

    if (maxValue > 1.0f)
    {
        for (int i = 0; i < 2048; ++i)
        {
            currentWaveform[i] /= maxValue;
        }
    }
}

void WavetableEditor::loadWavFile()
{
    // Create a file chooser dialog to select a .wav file
    fileChooser = std::make_unique<juce::FileChooser>("Load WAV file...",
                              juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
                              "*.wav");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto selectedFile = fc.getResult();
            if (selectedFile == juce::File{})
                return;

            // Try to load the file
            juce::AudioFormatManager formatManager;
            formatManager.registerBasicFormats();

            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(selectedFile));

            if (reader != nullptr)
            {
                // Read up to 2048 samples from the first channel
                juce::AudioBuffer<float> buffer(1, 2048);
                reader->read(&buffer, 0, 2048, 0, true, true);

                // Copy into our waveform array
                auto* readData = buffer.getReadPointer(0);
                for (int i = 0; i < 2048; ++i)
                {
                    currentWaveform[i] = readData[i];
                }

                // Replace only the selected frame in the wavetable
                commitEditedFrameToWavetable();

                repaint();
            }
        });
}

void WavetableEditor::loadFactoryWavetable(int index)
{
    currentFactoryIndex = juce::jlimit(0, 2, index);

    // Replace the entire working wavetable with the selected factory wavetable
    switch (currentFactoryIndex)
    {
        case 0: workingWavetable.generateBasicWavetable(); break;   // Sine->Tri->Saw->Square
        case 1: workingWavetable.generateAnalogWavetable(); break;  // Warm analog harmonics
        case 2: workingWavetable.generateSpectralWavetable(); break; // Evolving spectral
    }

    // Reset to first frame
    selectedFrameIndex = 0;
    syncCurrentWaveformFromFrame();

    // Update harmonics from first frame
    harmonicAmplitudes.fill(0.0f);
    harmonicAmplitudes[0] = 0.7f;

    repaint();

    // Fire callback with the full working wavetable
    if (onWavetableChanged)
        onWavetableChanged(workingWavetable);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void WavetableEditor::syncCurrentWaveformFromFrame()
{
    // Load the currently selected frame into currentWaveform
    if (selectedFrameIndex >= 0 && selectedFrameIndex < workingWavetable.numFrames)
    {
        currentWaveform = workingWavetable.frames[selectedFrameIndex];
    }
    else
    {
        // Fallback: fill with silence
        currentWaveform.fill(0.0f);
    }
}

void WavetableEditor::commitEditedFrameToWavetable()
{
    // Write the currentWaveform back to the selected frame in the working wavetable
    if (selectedFrameIndex >= 0 && selectedFrameIndex < workingWavetable.numFrames)
    {
        workingWavetable.replaceFrame(selectedFrameIndex, currentWaveform);
    }

    // Fire callback with the full working wavetable
    if (onWavetableChanged)
        onWavetableChanged(workingWavetable);
}

// End of file
