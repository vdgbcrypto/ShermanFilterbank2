#pragma once

#include <JuceHeader.h>

class ShermanFilterbankAudioProcessor : public juce::AudioProcessor,
                                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    ShermanFilterbankAudioProcessor();
    ~ShermanFilterbankAudioProcessor() override = default;

    const juce::String getName() const override { return "Sherman Filterbank"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 5; }
    int getCurrentProgram() override { return currentProgramIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& apvts() { return apvtsState; }

private:
    struct BandFilter
    {
        juce::dsp::IIR::Filter<float> filter;
        void prepare(const juce::dsp::ProcessSpec& spec) { filter.prepare(spec); }
        void setCoefficients(const juce::dsp::IIR::Coefficients<float>& coeffs) { filter.setCoefficients(coeffs); }
        float processSample(float sample) { return filter.processSample(sample); }
    };

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void applyPreset(int presetIndex);
    void updateFilterCoefficients();

    juce::AudioProcessorValueTreeState apvtsState;
    juce::dsp::ProcessSpec spec;
    std::array<BandFilter, 4> bandFilters;
    std::array<float, 4> bandGains;
    std::array<double, 4> bandFrequencies { 120.0, 360.0, 1700.0, 6200.0 };
    std::array<double, 4> bandQs { 0.70, 0.95, 1.10, 1.30 };
    float wetMix = 0.72f;
    float masterGain = 1.0f;
    int currentProgramIndex = 0;
    bool isApplyingPreset = false;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
};
