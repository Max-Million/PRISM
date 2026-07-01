#include "MorphEngine.h"

namespace
{
    float blendLinear(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    float safeClip(float x)
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }
}

void MorphEngine::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    tube.prepare(sampleRate, samplesPerBlock, numChannels);
    hardClip.prepare(sampleRate, samplesPerBlock, numChannels);
    foldback.prepare(sampleRate, samplesPerBlock, numChannels);

    inputGainDb.reset(currentSampleRate, 0.02);
    outputGainDb.reset(currentSampleRate, 0.02);
    drive.reset(currentSampleRate, 0.035);
    mix.reset(currentSampleRate, 0.02);
    shape.reset(currentSampleRate, 0.04);

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

void MorphEngine::updateAlgorithmParameters(float currentDrive, float currentShape)
{
    tube.setDrive(currentDrive);
    hardClip.setDrive(currentDrive);
    foldback.setDrive(currentDrive);

    hardClip.setKnee(0.18f - 0.12f * currentShape);
    foldback.setFoldAmount(0.75f + 2.5f * currentShape);
}

void MorphEngine::process(juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float currentInputGainDb = inputGainDb.getNextValue();
            const float currentOutputGainDb = outputGainDb.getNextValue();
            const float currentDrive = drive.getNextValue();
            const float currentMix = mix.getNextValue();
            const float currentShape = shape.getNextValue();

            updateAlgorithmParameters(currentDrive, currentShape);

            const float inputGain = juce::Decibels::decibelsToGain(currentInputGainDb);
            const float outputGain = juce::Decibels::decibelsToGain(currentOutputGainDb);

            const float dry = samples[i] * inputGain;

            const float tubeOut = tube.processSample(dry) * 0.78f;
            const float clipOut = hardClip.processSample(dry) * 0.58f;
            const float foldOut = foldback.processSample(dry) * 0.42f;

            float algorithmWet = tubeOut;

            if (currentShape < 0.5f)
            {
                const float localT = currentShape * 2.0f;
                algorithmWet = blendLinear(tubeOut, clipOut, localT);
            }
            else
            {
                const float localT = (currentShape - 0.5f) * 2.0f;
                algorithmWet = blendLinear(clipOut, foldOut, localT);
            }

            samples[i] = safeClip(dry + currentMix * (algorithmWet - dry)) * outputGain;
        }
    }
}