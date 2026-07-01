#include "MorphEngine.h"

#include <cmath>

namespace
{
    float safeClip(float x)
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }
}

void MorphEngine::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    currentNumChannels = juce::jmax(1, numChannels);

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        static_cast<size_t> (currentNumChannels),
        static_cast<size_t> (oversamplingExponent),
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true,
        true);

    oversampler->initProcessing(static_cast<size_t> (juce::jmax(1, samplesPerBlock)));
    oversampler->reset();

    effectiveSampleRate = currentSampleRate
        * static_cast<double> (oversampler->getOversamplingFactor());

    tube.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    hardClip.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    foldback.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    fuzz.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    ampDrive.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);

    inputGainDb.reset(effectiveSampleRate, 0.03);
    outputGainDb.reset(effectiveSampleRate, 0.03);
    drive.reset(effectiveSampleRate, 0.06);
    mix.reset(effectiveSampleRate, 0.03);

    vectorX.reset(effectiveSampleRate, 0.12);
    vectorY.reset(effectiveSampleRate, 0.12);

    inputGainDb.setCurrentAndTargetValue(0.0f);
    outputGainDb.setCurrentAndTargetValue(-6.0f);
    drive.setCurrentAndTargetValue(6.0f);
    mix.setCurrentAndTargetValue(1.0f);
    vectorX.setCurrentAndTargetValue(0.5f);
    vectorY.setCurrentAndTargetValue(0.5f);

    reset();
}

void MorphEngine::reset()
{
    tube.reset();
    hardClip.reset();
    foldback.reset();
    fuzz.reset();
    ampDrive.reset();

    if (oversampler != nullptr)
        oversampler->reset();

    inputGainDb.setCurrentAndTargetValue(inputGainDb.getTargetValue());
    outputGainDb.setCurrentAndTargetValue(outputGainDb.getTargetValue());
    drive.setCurrentAndTargetValue(drive.getTargetValue());
    mix.setCurrentAndTargetValue(mix.getTargetValue());
    vectorX.setCurrentAndTargetValue(vectorX.getTargetValue());
    vectorY.setCurrentAndTargetValue(vectorY.getTargetValue());
}

int MorphEngine::getLatencySamples() const
{
    if (oversampler == nullptr)
        return 0;

    return static_cast<int> (std::ceil(oversampler->getLatencyInSamples()));
}

void MorphEngine::setInputGainDb(float newInputGainDb)
{
    inputGainDb.setTargetValue(newInputGainDb);
}

void MorphEngine::setOutputGainDb(float newOutputGainDb)
{
    outputGainDb.setTargetValue(newOutputGainDb);
}

void MorphEngine::setDrive(float newDrive)
{
    drive.setTargetValue(juce::jlimit(1.0f, 24.0f, newDrive));
}

void MorphEngine::setMix(float newMix)
{
    mix.setTargetValue(juce::jlimit(0.0f, 1.0f, newMix));
}

void MorphEngine::setVectorPosition(float newX, float newY)
{
    vectorX.setTargetValue(juce::jlimit(0.0f, 1.0f, newX));
    vectorY.setTargetValue(juce::jlimit(0.0f, 1.0f, newY));
}

void MorphEngine::updateAlgorithmParameters(float currentDrive)
{
    tube.setDrive(currentDrive);
    hardClip.setDrive(currentDrive);
    foldback.setDrive(currentDrive);
    fuzz.setDrive(currentDrive * 1.15f);
    ampDrive.setDrive(currentDrive);

    hardClip.setKnee(0.08f);
    foldback.setFoldAmount(2.0f);
    fuzz.setBias(0.16f);
    fuzz.setGate(0.012f);
    ampDrive.setWarmth(0.75f);
}

MorphEngine::MorphWeights MorphEngine::calculateWeights(float x, float y) const
{
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);

    MorphWeights weights;

    weights.tube = (1.0f - x) * (1.0f - y);
    weights.hardClip = x * (1.0f - y);
    weights.foldback = (1.0f - x) * y;
    weights.fuzz = x * y;

    const float dx = x - 0.5f;
    const float dy = y - 0.5f;
    const float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
    const float maxDistance = 0.70710678f;

    const float centerAmount = juce::jlimit(
        0.0f,
        1.0f,
        1.0f - distanceFromCenter / maxDistance);

    weights.ampDrive = centerAmount * centerAmount * 1.35f;

    const float total = weights.tube
        + weights.hardClip
        + weights.foldback
        + weights.fuzz
        + weights.ampDrive
        + 0.00001f;

    weights.tube /= total;
    weights.hardClip /= total;
    weights.foldback /= total;
    weights.fuzz /= total;
    weights.ampDrive /= total;

    return weights;
}

void MorphEngine::processAudioBlock(juce::dsp::AudioBlock<float>& block)
{
    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    for (size_t i = 0; i < numSamples; ++i)
    {
        const float currentInputGainDb = inputGainDb.getNextValue();
        const float currentOutputGainDb = outputGainDb.getNextValue();
        const float currentDrive = drive.getNextValue();
        const float currentMix = mix.getNextValue();
        const float currentVectorX = vectorX.getNextValue();
        const float currentVectorY = vectorY.getNextValue();

        updateAlgorithmParameters(currentDrive);

        const auto weights = calculateWeights(currentVectorX, currentVectorY);

        const float inputGain = juce::Decibels::decibelsToGain(currentInputGainDb);
        const float outputGain = juce::Decibels::decibelsToGain(currentOutputGainDb);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* samples = block.getChannelPointer(ch);

            const float dry = samples[i] * inputGain;

            const float tubeOut = tube.processSample(dry) * 0.78f;
            const float clipOut = hardClip.processSample(dry) * 0.52f;
            const float foldOut = foldback.processSample(dry) * 0.34f;
            const float fuzzOut = fuzz.processSample(dry) * 0.42f;
            const float driveOut = ampDrive.processSample(dry) * 0.64f;

            const float algorithmWet =
                tubeOut * weights.tube
                + clipOut * weights.hardClip
                + foldOut * weights.foldback
                + fuzzOut * weights.fuzz
                + driveOut * weights.ampDrive;

            samples[i] = safeClip(dry + currentMix * (algorithmWet - dry)) * outputGain;
        }
    }
}

void MorphEngine::process(juce::AudioBuffer<float>& buffer)
{
    if (oversampler == nullptr)
    {
        juce::dsp::AudioBlock<float> fallbackBlock(buffer);
        processAudioBlock(fallbackBlock);
        return;
    }

    juce::dsp::AudioBlock<float> inputBlock(buffer);

    auto oversampledBlock = oversampler->processSamplesUp(inputBlock);

    processAudioBlock(oversampledBlock);

    oversampler->processSamplesDown(inputBlock);
}