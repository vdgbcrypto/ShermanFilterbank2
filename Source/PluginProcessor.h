#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Sherman Filterbank 2 — JUCE VST3.
// Two 12 dB/oct switched-capacitor-style multimode filters (LTC1060 modelled as
// cascaded SVF stages) + input overdrive + modulation matrix. Authored under the
// Hermes VST Architect protocol: real MSVC compile + dsp_test/ numerical checks;
// no heap allocation inside processBlock; per-sample smoothing; C++17.

struct SVF
{
    double lp = 0.0, bp = 0.0, hp = 0.0;
};

class ShermanPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    ShermanPluginAudioProcessor();
    ~ShermanPluginAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    // Oversampling factor (set from the "oversample" param: 1, 4, 8).
    int mOSFactor = 1;
    double mSampleRate = 44100.0;
    double mParamSmoothCoef = 0.0;

    // Per-sample smoothed parameters.
    double mSmCut1 = 0.5, mSmReso1 = 0.0, mSmMode1 = 0.0, mSmCorr1 = 0.0, mSmDrive = 0.0;
    double mSmEnvAmt = 0.0, mSmDecay = 0.3, mSmLFO = 0.0, mSmLFODepth = 0.0;
    double mSmFreq2 = 0.5, mSmReso2 = 0.0, mSmMode2 = 0.0, mSmCorr2 = 0.0;
    double mHarmRatio = 1.0; bool mHarmOn = false; // Harmonics: F2 = F1 / ratio when on
    double mSmRouting = 0.0, mSmWet = 1.0, mSmBypass = 0.0;

    // Filter state (one 2-pole SVF section per filter, 2 channels).
    // Sherman is 12 dB/oct per filter, so a single multimode SVF section is the
    // correct topology (cascading would break the HP morph, see ERROR_LOG).
    SVF mF1[2], mF2[2];

    // Modulation sources.
    double mEnv = 0.0;       // envelope follower (0..1)
    double mBypassMix = 0.0; // smoothed dry/wet for click-free bypass
    double mLFOPhase = 0.0;  // 0..1
    double mLFOVal = 0.0;    // current LFO output (-1..1)
    double mSmFreq2Eff = 0.5;// effective F2 cutoff (0..1) after Harmonics lock

    // Internal helper DSP (no heap, all stack/double).
    double envelopeStep (double input);
    double softClip (double x) const;
    double driveSignal (double x, double drive) const;
    void processFilterStage (double& lp, double& bp, double& hp, double f, double q, double x);
    double filterVoice (double x, int ch, bool isFilter2);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShermanPluginAudioProcessor)
};
