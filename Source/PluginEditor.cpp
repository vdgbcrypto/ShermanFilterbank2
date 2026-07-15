#include "PluginEditor.h"

ShermanPluginAudioProcessorEditor::ShermanPluginAudioProcessorEditor (ShermanPluginAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    const juce::Colour green (0xff00ff41);

    // Continuous knobs.
    auto makeKnob = [&](juce::Slider& s, juce::Label& l, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
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

    makeKnob (freq1Slider, freq1Label, "FREQ 1");
    makeKnob (reso1Slider, reso1Label, "RESO 1");
    makeKnob (driveSlider, driveLabel, "DRIVE");
    makeKnob (envAmtSlider, envAmtLabel, "ENV");
    makeKnob (decaySlider, decayLabel, "DECAY");
    makeKnob (fmAmtSlider, fmAmtLabel, "FM");
    makeKnob (lfoRateSlider, lfoRateLabel, "LFO RATE");
    makeKnob (lfoDepthSlider, lfoDepthLabel, "LFO DEP");
    makeKnob (freq2Slider, freq2Label, "FREQ 2");
    makeKnob (reso2Slider, reso2Label, "RESO 2");
    makeKnob (wetSlider, wetLabel, "DRY/WET");
    makeKnob (bypassSlider, bypassLabel, "BYPASS");

    // Selector switches (discrete toggles).
    addAndMakeVisible (mode1Sw);
    addAndMakeVisible (corr1Sw);
    addAndMakeVisible (mode2Sw);
    addAndMakeVisible (corr2Sw);
    addAndMakeVisible (routingSw);
    addAndMakeVisible (harmSw);
    addAndMakeVisible (oversampleSw);

    mode1Sw.init     (p.apvts, "mode1",     juce::StringArray ("LP", "BP", "HP"));
    corr1Sw.init     (p.apvts, "corr1",     juce::StringArray ("Steepen", "Off", "Notch"));
    mode2Sw.init     (p.apvts, "mode2",     juce::StringArray ("LP", "BP", "HP"));
    corr2Sw.init     (p.apvts, "corr2",     juce::StringArray ("Steepen", "Off", "Notch"));
    routingSw.init   (p.apvts, "routing",   juce::StringArray ("Serial", "Mixed", "Parallel"));
    harmSw.init      (p.apvts, "harm",      juce::StringArray ("Off", "1", "2", "1.5", "0.5"));
    oversampleSw.init(p.apvts, "oversample", juce::StringArray ("1x", "4x", "8x"));

    freq1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "freq1", freq1Slider);
    reso1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "reso1", reso1Slider);
    driveAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "drive", driveSlider);
    envAmtAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "envAmt", envAmtSlider);
    decayAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "decay", decaySlider);
    fmAmtAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "fmAmt", fmAmtSlider);
    lfoRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "lfoRate", lfoRateSlider);
    lfoDepthAtt= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "lfoDepth", lfoDepthSlider);
    freq2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "freq2", freq2Slider);
    reso2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "reso2", reso2Slider);
    wetAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "wet", wetSlider);
    bypassAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "bypass", bypassSlider);

    // The switch labels are drawn as small captions above each switch.
    setSize (900, 360);
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

    const int knob = 52, gap = 10, labelH = 16;
    const juce::Colour green (0xff00ff41);

    // Knob row (continuous params).
    int x = b.getX();
    int yTop = b.getY();
    struct KnobRow { juce::Slider* s; juce::Label* l; };
    KnobRow knobs[] = {
        { &freq1Slider, &freq1Label }, { &reso1Slider, &reso1Label }, { &driveSlider, &driveLabel },
        { &envAmtSlider, &envAmtLabel }, { &decaySlider, &decayLabel }, { &fmAmtSlider, &fmAmtLabel },
        { &lfoRateSlider, &lfoRateLabel }, { &lfoDepthSlider, &lfoDepthLabel },
        { &freq2Slider, &freq2Label }, { &reso2Slider, &reso2Label }, { &wetSlider, &wetLabel }, { &bypassSlider, &bypassLabel } };
    for (auto& k : knobs)
    {
        k.s->setBounds (x, yTop, knob, knob);
        k.l->setBounds (x, yTop + knob, knob, labelH);
        x += knob + gap;
    }

    // Switch rows (selectors). Each switch is a labelled segment row.
    caps.clear();
    int y = yTop + knob + labelH + 24;
    struct SwRow { SegmentedSwitch* sw; juce::String cap; int n; };
    SwRow rows[] = {
        { &mode1Sw, "MODE 1", 3 }, { &corr1Sw, "CORR 1", 3 },
        { &mode2Sw, "MODE 2", 3 }, { &corr2Sw, "CORR 2", 3 },
        { &routingSw, "ROUTING", 3 }, { &harmSw, "HARM", 5 }, { &oversampleSw, "OVERSAMPLE", 3 } };

    int swW = 150, swH = 24, swGap = 16;
    x = b.getX();
    for (auto& r : rows)
    {
        if (x + swW > b.getRight() - swW) { x = b.getX(); y += swH + 22; }
        auto* cap = caps.add (new juce::Label());
        cap->setText (r.cap, juce::dontSendNotification);
        cap->setJustificationType (juce::Justification::centred);
        cap->setColour (juce::Label::textColourId, green);
        cap->setFont (juce::Font (10.0f, juce::Font::bold));
        cap->setBounds (x, y, swW, 14);
        addAndMakeVisible (cap);
        r.sw->setBounds (x, y + 14, swW, swH);
        x += swW + swGap;
    }
}
