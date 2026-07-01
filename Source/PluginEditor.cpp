#include "PluginEditor.h"
#include "Parameters.h"

namespace
{
    juce::Rectangle<float> getPadBounds(juce::Rectangle<int> bounds)
    {
        return bounds.reduced(18).toFloat();
    }

    void drawNode(juce::Graphics& g,
        juce::Rectangle<float> pad,
        juce::Point<float> normalisedPosition,
        const juce::String& text)
    {
        const auto pos = juce::Point<float>(
            pad.getX() + normalisedPosition.x * pad.getWidth(),
            pad.getY() + normalisedPosition.y * pad.getHeight());

        g.setColour(juce::Colour::fromRGB(130, 110, 255).withAlpha(0.85f));
        g.fillEllipse(pos.x - 7.0f, pos.y - 7.0f, 14.0f, 14.0f);

        g.setColour(juce::Colours::white.withAlpha(0.78f));
        g.setFont(13.0f);

        juce::Rectangle<float> labelArea(pos.x - 48.0f, pos.y - 30.0f, 96.0f, 20.0f);

        if (normalisedPosition.y > 0.5f)
            labelArea.setY(pos.y + 10.0f);

        g.drawText(text, labelArea, juce::Justification::centred);
    }
}

VectorPadComponent::VectorPadComponent(MorphAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    vectorXParam = processor.apvts.getParameter(ParamID::vectorX);
    vectorYParam = processor.apvts.getParameter(ParamID::vectorY);

    startTimerHz(30);
}

void VectorPadComponent::paint(juce::Graphics& g)
{
    auto pad = getPadBounds(getLocalBounds());

    g.setColour(juce::Colour::fromRGB(30, 30, 40));
    g.fillRoundedRectangle(pad, 18.0f);

    g.setColour(juce::Colour::fromRGB(70, 70, 95));
    g.drawRoundedRectangle(pad, 18.0f, 1.5f);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(pad.getX(), pad.getCentreY(), pad.getRight(), pad.getCentreY(), 1.0f);
    g.drawLine(pad.getCentreX(), pad.getY(), pad.getCentreX(), pad.getBottom(), 1.0f);

    drawNode(g, pad, { 0.0f, 0.0f }, "Tube");
    drawNode(g, pad, { 1.0f, 0.0f }, "Clip");
    drawNode(g, pad, { 0.0f, 1.0f }, "Fold");
    drawNode(g, pad, { 1.0f, 1.0f }, "Fuzz");
    drawNode(g, pad, { 0.5f, 0.5f }, "Drive");

    const auto puck = getCurrentPuckPosition();

    g.setColour(juce::Colour::fromRGB(180, 150, 255).withAlpha(0.22f));
    g.fillEllipse(puck.x - 22.0f, puck.y - 22.0f, 44.0f, 44.0f);

    g.setColour(juce::Colour::fromRGB(210, 195, 255));
    g.fillEllipse(puck.x - 9.0f, puck.y - 9.0f, 18.0f, 18.0f);

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.drawEllipse(puck.x - 9.0f, puck.y - 9.0f, 18.0f, 18.0f, 1.2f);
}

void VectorPadComponent::mouseDown(const juce::MouseEvent& event)
{
    if (vectorXParam != nullptr)
        vectorXParam->beginChangeGesture();

    if (vectorYParam != nullptr)
        vectorYParam->beginChangeGesture();

    updateFromMousePosition(event.position);
}

void VectorPadComponent::mouseDrag(const juce::MouseEvent& event)
{
    updateFromMousePosition(event.position);
}

void VectorPadComponent::mouseUp(const juce::MouseEvent&)
{
    if (vectorXParam != nullptr)
        vectorXParam->endChangeGesture();

    if (vectorYParam != nullptr)
        vectorYParam->endChangeGesture();
}

void VectorPadComponent::timerCallback()
{
    repaint();
}

void VectorPadComponent::updateFromMousePosition(juce::Point<float> position)
{
    const auto pad = getPadBounds(getLocalBounds());

    const float x = juce::jlimit(0.0f, 1.0f, (position.x - pad.getX()) / pad.getWidth());
    const float y = juce::jlimit(0.0f, 1.0f, (position.y - pad.getY()) / pad.getHeight());

    if (vectorXParam != nullptr)
        vectorXParam->setValueNotifyingHost(x);

    if (vectorYParam != nullptr)
        vectorYParam->setValueNotifyingHost(y);

    repaint();
}

juce::Point<float> VectorPadComponent::getCurrentPuckPosition() const
{
    const auto pad = getPadBounds(getLocalBounds());

    const float x = vectorXParam != nullptr ? vectorXParam->getValue() : 0.5f;
    const float y = vectorYParam != nullptr ? vectorYParam->getValue() : 0.5f;

    return {
        pad.getX() + x * pad.getWidth(),
        pad.getY() + y * pad.getHeight()
    };
}

MorphAudioProcessorEditor::MorphAudioProcessorEditor(MorphAudioProcessor& p)
    : AudioProcessorEditor(&p),
    processor(p),
    vectorPad(p)
{
    setSize(720, 500);
    setResizable(true, true);

    titleLabel.setText("PRISM", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(vectorPad);

    configureSlider(inputSlider);
    configureSlider(driveSlider);
    configureSlider(mixSlider);
    configureSlider(outputSlider);

    configureLabel(inputLabel, "Input");
    configureLabel(driveLabel, "Drive");
    configureLabel(mixLabel, "Mix");
    configureLabel(outputLabel, "Output");

    inputAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::inputGain, inputSlider);

    driveAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::drive, driveSlider);

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
}

void MorphAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(24);

    titleLabel.setBounds(bounds.removeFromTop(48));

    vectorPad.setBounds(bounds.removeFromTop(290));

    bounds.removeFromTop(10);

    auto controls = bounds.removeFromBottom(120);

    const int controlWidth = controls.getWidth() / 4;

    layoutControl(controls.removeFromLeft(controlWidth), inputSlider, inputLabel);
    layoutControl(controls.removeFromLeft(controlWidth), driveSlider, driveLabel);
    layoutControl(controls.removeFromLeft(controlWidth), mixSlider, mixLabel);
    layoutControl(controls, outputSlider, outputLabel);
}