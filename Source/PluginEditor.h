#pragma once

#include "PluginProcessor.h"

// A discrete toggle-switch control: a single component that draws N segments
// and hit-tests by x-position (no child buttons -> no overlap/click ambiguity).
// Bound directly to an AudioParameterChoice; reflects host/automation via the
// APVTS listener. Used for all the Sherman "selector" params.
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
        labels = items;
        state->addParameterListener (pid, this);
        setIndex ((int) (state->getRawParameterValue (pid)->load() + 0.5f));
    }

    ~SegmentedSwitch() override { if (state) state->removeParameterListener (pid, this); }

    void paint (juce::Graphics& g) override
    {
        const juce::Colour green (0xff00ff41);
        const juce::Colour dark  (0xff223322);
        const juce::Colour outline (0xff557755);
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (outline);
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
        int n = labels.size();
        float w = r.getWidth() / (float) n;
        for (int i = 0; i < n; ++i)
        {
            juce::Rectangle<float> seg (r.getX() + i * w, r.getY(), w, r.getHeight());
            g.setColour (i == current ? green : dark);
            g.fillRoundedRectangle (seg.reduced (1.0f), 3.0f);
            g.setColour (i == current ? juce::Colours::black : green);
            g.setFont (juce::Font (10.0f, juce::Font::bold));
            g.drawText (labels[i], seg, juce::Justification::centred, true);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        int n = juce::jmax (1, labels.size());
        int idx = (int) ((e.x / (float) getWidth()) * (float) n);
        idx = juce::jlimit (0, n - 1, idx);
        setIndex (idx);
        state->getParameter (pid)->setValueNotifyingHost ((float) idx);
    }

    void parameterChanged (const juce::String&, float v) override
    {
        setIndex ((int) (v + 0.5f));
    }

private:
    void setIndex (int i) { if (i != current) { current = i; repaint(); } }

    juce::AudioProcessorValueTreeState* state = nullptr;
    juce::String pid;
    juce::StringArray labels;
    int current = 0;
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
