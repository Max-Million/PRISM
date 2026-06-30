#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MorphAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit MorphAudioProcessorEditor (MorphAudioProcessor&);
    ~MorphAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MorphAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Slider inputSlider;
    juce::Slider outputSlider;
    juce::Slider driveSlider;
    juce::Slider toneSlider;
    juce::Slider mixSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;

    void configureSlider (juce::Slider& slider, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MorphAudioProcessorEditor)
};
