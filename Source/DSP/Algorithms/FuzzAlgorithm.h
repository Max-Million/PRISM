#pragma once

#include "DistortionAlgorithm.h"

class FuzzAlgorithm final : public DistortionAlgorithm
{
public:
    FuzzAlgorithm() = default;
    ~FuzzAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setBias (float newBias);
    void setGate (float newGate);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float bias = 0.12f;
    float gate = 0.015f;
};