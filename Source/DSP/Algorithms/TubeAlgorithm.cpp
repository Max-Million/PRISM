#include "TubeAlgorithm.h"

#include <cmath>

void TubeAlgorithm::prepare (double, int, int)
{
    reset();
}

void TubeAlgorithm::reset()
{
}

void TubeAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 24.0f, newDrive);
}

void TubeAlgorithm::setBias (float newBias)
{
    bias = juce::jlimit (-0.35f, 0.35f, newBias);
}

float TubeAlgorithm::processSample (float input)
{
    const float driven = (input + bias) * drive;

    const float soft = std::tanh (driven);
    const float asymmetry = 0.12f * std::tanh (driven * 0.5f);
    const float shaped = soft + asymmetry;

    // Subtract the zero-input bias point.
    // This keeps the asymmetric tube character but prevents DC/pop at silence.
    const float zeroDriven = bias * drive;
    const float zeroSoft = std::tanh (zeroDriven);
    const float zeroAsymmetry = 0.12f * std::tanh (zeroDriven * 0.5f);
    const float zeroPoint = zeroSoft + zeroAsymmetry;

    const float normaliser = std::tanh (drive + 0.0001f);
    const float compensated = (shaped - zeroPoint) / normaliser;

    return juce::jlimit (-1.0f, 1.0f, compensated);
}