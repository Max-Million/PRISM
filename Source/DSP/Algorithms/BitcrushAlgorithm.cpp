#include "BitcrushAlgorithm.h"

#include <cmath>

void BitcrushAlgorithm::prepare (double, int, int)
{
    reset();
}

void BitcrushAlgorithm::reset()
{
}

void BitcrushAlgorithm::setDrive (float newDrive)
{
    drive = juce::jlimit (1.0f, 32.0f, newDrive);
}

void BitcrushAlgorithm::setCrushAmount (float newCrushAmount)
{
    crushAmount = juce::jlimit (0.0f, 1.0f, newCrushAmount);
}

float BitcrushAlgorithm::processSample (float input)
{
    const float driven = std::tanh (input * drive);

    const float bitDepth = 16.0f - crushAmount * 12.0f; // 16-bit down to 4-bit
    const float levels = std::pow (2.0f, bitDepth);

    const float quantized = std::round (driven * levels) / levels;

    const float digitalEdge =
        std::sin (quantized * 73.0f) * 0.035f * crushAmount;

    return juce::jlimit (-1.0f, 1.0f, quantized + digitalEdge);
}