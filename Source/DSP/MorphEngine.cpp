#include "MorphEngine.h"

void MorphEngine::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    tube.prepare (sampleRate, samplesPerBlock, numChannels);
    reset();
}

void MorphEngine::reset()
{
    tube.reset();
}

void MorphEngine::setInputGainDb (float newInputGainDb)
{
    inputGainDb = newInputGainDb;
}

void MorphEngine::setOutputGainDb (float newOutputGainDb)
{
    outputGainDb = newOutputGainDb;
}

void MorphEngine::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 24.0f, newDrive);
    tube.setDrive (drive);
}

void MorphEngine::setMix (float newMix)
{
    mix = juce::jlimit (0.0f, 1.0f, newMix);
}

void MorphEngine::process (juce::AudioBuffer<float>& buffer)
{
    const float inputGain = juce::Decibels::decibelsToGain (inputGainDb);
    const float outputGain = juce::Decibels::decibelsToGain (outputGainDb);

    buffer.applyGain (inputGain);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer (ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = samples[i];
            const float wet = tube.processSample (dry);

            samples[i] = dry + mix * (wet - dry);
        }
    }

    buffer.applyGain (outputGain);
}