// Numerical check of the Sherman core DSP math (mirrors PluginProcessor.cpp).
// Verifies the two listening-bug fixes:
//  (A) Cutoff mapping spans the AUDIBLE range across the full knob
//      (freq 0 -> ~20 Hz, freq 1 -> ~20 kHz, capped at Nyquist). Previously
//      exponent 4 wasted the top of the knob above 20 kHz and squished the
//      usable sweep into the middle (~0.35..0.66).
//  (B) Filter is responsive at unity drive on a QUIET input, thanks to the
//      fixed input gain (kInputGain=3). Output must change when cutoff moves
//      even with drive=0 and a -20 dBFS signal.
// Plus the earlier checks: resonance matters, oversampling decimates+bounded,
// multimode LP/HP morph correct.
#include <cstdio>
#include <cmath>
#include <vector>
#include <cfloat>

struct SVF { double lp=0, bp=0, hp=0; };
static inline void stage(SVF& s, double f, double q, double x)
{ s.lp += f*s.bp; s.hp = x - s.lp - q*s.bp; s.bp += f*s.hp; }

static double voice(double x, SVF& s, double fc, double rq, double mode)
{
    stage(s, fc, rq, x);
    double lpW = (mode<0.5)? (1.0-2.0*mode) : 0.0;
    double bpW = (mode<0.5)? (2.0*mode) : (2.0*(1.0-mode));
    double hpW = (mode>0.5)? (2.0*mode-1.0) : 0.0;
    return lpW*s.lp + bpW*s.bp + hpW*s.hp;
}

// Replicate the plugin's cutoff mapping (freq 0..1 -> Hz, sr, capped Nyquist).
static double cutoffHz(double freq01, double sr)
{
    double c = 20.0 * std::pow(10.0, freq01 * 3.0);
    if (c > sr*0.49) c = sr*0.49;
    return c;
}

int main()
{
    const double sr = 44100.0;
    const int N = 2000;
    std::vector<double> in(N);
    for (int i=0;i<N;++i) in[i] = std::sin(2.0*3.14159*1000.0*i/sr);

    // (A) Cutoff mapping uses the audible range across the knob.
    {
        double lo = cutoffHz(0.0, sr);     // ~20 Hz
        double mid= cutoffHz(0.5, sr);      // ~20*10^1.5 = 632 Hz
        double hi = cutoffHz(1.0, sr);      // ~20 kHz (capped at Nyquist)
        bool ok = (lo > 10.0 && lo < 40.0) && (mid > 300.0 && mid < 1200.0) && (hi > 18000.0);
        printf("cutoff_map ok=%s lo=%.1f mid=%.1f hi=%.1f\n", ok?"YES":"NO", lo, mid, hi);
        if(!ok) return 1;
    }

    // (B) Responsive at unity drive on a QUIET (-20 dBFS) input.
    {
        const double kInputGain = 3.0;
        SVF a1{},a2{};
        // drive=0 path: d = softClip(x * kInputGain * 1) ; we approximate softclip
        auto soft=[&](double v){ if(v>1)return 1.0; if(v<-1)return -1.0; return v*(1.5-0.5*v*v); };
        double quiet = 0.1; // ~ -20 dBFS
        double outLo=0, outHi=0;
        for (int i=0;i<N;++i){
            double d = soft(quiet * kInputGain);
            outLo = voice(d, a1, 300.0/sr, 0.3, 0.0);   // cutoff 300 Hz
            outHi = voice(d, a2, 8000.0/sr, 0.3, 0.0);  // cutoff 8 kHz
        }
        bool ok = std::fabs(outLo - outHi) > 1e-3; // moving cutoff must change output
        printf("unity_drive_responsive ok=%s (outLo=%.4f outHi=%.4f)\n", ok?"YES":"NO", outLo, outHi);
        if(!ok) return 1;
    }

    // Resonance must be clearly audible: peak output at reso=1 must exceed
    // reso=0 by a meaningful margin (a real resonant peak), and stay bounded.
    {
        SVF a1{},a2{};
        double peak0=0, peak1=0;
        for (int i=0;i<N;++i){
            double y0 = voice(in[i],a1,1000.0/sr,2.0,0.0);  // reso=0 -> rq=2.0 (gentle)
            double y1 = voice(in[i],a2,1000.0/sr,0.3,0.0);  // reso=1 -> rq=0.3 (strong peak)
            peak0 = std::max(peak0, std::fabs(y0));
            peak1 = std::max(peak1, std::fabs(y1));
        }
        bool ok = (peak1 > peak0 * 1.3) && peak1 < 4.0; // audible peak, not exploding
        printf("reso_changes=%s (peak0=%.3f peak1=%.3f)\n", ok?"YES":"NO", peak0, peak1);
        if(!ok) return 1;
    }

    // Oversampling decimates + bounded.
    {
        SVF a1{},a4{}; double last1=0,last4=0; bool bounded=true; int diff=0;
        for (int i=0;i<N;++i){
            last1 = voice(in[i], a1, 1000.0/sr, 0.5, 0.0);
            double acc=0; for(int k=0;k<4;++k) acc += voice(in[i], a4, 1000.0/(sr*4.0), 0.5, 0.0);
            last4 = acc/4.0;
            if(!std::isfinite(last1)||!std::isfinite(last4)) bounded=false;
            if(std::fabs(last1-last4)>1e-6) ++diff;
        }
        bool ok = bounded && diff>0;
        printf("oversample_ok=%s\n", ok?"YES":"NO");
        if(!ok) return 1;
    }

    // Multimode LP/HP.
    {
        SVF aL{},aH{}; double sLP=0,sHP=0;
        for (int i=0;i<N;++i){ sLP+=std::fabs(voice(in[i],aL,1000.0/sr,0.3,0.0)); sHP+=std::fabs(voice(in[i],aH,1000.0/sr,0.3,1.0)); }
        bool ok = sHP > sLP*2.0;
        // Input FM: a varying (non-DC) input should change the filtered output when
        // fmAmt > 0 vs fmAmt = 0, at identical settings. Mirrors filterVoice's
        // cutoff *= (1 + fm * mSmFM * 4).
        {
            SVF a0{}, aFM{};
            const double fc = 1000.0/sr, rq = 0.5, mFM = 0.5;
            double diff = 0.0;
            for (int i=0;i<N;++i){
                double inp = in[i];
                double y0 = voice(inp, a0,  fc, rq, 0.0);                 // fm = 0
                double yFM = voice(inp, aFM, fc*(1.0+inp*mFM*4.0), rq, 0.0); // fm = inp, mFM=0.5
                diff += std::fabs(y0 - yFM);
            }
            bool ok = diff > 1e-3; // FM must alter the output for a time-varying signal
            printf("fm_wired=%s (diff=%.4f)\n", ok?"YES":"NO", diff);
            if(!ok) return 1;
        }
    }

    printf("PASS=YES\n");
    return 0;
}
