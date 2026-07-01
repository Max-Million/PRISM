#pragma once

#include "DistortionAlgorithm.h"

class RectifierAlgorithm final : public DistortionAlgorithm
{
public:
    RectifierAlgorithm() = default;
    ~RectifierAlgorithm() override = default;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numChannels) override;

    void reset() override;

    void setDrive (float newDrive);
    void setRectifyAmount (float newRectifyAmount);

    float processSample (float input) override;

private:
    float drive = 1.0f;
    float rectifyAmount = 0.65f;
};