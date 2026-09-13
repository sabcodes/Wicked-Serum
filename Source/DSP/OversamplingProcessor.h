/*
  ==============================================================================
    OversamplingProcessor.h

    2x Oversampling wrapper using JUCE's built-in Oversampling class.
    Works on audio blocks (not individual samples) for efficiency.

    Usage: Wrap your processBlock with oversample/downsample calls.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class OversamplingProcessor
{
public:
    OversamplingProcessor();
    ~OversamplingProcessor() = default;

    void prepare(double sampleRate, int blockSize);
    void reset();

    // Upsample an input block — returns reference to the oversampled block
    // Process your DSP on the returned block at the higher sample rate
    juce::dsp::AudioBlock<float> upsample(juce::dsp::AudioBlock<float>& inputBlock);

    // Downsample the processed oversampled block back into the original block
    void downsample(juce::dsp::AudioBlock<float>& outputBlock);

    // Get the oversampled sample rate (for preparing filters/oscillators)
    double getOversampledRate() const { return currentSampleRate * 2.0; }

    void setEnabled(bool shouldBeEnabled) { enabled = shouldBeEnabled; }
    bool isEnabled() const { return enabled; }

private:
    // 2 channels, 1x oversampling factor (= 2x total), polyphase IIR filter
    juce::dsp::Oversampling<float> oversampler;
    bool enabled = false;  // Off by default — CPU intensive
    double currentSampleRate = 44100.0;
};
