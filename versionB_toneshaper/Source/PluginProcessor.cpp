#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr std::array<const char*, 7> parameterIds = {"band1", "band2", "band3", "band4", "mix", "master", "preset"};

    juce::StringArray getPresetNames()
    {
        juce::StringArray names;
        names.add("Clean");
        names.add("Warm");
        names.add("Bright");
        names.add("Punchy");
        names.add("Dark");
        return names;
    }

    std::array<std::array<float, 6>, 5> getPresetData()
    {
        return {{
            {{0.35f, 0.45f, 0.55f, 0.35f, 0.45f, 1.00f}},
            {{0.60f, 0.70f, 0.45f, 0.25f, 0.55f, 0.95f}},
            {{0.25f, 0.40f, 0.70f, 0.60f, 0.65f, 1.05f}},
            {{0.75f, 0.80f, 0.55f, 0.35f, 0.70f, 1.10f}},
            {{0.30f, 0.35f, 0.35f, 0.75f, 0.40f, 0.90f}}
        }};
    }
}

ShermanFilterbankAudioProcessor::ShermanFilterbankAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvtsState(*this, nullptr, "Parameters", createParameterLayout())
{
    apvtsState.addParameterListener(parameterIds[6], this);
    applyPreset(0);
}

juce::AudioProcessorValueTreeState::ParameterLayout ShermanFilterbankAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < 4; ++i)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            parameterIds[i],
            juce::String("Band ") + juce::String(i + 1),
            0.0f, 1.0f, 0.5f));
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameterIds[4], "Mix", 0.0f, 1.0f, 0.6f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(parameterIds[5], "Master", 0.7f, 1.6f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterIds[6],
        "Preset",
        getPresetNames(),
        0));

    return layout;
}

void ShermanFilterbankAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == parameterIds[6] && !isApplyingPreset)
        applyPreset(static_cast<int>(newValue));
}

void ShermanFilterbankAudioProcessor::applyPreset(int presetIndex)
{
    const auto presetValues = getPresetData();
    const auto clampedPreset = juce::jlimit(0, static_cast<int>(presetValues.size()) - 1, presetIndex);
    currentProgramIndex = clampedPreset;

    isApplyingPreset = true;

    for (int i = 0; i < 4; ++i)
    {
        if (auto* parameter = apvtsState.getParameter(parameterIds[i]))
            parameter->setValueNotifyingHost(presetValues[clampedPreset][i]);
    }

    if (auto* parameter = apvtsState.getParameter(parameterIds[4]))
        parameter->setValueNotifyingHost(presetValues[clampedPreset][4]);

    if (auto* parameter = apvtsState.getParameter(parameterIds[5]))
        parameter->setValueNotifyingHost(presetValues[clampedPreset][5]);

    if (auto* parameter = apvtsState.getParameter(parameterIds[6]))
        parameter->setValueNotifyingHost(static_cast<float>(clampedPreset));

    isApplyingPreset = false;
}

void ShermanFilterbankAudioProcessor::setCurrentProgram(int index)
{
    applyPreset(index);
}

const juce::String ShermanFilterbankAudioProcessor::getProgramName(int index)
{
    const auto names = getPresetNames();
    return names[index < names.size() ? index : 0];
}

bool ShermanFilterbankAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void ShermanFilterbankAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    for (auto& band : bandFilters)
        band.prepare(spec);

    updateFilterCoefficients();
}

void ShermanFilterbankAudioProcessor::updateFilterCoefficients()
{
    const auto sr = static_cast<double>(spec.sampleRate);

    for (int i = 0; i < 4; ++i)
    {
        if (auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(sr, bandFrequencies[i], bandQs[i]))
            bandFilters[i].setCoefficients(*coeffs);
    }
}

void ShermanFilterbankAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    if (buffer.getNumSamples() == 0)
        return;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int i = 0; i < 4; ++i)
        bandGains[i] = static_cast<float>(*apvtsState.getRawParameterValue(parameterIds[i]));

    wetMix = static_cast<float>(*apvtsState.getRawParameterValue(parameterIds[4]));
    masterGain = static_cast<float>(*apvtsState.getRawParameterValue(parameterIds[5]));

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* data = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto input = data[sample];
            float wet = 0.0f;

            for (int band = 0; band < 4; ++band)
            {
                const auto filtered = bandFilters[band].processSample(input);
                wet += filtered * bandGains[band];
            }

            const auto dryGain = 1.0f - (wetMix * 0.7f);
            const auto processed = input * dryGain + wet * (wetMix * masterGain * 0.85f);
            data[sample] = juce::jlimit(-1.0f, 1.0f, processed);
        }
    }
}

juce::AudioProcessorEditor* ShermanFilterbankAudioProcessor::createEditor()
{
    return new ShermanFilterbankAudioProcessorEditor(*this);
}

void ShermanFilterbankAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvtsState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ShermanFilterbankAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if (xmlState != nullptr)
        apvtsState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShermanFilterbankAudioProcessor();
}
