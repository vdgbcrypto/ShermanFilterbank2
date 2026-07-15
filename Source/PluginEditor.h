#pragma once

#include "PluginProcessor.h"

// A discrete toggle-switch control: a row of mutually-exclusive buttons bound
// directly to an AudioParameterChoice. Shows the selected position; reflects
// host/automation changes via the APVTS listener. Used for all the Sherman
// "selector" params (Mode, Correction, Routing, Harmonics, Oversample).
class SegmentedSwitch  : public juce::Component,
                          private juce::AudioProcessorValueTreeState::Listener
{
public:
    SegmentedSwitch() = default;

    void init (juce::AudioProcessorValueTreeState& apvts,
               const juce::String& paramID,
               const juce::StringArray& items)
    {
        state = &apvts;
        pid = paramID;
        for (int i = 0; i < items.size(); ++i)
        {
            auto* b = buttons.add (new juce::TextButton (items[i]));
            b->setClickingTogglesState (false);
            b->onClick = [this, i] { state->getParameter (pid)->setValueNotifyingHost ((float) i); };
            addAndMakeVisible (b);
        }
        state->addParameterListener (pid, this);
        updateButtons ((int) (state->getRawParameterValue (pid)->load() + 0.5f));
    }

    ~SegmentedSwitch() override { if (state) state->removeParameterListener (pid, this); }

    void resized() override
    {
        auto r = getLocalBounds();
        int n = buttons.size();
        int w = r.getWidth() / n;
        for (int i = 0; i < n; ++i)
            buttons[i]->setBounds (r.getX() + i * w, r.getY(), w - 2, r.getHeight());
    }

    void parameterChanged (const juce::String&, float v) override
    {
        updateButtons ((int) (v + 0.5f));
    }

private:
    void updateButtons (int idx)
    {
        const juce::Colour green (0xff00ff41);
        const juce::Colour dark  (0xff223322);
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setColour (juce::TextButton::buttonColourId, (i == idx) ? green : dark);
    }

    juce::AudioProcessorValueTreeState* state = nullptr;
    juce::String pid;
    juce::OwnedArray<juce::TextButton> buttons;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentedSwitch)
};

class ShermanPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit ShermanPluginAudioProcessorEditor (ShermanPluginAudioProcessor&);
    ~ShermanPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ShermanPluginAudioProcessor& processorRef;

    juce::Slider freq1Slider, reso1Slider, driveSlider, envAmtSlider, decaySlider, fmAmtSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Slider freq2Slider, reso2Slider, wetSlider, bypassSlider;

    SegmentedSwitch mode1Sw, corr1Sw, mode2Sw, corr2Sw, routingSw, harmSw, oversampleSw;
    juce::OwnedArray<juce::Label> caps; // switch captions (rebuilt in resized)

    juce::Label freq1Label, reso1Label, driveLabel, envAmtLabel, decayLabel, fmAmtLabel;
    juce::Label lfoRateLabel, lfoDepthLabel;
    juce::Label freq2Label, reso2Label, wetLabel, bypassLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freq1Att, reso1Att, driveAtt, envAmtAtt, decayAtt, fmAmtAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freq2Att, reso2Att, wetAtt, bypassAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShermanPluginAudioProcessorEditor)
};
