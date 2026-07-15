#pragma once

#include "PluginProcessor.h"

class ShermanPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit ShermanPluginAudioProcessorEditor (ShermanPluginAudioProcessor&);
    ~ShermanPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ShermanPluginAudioProcessor& processorRef;

    juce::Slider freq1Slider, reso1Slider, mode1Slider, corr1Slider;
    juce::Slider driveSlider, envAmtSlider, decaySlider, fmAmtSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Slider freq2Slider, reso2Slider, mode2Slider, corr2Slider, harmSlider;
    juce::Slider routingSlider, wetSlider, bypassSlider;
    juce::ComboBox oversampleCombo;

    juce::Label freq1Label, reso1Label, mode1Label, corr1Label;
    juce::Label driveLabel, envAmtLabel, decayLabel, fmAmtLabel;
    juce::Label lfoRateLabel, lfoDepthLabel;
    juce::Label freq2Label, reso2Label, mode2Label, corr2Label, harmLabel;
    juce::Label routingLabel, wetLabel, bypassLabel, oversampleLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freq1Att, reso1Att, mode1Att, corr1Att;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAtt, envAmtAtt, decayAtt, fmAmtAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freq2Att, reso2Att, mode2Att, corr2Att, harmAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> routingAtt, wetAtt, bypassAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversampleAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShermanPluginAudioProcessorEditor)
};
