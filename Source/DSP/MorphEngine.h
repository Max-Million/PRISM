#pragma once

#include <JuceHeader.h>

class MorphEngine
{
public:
    MorphEngine() = default;
    ~MorphEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setInputGainDb(float newInputGainDb);
    void setOutputGainDb(float newOutputGainDb);

    void process(juce::AudioBuffer<float>& buffer);

private:
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
};