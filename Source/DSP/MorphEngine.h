#pragma once

#include <JuceHeader.h>

#include "Algorithms/TubeAlgorithm.h"

class MorphEngine
{
public:
    MorphEngine() = default;
    ~MorphEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setInputGainDb (float newInputGainDb);
    void setOutputGainDb (float newOutputGainDb);

    void setDrive (float newDrive);
    void setMix (float newMix);

    void process (juce::AudioBuffer<float>& buffer);

private:
    TubeAlgorithm tube;

    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;

    float drive = 1.0f;
    float mix = 0.0f;
};