#include <JuceHeader.h>
#include "../Source/DSP/Arpeggiator.h"
#include "../Source/DSP/SynthVoice.h"
#include "../Source/DSP/WavetableOscillator.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
int failures = 0;

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
    else
    {
        std::cout << "PASS: " << message << '\n';
    }
}

float renderVoiceRms(bool addMacroLevelRoute)
{
    SynthVoice voice;
    voice.prepare(48000.0);
    voice.getOscA().setEnabled(true);
    voice.getOscB().setEnabled(false);
    voice.getSubOsc().setEnabled(false);
    voice.getNoise().setEnabled(false);
    voice.getFilter().setEnabled(false);
    voice.getAmpEnvelope().setAttack(0.001f);
    voice.getAmpEnvelope().setDecay(0.001f);
    voice.getAmpEnvelope().setSustain(1.0f);
    voice.setOscALevelBase(0.8f);

    if (addMacroLevelRoute)
    {
        voice.getModMatrix().addRoute(ModSource::Macro1,
                                      ModDestination::OscA_Level,
                                      -0.75f);
        voice.getModMatrix().setSourceValue(ModSource::Macro1, 1.0f);
    }

    voice.noteOn(69, 1.0f);
    juce::AudioBuffer<float> buffer(2, 8192);
    buffer.clear();
    voice.renderBlock(buffer, 0, buffer.getNumSamples());
    return buffer.getRMSLevel(0, 1024, buffer.getNumSamples() - 1024);
}

void testModulationChangesOscillatorLevel()
{
    const auto dryRms = renderVoiceRms(false);
    const auto modRms = renderVoiceRms(true);
    expect(dryRms > 0.01f, "voice produces a measurable reference signal");
    expect(modRms < dryRms * 0.4f,
           "Macro 1 routed to Osc A level audibly changes the destination");
}

void testArpeggiatorQuarterNoteTiming()
{
    Arpeggiator arp;
    arp.prepare(48000.0);
    arp.setEnabled(true);
    arp.setRate(Arpeggiator::Rate::OneQuarter);

    juce::MidiBuffer input;
    input.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100), 0);
    input.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8) 100), 0);

    juce::MidiBuffer output;
    arp.processBlock(input, output, 48001, 120.0, 48000.0);

    std::vector<int> noteOnPositions;
    for (const auto metadata : output)
        if (metadata.getMessage().isNoteOn())
            noteOnPositions.push_back(metadata.samplePosition);

    expect(noteOnPositions.size() >= 3,
           "quarter-note arpeggiator emits steps for one second at 120 BPM");
    if (noteOnPositions.size() >= 2)
        expect(std::abs(noteOnPositions[1] - 24000) <= 1,
               "quarter-note arpeggiator interval is 0.5 seconds at 120 BPM");
}

void testWavetableFrameEditingPreservesMorphing()
{
    Wavetable table;
    table.generateBasicWavetable();
    const int originalFrameCount = table.numFrames;
    const auto untouchedFrame = table.frames.back();

    std::array<float, WAVETABLE_SIZE> editedFrame{};
    editedFrame.fill(0.25f);
    expect(table.replaceFrame(1, editedFrame), "selected wavetable frame can be replaced");
    expect(table.numFrames == originalFrameCount,
           "editing one frame preserves all wavetable positions");
    expect(table.frames.back() == untouchedFrame,
           "editing one frame leaves other morph frames unchanged");
}
}

int main()
{
    testModulationChangesOscillatorLevel();
    testArpeggiatorQuarterNoteTiming();
    testWavetableFrameEditingPreservesMorphing();

    if (failures != 0)
        std::cerr << failures << " regression check(s) failed\n";
    return failures == 0 ? 0 : 1;
}
