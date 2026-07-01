#include "RectifierAlgorithm.h"

#include <cmath>

void RectifierAlgorithm::prepare (double, int, int)
{
    reset();
}

void RectifierAlgorithm::reset()
{
}

void RectifierAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 32.0f, newDrive);
}

void RectifierAlgorithm::setRectifyAmount (float newRectifyAmount)
{
    rectifyAmount = juce::jlimit (0.0f, 1.0f, newRectifyAmount);
}

float RectifierAlgorithm::processSample (float input)
{
    const float x = input * drive;

    const float asymmetrical = x >= 0.0f
        ? std::tanh (x * 1.2f)
        : -0.28f * std::tanh (-x * 0.9f);

    const float fullWave = std::abs (std::tanh (x * 1.25f));

    // Keeps the rectified sound from becoming pure DC.
    const float compensatedRectified =
        fullWave - 0.35f * std::tanh (std::abs (x) * 0.35f);

    const float blended = asymmetrical * (1.0f - rectifyAmount)
                        + compensatedRectified * rectifyAmount;

    return juce::jlimit (-1.0f, 1.0f, blended);
}