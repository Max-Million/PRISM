#include "PluginEditor.h"
#include "Parameters.h"

MorphAudioProcessorEditor::MorphAudioProcessorEditor(MorphAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(720, 440);
    setResizable(true, true);

    titleLabel.setText("MORPH", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    configureSlider(inputSlider);
    configureSlider(driveSlider);
    configureSlider(toneSlider);
    configureSlider(mixSlider);
    configureSlider(outputSlider);

    configureLabel(inputLabel, "Input");
    configureLabel(driveLabel, "Drive");
    configureLabel(toneLabel, "Tone");
    configureLabel(mixLabel, "Mix");
    configureLabel(outputLabel, "Output");

    inputAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::inputGain, inputSlider);

    driveAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::drive, driveSlider);

    toneAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::tone, toneSlider);

    mixAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::mix, mixSlider);

    outputAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::outputGain, outputSlider);
}

void MorphAudioProcessorEditor::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 22);

    slider.setColour(juce::Slider::rotarySliderFillColourId,
        juce::Colour::fromRGB(160, 130, 255));

    slider.setColour(juce::Slider::rotarySliderOutlineColourId,
        juce::Colour::fromRGB(55, 55, 68));

    slider.setColour(juce::Slider::thumbColourId,
        juce::Colours::white.withAlpha(0.9f));

    slider.setColour(juce::Slider::textBoxTextColourId,
        juce::Colours::white.withAlpha(0.85f));

    slider.setColour(juce::Slider::textBoxOutlineColourId,
        juce::Colours::transparentBlack);

    addAndMakeVisible(slider);
}

void MorphAudioProcessorEditor::configureLabel(juce::Label& label,
    const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.82f));

    addAndMakeVisible(label);
}

void MorphAudioProcessorEditor::layoutControl(juce::Rectangle<int> area,
    juce::Slider& slider,
    juce::Label& label)
{
    area = area.reduced(8);

    label.setBounds(area.removeFromTop(22));
    slider.setBounds(area);
}

void MorphAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(18, 18, 22));

    auto pad = getLocalBounds()
        .reduced(32)
        .withTrimmedTop(82)
        .withTrimmedBottom(138);

    g.setColour(juce::Colour::fromRGB(38, 38, 48));
    g.fillRoundedRectangle(pad.toFloat(), 18.0f);

    g.setColour(juce::Colour::fromRGB(90, 90, 110));
    g.drawRoundedRectangle(pad.toFloat(), 18.0f, 1.5f);

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.setFont(16.0f);
    g.drawText("Vector pad placeholder — morph engine comes next",
        pad,
        juce::Justification::centred);
}

void MorphAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(24);

    titleLabel.setBounds(bounds.removeFromTop(54));

    bounds.removeFromTop(230);

    auto controls = bounds.removeFromBottom(120);

    const int controlWidth = controls.getWidth() / 5;

    layoutControl(controls.removeFromLeft(controlWidth), inputSlider, inputLabel);
    layoutControl(controls.removeFromLeft(controlWidth), driveSlider, driveLabel);
    layoutControl(controls.removeFromLeft(controlWidth), toneSlider, toneLabel);
    layoutControl(controls.removeFromLeft(controlWidth), mixSlider, mixLabel);
    layoutControl(controls, outputSlider, outputLabel);
}