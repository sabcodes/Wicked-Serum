/*
  ==============================================================================
    OversamplingProcessor.cpp
  ==============================================================================
*/

#include "OversamplingProcessor.h"

OversamplingProcessor::OversamplingProcessor()
    : oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false)
{
}

void OversamplingProcessor::prepare(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    oversampler.initProcessing(static_cast<size_t>(blockSize));
    reset();
}

void OversamplingProcessor::reset()
{
    oversampler.reset();
}

juce::dsp::AudioBlock<float> OversamplingProcessor::upsample(juce::dsp::AudioBlock<float>& inputBlock)
{
    if (!enabled)
        return inputBlock;

    return oversampler.processSamplesUp(inputBlock);
}

void OversamplingProcessor::downsample(juce::dsp::AudioBlock<float>& outputBlock)
{
    if (!enabled)
        return;

    oversampler.processSamplesDown(outputBlock);
}
