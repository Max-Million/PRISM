#pragma once

#include "DistortionAlgorithm.h"

class FoldbackAlgorithm final : public DistortionAlgorithm
{
public:
    FoldbackAlgorithm() = default;
    ~FoldbackAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setFoldAmount (float newFoldAmount);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float foldAmount = 1.0f;
};