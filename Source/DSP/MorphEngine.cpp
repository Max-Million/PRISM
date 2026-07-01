#include "MorphEngine.h"

void MorphEngine::prepare(double, int, int)
{
    reset();
}

void MorphEngine::reset()
{
}

void MorphEngine::setInputGainDb(float newInputGainDb)
{
    inputGainDb = newInputGainDb;
}

void MorphEngine::setOutputGainDb(float newOutputGainDb)
{
    outputGainDb = newOutputGainDb;
}

void MorphEngine::process(juce::AudioBuffer<float>& buffer)
{
    const float inputGain = juce::Decibels::decibelsToGain(inputGainDb);
    const float outputGain = juce::Decibels::decibelsToGain(outputGainDb);

    buffer.applyGain(inputGain * outputGain);
}