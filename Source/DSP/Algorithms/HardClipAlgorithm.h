#pragma once

#include "DistortionAlgorithm.h"

class HardClipAlgorithm final : public DistortionAlgorithm
{
public:
    HardClipAlgorithm() = default;
    ~HardClipAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setKnee (float newKnee);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float knee = 0.12f;
};