/*
  ==============================================================================
    ModulationMatrix.cpp
  ==============================================================================
*/

#include "ModulationMatrix.h"

int ModulationMatrix::addRoute(ModSource source, ModDestination destination, float amount)
{
    if (static_cast<int>(routes.size()) >= MAX_MOD_ROUTES)
        return -1; // Matrix is full

    routes.push_back({ source, destination, juce::jlimit(-1.0f, 1.0f, amount), true });
    return static_cast<int>(routes.size()) - 1;
}

void ModulationMatrix::removeRoute(int index)
{
    if (index >= 0 && index < static_cast<int>(routes.size()))
        routes.erase(routes.begin() + index);
}

void ModulationMatrix::clearAllRoutes()
{
    routes.clear();
}

void ModulationMatrix::setRouteAmount(int index, float amount)
{
    if (index >= 0 && index < static_cast<int>(routes.size()))
        routes[index].amount = juce::jlimit(-1.0f, 1.0f, amount);
}

void ModulationMatrix::setRouteEnabled(int index, bool enabled)
{
    if (index >= 0 && index < static_cast<int>(routes.size()))
        routes[index].enabled = enabled;
}

void ModulationMatrix::setSourceValue(ModSource source, float value)
{
    int idx = static_cast<int>(source);
    if (idx >= 0 && idx < static_cast<int>(ModSource::NumSources))
        sourceValues[idx] = value;
}

float ModulationMatrix::getModulatedValue(ModDestination destination, float baseValue) const
{
    return baseValue + getModulationAmount(destination);
}

float ModulationMatrix::getModulationAmount(ModDestination destination) const
{
    float totalMod = 0.0f;

    for (const auto& route : routes)
    {
        if (route.enabled && route.destination == destination && route.source != ModSource::None)
        {
            float srcVal = sourceValues[static_cast<int>(route.source)];
            totalMod += srcVal * route.amount;
        }
    }

    return totalMod;
}

float ModulationMatrix::getSourceValue(ModSource source) const
{
    int idx = static_cast<int>(source);
    if (idx >= 0 && idx < static_cast<int>(ModSource::NumSources))
        return sourceValues[idx];
    return 0.0f;
}

juce::String ModulationMatrix::getSourceName(ModSource source)
{
    switch (source)
    {
        case ModSource::None:        return "None";
        case ModSource::Env1:        return "ENV 1";
        case ModSource::Env2:        return "ENV 2";
        case ModSource::Env3:        return "ENV 3";
        case ModSource::LFO1:        return "LFO 1";
        case ModSource::LFO2:        return "LFO 2";
        case ModSource::LFO3:        return "LFO 3";
        case ModSource::LFO4:        return "LFO 4";
        case ModSource::Velocity:    return "Velocity";
        case ModSource::NoteNumber:  return "Note";
        case ModSource::ModWheel:    return "Mod Wheel";
        case ModSource::Aftertouch:  return "Aftertouch";
        case ModSource::PitchBend:   return "Pitch Bend";
        case ModSource::Macro1:          return "Macro 1";
        case ModSource::Macro2:          return "Macro 2";
        case ModSource::Macro3:          return "Macro 3";
        case ModSource::Macro4:          return "Macro 4";
        case ModSource::PerNotePressure: return "MPE Pressure";
        case ModSource::PerNoteSlide:    return "MPE Slide";
        case ModSource::PerNotePitchBend:return "MPE Pitch Bend";
        default:                         return "Unknown";
    }
}

juce::String ModulationMatrix::getDestinationName(ModDestination destination)
{
    switch (destination)
    {
        case ModDestination::None:              return "None";
        case ModDestination::OscA_WTPosition:   return "Osc A WT Pos";
        case ModDestination::OscA_Level:        return "Osc A Level";
        case ModDestination::OscA_Pan:          return "Osc A Pan";
        case ModDestination::OscA_Detune:       return "Osc A Detune";
        case ModDestination::OscA_UnisonDetune: return "Osc A Unison";
        case ModDestination::OscA_WarpAmount:   return "Osc A Warp";
        case ModDestination::OscA_Octave:       return "Osc A Octave";
        case ModDestination::OscB_WTPosition:   return "Osc B WT Pos";
        case ModDestination::OscB_Level:        return "Osc B Level";
        case ModDestination::OscB_Pan:          return "Osc B Pan";
        case ModDestination::OscB_Detune:       return "Osc B Detune";
        case ModDestination::OscB_UnisonDetune: return "Osc B Unison";
        case ModDestination::OscB_WarpAmount:   return "Osc B Warp";
        case ModDestination::OscB_Octave:       return "Osc B Octave";
        case ModDestination::Sub_Level:         return "Sub Level";
        case ModDestination::Noise_Level:       return "Noise Level";
        case ModDestination::Filter_Cutoff:     return "Filter Cutoff";
        case ModDestination::Filter_Resonance:  return "Filter Res";
        case ModDestination::Filter_Drive:      return "Filter Drive";
        case ModDestination::Filter_Mix:        return "Filter Mix";
        case ModDestination::MasterVolume:      return "Master Vol";
        case ModDestination::MasterPan:         return "Master Pan";
        case ModDestination::MasterPitch:       return "Master Pitch";
        case ModDestination::LFO1_Rate:         return "LFO 1 Rate";
        case ModDestination::LFO2_Rate:         return "LFO 2 Rate";
        case ModDestination::LFO3_Rate:         return "LFO 3 Rate";
        case ModDestination::LFO4_Rate:         return "LFO 4 Rate";
        case ModDestination::FX_ReverbMix:      return "Reverb Mix";
        case ModDestination::FX_DelayMix:       return "Delay Mix";
        case ModDestination::FX_DistortionDrive:return "Distortion";
        case ModDestination::FX_ChorusMix:      return "Chorus Mix";
        default:                                return "Unknown";
    }
}
