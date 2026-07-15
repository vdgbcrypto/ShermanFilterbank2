# Sherman Filterbank 2 — Permanent Error Log (L1–L5)

Protocol: real MSVC compile + dsp_test/ numerical checks; no heap in
processBlock; per-sample smoothing; C++17. Derived from the VST3 SDK
`ShermanFilter2` prototype (raw SDK) and the tb303-juce JUCE pipeline.

### L1 — Resonance must actually modulate the filter (ported bug fix)
The `ShermanFilter2` prototype computed a resonance-dependent `q` in
`updateFilterCoefficients()` but then **overwrote it** in `processBlock()`:
`stage.q = 0.707`. So the Reso knob did nothing. Fix: `filterVoice()` derives
`rq = 0.1 + smReso * 1.15` and passes it to `processFilterStage()` — Reso now
drives the feedback coefficient (partial self-osc near 1.0). dsp_test confirms
reso=1 vs reso=0 produce different output.

### L2 — Oversampling must really upsample + decimate (ported bug fix)
`ShermanFilter2` ran an `mOversample`-stage loop but only kept the `k==0`
sample (`if (k==0) outL=fL`) — the rest were discarded, so it was a fake
oversampler (no anti-alias benefit). Fix: `processBlock()` accumulates every
sub-sample into `accF1/accF2` and **decimates by averaging** (`/ mOSFactor`).
dsp_test confirms the 4x path differs from 1x and stays bounded (no NaN/clip).

### L3 — Multimode morph must come from ONE SVF section's lp/bp/hp
First cut cascaded TWO SVF stages per filter and took the morph from `sb`
(stage 2). But stage 2's input is stage 1's lowpassed output, so its `hp`
buss cannot recover the original highs → HP morph was broken (HP ≈ 0 at high
cutoff). Sherman is **12 dB/oct per filter = a single 2-pole multimode SVF**,
so the morph must use that one section's lp/bp/hp. Fix: `filterVoice()` uses a
single `SVF mF1[2]/mF2[2]` and weights `lpW/bpW/hpW` from it. dsp_test:
mode=0 passes a 1kHz tone (LP), mode=1 attenuates it (HP).

### L4 — Correction knob: left steepens LP, right adds notch
`voice -= (-corr)*bp*0.5` (corr<0 cancels bandpass energy → steeper LP);
`voice += corr*(lp+hp-bp)` (corr>0 adds the notch buss). Center = identity.

### L5 — Harmonics locks F2 freq to F1 by musical ratio
When `harm>0`, F2 cutoff is blended toward `f1 * 2^(-harm*2)` (up to 2 oct
down) and expressed back into the 0..1 cutoff space (`mSmFreq2Eff`) that
`filterVoice()` reads. Keeps the dual-filter tracking musical, per hardware.

### L8 — Selectors are discrete toggle switches, not knobs
The Mode / Correction / Routing / Harmonics / Oversample params were
AudioParameterFloat knobs; the user wanted real toggle switches. Converted
them to AudioParameterChoice (LP/BP/HP, Steepen/Off/Notch, Serial/Mixed/
Parallel, Off/1/2/1.5/0.5, 1x/4x/8x) and built a custom SegmentedSwitch
component (mutually-exclusive TextButtons bound to the APVTS, reflecting
host/automation via a parameter listener). processBlock maps the choice
index to the DSP float: mode=idx*0.5, corr=idx-1, routing=idx*0.5,
harm -> mHarmRatio/mHarmOn (F2 = F1/ratio), oversample -> mOSFactor.

### NOTE — recurring include revert
The `#include "PluginEditor.h"` in PluginProcessor.cpp was reverted twice by
a concurrent sibling subagent during this session, which made createEditor()
fall back to GenericAudioProcessorEditor (killing the custom UI) and broke the
build (C2061 'ShermanPluginAudioProcessorEditor' undeclared). Re-added; if the
custom editor ever disappears again after a build, check this include first.
- **Input FM** (`fmAmt`): param exists and is "live" but is not yet folded into
  the cutoff in `filterVoice()`. The envelope-follower already provides
  input-coupled cutoff modulation; FM should modulate cutoff by the sample
  value. Add a per-sample FM term to the cutoff computation.
- **Clock-noise / band-limited noise layer**: the research flags SCF clock
  leakage as core to the tone; currently not emulated (clean SVF). Future phase.
- **LFO waveform**: only sine implemented; saw/triangle + AR-retrigger pending.
- **ADSR + envelope-follower MODE switch**: currently env-follower only.

### L9 — Resonance was INVERTED (raising Reso weakened it) + low-Reso distortion
The Chamberlin SVF form `hp = x - lp - q*bp` uses q as DAMPING (~1/Q): low q =
strong resonance, high q = gentle. The maps `0.1+reso*1.7` and `0.5+reso*3.0`
RAISED q with Reso, so the peak got weaker — user heard "Reso has little
effect". Fix: `rq = 2.0 - reso*1.7` (reso 0 -> q 2.0 gentle, reso 1 -> q 0.3
strong peak), clamped `q < 0.9/fc` for stability. Also fixed input gain 3.0->1.5
and soft-limited output to kill the low-Reso clipping distortion. dsp_test now
asserts peak(reso=1) > 1.3 * peak(reso=0).

### L10 — 3-way switches behaved like 2-way (middle unclickable)
SegmentedSwitch used child TextButtons; the middle button's hit area was not
reliably receiving clicks, so only extremes were selectable (LP or HP, never
BP). Fix: rewrote SegmentedSwitch as a single self-drawn component that
hit-tests by x-position (mouseDown -> idx = x/width * n), no child buttons.
LP/BP/HP, Serial/Mixed/Parallel, Off/1/2/1.5/0.5 all now selectable.

### L11 — Input FM knob (fmAmt) wired into the cutoff
The fmAmt param existed but was a no-op (dead `(void) fm` placeholder). Per the
research, the input signal is the default FM source for the filter cutoff.
Fix: filterVoice() now takes an `fm` sample arg; cutoff *= (1 + fm*mSmFM*4).
mSmFM smoothed from the fmAmt param. F1 FM'd by the input sample; in serial
routing, F2 is FM'd by the F1 output. dsp_test asserts FM changes the output
for a time-varying signal (fm_wired=YES).
NOTE: the sibling subagent reverted the header's filterVoice() declaration
(3-arg) during this session, breaking the 4-arg .cpp — re-declared with the
`double fm` param. If FM stops working after a build, check the header decl.
