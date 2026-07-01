#include "HardClipAlgorithm.h"

#include <cmath>

void HardClipAlgorithm::prepare (double, int, int)
{
    reset();
}

void HardClipAlgorithm::reset()
{
}

void HardClipAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 32.0f, newDrive);
}

void HardClipAlgorithm::setKnee (float newKnee)
{
    knee = juce::jlimit (0.01f, 0.35f, newKnee);
}

float HardClipAlgorithm::processSample (float input)
{
    const float x = input * drive;

    const float clipPoint = 1.0f - knee;
    const float absX = std::abs (x);

    if (absX <= clipPoint)
        return x;

    if (absX >= 1.0f)
        return x > 0.0f ? 1.0f : -1.0f;

    const float t = (absX - clipPoint) / knee;
    const float smooth = clipPoint + knee * std::sin (t * juce::MathConstants<float>::halfPi);

    return x > 0.0f ? smooth : -smooth;
}