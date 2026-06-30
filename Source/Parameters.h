#pragma once

#include <JuceHeader.h>

namespace ParamID
{
    static constexpr auto inputGain = "inputGain";
    static constexpr auto outputGain = "outputGain";
    static constexpr auto drive = "drive";
    static constexpr auto tone = "tone";
    static constexpr auto mix = "mix";
    static constexpr auto vectorX = "vectorX";
    static constexpr auto vectorY = "vectorY";
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::inputGain, "Input", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::outputGain, "Output", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::drive, "Drive", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::tone, "Tone", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix, "Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::vectorX, "Vector X", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::vectorY, "Vector Y", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    return { params.begin(), params.end() };
}
