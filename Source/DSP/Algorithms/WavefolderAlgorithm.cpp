#include "WavefolderAlgorithm.h"

#include <cmath>

void WavefolderAlgorithm::prepare (double, int, int)
{
    reset();
}

void WavefolderAlgorithm::reset()
{
}

void WavefolderAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 32.0f, newDrive);
}

void WavefolderAlgorithm::setFoldAmount (float newFoldAmount)
{
    foldAmount = juce::jlimit (0.5f, 5.0f, newFoldAmount);
}

void WavefolderAlgorithm::setSmoothness (float newSmoothness)
{
    smoothness = juce::jlimit (0.0f, 1.0f, newSmoothness);
}

float WavefolderAlgorithm::processSample (float input)
{
    const float x = input * drive * foldAmount;

    // Smooth sine-based folding. More synth-like and less jagged than hard foldback.
    const float folded = std::sin (x * juce::MathConstants<float>::halfPi);

    const float rounded = std::tanh (folded * (1.4f + smoothness * 1.2f));

    const float blended = folded * (1.0f - smoothness)
                        + rounded * smoothness;

    return juce::jlimit (-1.0f, 1.0f, blended);
}