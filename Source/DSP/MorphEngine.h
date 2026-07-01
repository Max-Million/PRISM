#pragma once

#include <JuceHeader.h>

#include "Algorithms/TubeAlgorithm.h"
#include "Algorithms/HardClipAlgorithm.h"
#include "Algorithms/FoldbackAlgorithm.h"

class MorphEngine
{
public:
    MorphEngine() = default;
    ~MorphEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setInputGainDb(float newInputGainDb);
    void setOutputGainDb(float newOutputGainDb);

    void setDrive(float newDrive);
    void setMix(float newMix);
    void setShape(float newShape);

    void process(juce::AudioBuffer<float>& buffer);

private:
    void updateAlgorithmParameters(float currentDrive, float currentShape);

    TubeAlgorithm tube;
    HardClipAlgorithm hardClip;
    FoldbackAlgorithm foldback;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> drive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> shape;

    double currentSampleRate = 44100.0;
};