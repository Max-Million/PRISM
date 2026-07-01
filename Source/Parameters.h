#pragma once

#include <JuceHeader.h>

namespace ParamID
{
    static constexpr auto inputGain = "inputGain";
    static constexpr auto outputGain = "outputGain";
    static constexpr auto drive = "drive";
    static constexpr auto mix = "mix";
    static constexpr auto tone = "tone";
    static constexpr auto vectorX = "vectorX";
    static constexpr auto vectorY = "vectorY";
    static constexpr auto bypass = "bypass";
    static constexpr auto outputMode = "outputMode";

    static constexpr auto topLeftAlgorithm = "topLeftAlgorithm";
    static constexpr auto topRightAlgorithm = "topRightAlgorithm";
    static constexpr auto bottomLeftAlgorithm = "bottomLeftAlgorithm";
    static constexpr auto bottomRightAlgorithm = "bottomRightAlgorithm";
}

inline juce::StringArray getAlgorithmChoices()
{
    return {
        "Tube",
        "Hard Clip",
        "Foldback",
        "Fuzz",
        "Amp Drive",
        "Bitcrush",
        "Wavefolder",
        "Rectifier"
    };
}

inline juce::StringArray getOutputModeChoices()
{
    return {
        "Stereo",
        "Mono",
        "Mid Only",
        "Side Only"
    };
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const auto algorithmChoices = getAlgorithmChoices();
    const auto outputModeChoices = getOutputModeChoices();

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::bypass,
        "Bypass",
        false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::outputMode,
        "Output Mode",
        outputModeChoices,
        0)); // Stereo

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::inputGain,
        "Input",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::outputGain,
        "Output",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        -6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::drive,
        "Drive",
        juce::NormalisableRange<float>(1.0f, 24.0f, 0.1f),
        6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix,
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::tone,
        "Tone",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f)); // 0 = dark, 50 = neutral, 100 = bright

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::vectorX,
        "Vector X",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::vectorY,
        "Vector Y",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::topLeftAlgorithm,
        "Top Left Algorithm",
        algorithmChoices,
        0)); // Tube

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::topRightAlgorithm,
        "Top Right Algorithm",
        algorithmChoices,
        1)); // Hard Clip

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::bottomLeftAlgorithm,
        "Bottom Left Algorithm",
        algorithmChoices,
        2)); // Foldback

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::bottomRightAlgorithm,
        "Bottom Right Algorithm",
        algorithmChoices,
        3)); // Fuzz

    return { params.begin(), params.end() };
}