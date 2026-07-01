#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MorphAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit MorphAudioProcessorEditor(MorphAudioProcessor&);
    ~MorphAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MorphAudioProcessor& processor;

    juce::Label titleLabel;

    juce::Slider inputSlider;
    juce::Slider driveSlider;
    juce::Slider toneSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;

    juce::Label inputLabel;
    juce::Label driveLabel;
    juce::Label toneLabel;
    juce::Label mixLabel;
    juce::Label outputLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    void configureSlider(juce::Slider& slider);
    void configureLabel(juce::Label& label, const juce::String& text);
    void layoutControl(juce::Rectangle<int> area,
        juce::Slider& slider,
        juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphAudioProcessorEditor)
};