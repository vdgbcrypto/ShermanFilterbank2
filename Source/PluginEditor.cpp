#include "PluginEditor.h"

ShermanPluginAudioProcessorEditor::ShermanPluginAudioProcessorEditor (ShermanPluginAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    const juce::Colour green (0xff00ff41);
    auto make = [&](juce::Slider& s, juce::Label& l, const juce::String& name, bool rot)
    {
        s.setSliderStyle (rot ? juce::Slider::RotaryVerticalDrag : juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 46, 14);
        s.setRange (0.0, 1.0, 0.001);
        s.setColour (juce::Slider::rotarySliderFillColourId, green);
        s.setColour (juce::Slider::textBoxTextColourId, green);
        addAndMakeVisible (s);
        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, green);
        l.setFont (juce::Font (11.0f, juce::Font::bold));
        addAndMakeVisible (l);
    };

    make (freq1Slider, freq1Label, "FREQ 1", true);
    make (reso1Slider, reso1Label, "RESO 1", true);
    make (mode1Slider, mode1Label, "MODE 1", true);
    make (corr1Slider, corr1Label, "CORR 1", true);
    make (driveSlider, driveLabel, "DRIVE", true);
    make (envAmtSlider, envAmtLabel, "ENV", true);
    make (decaySlider, decayLabel, "DECAY", true);
    make (fmAmtSlider, fmAmtLabel, "FM", true);
    make (lfoRateSlider, lfoRateLabel, "LFO RATE", true);
    make (lfoDepthSlider, lfoDepthLabel, "LFO DEP", true);
    make (freq2Slider, freq2Label, "FREQ 2", true);
    make (reso2Slider, reso2Label, "RESO 2", true);
    make (mode2Slider, mode2Label, "MODE 2", true);
    make (corr2Slider, corr2Label, "CORR 2", true);
    make (harmSlider, harmLabel, "HARM", true);
    make (routingSlider, routingLabel, "ROUTING", true);
    make (wetSlider, wetLabel, "DRY/WET", true);
    make (bypassSlider, bypassLabel, "BYPASS", true);

    oversampleCombo.addItem ("1x", 1);
    oversampleCombo.addItem ("4x", 2);
    oversampleCombo.addItem ("8x", 3);
    oversampleCombo.setSelectedItemIndex (0, juce::dontSendNotification);
    addAndMakeVisible (oversampleCombo);
    oversampleLabel.setText ("OVERSAMPLE", juce::dontSendNotification);
    oversampleLabel.setJustificationType (juce::Justification::centred);
    oversampleLabel.setColour (juce::Label::textColourId, green);
    oversampleLabel.setFont (juce::Font (11.0f, juce::Font::bold));
    addAndMakeVisible (oversampleLabel);

    freq1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "freq1", freq1Slider);
    reso1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "reso1", reso1Slider);
    mode1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "mode1", mode1Slider);
    corr1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "corr1", corr1Slider);
    driveAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "drive", driveSlider);
    envAmtAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "envAmt", envAmtSlider);
    decayAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "decay", decaySlider);
    fmAmtAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "fmAmt", fmAmtSlider);
    lfoRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "lfoRate", lfoRateSlider);
    lfoDepthAtt= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "lfoDepth", lfoDepthSlider);
    freq2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "freq2", freq2Slider);
    reso2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "reso2", reso2Slider);
    mode2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "mode2", mode2Slider);
    corr2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "corr2", corr2Slider);
    harmAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "harm", harmSlider);
    routingAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "routing", routingSlider);
    wetAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "wet", wetSlider);
    bypassAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "bypass", bypassSlider);
    oversampleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "oversample", oversampleCombo);

    setSize (880, 280);
}

ShermanPluginAudioProcessorEditor::~ShermanPluginAudioProcessorEditor() {}

void ShermanPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a1a));
    g.setColour (juce::Colour (0xff00ff41));
    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.drawText ("Sherman Filterbank 2", getLocalBounds().removeFromTop (24), juce::Justification::centred, true);
    g.setColour (juce::Colour (0xff557755));
    g.drawRect (getLocalBounds().reduced (4).removeFromTop (24), 1);
}

void ShermanPluginAudioProcessorEditor::resized()
{
    auto b = getLocalBounds().reduced (12);
    b.removeFromTop (30);

    // Two rows of rotary knobs (Filter 1 block + Drive/Mod, then Filter 2 block).
    const int knob = 52, gap = 8, labelH = 16;
    int x = b.getX();
    int yTop = b.getY();
    int yBot = yTop + knob + labelH + 10;

    struct KnobRow { juce::Slider* s; juce::Label* l; };
    KnobRow row1[] = {
        { &freq1Slider, &freq1Label }, { &reso1Slider, &reso1Label }, { &mode1Slider, &mode1Label },
        { &corr1Slider, &corr1Label }, { &driveSlider, &driveLabel }, { &envAmtSlider, &envAmtLabel },
        { &decaySlider, &decayLabel }, { &fmAmtSlider, &fmAmtLabel }, { &lfoRateSlider, &lfoRateLabel },
        { &lfoDepthSlider, &lfoDepthLabel } };
    for (auto& k : row1)
    {
        k.s->setBounds (x, yTop, knob, knob);
        k.l->setBounds (x, yTop + knob, knob, labelH);
        x += knob + gap;
    }
    x = b.getX();
    KnobRow row2[] = {
        { &freq2Slider, &freq2Label }, { &reso2Slider, &reso2Label }, { &mode2Slider, &mode2Label },
        { &corr2Slider, &corr2Label }, { &harmSlider, &harmLabel }, { &routingSlider, &routingLabel },
        { &wetSlider, &wetLabel }, { &bypassSlider, &bypassLabel } };
    for (auto& k : row2)
    {
        k.s->setBounds (x, yBot, knob, knob);
        k.l->setBounds (x, yBot + knob, knob, labelH);
        x += knob + gap;
    }

    // Oversample dropdown at far right of bottom row.
    oversampleCombo.setBounds (b.getRight() - 90, yBot, 80, 24);
    oversampleLabel.setBounds (b.getRight() - 90, yBot + 26, 80, labelH);
}
