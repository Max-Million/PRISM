#include "MorphEngine.h"

#include <array>

namespace
{
    float safeClip(float x)
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }

    float equalPowerBlend(float a, float b, float t)
    {
        const float clampedT = juce::jlimit(0.0f, 1.0f, t);

        const float angle = clampedT * juce::MathConstants<float>::halfPi;
        const float gainA = std::cos(angle);
        const float gainB = std::sin(angle);

        return a * gainA + b * gainB;
    }
}

void MorphEngine::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    tube.prepare(sampleRate, samplesPerBlock, numChannels);
    hardClip.prepare(sampleRate, samplesPerBlock, numChannels);
    foldback.prepare(sampleRate, samplesPerBlock, numChannels);
    fuzz.prepare(sampleRate, samplesPerBlock, numChannels);
    ampDrive.prepare(sampleRate, samplesPerBlock, numChannels);

    inputGainDb.reset(currentSampleRate, 0.03);
    outputGainDb.reset(currentSampleRate, 0.03);
    drive.reset(currentSampleRate, 0.06);
    mix.reset(currentSampleRate, 0.03);

    // Tone / Shape changes nonlinear character, so it needs a longer smoothing time.
    shape.reset(currentSampleRate, 0.18);

    inputGainDb.setCurrentAndTargetValue(0.0f);
    outputGainDb.setCurrentAndTargetValue(-6.0f);
    drive.setCurrentAndTargetValue(6.0f);
    mix.setCurrentAndTargetValue(1.0f);
    shape.setCurrentAndTargetValue(0.0f);

    reset();
}

void MorphEngine::reset()
{
    tube.reset();
    hardClip.reset();
    foldback.reset();
    fuzz.reset();
    ampDrive.reset();

    inputGainDb.setCurrentAndTargetValue(inputGainDb.getTargetValue());
    outputGainDb.setCurrentAndTargetValue(outputGainDb.getTargetValue());
    drive.setCurrentAndTargetValue(drive.getTargetValue());
    mix.setCurrentAndTargetValue(mix.getTargetValue());
    shape.setCurrentAndTargetValue(shape.getTargetValue());
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

void MorphEngine::setShape(float newShape)
{
    shape.setTargetValue(juce::jlimit(0.0f, 1.0f, newShape));
}

void MorphEngine::updateAlgorithmParameters(float currentDrive)
{
    tube.setDrive(currentDrive);
    hardClip.setDrive(currentDrive);
    foldback.setDrive(currentDrive);
    fuzz.setDrive(currentDrive * 1.15f);
    ampDrive.setDrive(currentDrive);

    // Keep these stable while Tone moves.
    // This prevents crackles caused by modulating gate/fold/knee values during playback.
    hardClip.setKnee(0.08f);
    foldback.setFoldAmount(2.0f);
    fuzz.setBias(0.16f);
    fuzz.setGate(0.012f);
    ampDrive.setWarmth(0.75f);
}

void MorphEngine::process(juce::AudioBuffer<float>& buffer)
{
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float currentInputGainDb = inputGainDb.getNextValue();
        const float currentOutputGainDb = outputGainDb.getNextValue();
        const float currentDrive = drive.getNextValue();
        const float currentMix = mix.getNextValue();
        const float currentShape = shape.getNextValue();

        updateAlgorithmParameters(currentDrive);

        const float inputGain = juce::Decibels::decibelsToGain(currentInputGainDb);
        const float outputGain = juce::Decibels::decibelsToGain(currentOutputGainDb);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* samples = buffer.getWritePointer(ch);

            const float dry = samples[i] * inputGain;

            const float tubeOut = tube.processSample(dry) * 0.78f;
            const float clipOut = hardClip.processSample(dry) * 0.52f;
            const float foldOut = foldback.processSample(dry) * 0.34f;
            const float fuzzOut = fuzz.processSample(dry) * 0.42f;
            const float driveOut = ampDrive.processSample(dry) * 0.64f;

            const float scaledShape = currentShape * 4.0f;
            const int segment = juce::jlimit(0, 3, static_cast<int> (scaledShape));
            const float localT = scaledShape - static_cast<float> (segment);

            float algorithmWet = tubeOut;

            if (segment == 0)
                algorithmWet = equalPowerBlend(tubeOut, clipOut, localT);
            else if (segment == 1)
                algorithmWet = equalPowerBlend(clipOut, foldOut, localT);
            else if (segment == 2)
                algorithmWet = equalPowerBlend(foldOut, fuzzOut, localT);
            else
                algorithmWet = equalPowerBlend(fuzzOut, driveOut, localT);

            samples[i] = safeClip(dry + currentMix * (algorithmWet - dry)) * outputGain;
        }
    }
}