/*
  ==============================================================================
    VelocityCurve.cpp
  ==============================================================================
*/

#include "VelocityCurve.h"

VelocityCurve::VelocityCurve()
{
}

float VelocityCurve::processCurve(float inputVelocity) const
{
    inputVelocity = juce::jlimit(0.0f, 1.0f, inputVelocity);

    switch (shape)
    {
        case VelocityCurveShape::Linear:
            // 1:1 mapping
            return inputVelocity;

        case VelocityCurveShape::Soft:
            // Square root — flattens the curve, makes quiet hits quieter
            // Input 0.25 → 0.5 (instead of 0.25)
            return std::sqrt(inputVelocity);

        case VelocityCurveShape::Hard:
            // Squared — steepens the curve, magnifies small velocity changes
            // Input 0.5 → 0.25 (instead of 0.5)
            return inputVelocity * inputVelocity;

        case VelocityCurveShape::SCurve:
        {
            // S-curve using polynomial: 3x^2 - 2x^3
            // Creates more contrast: quiet stays quiet, loud gets louder
            float x = inputVelocity;
            return x * x * (3.0f - 2.0f * x);
        }

        case VelocityCurveShape::Fixed:
            // Ignore input, always return the fixed velocity
            return fixedVelocity;

        default:
            return inputVelocity;
    }
}
