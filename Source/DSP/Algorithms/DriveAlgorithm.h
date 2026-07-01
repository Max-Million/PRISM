#pragma once

#include "DistortionAlgorithm.h"

class DriveAlgorithm final : public DistortionAlgorithm
{
public:
    DriveAlgorithm() = default;
    ~DriveAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setWarmth (float newWarmth);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float warmth = 0.5f;
};