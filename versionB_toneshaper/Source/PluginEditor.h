#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ShermanFilterbankAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ShermanFilterbankAudioProcessorEditor(ShermanFilterbankAudioProcessor&);
    ~ShermanFilterbankAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ShermanFilterbankAudioProcessor& processor;
    std::array<juce::Slider, 6> sliders;
    std::array<juce::Label, 6> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> attachments;
    juce::ComboBox presetBox;
    juce::Label presetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShermanFilterbankAudioProcessorEditor)
};
