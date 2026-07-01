#pragma once

#include <JuceHeader.h>

#include "Algorithms/TubeAlgorithm.h"
#include "Algorithms/HardClipAlgorithm.h"
#include "Algorithms/FoldbackAlgorithm.h"
#include "Algorithms/FuzzAlgorithm.h"
#include "Algorithms/DriveAlgorithm.h"
#include "Algorithms/BitcrushAlgorithm.h"
#include "Algorithms/WavefolderAlgorithm.h"
#include "Algorithms/RectifierAlgorithm.h"

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
    void setBypassed(bool shouldBypass);
    void setOutputMode(int newOutputMode);

    void setVectorPosition(float newX, float newY);

    void setCornerAlgorithms(int newTopLeftAlgorithm,
        int newTopRightAlgorithm,
        int newBottomLeftAlgorithm,
        int newBottomRightAlgorithm);

    void process(juce::AudioBuffer<float>& buffer);

private:
    struct MorphWeights
    {
        float topLeft = 0.0f;
        float topRight = 0.0f;
        float bottomLeft = 0.0f;
        float bottomRight = 0.0f;
    };

    enum AlgorithmIndex
    {
        tubeIndex = 0,
        hardClipIndex,
        foldbackIndex,
        fuzzIndex,
        ampDriveIndex,
        bitcrushIndex,
        wavefolderIndex,
        rectifierIndex,
        numAlgorithms
    };

    enum OutputMode
    {
        stereoMode = 0,
        monoMode,
        midOnlyMode,
        sideOnlyMode,
        numOutputModes
    };

    void updateAlgorithmParameters(float currentDrive);
    MorphWeights calculateWeights(float x, float y) const;

    void processAudioBlock(juce::dsp::AudioBlock<float>& block);

    float processDcBlocker(float sample, size_t channel);

    TubeAlgorithm tube;
    HardClipAlgorithm hardClip;
    FoldbackAlgorithm foldback;
    FuzzAlgorithm fuzz;
    DriveAlgorithm ampDrive;
    BitcrushAlgorithm bitcrush;
    WavefolderAlgorithm wavefolder;
    RectifierAlgorithm rectifier;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainDb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> drive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> vectorX;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> vectorY;

    std::vector<float> dcBlockerInputState;
    std::vector<float> dcBlockerOutputState;
    float dcBlockerCoefficient = 0.995f;

    int outputMode = stereoMode;

    int topLeftAlgorithm = tubeIndex;
    int topRightAlgorithm = hardClipIndex;
    int bottomLeftAlgorithm = foldbackIndex;
    int bottomRightAlgorithm = fuzzIndex;

    double currentSampleRate = 44100.0;
    double effectiveSampleRate = 88200.0;

    int currentNumChannels = 2;

    static constexpr int oversamplingExponent = 1; // 2x oversampling
};