#include "MorphEngine.h"

#include <array>
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
    bitcrush.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    wavefolder.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);
    rectifier.prepare(effectiveSampleRate, samplesPerBlock * 2, currentNumChannels);

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
    bitcrush.reset();
    wavefolder.reset();
    rectifier.reset();

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

void MorphEngine::setCornerAlgorithms(int newTopLeftAlgorithm,
    int newTopRightAlgorithm,
    int newBottomLeftAlgorithm,
    int newBottomRightAlgorithm)
{
    topLeftAlgorithm = juce::jlimit(0, numAlgorithms - 1, newTopLeftAlgorithm);
    topRightAlgorithm = juce::jlimit(0, numAlgorithms - 1, newTopRightAlgorithm);
    bottomLeftAlgorithm = juce::jlimit(0, numAlgorithms - 1, newBottomLeftAlgorithm);
    bottomRightAlgorithm = juce::jlimit(0, numAlgorithms - 1, newBottomRightAlgorithm);
}

void MorphEngine::updateAlgorithmParameters(float currentDrive)
{
    tube.setDrive(currentDrive);

    hardClip.setDrive(currentDrive * 1.55f);
    hardClip.setKnee(0.015f);

    foldback.setDrive(currentDrive);
    foldback.setFoldAmount(2.0f);

    fuzz.setDrive(currentDrive * 1.15f);
    fuzz.setBias(0.16f);
    fuzz.setGate(0.012f);

    ampDrive.setDrive(currentDrive * 0.95f);
    ampDrive.setWarmth(0.62f);

    bitcrush.setDrive(currentDrive * 1.10f);
    bitcrush.setCrushAmount(0.62f);

    wavefolder.setDrive(currentDrive * 0.90f);
    wavefolder.setFoldAmount(1.75f);
    wavefolder.setSmoothness(0.78f);

    rectifier.setDrive(currentDrive * 0.95f);
    rectifier.setRectifyAmount(0.72f);
}

MorphEngine::MorphWeights MorphEngine::calculateWeights(float x, float y) const
{
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);

    MorphWeights weights;

    weights.topLeft = (1.0f - x) * (1.0f - y);
    weights.topRight = x * (1.0f - y);
    weights.bottomLeft = (1.0f - x) * y;
    weights.bottomRight = x * y;

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

            const std::array<float, numAlgorithms> outputs
            { {
                tube.processSample(dry) * 0.72f,
                hardClip.processSample(dry) * 0.60f,
                foldback.processSample(dry) * 0.34f,
                fuzz.processSample(dry) * 0.42f,
                ampDrive.processSample(dry) * 0.74f,
                bitcrush.processSample(dry) * 0.46f,
                wavefolder.processSample(dry) * 0.48f,
                rectifier.processSample(dry) * 0.44f
            } };

            const float algorithmWet =
                outputs[static_cast<size_t> (topLeftAlgorithm)] * weights.topLeft
                + outputs[static_cast<size_t> (topRightAlgorithm)] * weights.topRight
                + outputs[static_cast<size_t> (bottomLeftAlgorithm)] * weights.bottomLeft
                + outputs[static_cast<size_t> (bottomRightAlgorithm)] * weights.bottomRight;

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