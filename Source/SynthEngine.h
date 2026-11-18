// SawDeck - supersaw synth engine
// Plain C++ (no JUCE dependency) so it can be tested on its own.
//
// Supersaw model follows Adam Szabo's analysis of the Roland JP-8000
// ("How to Emulate the Super Saw", 2010):
//   * 7 free-running sawtooths with fixed relative detune offsets
//   * a non-linear DETUNE curve (11th-order polynomial)
//   * a MIX curve that fades the centre saw down while the side saws come up
//   * a high-pass filter that tracks the played pitch (cleans the low end)
// For saw counts other than 7 the same curves are used with a generated
// offset table that matches the JP spacing.

#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <random>

namespace sawdeck
{
constexpr int kMaxSaws   = 16;
constexpr int kMaxVoices = 16;
constexpr int kCtrl      = 16;   // control-rate interval in samples
constexpr double kPi     = 3.14159265358979323846;

struct Params
{
    int   saws = 7;
    float detune = 0.35f, mix = 0.6f, spread = 0.75f;
    bool  randomPhase = true, jpHpf = true;
    int   octave = 0;
    float fineCents = 0.0f, sub = 0.0f;

    int   filterType = 0;          // 0 LP, 1 BP, 2 HP
    bool  slope24 = false;
    float cutoff = 20000.0f, reso = 0.1f, envAmt = 0.0f, keyTrack = 0.0f;
    float fA = 0.005f, fD = 0.5f, fS = 0.5f, fR = 0.3f;
    float aA = 0.005f, aD = 0.4f, aS = 1.0f, aR = 0.35f;
    float velSens = 0.3f;

    float glide = 0.0f;
    int   polyphony = 8;
    float bendRange = 2.0f;
};

// ---------- JP-8000 curves (Szabo) ----------
inline double jpDetuneCurve (double x)
{
    return 10028.7312891634 * std::pow (x, 11) - 50818.8652045924 * std::pow (x, 10)
         + 111363.4808729368 * std::pow (x, 9) - 138150.6761080548 * std::pow (x, 8)
         + 106649.6679158292 * std::pow (x, 7) - 53046.9642751875 * std::pow (x, 6)
         + 17019.9518580080 * std::pow (x, 5) - 3425.0836591318 * std::pow (x, 4)
         + 404.2703938388 * std::pow (x, 3) - 24.1878824391 * x * x
         + 0.6717417634 * x + 0.0030115596;
}
inline double jpCentreGain (double m) { return -0.55366 * m + 0.99785; }
inline double jpSideGain   (double m) { return -0.73764 * m * m + 1.2841 * m + 0.044372; }

inline float polyBlep (double t, double dt)
{
    if (t < dt)       { t /= dt;             return (float) (t + t - t * t - 1.0); }
    if (t > 1.0 - dt) { t = (t - 1.0) / dt;  return (float) (t * t + t + t + 1.0); }
    return 0.0f;
}

// ---------- Envelope ----------
struct Envelope
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    float level = 0.0f, attackInc = 1.0f, decayCoef = 0.0f, releaseCoef = 0.0f, sustain = 1.0f;

    void set (float a, float d, float s, float r, double sr)
    {
        attackInc   = 1.0f / (float) std::max (1.0, a * sr);
        decayCoef   = (float) std::exp (-5.0 / std::max (1.0, d * sr));
        releaseCoef = (float) std::exp (-5.0 / std::max (1.0, r * sr));
        sustain = s;
    }
    void noteOn()  { stage = Attack; }
    void noteOff() { if (stage != Idle) stage = Release; }
    void reset()   { stage = Idle; level = 0.0f; }
    bool active() const { return stage != Idle; }

    inline float next()
    {
        switch (stage)
        {
            case Attack:
                level += attackInc;
                if (level >= 1.0f) { level = 1.0f; stage = Decay; }
                break;
            case Decay:
                level = sustain + (level - sustain) * decayCoef;
                if (level - sustain < 1.0e-4f) stage = Sustain;
                break;
            case Sustain:
                level += (sustain - level) * 0.002f;   // follows live sustain changes smoothly
                break;
            case Release:
                level *= releaseCoef;
                if (level < 1.0e-5f) { level = 0.0f; stage = Idle; }
                break;
            case Idle: break;
        }
        return level;
    }
};

// ---------- TPT state-variable filter (Cytomic / Simper) ----------
struct SVF
{
    float ic1 = 0.0f, ic2 = 0.0f;
    void reset() { ic1 = ic2 = 0.0f; }
    inline void process (float v0, float a1, float a2, float a3, float k, float& lp, float& bp, float& hp)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = v0 - k * v1 - v2;
    }
};

struct FilterCoefs { float a1 = 0, a2 = 0, a3 = 0, k = 2; };

inline FilterCoefs makeCoefs (double fc, double sr, float k)
{
    FilterCoefs c;
    const double g = std::tan (kPi * fc / sr);
    c.k  = k;
    c.a1 = (float) (1.0 / (1.0 + g * (g + k)));
    c.a2 = (float) (g * c.a1);
    c.a3 = (float) (g * c.a2);
    return c;
}

// ---------- Voice ----------
struct Voice
{
    int      note = -1;
    bool     gate = false, sustained = false;
    float    velocity = 1.0f;
    uint64_t age = 0;
    double   currentNote = 60.0, targetNote = 60.0;

    double phase[kMaxSaws] {};
    double inc[kMaxSaws] {};
    double subPhase = 0.0, subInc = 0.0;
    float  hpA = 0.0f, hpY[2] {}, hpX[2] {};

    Envelope ampEnv, filtEnv;
    SVF f1[2], f2[2];
    FilterCoefs c1, c2;
    int ctrlCounter = 0;

    bool active() const { return ampEnv.active(); }
};

// ---------- Engine ----------
class Engine
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) { v = Voice(); }
        heldStack.clear();
        heldStack.reserve (128);
        rebuildLayout();
    }

    void setParams (const Params& p)
    {
        const bool layoutChanged = p.saws != params.saws || p.detune != params.detune
                                || p.mix != params.mix || p.spread != params.spread;
        const bool modeChanged = (p.polyphony == 1) != (params.polyphony == 1);
        params = p;
        if (layoutChanged) rebuildLayout();
        if (modeChanged) heldStack.clear();
        for (auto& v : voices)
        {
            v.ampEnv.set  (p.aA, p.aD, p.aS, p.aR, sampleRate);
            v.filtEnv.set (p.fA, p.fD, p.fS, p.fR, sampleRate);
        }
    }

    // ---- MIDI ----
    void noteOn (int note, float vel)
    {
        if (params.polyphony <= 1) { monoNoteOn (note, vel); lastNote = note; return; }

        const int n = std::clamp (params.polyphony, 1, kMaxVoices);
        Voice* v = nullptr;
        for (int i = 0; i < n && v == nullptr; ++i)
            if (voices[i].note == note && voices[i].active()) v = &voices[i];
        for (int i = 0; i < n && v == nullptr; ++i)
            if (! voices[i].active()) v = &voices[i];
        if (v == nullptr)   // steal: oldest released voice first, then oldest overall
        {
            Voice* oldestReleased = nullptr; Voice* oldest = nullptr;
            for (int i = 0; i < n; ++i)
            {
                Voice& c = voices[i];
                if (! c.gate && ! c.sustained && (oldestReleased == nullptr || c.age < oldestReleased->age)) oldestReleased = &c;
                if (oldest == nullptr || c.age < oldest->age) oldest = &c;
            }
            v = oldestReleased ? oldestReleased : oldest;
        }
        startVoice (*v, note, vel);
        lastNote = note;
    }

    void noteOff (int note)
    {
        if (params.polyphony <= 1) { monoNoteOff (note); return; }
        for (auto& v : voices)
            if (v.note == note && v.gate)
            {
                v.gate = false;
                if (sustainDown) v.sustained = true;
                else releaseVoice (v);
            }
    }

    void sustainPedal (bool down)
    {
        sustainDown = down;
        if (! down)
            for (auto& v : voices)
                if (v.sustained) { v.sustained = false; releaseVoice (v); }
    }

    void pitchBend (int value14) { bendSemis = (value14 - 8192) / 8192.0 * params.bendRange; }

    void allNotesOff()
    {
        heldStack.clear();
        for (auto& v : voices) { v.gate = false; v.sustained = false; releaseVoice (v); }
    }

    int activeVoiceCount() const
    {
        int c = 0;
        for (auto& v : voices) if (v.active()) ++c;
        return c;
    }

    // Adds into L/R from sample `start` for `num` samples.
    void render (float* L, float* R, int start, int num)
    {
        const int   n      = std::clamp (params.saws, 1, kMaxSaws);
        const bool  hpOn   = params.jpHpf && n > 1;
        const float subLvl = params.sub * 0.9f;
        const int   ftype  = params.filterType;
        const bool  two    = params.slope24;
        const float velS   = params.velSens;

        for (auto& v : voices)
        {
            if (! v.active()) continue;
            const float velGain = 1.0f - velS + velS * v.velocity;

            for (int s = start; s < start + num; ++s)
            {
                if (--v.ctrlCounter <= 0) { updateControl (v, n); v.ctrlCounter = kCtrl; }

                const float env  = v.ampEnv.next();
                v.filtEnv.next();

                float sL = 0.0f, sR = 0.0f;
                for (int i = 0; i < n; ++i)
                {
                    double ph = v.phase[i];
                    const double dt = v.inc[i];
                    const float saw = (float) (2.0 * ph - 1.0) - polyBlep (ph, dt);
                    ph += dt; if (ph >= 1.0) ph -= 1.0;
                    v.phase[i] = ph;
                    sL += saw * gainL[i];
                    sR += saw * gainR[i];
                }

                if (hpOn)
                {
                    const float yL = v.hpA * (v.hpY[0] + sL - v.hpX[0]);
                    const float yR = v.hpA * (v.hpY[1] + sR - v.hpX[1]);
                    v.hpX[0] = sL; v.hpX[1] = sR; v.hpY[0] = yL; v.hpY[1] = yR;
                    sL = yL; sR = yR;
                }

                if (subLvl > 0.0f)
                {
                    const float sub = subLvl * (float) std::sin (2.0 * kPi * v.subPhase);
                    sL += sub; sR += sub;
                }
                v.subPhase += v.subInc; if (v.subPhase >= 1.0) v.subPhase -= 1.0;

                float in[2] = { sL, sR };
                for (int ch = 0; ch < 2; ++ch)
                {
                    float lp, bp, hp;
                    v.f1[ch].process (in[ch], v.c1.a1, v.c1.a2, v.c1.a3, v.c1.k, lp, bp, hp);
                    float x = ftype == 0 ? lp : ftype == 1 ? bp * v.c1.k : hp;
                    if (two)
                    {
                        v.f2[ch].process (x, v.c2.a1, v.c2.a2, v.c2.a3, v.c2.k, lp, bp, hp);
                        x = ftype == 0 ? lp : ftype == 1 ? bp * v.c2.k : hp;
                    }
                    in[ch] = x;
                }

                const float g = env * velGain * 0.14f;
                L[s] += in[0] * g;
                R[s] += in[1] * g;

                if (! v.ampEnv.active()) { v.note = -1; break; }
            }
        }
    }

    // Exposed for the GUI display
    int   layoutCount() const { return layoutN; }
    float layoutOffset (int i) const { return (float) offsets[i]; }
    float layoutGain   (int i) const { return gains[i]; }
    float layoutPan    (int i) const { return pans[i]; }
    double detuneAmount() const { return detAmt; }

private:
    void startVoice (Voice& v, int note, float vel)
    {
        const bool wasActive = v.active();
        v.note = note; v.gate = true; v.sustained = false;
        v.velocity = vel; v.age = ++ageCounter;
        v.targetNote  = note;
        v.currentNote = (params.glide > 0.0f && lastNote >= 0) ? (double) lastNote : (double) note;

        if (! wasActive)
        {
            for (int i = 0; i < kMaxSaws; ++i)
                v.phase[i] = params.randomPhase ? uni (rng) : std::fmod (0.1 + i * 0.6180339887, 1.0);
            v.subPhase = 0.0;
            v.hpY[0] = v.hpY[1] = v.hpX[0] = v.hpX[1] = 0.0f;
            for (int ch = 0; ch < 2; ++ch) { v.f1[ch].reset(); v.f2[ch].reset(); }
            v.ampEnv.reset(); v.filtEnv.reset();
        }
        v.ampEnv.noteOn();
        v.filtEnv.noteOn();
        v.ctrlCounter = 0;
    }

    void releaseVoice (Voice& v) { v.ampEnv.noteOff(); v.filtEnv.noteOff(); }

    void monoNoteOn (int note, float vel)
    {
        heldStack.erase (std::remove (heldStack.begin(), heldStack.end(), note), heldStack.end());
        heldStack.push_back (note);
        Voice& v = voices[0];
        for (int i = 1; i < kMaxVoices; ++i) releaseVoice (voices[i]);

        if (v.active() && (v.gate || v.sustained))
        {   // legato: no retrigger, just move pitch
            v.note = note; v.gate = true; v.sustained = false; v.targetNote = note;
            if (params.glide <= 0.0f) v.currentNote = note;
        }
        else
            startVoice (v, note, vel);
    }

    void monoNoteOff (int note)
    {
        heldStack.erase (std::remove (heldStack.begin(), heldStack.end(), note), heldStack.end());
        Voice& v = voices[0];
        if (v.note != note || ! v.gate) return;
        if (! heldStack.empty())
        {
            const int back = heldStack.back();
            v.note = back; v.targetNote = back;
            if (params.glide <= 0.0f) v.currentNote = back;
            lastNote = back;
        }
        else
        {
            v.gate = false;
            if (sustainDown) v.sustained = true; else releaseVoice (v);
        }
    }

    void updateControl (Voice& v, int n)
    {
        // glide (exponential, in semitones)
        if (params.glide > 0.0f)
        {
            const double tau  = std::max (0.0005, (double) params.glide * 0.3);
            const double coef = std::exp (-(double) kCtrl / (tau * sampleRate));
            v.currentNote = v.targetNote + (v.currentNote - v.targetNote) * coef;
            if (std::abs (v.currentNote - v.targetNote) < 1.0e-4) v.currentNote = v.targetNote;
        }
        else v.currentNote = v.targetNote;

        const double noteNum = v.currentNote + params.octave * 12.0 + params.fineCents / 100.0 + bendSemis;
        const double f0 = 440.0 * std::pow (2.0, (noteNum - 69.0) / 12.0);
        const double maxInc = 0.45;
        for (int i = 0; i < n; ++i)
            v.inc[i] = std::min (maxInc, f0 * (1.0 + offsets[i] * detAmt) / sampleRate);
        v.subInc = std::min (maxInc, 0.5 * f0 / sampleRate);
        v.hpA = (float) std::exp (-2.0 * kPi * std::min (f0, sampleRate * 0.25) / sampleRate);

        // filter
        const double semis = params.envAmt * 60.0 * v.filtEnv.level + params.keyTrack * (v.currentNote - 60.0);
        const double fc = std::clamp ((double) params.cutoff * std::pow (2.0, semis / 12.0), 20.0, sampleRate * 0.45);
        const float resK = 2.0f - 1.94f * std::clamp (params.reso, 0.0f, 1.0f);
        if (params.slope24)
        {
            v.c1 = makeCoefs (fc, sampleRate, 1.41421f);
            v.c2 = makeCoefs (fc, sampleRate, resK);
        }
        else v.c1 = makeCoefs (fc, sampleRate, resK);
    }

    void rebuildLayout()
    {
        static const double jpOffsets[7] = { -0.11002313, -0.06288439, -0.01952356, 0.0,
                                              0.01991221,  0.06216538,  0.10745242 };
        const int n = std::clamp (params.saws, 1, kMaxSaws);
        layoutN = n;
        detAmt = jpDetuneCurve (std::clamp ((double) params.detune, 0.0, 1.0));

        double pos[kMaxSaws] {};
        for (int i = 0; i < n; ++i)
        {
            pos[i] = n == 1 ? 0.0 : -1.0 + 2.0 * i / (n - 1);
            if (n == 7) offsets[i] = jpOffsets[i];
            else
            {
                const double a = std::abs (pos[i]);
                offsets[i] = (pos[i] < 0 ? -1.0 : 1.0) * std::pow (a, 1.45) * 0.11;
            }
        }

        const double m  = std::clamp ((double) params.mix, 0.0, 1.0);
        const double cg = jpCentreGain (m), sg = jpSideGain (m);
        int numCentre = n == 1 ? 1 : (n % 2 == 1 ? 1 : 2);
        if (n == 2) numCentre = 0;
        const int numSides = n - numCentre;
        const double sideNorm = numSides > 0 ? std::sqrt (6.0 / numSides) : 0.0;

        for (int i = 0; i < n; ++i)
        {
            bool centre = false;
            if (n == 1) centre = true;
            else if (n % 2 == 1) centre = (i == n / 2);
            else if (n > 2) centre = (i == n / 2 - 1 || i == n / 2);

            double g;
            if (n == 1)       g = 1.0;
            else if (n == 2)  g = 0.5 * (cg + sg);
            else if (centre)  g = cg / (numCentre == 2 ? 1.41421356 : 1.0);
            else              g = sg * sideNorm;
            gains[i] = (float) g;

            const double alt = (i % 2 == 0) ? 1.0 : -1.0;
            const double pan = std::clamp ((double) params.spread * pos[i] * alt, -1.0, 1.0);
            pans[i] = (float) pan;
            const double ang = (pan + 1.0) * kPi * 0.25;
            gainL[i] = (float) (g * std::cos (ang) * 1.41421356);
            gainR[i] = (float) (g * std::sin (ang) * 1.41421356);
        }
        for (int i = n; i < kMaxSaws; ++i) gains[i] = gainL[i] = gainR[i] = pans[i] = 0.0f;
    }

    double sampleRate = 44100.0;
    Params params;
    Voice  voices[kMaxVoices];
    std::vector<int> heldStack;
    int    lastNote = -1;
    bool   sustainDown = false;
    double bendSemis = 0.0;
    uint64_t ageCounter = 0;

    int    layoutN = 7;
    double detAmt = 0.0;
    double offsets[kMaxSaws] {};
    float  gains[kMaxSaws] {}, gainL[kMaxSaws] {}, gainR[kMaxSaws] {}, pans[kMaxSaws] {};

    std::mt19937 rng { 0x5a3d };
    std::uniform_real_distribution<double> uni { 0.0, 1.0 };
};
} // namespace sawdeck
