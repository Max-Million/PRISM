#pragma once

#include <JuceHeader.h>

#include "Algorithms/TubeAlgorithm.h"
#include "Algorithms/HardClipAlgorithm.h"
#include "Algorithms/FoldbackAlgorithm.h"
#include "Algorithms/FuzzAlgorithm.h"
#include "Algorithms/DriveAlgorithm.h"

class MorphEngine
{
public:
    MorphEngine() = default;
    ~MorphEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    int getLatencySamples() const;

    void setInputGainDb(float newInputGainDb);
    void setOutputGainDb(float newOutputGainDb);

    void setDrive(float newDrive);
    void setMix(float newMix);

    void setVectorPosition(float newX, float newY);

    void process(juce::AudioBuffer<float>& buffer);

private:
    struct MorphWeights
    {
        float tube = 0.0f;
        float hardClip = 0.0f;
        float foldback = 0.0f;
        float fuzz = 0.0f;
        float ampDrive = 0.0f;
    };

    void updateAlgorithmParameters(float currentDrive);
    MorphWeights calculateWeights(float x, float y) const;

    void processAudioBlock(juce::dsp::AudioBlock<float>& block);

    TubeAlgorithm tube;
    HardClipAlgorithm hardClip;
    FoldbackAlgorithm foldback;
    FuzzAlgorithm fuzz;
    DriveAlgorithm ampDrive;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> drive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> vectorX;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> vectorY;

    double currentSampleRate = 44100.0;
    double effectiveSampleRate = 88200.0;

    int currentNumChannels = 2;

    static constexpr int oversamplingExponent = 1; // 2x oversampling
};