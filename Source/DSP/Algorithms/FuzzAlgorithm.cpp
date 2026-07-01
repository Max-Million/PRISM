#include "FuzzAlgorithm.h"

#include <cmath>

void FuzzAlgorithm::prepare (double, int, int)
{
    reset();
}

void FuzzAlgorithm::reset()
{
}

void FuzzAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 36.0f, newDrive);
}

void FuzzAlgorithm::setBias (float newBias)
{
    bias = juce::jlimit (-0.5f, 0.5f, newBias);
}

void FuzzAlgorithm::setGate (float newGate)
{
    gate = juce::jlimit (0.0f, 0.08f, newGate);
}

float FuzzAlgorithm::processSample (float input)
{
    if (std::abs (input) < gate)
        return 0.0f;

    const float biased = input + bias;
    const float driven = biased * drive;

    const float rectified = driven >= 0.0f
        ? driven
        : -0.65f * driven;

    const float saturated = std::tanh (rectified * 1.7f);

    const float signRestored = input >= 0.0f
        ? saturated
        : -saturated;

    return juce::jlimit (-1.0f, 1.0f, signRestored);
}