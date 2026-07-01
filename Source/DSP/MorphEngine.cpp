#include "MorphEngine.h"

#include <cmath>

namespace
{
    float safeClip(float x)
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }

    float safetyLimit(float x)
    {
        constexpr float ceiling = 0.98f;

        const float absX = std::abs(x);

        if (absX <= ceiling)
            return x;

        const float sign = x >= 0.0f ? 1.0f : -1.0f;
        const float excess = absX - ceiling;
        const float remainingHeadroom = 1.0f - ceiling;

        const float limited = ceiling
            + remainingHeadroom * std::tanh(excess / remainingHeadroom);

        return sign * juce::jlimit(ceiling, 1.0f, limited);
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
    bypassAmount.reset(effectiveSampleRate, 0.02);

    vectorX.reset(effectiveSampleRate, 0.12);
    vectorY.reset(effectiveSampleRate, 0.12);

    inputGainDb.setCurrentAndTargetValue(0.0f);
    outputGainDb.setCurrentAndTargetValue(-6.0f);
    drive.setCurrentAndTargetValue(6.0f);
    mix.setCurrentAndTargetValue(1.0f);
    bypassAmount.setCurrentAndTargetValue(0.0f);
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
    bypassAmount.setCurrentAndTargetValue(bypassAmount.getTargetValue());
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

void MorphEngine::setBypassed(bool shouldBypass)
{
    bypassAmount.setTargetValue(shouldBypass ? 1.0f : 0.0f);
}

void MorphEngine::setVectorPosition(float newX, float newY)
{
    vectorX.setTargetValue(juce::jlimit(0.0f, 1.0f, newX));
    vectorY.setTargetValue(juce::jlimit(0.0f, 1.0f, newY));
}

void MorphEngine::updateAlgorithmParameters(float currentDrive)
{
    tube.setDrive(currentDrive);

    // Top-right: hard, squared-off clipping.
    hardClip.setDrive(currentDrive * 1.55f);
    hardClip.setKnee(0.015f);

    foldback.setDrive(currentDrive);
    foldback.setFoldAmount(2.0f);

    fuzz.setDrive(currentDrive * 1.15f);
    fuzz.setBias(0.16f);
    fuzz.setGate(0.012f);

    // Center: amp-like drive with grit and edge.
    // Smoother than Hard Clip, but not overly soft.
    ampDrive.setDrive(currentDrive * 0.95f);
    ampDrive.setWarmth(0.62f);
}

MorphEngine::MorphWeights MorphEngine::calculateWeights(float x, float y) const
{
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);

    MorphWeights weights;

    const float cornerTube = (1.0f - x) * (1.0f - y);
    const float cornerHardClip = x * (1.0f - y);
    const float cornerFoldback = (1.0f - x) * y;
    const float cornerFuzz = x * y;

    const float dx = x - 0.5f;
    const float dy = y - 0.5f;
    const float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
    const float maxDistance = 0.70710678f;

    const float centerRaw = juce::jlimit(
        0.0f,
        1.0f,
        1.0f - distanceFromCenter / maxDistance);

    // Focus the center node so Amp Drive has a clear identity.
    const float centerAmount = centerRaw * centerRaw;

    // Pull corner algorithms down near the center.
    const float cornerScale = 1.0f - centerAmount * 0.90f;

    weights.tube = cornerTube * cornerScale;
    weights.hardClip = cornerHardClip * cornerScale;
    weights.foldback = cornerFoldback * cornerScale;
    weights.fuzz = cornerFuzz * cornerScale;

    weights.ampDrive = centerAmount * 4.0f;

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
        const float currentBypass = bypassAmount.getNextValue();
        const float currentVectorX = vectorX.getNextValue();
        const float currentVectorY = vectorY.getNextValue();

        updateAlgorithmParameters(currentDrive);

        const auto weights = calculateWeights(currentVectorX, currentVectorY);

        const float inputGain = juce::Decibels::decibelsToGain(currentInputGainDb);
        const float outputGain = juce::Decibels::decibelsToGain(currentOutputGainDb);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* samples = block.getChannelPointer(ch);

            const float cleanInput = samples[i];
            const float dry = cleanInput * inputGain;

            const float tubeOut = tube.processSample(dry) * 0.78f;
            const float clipOut = hardClip.processSample(dry) * 0.62f;
            const float foldOut = foldback.processSample(dry) * 0.34f;
            const float fuzzOut = fuzz.processSample(dry) * 0.42f;
            const float driveOut = ampDrive.processSample(dry) * 0.76f;

            const float algorithmWet =
                tubeOut * weights.tube
                + clipOut * weights.hardClip
                + foldOut * weights.foldback
                + fuzzOut * weights.fuzz
                + driveOut * weights.ampDrive;

            const float effected =
                safetyLimit(safeClip(dry + currentMix * (algorithmWet - dry)) * outputGain);

            samples[i] = cleanInput + (effected - cleanInput) * (1.0f - currentBypass);
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