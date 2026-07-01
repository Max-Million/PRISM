#pragma once

#include "DistortionAlgorithm.h"

class WavefolderAlgorithm final : public DistortionAlgorithm
{
public:
    WavefolderAlgorithm() = default;
    ~WavefolderAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setFoldAmount (float newFoldAmount);
    void setSmoothness (float newSmoothness);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float foldAmount = 1.0f;
    float smoothness = 0.65f;
};