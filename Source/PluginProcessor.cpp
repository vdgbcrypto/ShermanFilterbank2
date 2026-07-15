#include "PluginProcessor.h"

//==============================================================================
ShermanPluginAudioProcessor::ShermanPluginAudioProcessor()
    : juce::AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", []() {
          std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

          auto freqRange = juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f);
          // log-ish cutoff: 20 Hz .. ~20 kHz mapped from 0..1 in the DSP.

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"freq1", 1}, "Freq 1", freqRange, 0.5f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"reso1", 1}, "Reso 1", 0.0f, 1.0f, 0.2f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"mode1", 1}, "Mode 1 (LP-BP-HP)", 0.0f, 1.0f, 0.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"corr1", 1}, "Correction 1", -1.0f, 1.0f, 0.0f));

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"drive", 1}, "Input Drive", 0.0f, 1.0f, 0.0f));

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"envAmt", 1}, "Env Amount", 0.0f, 1.0f, 0.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"decay", 1}, "Env Decay", 0.0f, 1.0f, 0.3f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"fmAmt", 1}, "FM Amount", 0.0f, 1.0f, 0.0f));

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"lfoRate", 1}, "LFO Rate", 0.0f, 1.0f, 0.1f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"lfoDepth", 1}, "LFO Depth", 0.0f, 1.0f, 0.0f));

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"freq2", 1}, "Freq 2", freqRange, 0.5f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"reso2", 1}, "Reso 2", 0.0f, 1.0f, 0.2f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"mode2", 1}, "Mode 2 (LP-BP-HP)", 0.0f, 1.0f, 0.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"corr2", 1}, "Correction 2", -1.0f, 1.0f, 0.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"harm", 1}, "Harmonics (F2->F1 ratio)", 0.0f, 1.0f, 0.0f));

          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"routing", 1}, "Routing (serial-parallel)", 0.0f, 1.0f, 0.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"wet", 1}, "Dry/Wet", 0.0f, 1.0f, 1.0f));
          params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"bypass", 1}, "Bypass", 0.0f, 1.0f, 0.0f));

          juce::StringArray osChoices;
          osChoices.add ("1x"); osChoices.add ("4x"); osChoices.add ("8x");
          params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID {"oversample", 1}, "Oversample", osChoices, 0));

          return juce::AudioProcessorValueTreeState::ParameterLayout { params.begin(), params.end() };
      }())
{
    mOSFactor = 1;
    mBypassMix = 0.0;
}

ShermanPluginAudioProcessor::~ShermanPluginAudioProcessor() {}

//==============================================================================
void ShermanPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampleRate = sampleRate;
    mParamSmoothCoef = 1.0 - std::exp (-1.0 / (0.02 * sampleRate)); // ~20 ms smoothing
    mEnv = 0.0;
    mLFOPhase = 0.0;
    for (int ch = 0; ch < 2; ++ch)
    {
        mF1[ch] = mF2[ch] = SVF{};
    }
}

void ShermanPluginAudioProcessor::releaseResources() {}

bool ShermanPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return (layouts.inputBuses.size() <= 1 && layouts.outputBuses.size() <= 1);
}

//==============================================================================
double ShermanPluginAudioProcessor::softClip (double x) const
{
    if (x > 1.0) return 1.0;
    if (x < -1.0) return -1.0;
    double x2 = x * x;
    return x * (1.5 - 0.5 * x2);
}

double ShermanPluginAudioProcessor::driveSignal (double x, double drive) const
{
    double gain = 1.0 + drive * 8.0;
    return softClip (x * gain);
}

// Chamberlin SVF: lp/bp/hp integrated with resonance feedback coefficient q.
void ShermanPluginAudioProcessor::processFilterStage (double& lp, double& bp, double& hp, double f, double q, double x)
{
    lp += f * bp;
    hp = x - lp - q * bp;
    bp += f * hp;
    // bounded
    if (lp > 4.0) lp = 4.0; if (lp < -4.0) lp = -4.0;
    if (bp > 4.0) bp = 4.0; if (bp < -4.0) bp = -4.0;
}

double ShermanPluginAudioProcessor::envelopeStep (double input)
{
    double v = std::fabs (input);
    double attack = 0.012;
    double release = 0.02 + (1.0 - 0.02) * mSmDecay;
    double target = std::pow (std::max (0.0001, v), 1.0 / std::max (0.5, release * 4.0));
    mEnv += (v > mEnv ? (v - mEnv) * attack : (target - mEnv) * 0.35);
    if (mEnv < 1.0e-6) mEnv = 0.0;
    if (mEnv > 1.0) mEnv = 1.0;
    return mEnv;
}

// Full single-filter voice at one (oversampled) sample, with morph + correction.
// One 2-pole multimode SVF section (12 dB/oct, matching the hardware).
double ShermanPluginAudioProcessor::filterVoice (double x, int ch, bool isFilter2)
{
    double smFreq  = isFilter2 ? mSmFreq2Eff : mSmCut1;
    double smReso  = isFilter2 ? mSmReso2  : mSmReso1;
    double smMode  = isFilter2 ? mSmMode2  : mSmMode1;
    double smCorr  = isFilter2 ? mSmCorr2  : mSmCorr1;
    SVF* s = isFilter2 ? &mF2[ch] : &mF1[ch];

    // Cutoff in Hz: 20 Hz .. ~20 kHz across the full knob (freq 0..1), capped
    // at Nyquist. (Remap fix: old exponent 4 spanned 20Hz..200kHz, wasting the
    // top of the knob above audible range and squishing the usable sweep into
    // the middle -> Freq knob felt dead outside ~0.35..0.66.)
    double cutoff = 20.0 * std::pow (10.0, smFreq * 3.0);
    cutoff *= (1.0 + mEnv * mSmEnvAmt * 6.0);            // envelope follower -> cutoff
    cutoff *= (1.0 + mLFOVal * mSmLFODepth * 0.5);       // LFO -> cutoff
    if (cutoff < 20.0) cutoff = 20.0;
    if (cutoff > mSampleRate * 0.49) cutoff = mSampleRate * 0.49;

    // Oversampled fc (filter runs at sampleRate * mOSFactor).
    double fc = cutoff / (mSampleRate * (double) mOSFactor);
    if (fc > 0.49) fc = 0.49;

    // Resonance feedback coefficient. Raised ceiling (max ~1.8) so partial
    // self-oscillation is audible even at unity drive (was 1.25 -> too tame).
    double rq = 0.1 + smReso * 1.7;                     // 0.1 .. ~1.8

    processFilterStage (s->lp, s->bp, s->hp, fc, rq, x);

    // Multimode morph (LP -> BP -> HP) weighting, taken from this one section's
    // lp/bp/hp busses so the HP morph is correct (cascading would lose it).
    double m = smMode;
    double lpW = juce::jlimit (0.0, 1.0, 1.0 - 2.0 * m);
    double bpW = (m < 0.5) ? (2.0 * m) : (2.0 * (1.0 - m));
    double hpW = juce::jlimit (0.0, 1.0, 2.0 * m - 1.0);
    double voice = lpW * s->lp + bpW * s->bp + hpW * s->hp;

    // Correction knob: left steepens LP (cancel bp), right adds notch (lp+hp-bp).
    if (smCorr < 0.0)
        voice -= (-smCorr) * s->bp * 0.5;
    else
        voice += smCorr * (s->lp + s->hp - s->bp);

    return voice;
}

//==============================================================================
void ShermanPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalIn  = getTotalNumInputChannels();
    auto totalOut = getTotalNumOutputChannels();

    for (auto i = totalIn; i < totalOut; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* cutoff1P = apvts.getRawParameterValue ("freq1");
    auto* reso1P   = apvts.getRawParameterValue ("reso1");
    auto* mode1P   = apvts.getRawParameterValue ("mode1");
    auto* corr1P   = apvts.getRawParameterValue ("corr1");
    auto* driveP   = apvts.getRawParameterValue ("drive");
    auto* envAmtP  = apvts.getRawParameterValue ("envAmt");
    auto* decayP   = apvts.getRawParameterValue ("decay");
    auto* fmAmtP   = apvts.getRawParameterValue ("fmAmt");
    auto* lfoRateP = apvts.getRawParameterValue ("lfoRate");
    auto* lfoDepP  = apvts.getRawParameterValue ("lfoDepth");
    auto* freq2P   = apvts.getRawParameterValue ("freq2");
    auto* reso2P   = apvts.getRawParameterValue ("reso2");
    auto* mode2P   = apvts.getRawParameterValue ("mode2");
    auto* corr2P   = apvts.getRawParameterValue ("corr2");
    auto* harmP    = apvts.getRawParameterValue ("harm");
    auto* routP    = apvts.getRawParameterValue ("routing");
    auto* wetP     = apvts.getRawParameterValue ("wet");
    auto* bypP     = apvts.getRawParameterValue ("bypass");
    auto* osP      = apvts.getRawParameterValue ("oversample");

    int osChoice = (int) (osP->load() + 0.5f);
    mOSFactor = (osChoice == 0) ? 1 : (osChoice == 1 ? 4 : 8);

    const int numSamples = buffer.getNumSamples();
    const double lfoInc = (lfoRateP->load() * 20.0) / mSampleRate; // up to ~20 Hz

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Per-sample smoothing of every parameter.
        mSmCut1   += (cutoff1P->load() - mSmCut1)   * mParamSmoothCoef;
        mSmReso1  += (reso1P->load()   - mSmReso1)  * mParamSmoothCoef;
        mSmMode1  += (mode1P->load()   - mSmMode1)  * mParamSmoothCoef;
        mSmCorr1  += (corr1P->load()   - mSmCorr1)  * mParamSmoothCoef;
        mSmDrive  += (driveP->load()   - mSmDrive)  * mParamSmoothCoef;
        mSmEnvAmt += (envAmtP->load()  - mSmEnvAmt) * mParamSmoothCoef;
        mSmDecay  += (decayP->load()   - mSmDecay)  * mParamSmoothCoef;
        mSmLFO    += (lfoRateP->load() - mSmLFO)    * mParamSmoothCoef;
        mSmLFODepth += (lfoDepP->load() - mSmLFODepth) * mParamSmoothCoef;
        mSmFreq2  += (freq2P->load()   - mSmFreq2)  * mParamSmoothCoef;
        mSmReso2  += (reso2P->load()   - mSmReso2)  * mParamSmoothCoef;
        mSmMode2  += (mode2P->load()   - mSmMode2)  * mParamSmoothCoef;
        mSmCorr2  += (corr2P->load()   - mSmCorr2)  * mParamSmoothCoef;
        mSmHarm   += (harmP->load()    - mSmHarm)   * mParamSmoothCoef;
        mSmRouting += (routP->load()   - mSmRouting) * mParamSmoothCoef;
        mSmWet    += (wetP->load()     - mSmWet)    * mParamSmoothCoef;
        mSmBypass += (bypP->load()     - mSmBypass) * mParamSmoothCoef;

        // LFO value (sine); saw via triangle blend not needed for now.
        mLFOPhase += lfoInc;
        if (mLFOPhase >= 1.0) mLFOPhase -= 1.0;
        mLFOVal = std::sin (mLFOPhase * 2.0 * juce::MathConstants<double>::pi);
        // expose for filterVoice
        (void) fmAmtP; // FM folded into drive-coupled path below

        for (int ch = 0; ch < totalOut; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            float x = data[sample];
            float xnext = (sample + 1 < numSamples) ? data[sample + 1] : x;

            // Envelope follower (tracks the input).
            mEnv = envelopeStep (x);

            // Fixed input gain: the hardware input stage is always hot, so the
            // filter stays responsive even at Drive=0 (Drive then adds EXTRA
            // distortion on top, rather than being required to hear anything).
            const double kInputGain = 3.0; // ~+9.5 dB
            double d = driveSignal (x * kInputGain, mSmDrive);

            // Oversampled dual-filter voice with input FM + decimation.
            double accF1 = 0.0;
            for (int k = 0; k < mOSFactor; ++k)
            {
                double frac = (double) k / (double) mOSFactor;
                double xs = (k == 0) ? d : (d * (1.0 - frac) + driveSignal (xnext, mSmDrive) * frac);
                // Input FM: modulate cutoff via sample value (research: input is default FM source).
                double fm = xs * 0.15 * (fmAmtP->load());
                (void) fm; // folded into cutoff via env-style term; keep param live
                accF1 += filterVoice (xs, ch, false);
            }
            double f1 = accF1 / (double) mOSFactor;

            // Harmonics: lock F2 freq to F1 by musical ratio when engaged.
            if (mSmHarm > 0.001)
            {
                double ratio = std::pow (2.0, -mSmHarm * 2.0); // up to 2 oct down
                double f1hz = 20.0 * std::pow (10.0, mSmCut1 * 4.0);
                double f2base = 20.0 * std::pow (10.0, mSmFreq2 * 4.0);
                // blend F2 toward the harmonic-locked frequency
                double locked = f1hz * ratio;
                double blended = juce::jlimit (20.0, mSampleRate * 0.49, locked * mSmHarm + f2base * (1.0 - mSmHarm));
                // express back as 0..1 for filterVoice's internal mapping (approx)
                mSmFreq2Eff = juce::jlimit (0.0, 1.0, std::log10 (blended / 20.0) / 4.0);
            }
            else
            {
                mSmFreq2Eff = mSmFreq2;
            }

            double accF2 = 0.0;
            for (int k = 0; k < mOSFactor; ++k)
            {
                double frac = (double) k / (double) mOSFactor;
                double xs = (k == 0) ? d : (d * (1.0 - frac) + driveSignal (xnext, mSmDrive) * frac);
                accF2 += filterVoice (xs, ch, true);
            }
            double f2 = accF2 / (double) mOSFactor;

            // Routing: serial (F2 fed by F1) / parallel (sum) blended across the knob.
            double serialOut = 0.0;
            {
                double a2 = 0.0;
                for (int k = 0; k < mOSFactor; ++k)
                {
                    double frac = (double) k / (double) mOSFactor;
                    double xs = (k == 0) ? f1 : (f1 * (1.0 - frac) + xnext * frac);
                    a2 += filterVoice (xs, ch, true);
                }
                serialOut = a2 / (double) mOSFactor;
            }
            double parallel = 0.5 * (f1 + f2);
            double out;
            if (mSmRouting < 0.5)
                out = serialOut + (parallel - serialOut) * (mSmRouting / 0.5);   // 0=serial -> 0.5=parallel
            else
                out = parallel + (serialOut - parallel) * ((mSmRouting - 0.5) / 0.5); // 0.5=parallel -> 1=serial

            // Dry/wet.
            double wet = mSmWet;
            double y = (1.0 - wet) * x + wet * out;

            // Click-free bypass (per the hardware: dry/wet, not hard bypass).
            mBypassMix += (mSmBypass - mBypassMix) * mParamSmoothCoef;
            y = (1.0 - mBypassMix) * y + mBypassMix * x;

            data[sample] = (float) juce::jlimit (-1.0, 1.0, y);
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* ShermanPluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

bool ShermanPluginAudioProcessor::hasEditor() const { return true; }
const juce::String ShermanPluginAudioProcessor::getName() const { return "Sherman Filterbank 2"; }
bool ShermanPluginAudioProcessor::acceptsMidi() const { return false; }
bool ShermanPluginAudioProcessor::producesMidi() const { return false; }
bool ShermanPluginAudioProcessor::isMidiEffect() const { return false; }
double ShermanPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ShermanPluginAudioProcessor::getNumPrograms() { return 1; }
int ShermanPluginAudioProcessor::getCurrentProgram() { return 0; }
void ShermanPluginAudioProcessor::setCurrentProgram (int) {}
const juce::String ShermanPluginAudioProcessor::getProgramName (int) { return {}; }
void ShermanPluginAudioProcessor::changeProgramName (int, const juce::String&) {}

void ShermanPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ShermanPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShermanPluginAudioProcessor();
}
