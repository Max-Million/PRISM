#pragma once

#include "DistortionAlgorithm.h"

class BitcrushAlgorithm final : public DistortionAlgorithm
{
public:
    BitcrushAlgorithm() = default;
    ~BitcrushAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setCrushAmount (float newCrushAmount);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float crushAmount = 0.5f;
};