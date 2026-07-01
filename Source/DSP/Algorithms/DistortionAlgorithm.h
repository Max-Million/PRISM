#pragma once

#include <JuceHeader.h>

class DistortionAlgorithm
{
public:
    virtual ~DistortionAlgorithm() = default;

    virtual void prepare (double sampleRate,
                          int samplesPerBlock,
                          int numChannels) = 0;

    virtual void reset() = 0;

    virtual float processSample (float input) = 0;
};