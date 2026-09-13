/*
  ==============================================================================
    VelocityCurve.h

    Velocity response curve processor. Maps input velocity (0-1) to output
    velocity (0-1) using different curve shapes to control how hard you need
    to hit keys.

    Curve shapes:
    - Linear: 1:1 mapping (default behavior)
    - Soft: Flatter response — quiet hits stay quiet, loud hits get boosted
    - Hard: Steeper response — small changes in velocity make big differences
    - SCurve: S-shaped — quiet hits quieter, loud hits louder, more extremes
    - Fixed: Always outputs the same velocity regardless of input (for locked velocity)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

enum class VelocityCurveShape
{
    Linear,
    Soft,
    Hard,
    SCurve,
    Fixed
};

class VelocityCurve
{
public:
    VelocityCurve();
    ~VelocityCurve() = default;

    // Set which curve shape to use
    void setShape(VelocityCurveShape newShape) { shape = newShape; }
    VelocityCurveShape getShape() const { return shape; }

    // Set the fixed velocity (only used with Fixed shape)
    void setFixedVelocity(float velocity) { fixedVelocity = juce::jlimit(0.0f, 1.0f, velocity); }
    float getFixedVelocity() const { return fixedVelocity; }

    // Apply the curve to an input velocity value (0-1) and return output (0-1)
    float processCurve(float inputVelocity) const;

private:
    VelocityCurveShape shape = VelocityCurveShape::Linear;
    float fixedVelocity = 0.8f;
};
