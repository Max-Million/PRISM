#include "PluginEditor.h"
#include "Parameters.h"

MorphAudioProcessorEditor::MorphAudioProcessorEditor (MorphAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (720, 440);
    setResizable (true, true);

    titleLabel.setText ("MORPH", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (32.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    configureSlider (inputSlider, "Input");
    configureSlider (outputSlider, "Output");
    configureSlider (driveSlider, "Drive");
    configureSlider (toneSlider, "Tone");
    configureSlider (mixSlider, "Mix");

    inputAttachment = std::make_unique<SliderAttachment> (processor.apvts, ParamID::inputGain, inputSlider);
    outputAttachment = std::make_unique<SliderAttachment> (processor.apvts, ParamID::outputGain, outputSlider);
    driveAttachment = std::make_unique<SliderAttachment> (processor.apvts, ParamID::drive, driveSlider);
    toneAttachment = std::make_unique<SliderAttachment> (processor.apvts, ParamID::tone, toneSlider);
    mixAttachment = std::make_unique<SliderAttachment> (processor.apvts, ParamID::mix, mixSlider);
}

void MorphAudioProcessorEditor::configureSlider (juce::Slider& slider, const juce::String& name)
{
    slider.setName (name);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 22);
    addAndMakeVisible (slider);
}

void MorphAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 22));

    auto pad = getLocalBounds().reduced (32).withTrimmedTop (82).withTrimmedBottom (138);
    g.setColour (juce::Colour::fromRGB (38, 38, 48));
    g.fillRoundedRectangle (pad.toFloat(), 18.0f);

    g.setColour (juce::Colour::fromRGB (90, 90, 110));
    g.drawRoundedRectangle (pad.toFloat(), 18.0f, 1.5f);

    g.setColour (juce::Colours::white.withAlpha (0.72f));
    g.setFont (16.0f);
    g.drawText ("Vector pad placeholder — DSP morph engine comes next", pad, juce::Justification::centred);
}

void MorphAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (24);
    titleLabel.setBounds (bounds.removeFromTop (54));

    bounds.removeFromTop (230);
    auto controls = bounds.removeFromBottom (110);

    auto width = controls.getWidth() / 5;
    inputSlider.setBounds  (controls.removeFromLeft (width).reduced (8));
    driveSlider.setBounds  (controls.removeFromLeft (width).reduced (8));
    toneSlider.setBounds   (controls.removeFromLeft (width).reduced (8));
    mixSlider.setBounds    (controls.removeFromLeft (width).reduced (8));
    outputSlider.setBounds (controls.reduced (8));
}
