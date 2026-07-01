#pragma once

#include "DistortionAlgorithm.h"

class TubeAlgorithm final : public DistortionAlgorithm
{
public:
    TubeAlgorithm() = default;
    ~TubeAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setBias (float newBias);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float bias = 0.08f;
};