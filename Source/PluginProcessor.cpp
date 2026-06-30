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

void MorphAudioProcessor::prepareToPlay(double, int)
{
}

void MorphAudioProcessor::releaseResources()
{
}

bool MorphAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    return input == output
        && (input == juce::AudioChannelSet::mono()
            || input == juce::AudioChannelSet::stereo());
}

void MorphAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const auto* inputGainParam = apvts.getRawParameterValue(ParamID::inputGain);
    const auto* outputGainParam = apvts.getRawParameterValue(ParamID::outputGain);

    const float inputGainDb = inputGainParam != nullptr ? inputGainParam->load() : 0.0f;
    const float outputGainDb = outputGainParam != nullptr ? outputGainParam->load() : 0.0f;

    const float inputGain = juce::Decibels::decibelsToGain(inputGainDb);
    const float outputGain = juce::Decibels::decibelsToGain(outputGainDb);

    buffer.applyGain(inputGain * outputGain);
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