#include "PluginEditor.h"

ShermanFilterbankAudioProcessorEditor::ShermanFilterbankAudioProcessorEditor(ShermanFilterbankAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 360);
    setResizeLimits(760, 360, 1024, 768);

    for (int i = 0; i < 6; ++i)
    {
        auto& slider = sliders[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        slider.setDoubleClickReturnValue(true, i < 4 ? 0.5 : (i == 4 ? 0.6 : 1.0));
        slider.setRange(0.0, i < 4 ? 1.0 : (i == 4 ? 1.0 : 1.6));
        slider.setSkewFactorFromMidPoint(0.5);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::white);
        addAndMakeVisible(slider);

        auto& label = labels[i];
        label.setFont(juce::Font(14.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
        addAndMakeVisible(label);

        const juce::String id = i < 4 ? juce::String("band") + juce::String(i + 1)
                                     : (i == 4 ? "mix" : "master");
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts(), id, slider);
    }

    labels[0].setText("Low", juce::dontSendNotification);
    labels[1].setText("Mid Low", juce::dontSendNotification);
    labels[2].setText("Mid High", juce::dontSendNotification);
    labels[3].setText("High", juce::dontSendNotification);
    labels[4].setText("Mix", juce::dontSendNotification);
    labels[5].setText("Master", juce::dontSendNotification);

    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(presetLabel);

    presetBox.addItemList({"Clean", "Warm", "Bright", "Punchy", "Dark"}, 1);
    presetBox.setTextWhenNothingSelected("Preset");
    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkslategrey.darker(0.3f));
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colours::whitesmoke);
    presetBox.setColour(juce::ComboBox::arrowColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(presetBox);
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts(), "preset", presetBox);
}

void ShermanFilterbankAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto background = juce::Colour::fromRGB(16, 24, 34);
    g.fillAll(background);

    g.setColour(juce::Colours::grey);
    g.drawRoundedRectangle(8.0f, 8.0f, getWidth() - 16.0f, getHeight() - 16.0f, 18.0f, 2.0f);

    g.setColour(juce::Colours::whitesmoke);
    g.setFont(24.0f);
    g.drawText("Sherman Filterbank", juce::Rectangle<int>(20, 18, getWidth() - 40, 34), juce::Justification::left, false);

    g.setFont(13.0f);
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Live-ready 4-band tone shaping with instant presets", juce::Rectangle<int>(20, 48, getWidth() - 40, 24), juce::Justification::left, false);
}

void ShermanFilterbankAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds().reduced(20);
    presetLabel.setBounds(bounds.removeFromTop(24).withTrimmedLeft(10).withTrimmedRight(10));
    presetBox.setBounds(bounds.removeFromTop(28).withTrimmedLeft(10).withTrimmedRight(10));

    auto controls = bounds.reduced(6);
    const int columns = 3;
    const int rows = 2;
    const int cellW = controls.getWidth() / columns;
    const int cellH = controls.getHeight() / rows;

    for (int i = 0; i < 6; ++i)
    {
        const int col = i % columns;
        const int row = i / columns;
        auto area = juce::Rectangle<int>(controls.getX() + col * cellW,
                                         controls.getY() + row * cellH,
                                         cellW,
                                         cellH).reduced(10);

        sliders[i].setBounds(area.removeFromTop(area.getHeight() - 54));
        labels[i].setBounds(area.withHeight(24));
    }
}
