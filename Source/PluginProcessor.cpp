#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

MorphAudioProcessor::MorphAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void MorphAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    setLatencySamples(engine.getLatencySamples());
}

void MorphAudioProcessor::releaseResources()
{
    engine.reset();
}

bool MorphAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    return input == output
        && (input == juce::AudioChannelSet::mono()
            || input == juce::AudioChannelSet::stereo());
}

void MorphAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const auto* bypassParam = apvts.getRawParameterValue(ParamID::bypass);
    const auto* outputModeParam = apvts.getRawParameterValue(ParamID::outputMode);
    const auto* inputGainParam = apvts.getRawParameterValue(ParamID::inputGain);
    const auto* outputGainParam = apvts.getRawParameterValue(ParamID::outputGain);
    const auto* driveParam = apvts.getRawParameterValue(ParamID::drive);
    const auto* mixParam = apvts.getRawParameterValue(ParamID::mix);
    const auto* toneParam = apvts.getRawParameterValue(ParamID::tone);
    const auto* vectorXParam = apvts.getRawParameterValue(ParamID::vectorX);
    const auto* vectorYParam = apvts.getRawParameterValue(ParamID::vectorY);
    const auto* topLeftAlgorithmParam = apvts.getRawParameterValue(ParamID::topLeftAlgorithm);
    const auto* topRightAlgorithmParam = apvts.getRawParameterValue(ParamID::topRightAlgorithm);
    const auto* bottomLeftAlgorithmParam = apvts.getRawParameterValue(ParamID::bottomLeftAlgorithm);
    const auto* bottomRightAlgorithmParam = apvts.getRawParameterValue(ParamID::bottomRightAlgorithm);

    const bool bypassed = bypassParam != nullptr && bypassParam->load() >= 0.5f;
    const int outputMode = outputModeParam != nullptr ? static_cast<int> (outputModeParam->load()) : 0;

    const float inputGainDb = inputGainParam != nullptr ? inputGainParam->load() : 0.0f;
    const float outputGainDb = outputGainParam != nullptr ? outputGainParam->load() : -6.0f;
    const float drive = driveParam != nullptr ? driveParam->load() : 6.0f;
    const float tone = toneParam != nullptr ? toneParam->load() : 50.0f;

    const float mixPercent = mixParam != nullptr ? mixParam->load() : 100.0f;
    const float mix = juce::jlimit(0.0f, 1.0f, mixPercent / 100.0f);

    const float vectorX = vectorXParam != nullptr ? vectorXParam->load() : 0.5f;
    const float vectorY = vectorYParam != nullptr ? vectorYParam->load() : 0.5f;

    const int topLeftAlgorithm = topLeftAlgorithmParam != nullptr
        ? static_cast<int> (topLeftAlgorithmParam->load())
        : 0;

    const int topRightAlgorithm = topRightAlgorithmParam != nullptr
        ? static_cast<int> (topRightAlgorithmParam->load())
        : 1;

    const int bottomLeftAlgorithm = bottomLeftAlgorithmParam != nullptr
        ? static_cast<int> (bottomLeftAlgorithmParam->load())
        : 2;

    const int bottomRightAlgorithm = bottomRightAlgorithmParam != nullptr
        ? static_cast<int> (bottomRightAlgorithmParam->load())
        : 3;

    engine.setBypassed(bypassed);
    engine.setOutputMode(outputMode);
    engine.setInputGainDb(inputGainDb);
    engine.setOutputGainDb(outputGainDb);
    engine.setDrive(drive);
    engine.setMix(mix);
    engine.setTone(tone);
    engine.setVectorPosition(vectorX, vectorY);
    engine.setCornerAlgorithms(topLeftAlgorithm,
        topRightAlgorithm,
        bottomLeftAlgorithm,
        bottomRightAlgorithm);

    engine.process(buffer);
}

void MorphAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void MorphAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* MorphAudioProcessor::createEditor()
{
    return new MorphAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MorphAudioProcessor();
}