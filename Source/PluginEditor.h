#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VectorPadComponent final : public juce::Component,
    private juce::Timer
{
public:
    explicit VectorPadComponent(MorphAudioProcessor& processorToUse);
    ~VectorPadComponent() override = default;

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    MorphAudioProcessor& processor;

    juce::RangedAudioParameter* vectorXParam = nullptr;
    juce::RangedAudioParameter* vectorYParam = nullptr;

    void timerCallback() override;

    void updateFromMousePosition(juce::Point<float> position);
    juce::Point<float> getCurrentPuckPosition() const;

    juce::String getCornerAlgorithmName(const juce::String& parameterID) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VectorPadComponent)
};

class MorphAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit MorphAudioProcessorEditor(MorphAudioProcessor&);
    ~MorphAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MorphAudioProcessor& processor;

    VectorPadComponent vectorPad;

    juce::Label titleLabel;
    juce::Label subtitleLabel;

    juce::ToggleButton bypassButton;

    juce::ComboBox presetBox;
    juce::Label presetLabel;

    juce::ComboBox outputModeBox;
    juce::Label outputModeLabel;

    juce::ComboBox topLeftBox;
    juce::ComboBox topRightBox;
    juce::ComboBox bottomLeftBox;
    juce::ComboBox bottomRightBox;

    juce::Label topLeftLabel;
    juce::Label topRightLabel;
    juce::Label bottomLeftLabel;
    juce::Label bottomRightLabel;

    juce::Slider inputSlider;
    juce::Slider driveSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;

    juce::Label inputLabel;
    juce::Label driveLabel;
    juce::Label mixLabel;
    juce::Label outputLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    std::unique_ptr<ButtonAttachment> bypassAttachment;

    std::unique_ptr<ComboBoxAttachment> outputModeAttachment;

    std::unique_ptr<ComboBoxAttachment> topLeftAttachment;
    std::unique_ptr<ComboBoxAttachment> topRightAttachment;
    std::unique_ptr<ComboBoxAttachment> bottomLeftAttachment;
    std::unique_ptr<ComboBoxAttachment> bottomRightAttachment;

    void configureSlider(juce::Slider& slider);
    void configureLabel(juce::Label& label, const juce::String& text);
    void configureBypassButton();

    void configurePresetBox();
    void configureOutputModeBox();
    void configureAlgorithmBox(juce::ComboBox& box);
    void configureCornerLabel(juce::Label& label, const juce::String& text);

    void applyFactoryPreset(int presetIndex);

    void layoutSmallSelector(juce::Rectangle<int> area,
        juce::Label& label,
        juce::ComboBox& box);

    void layoutCornerSelector(juce::Rectangle<int> area,
        juce::Label& label,
        juce::ComboBox& box);

    void layoutControl(juce::Rectangle<int> area,
        juce::Slider& slider,
        juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphAudioProcessorEditor)
};