#include "PluginEditor.h"
#include "Parameters.h"

#include <array>
#include <cmath>

namespace
{
    struct FactoryPreset
    {
        juce::String name;

        float inputGainDb = 0.0f;
        float outputGainDb = -6.0f;
        float drive = 6.0f;
        float tone = 50.0f;
        float mixPercent = 100.0f;
        float vectorX = 0.5f;
        float vectorY = 0.5f;

        int outputMode = 0;

        int topLeftAlgorithm = 0;
        int topRightAlgorithm = 1;
        int bottomLeftAlgorithm = 2;
        int bottomRightAlgorithm = 3;
    };

    const std::vector<FactoryPreset>& getFactoryPresets()
    {
        static const std::vector<FactoryPreset> presets
        {
            {
                "Init Prism",
                0.0f, -6.0f, 6.0f, 50.0f, 100.0f, 0.50f, 0.50f,
                0,
                0, 1, 2, 3
            },
            {
                "Warm Tube Drive",
                -3.0f, -5.0f, 5.5f, 38.0f, 85.0f, 0.25f, 0.25f,
                0,
                0, 0, 4, 7
            },
            {
                "Hard Clip Bite",
                -8.0f, -9.0f, 10.5f, 62.0f, 100.0f, 0.78f, 0.18f,
                0,
                0, 1, 4, 6
            },
            {
                "Foldback Motion",
                -8.0f, -10.0f, 12.0f, 56.0f, 92.0f, 0.30f, 0.74f,
                0,
                2, 6, 0, 4
            },
            {
                "Fuzz Wall",
                -10.0f, -12.0f, 14.0f, 36.0f, 100.0f, 0.72f, 0.80f,
                0,
                3, 1, 7, 3
            },
            {
                "Crushed Sparks",
                -8.0f, -10.0f, 11.0f, 70.0f, 88.0f, 0.54f, 0.62f,
                0,
                5, 1, 6, 2
            },
            {
                "Wavefold Glass",
                -7.0f, -9.0f, 9.5f, 64.0f, 82.0f, 0.62f, 0.42f,
                0,
                6, 0, 2, 4
            },
            {
                "Rectified Growl",
                -9.0f, -11.0f, 13.0f, 42.0f, 95.0f, 0.42f, 0.82f,
                0,
                7, 3, 4, 2
            },
            {
                "Parallel Grit",
                -6.0f, -8.0f, 8.0f, 50.0f, 55.0f, 0.50f, 0.50f,
                0,
                0, 1, 5, 3
            },
            {
                "Side Texture",
                -7.0f, -10.0f, 9.0f, 68.0f, 78.0f, 0.66f, 0.48f,
                3,
                6, 5, 2, 7
            }
        };

        return presets;
    }

    void setParameterValue(MorphAudioProcessor& processor,
        const juce::String& parameterID,
        float newValue)
    {
        if (auto* parameter = processor.apvts.getParameter(parameterID))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(newValue));
            parameter->endChangeGesture();
        }
    }

    float getRandomFloat(juce::Random& random, float minimum, float maximum)
    {
        return minimum + random.nextFloat() * (maximum - minimum);
    }

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

    void addOutputModeChoicesToBox(juce::ComboBox& box)
    {
        const auto choices = getOutputModeChoices();

        for (int i = 0; i < choices.size(); ++i)
            box.addItem(choices[i], i + 1);
    }

    void addFactoryPresetsToBox(juce::ComboBox& box)
    {
        const auto& presets = getFactoryPresets();

        for (int i = 0; i < static_cast<int>(presets.size()); ++i)
            box.addItem(presets[static_cast<size_t>(i)].name, i + 1);
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
    setSize(820, 700);
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
    configureRandomizeButton();
    configurePresetBox();
    configureOutputModeBox();

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
    configureSlider(toneSlider);
    configureSlider(mixSlider);
    configureSlider(outputSlider);

    configureLabel(inputLabel, "INPUT");
    configureLabel(driveLabel, "DRIVE");
    configureLabel(toneLabel, "TONE");
    configureLabel(mixLabel, "MIX");
    configureLabel(outputLabel, "OUTPUT");

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

    bypassAttachment = std::make_unique<ButtonAttachment>(
        processor.apvts, ParamID::bypass, bypassButton);

    outputModeAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, ParamID::outputMode, outputModeBox);

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

void MorphAudioProcessorEditor::configureRandomizeButton()
{
    randomizeButton.setButtonText("RANDOMIZE");

    randomizeButton.setColour(juce::TextButton::buttonColourId,
        juce::Colour::fromRGB(42, 35, 70));

    randomizeButton.setColour(juce::TextButton::buttonOnColourId,
        juce::Colour::fromRGB(70, 55, 115));

    randomizeButton.setColour(juce::TextButton::textColourOffId,
        juce::Colours::white.withAlpha(0.82f));

    randomizeButton.setColour(juce::TextButton::textColourOnId,
        juce::Colours::white);

    randomizeButton.onClick = [this]
        {
            presetBox.setSelectedId(0, juce::dontSendNotification);
            applyRandomize();
        };

    addAndMakeVisible(randomizeButton);
}

void MorphAudioProcessorEditor::configurePresetBox()
{
    presetLabel.setText("FACTORY PRESET", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    presetLabel.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.64f));
    addAndMakeVisible(presetLabel);

    addFactoryPresetsToBox(presetBox);

    presetBox.setTextWhenNothingSelected("Select preset");

    presetBox.setColour(juce::ComboBox::backgroundColourId,
        juce::Colour::fromRGB(28, 28, 38));

    presetBox.setColour(juce::ComboBox::outlineColourId,
        juce::Colour::fromRGB(80, 70, 120));

    presetBox.setColour(juce::ComboBox::textColourId,
        juce::Colours::white.withAlpha(0.88f));

    presetBox.setColour(juce::ComboBox::arrowColourId,
        juce::Colour::fromRGB(210, 195, 255));

    presetBox.onChange = [this]
        {
            const int presetIndex = presetBox.getSelectedId() - 1;

            if (presetIndex >= 0)
                applyFactoryPreset(presetIndex);
        };

    addAndMakeVisible(presetBox);
}

void MorphAudioProcessorEditor::configureOutputModeBox()
{
    outputModeLabel.setText("OUTPUT MODE", juce::dontSendNotification);
    outputModeLabel.setJustificationType(juce::Justification::centred);
    outputModeLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    outputModeLabel.setColour(juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.64f));
    addAndMakeVisible(outputModeLabel);

    addOutputModeChoicesToBox(outputModeBox);

    outputModeBox.setColour(juce::ComboBox::backgroundColourId,
        juce::Colour::fromRGB(28, 28, 38));

    outputModeBox.setColour(juce::ComboBox::outlineColourId,
        juce::Colour::fromRGB(80, 70, 120));

    outputModeBox.setColour(juce::ComboBox::textColourId,
        juce::Colours::white.withAlpha(0.88f));

    outputModeBox.setColour(juce::ComboBox::arrowColourId,
        juce::Colour::fromRGB(210, 195, 255));

    addAndMakeVisible(outputModeBox);
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

void MorphAudioProcessorEditor::applyFactoryPreset(int presetIndex)
{
    const auto& presets = getFactoryPresets();

    if (!juce::isPositiveAndBelow(presetIndex, static_cast<int> (presets.size())))
        return;

    const auto& preset = presets[static_cast<size_t> (presetIndex)];

    setParameterValue(processor, ParamID::bypass, 0.0f);

    setParameterValue(processor, ParamID::inputGain, preset.inputGainDb);
    setParameterValue(processor, ParamID::outputGain, preset.outputGainDb);
    setParameterValue(processor, ParamID::drive, preset.drive);
    setParameterValue(processor, ParamID::tone, preset.tone);
    setParameterValue(processor, ParamID::mix, preset.mixPercent);

    setParameterValue(processor, ParamID::vectorX, preset.vectorX);
    setParameterValue(processor, ParamID::vectorY, preset.vectorY);

    setParameterValue(processor, ParamID::outputMode, static_cast<float> (preset.outputMode));

    setParameterValue(processor, ParamID::topLeftAlgorithm,
        static_cast<float> (preset.topLeftAlgorithm));

    setParameterValue(processor, ParamID::topRightAlgorithm,
        static_cast<float> (preset.topRightAlgorithm));

    setParameterValue(processor, ParamID::bottomLeftAlgorithm,
        static_cast<float> (preset.bottomLeftAlgorithm));

    setParameterValue(processor, ParamID::bottomRightAlgorithm,
        static_cast<float> (preset.bottomRightAlgorithm));

    vectorPad.repaint();
}

void MorphAudioProcessorEditor::applyRandomize()
{
    auto& random = juce::Random::getSystemRandom();

    const float randomizedDrive = getRandomFloat(random, 4.0f, 16.5f);
    const float randomizedTone = getRandomFloat(random, 24.0f, 78.0f);
    const float randomizedMix = getRandomFloat(random, 55.0f, 100.0f);
    const float randomizedVectorX = getRandomFloat(random, 0.08f, 0.92f);
    const float randomizedVectorY = getRandomFloat(random, 0.08f, 0.92f);

    std::vector<int> algorithmPool;

    const int algorithmCount = getAlgorithmChoices().size();

    for (int i = 0; i < algorithmCount; ++i)
        algorithmPool.push_back(i);

    std::array<int, 4> randomizedAlgorithms{};

    for (auto& algorithm : randomizedAlgorithms)
    {
        const int poolIndex = random.nextInt(static_cast<int>(algorithmPool.size()));

        algorithm = algorithmPool[static_cast<size_t>(poolIndex)];
        algorithmPool.erase(algorithmPool.begin() + poolIndex);
    }

    // Randomize only creative parameters.
    // Input, Output, Output Mode, and Bypass are intentionally left untouched.
    setParameterValue(processor, ParamID::drive, randomizedDrive);
    setParameterValue(processor, ParamID::tone, randomizedTone);
    setParameterValue(processor, ParamID::mix, randomizedMix);

    setParameterValue(processor, ParamID::vectorX, randomizedVectorX);
    setParameterValue(processor, ParamID::vectorY, randomizedVectorY);

    setParameterValue(processor, ParamID::topLeftAlgorithm,
        static_cast<float> (randomizedAlgorithms[0]));

    setParameterValue(processor, ParamID::topRightAlgorithm,
        static_cast<float> (randomizedAlgorithms[1]));

    setParameterValue(processor, ParamID::bottomLeftAlgorithm,
        static_cast<float> (randomizedAlgorithms[2]));

    setParameterValue(processor, ParamID::bottomRightAlgorithm,
        static_cast<float> (randomizedAlgorithms[3]));

    vectorPad.repaint();
}

void MorphAudioProcessorEditor::layoutSmallSelector(juce::Rectangle<int> area,
    juce::Label& label,
    juce::ComboBox& box)
{
    area = area.reduced(6);

    label.setBounds(area.removeFromTop(20));
    box.setBounds(area.removeFromTop(28));
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

    randomizeButton.setBounds(header.removeFromLeft(122).withSizeKeepingCentre(112, 28));
    bypassButton.setBounds(header.removeFromRight(104).withSizeKeepingCentre(94, 28));

    titleLabel.setBounds(header.removeFromTop(38));
    subtitleLabel.setBounds(header.removeFromTop(20));

    bounds.removeFromTop(8);

    auto presetOutputArea = bounds.removeFromTop(64);
    const int presetOutputWidth = presetOutputArea.getWidth() / 2;

    layoutSmallSelector(presetOutputArea.removeFromLeft(presetOutputWidth),
        presetLabel,
        presetBox);

    layoutSmallSelector(presetOutputArea,
        outputModeLabel,
        outputModeBox);

    bounds.removeFromTop(8);

    auto selectorArea = bounds.removeFromTop(64);
    const int selectorWidth = selectorArea.getWidth() / 4;

    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), topLeftLabel, topLeftBox);
    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), topRightLabel, topRightBox);
    layoutCornerSelector(selectorArea.removeFromLeft(selectorWidth), bottomLeftLabel, bottomLeftBox);
    layoutCornerSelector(selectorArea, bottomRightLabel, bottomRightBox);

    bounds.removeFromTop(8);

    vectorPad.setBounds(bounds.removeFromTop(320));

    bounds.removeFromTop(8);

    auto controls = bounds.removeFromBottom(112);

    const int controlWidth = controls.getWidth() / 5;

    layoutControl(controls.removeFromLeft(controlWidth), inputSlider, inputLabel);
    layoutControl(controls.removeFromLeft(controlWidth), driveSlider, driveLabel);
    layoutControl(controls.removeFromLeft(controlWidth), toneSlider, toneLabel);
    layoutControl(controls.removeFromLeft(controlWidth), mixSlider, mixLabel);
    layoutControl(controls, outputSlider, outputLabel);
}