#include "FoldbackAlgorithm.h"

#include <cmath>

void FoldbackAlgorithm::prepare (double, int, int)
{
    reset();
}

void FoldbackAlgorithm::reset()
{
}

void FoldbackAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 32.0f, newDrive);
}

void FoldbackAlgorithm::setFoldAmount (float newFoldAmount)
{
    foldAmount = juce::jlimit (0.25f, 4.0f, newFoldAmount);
}

float FoldbackAlgorithm::processSample (float input)
{
    float x = input * drive * foldAmount;

    constexpr float threshold = 1.0f;

    while (x > threshold || x < -threshold)
    {
        if (x > threshold)
            x = threshold - (x - threshold);
        else if (x < -threshold)
            x = -threshold - (x + threshold);
    }

    return juce::jlimit (-1.0f, 1.0f, x);
}