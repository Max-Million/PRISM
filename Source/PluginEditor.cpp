#include "PluginEditor.h"
#include "Parameters.h"

namespace
{
    juce::Rectangle<float> getPadBounds(juce::Rectangle<int> bounds)
    {
        return bounds.reduced(22).toFloat();
    }

    int getChoiceIndexFromParameter(MorphAudioProcessor& processor,
        const juce::String& parameterID)
    {
        const auto* value = processor.apvts.getRawParameterValue(parameterID);

        if (value == nullptr)
            return 0;

        return juce::jlimit(0, getAlgorithmChoices().size() - 1,
            static_cast<int> (std::round(value->load())));
    }

    void addAlgorithmChoicesToBox(juce::ComboBox& box)
    {
        const auto choices = getAlgorithmChoices();

        for (int i = 0; i < choices.size(); ++i)
            box.addItem(choices[i], i + 1);
    }

    void drawCornerNode(juce::Graphics& g,
        juce::Rectangle<float> pad,
        juce::Point<float> normalisedPosition,
        const juce::String& cornerName,
        const juce::String& algorithmName)
    {
        const auto pos = juce::Point<float>(
            pad.getX() + normalisedPosition.x * pad.getWidth(),
            pad.getY() + normalisedPosition.y * pad.getHeight());

        g.setColour(juce::Colour::fromRGB(150, 125, 255).withAlpha(0.92f));
        g.fillEllipse(pos.x - 7.0f, pos.y - 7.0f, 14.0f, 14.0f);

        juce::Rectangle<float> titleArea;
        juce::Rectangle<float> subtitleArea;

        if (normalisedPosition.y < 0.5f)
        {
            titleArea = { pos.x - 70.0f, pos.y + 12.0f, 140.0f, 18.0f };
            subtitleArea = { pos.x - 70.0f, pos.y + 29.0f, 140.0f, 16.0f };
        }
        else
        {
            titleArea = { pos.x - 70.0f, pos.y - 45.0f, 140.0f, 18.0f };
            subtitleArea = { pos.x - 70.0f, pos.y - 28.0f, 140.0f, 16.0f };
        }

        g.setColour(juce::Colours::white.withAlpha(0.72f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(cornerName, titleArea, juce::Justification::centred);

        g.setColour(juce::Colours::white.withAlpha(0.92f));
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(algorithmName, subtitleArea, juce::Justification::centred);
    }
}

VectorPadComponent::VectorPadComponent(MorphAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    vectorXParam = processor.apvts.getParameter(ParamID::vectorX);
    vectorYParam = processor.apvts.getParameter(ParamID::vectorY);

    startTimerHz(30);
}

juce::String VectorPadComponent::getCornerAlgorithmName(const juce::String& parameterID) const
{
    const auto choices = getAlgorithmChoices();
    const int index = getChoiceIndexFromParameter(processor, parameterID);

    if (juce::isPositiveAndBelow(index, choices.size()))
        return choices[index];

    return "Unknown";
}

void VectorPadComponent::paint(juce::Graphics& g)
{
    auto pad = getPadBounds(getLocalBounds());

    g.setColour(juce::Colour::fromRGB(27, 27, 36));
    g.fillRoundedRectangle(pad, 20.0f);

    g.setColour(juce::Colour::fromRGB(70, 70, 95));
    g.drawRoundedRectangle(pad, 20.0f, 1.4f);

    g.setColour(juce::Colours::white.withAlpha(0.055f));

    for (int i = 1; i < 4; ++i)
    {
        const float t = static_cast<float>(i) / 4.0f;

        g.drawLine(pad.getX() + pad.getWidth() * t,
            pad.getY(),
            pad.getX() + pad.getWidth() * t,
            pad.getBottom(),
            1.0f);

        g.drawLine(pad.getX(),
            pad.getY() + pad.getHeight() * t,
            pad.getRight(),
            pad.getY() + pad.getHeight() * t,
            1.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(pad.getX(), pad.getCentreY(), pad.getRight(), pad.getCentreY(), 1.2f);
    g.drawLine(pad.getCentreX(), pad.getY(), pad.getCentreX(), pad.getBottom(), 1.2f);

    const auto centre = pad.getCentre();

    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawLine(pad.getX(), pad.getY(), pad.getRight(), pad.getBottom(), 1.0f);
    g.drawLine(pad.getRight(), pad.getY(), pad.getX(), pad.getBottom(), 1.0f);

    drawCornerNode(g, pad, { 0.0f, 0.0f },
        "TOP LEFT",
        getCornerAlgorithmName(ParamID::topLeftAlgorithm));

    drawCornerNode(g, pad, { 1.0f, 0.0f },
        "TOP RIGHT",
        getCornerAlgorithmName(ParamID::topRightAlgorithm));

    drawCornerNode(g, pad, { 0.0f, 1.0f },
        "BOTTOM LEFT",
        getCornerAlgorithmName(ParamID::bottomLeftAlgorithm));

    drawCornerNode(g, pad, { 1.0f, 1.0f },
        "BOTTOM RIGHT",
        getCornerAlgorithmName(ParamID::bottomRightAlgorithm));

    g.setColour(juce::Colour::fromRGB(150, 125, 255).withAlpha(0.28f));
    g.fillEllipse(centre.x - 15.0f, centre.y - 15.0f, 30.0f, 30.0f);

    g.setColour(juce::Colours::white.withAlpha(0.58f));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("CENTER MIX",
        juce::Rectangle<float>(centre.x - 60.0f, centre.y + 18.0f, 120.0f, 18.0f),
        juce::Justification::centred);

    const auto puck = getCurrentPuckPosition();

    g.setColour(juce::Colour::fromRGB(180, 150, 255).withAlpha(0.20f));
    g.fillEllipse(puck.x - 25.0f, puck.y - 25.0f, 50.0f, 50.0f);

    g.setColour(juce::Colour::fromRGB(210, 195, 255).withAlpha(0.95f));
    g.fillEllipse(puck.x - 10.0f, puck.y - 10.0f, 20.0f, 20.0f);

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.drawEllipse(puck.x - 10.0f, puck.y - 10.0f, 20.0f, 20.0f, 1.2f);
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
    setSize(820, 640);
    setResizable(true, true);

    titleLabel.setText("PRISM", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("assignable vector distortion", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setFont(13.0f);
    subtitleLabel.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.55f));
    addAndMakeVisible(subtitleLabel);

    configureBypassButton();

    configureAlgorithmBox(topLeftBox);
    configureAlgorithmBox(topRightBox);
    configureAlgorithmBox(bottomLeftBox);
    configureAlgorithmBox(bottomRightBox);

    configureCornerLabel(topLeftLabel, "TOP LEFT");
    configureCornerLabel(topRightLabel, "TOP RIGHT");
    configureCornerLabel(bottomLeftLabel, "BOTTOM LEFT");
    configureCornerLabel(bottomRightLabel, "BOTTOM RIGHT");

    addAndMakeVisible(vectorPad);

    configureSlider(inputSlider);
    configureSlider(driveSlider);
    configureSlider(mixSlider);
    configureSlider(outputSlider);

    configureLabel(inputLabel, "INPUT");
    configureLabel(driveLabel, "DRIVE");
    configureLabel(mixLabel, "MIX");
    configureLabel(outputLabel, "OUTPUT");

    inputAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::inputGain, inputSlider);

    driveAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::drive, driveSlider);

    mixAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::mix, mixSlider);

    outputAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, ParamID::outputGain, outputSlider);

    bypassAttachment = std::make_unique<ButtonAttachment>(
        processor.apvts, ParamID::bypass, bypassButton);

    topLeftAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, ParamID::topLeftAlgorithm, topLeftBox);

    topRightAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, ParamID::topRightAlgorithm, topRightBox);

    bottomLeftAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, ParamID::bottomLeftAlgorithm, bottomLeftBox);

    bottomRightAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, ParamID::bottomRightAlgorithm, bottomRightBox);
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
    label.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.76f));

    addAndMakeVisible(label);
}

void MorphAudioProcessorEditor::configureBypassButton()
{
    bypassButton.setButtonText("BYPASS");
    bypassButton.setClickingTogglesState(true);

    bypassButton.setColour(juce::ToggleButton::textColourId,
        juce::Colours::white.withAlpha(0.78f));

    bypassButton.setColour(juce::ToggleButton::tickColourId,
        juce::Colour::fromRGB(210, 195, 255));

    bypassButton.setColour(juce::ToggleButton::tickDisabledColourId,
        juce::Colours::white.withAlpha(0.25f));

    addAndMakeVisible(bypassButton);
}

void MorphAudioProcessorEditor::configureAlgorithmBox(juce::ComboBox& box)
{
    addAlgorithmChoicesToBox(box);

    box.setColour(juce::ComboBox::backgroundColourId,
        juce::Colour::fromRGB(28, 28, 38));

    box.setColour(juce::ComboBox::outlineColourId,
        juce::Colour::fromRGB(80, 70, 120));

    box.setColour(juce::ComboBox::textColourId,
        juce::Colours::white.withAlpha(0.88f));

    box.setColour(juce::ComboBox::arrowColourId,
        juce::Colour::fromRGB(210, 195, 255));

    box.onChange = [this]
        {
            vectorPad.repaint();
        };

    addAndMakeVisible(box);
}

void MorphAudioProcessorEditor::configureCornerLabel(juce::Label& label,
    const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.62f));

    addAndMakeVisible(label);
}

void MorphAudioProcessorEditor::layoutCornerSelector(juce::Rectangle<int> area,
    juce::Label& label,
    juce::ComboBox& box)
{
    area = area.reduced(6);

    label.setBounds(area.removeFromTop(20));
    box.setBounds(area.removeFromTop(28));
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
    g.fillAll(juce::Colour::fromRGB(16, 16, 21));

    auto bounds = getLocalBounds().toFloat();

    g.setGradientFill(juce::ColourGradient(
        juce::Colour::fromRGB(34, 26, 58).withAlpha(0.55f),
        bounds.getCentreX(),
        0.0f,
        juce::Colour::fromRGB(16, 16, 21),
        bounds.getCentreX(),
        bounds.getBottom(),
        false));

    g.fillRect(bounds);
}

void MorphAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(24);

    auto header = bounds.removeFromTop(58);

    bypassButton.setBounds(header.removeFromRight(104).withSizeKeepingCentre(94, 28));

    titleLabel.setBounds(header.removeFromTop(38));
    subtitleLabel.setBounds(header.removeFromTop(20));

    bounds.removeFromTop(8);

    auto selectorArea = bounds.removeFromTop(64);
    const int selectorWidth = selectorArea.getWidth() / 4;

    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), topLeftLabel, topLeftBox);
    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), topRightLabel, topRightBox);
    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), bottomLeftLabel, bottomLeftBox);
    layoutCornerSelector(selectorArea, bottomRightLabel, bottomRightBox);

    bounds.removeFromTop(8);

    vectorPad.setBounds(bounds.removeFromTop(330));

    bounds.removeFromTop(8);

    auto controls = bounds.removeFromBottom(112);

    const int controlWidth = controls.getWidth() / 4;

    layoutControl(controls.removeFromLeft(controlWidth), inputSlider, inputLabel);
    layoutControl(controls.removeFromLeft(controlWidth), driveSlider, driveLabel);
    layoutControl(controls.removeFromLeft(controlWidth), mixSlider, mixLabel);
    layoutControl(controls, outputSlider, outputLabel);
}