#include "DriveAlgorithm.h"

#include <cmath>

void DriveAlgorithm::prepare (double, int, int)
{
    reset();
}

void DriveAlgorithm::reset()
{
}

void DriveAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 28.0f, newDrive);
}

void DriveAlgorithm::setWarmth (float newWarmth)
{
    warmth = juce::jlimit (0.0f, 1.0f, newWarmth);
}

float DriveAlgorithm::processSample (float input)
{
    const float preGain = drive * (0.65f + warmth * 0.45f);
    const float x = input * preGain;

    const float soft = x / (1.0f + std::abs (x));
    const float tubeLike = std::tanh (x * 0.85f);

    const float blended = soft * (1.0f - warmth) + tubeLike * warmth;

    const float makeup = 1.0f + warmth * 0.15f;

    return juce::jlimit (-1.0f, 1.0f, blended * makeup);
}