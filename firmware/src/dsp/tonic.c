/* SPDX-License-Identifier: GPL-3.0-only */
/* Tonic engine: 8 libpo32 voices, see tonic.h. libpo32 (po32/, MIT) does the synthesis. */
#include "tonic.h"
#include "fastmath.h"

/* Microtonic renders about 8.5 dB below libpo32 on the velocity-insensitive drums of the Rostoma
 * export (drums 1, 2, 4: -8.2..-10.3 dB normal, -7.6..-8.6 dB accent); applied to every voice.
 * Session 4: whole-pattern check (pattern a, mono voices, vs the Export 2.0 one-shots sequenced the
 * same way) showed the engine 1.9 dB rms too quiet, so the trim was raised by 1.5 dB: -7.0 dB */
#define TN_TRIM 0.44668f   /* 10^(-7.0/20) */
#ifndef TN_DUR_K
#define TN_DUR_K 2.0f
#endif
#ifndef TN_PAN_K
#define TN_PAN_K 0.858f
#endif
static float tn_exp2(float x)               /* 2^x, |x| < 30, rel. error < 2e-7 */
{
    int32_t i = (int32_t)x;
    float f, p;
    union { float f; uint32_t u; } b;
    if ((float)i > x) i--;
    f = x - (float)i;
    p = 1.0f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * (0.00961813f + f * (0.00133336f + f * 0.00015403f)))));
    b.u = (uint32_t)(i + 127) << 23;
    return p * b.f;
}
static float tn_pow10(float y) { return tn_exp2(y * 3.32192809f); }

/* Microtonic's pan law, fitted on the Rostoma one-shots (L/R ratio of drums 1,2,3,5,7 pure gain,
 * 0.117 dB per % of pan from 1.5 to 17 %, drum 8 at 48 % about 6.5 dB): a constant-power law
 * L = sqrt2 cos(th), R = sqrt2 sin(th), th = pi/4 (1 + 0.858 p), p = -1..+1 (centre = 1 / 1, so the
 * mono level is unchanged). Not measured above 48 %: the far side at hard pan is about -19 dB. */
static float tn_cos(float x)               /* 0 <= x <= 1.6, error < 3e-7 */
{
    x *= x;
    return 1.0f + x * (-0.5f + x * (0.041666667f + x * (-0.0013888889f + x * (2.4801587e-5f + x * (-2.7557319e-7f + x * 2.0876757e-9f)))));
}
static void tn_pan_gains(int pot, float *l, float *r)
{
    float p = (float)(pot - 64) * (1.0f / 63.5f), th;
    if (p > 1.0f) p = 1.0f;
    if (p < -1.0f) p = -1.0f;
    th = 0.78539816f * (1.0f + TN_PAN_K * p);
    *l = 1.41421356f * tn_cos(th);
    *r = 1.41421356f * tn_cos(1.57079633f - th);
}

static inline float u2f(uint16_t v) { return (float)v * (1.0f / 65535.0f); }

static void to_params(const uint16_t *p, po32_patch_params_t *o)
{
    float *f = &o->OscWave;                 /* the 21 floats are contiguous, in TNP_ order */
    int i;
    for (i = 0; i < TN_NPARAMS; i++) f[i] = u2f(p[i]);
}

/* power-on sound: a plain sine kick-ish voice, EQ and modulation neutral (0.5) */
static const uint16_t INIT[TN_NPARAMS] = {
    0, 30970, 0, 40000, 0, 32768, 32768, 0, 65535, 11140, 0, 0, 0, 0, 0, 0, 32768, 52000, 0, 0, 0};

uint16_t tonic_init_val(int i) { return (i >= 0 && i < TN_NPARAMS) ? INIT[i] : 0; }

static void shelf_coef(float *cf, int hi, int pot, uint32_t sr);

void tonic_init(tonic_t *t, uint32_t sr)
{
    int v, i;
    t->sr = sr;
    t->choke = 0;
    t->sc_in = 0; t->sc_int = 0; t->sc_mask = 0; t->sc_p = 0.0f; t->sc_e = 0.0f;   /* wip348: no sidechain */
    for (v = 0; v < TN_VOICES; v++) {
        for (i = 0; i < TN_NPARAMS; i++) t->p[v][i] = INIT[i];
        t->pan[v] = 64;
        t->snd_rev[v] = t->snd_dly[v] = 0;
        t->snd_rev_f[v] = t->snd_dly_f[v] = 0.0f;
        t->sh_lo[v] = 0; t->sh_hi[v] = 127;               /* wip289: no shelving cut */
        { int q; for (q = 0; q < 2; q++) t->sh_lo_z[v][q] = t->sh_hi_z[v][q] = 0.0f; }
        shelf_coef(t->sh_lo_c[v], 0, 0, sr);              /* wip318: the cuts are never off: LOW CUT 20 Hz, HIGH CUT 20 kHz (nothing audible) */
        shelf_coef(t->sh_hi_c[v], 1, 127, sr);
        tn_pan_gains(64, &t->pan_l[v], &t->pan_r[v]);
        t->vgain[v] = TN_TRIM;
        t->active[v] = 0;
        t->kill[v] = 0;
        po32_synth_voice_init(&t->vc[v], sr, 0, 0, 0.0f);   /* inert */
    }
}

/* wip251: one semitone on the pitch parameter. OscFreq is 0..1 over 20 Hz .. 20 kHz on an exponential scale (a ratio of
 * 1000 = 9.9658 octaves), so an octave is 1 / 9.9658 = 0.10034 of the range and a semitone 1 / 119.59 = 0.0083618 */
#define TN_SEMITONE 0.0083618f

void tonic_trigger(tonic_t *t, int v, int vel) { tonic_trigger_note(t, v, vel, 0); }

/* a hit of the voice, its pitch moved by semis semitones for this hit only: the stored parameter (t->p) is not touched, only the
 * local copy the voice is started from */
void tonic_trigger_note(tonic_t *t, int v, int vel, int semis)
{
    po32_patch_params_t pp;
    float d1, d2, a1, a2, dur, x;
    if (v < 0 || v >= TN_VOICES) return;
    if (vel < 1) vel = 1;
    if (vel > 127) vel = 127;
    to_params(t->p[v], &pp);
    if (semis) {                                          /* wip251: the keyboard of the ARP page (and, later, a step's note) */
        x = pp.OscFreq + (float)semis * TN_SEMITONE;
        pp.OscFreq = x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;
    }
    x = pp.OscAtk; a1 = x * tn_pow10((12.0f * x - 7.0f) * 0.2f);
    x = pp.NEnvAtk; a2 = x * tn_pow10((12.0f * x - 7.0f) * 0.2f);
    d1 = tn_pow10(3.0f * pp.OscDcy - 2.0f);
    d2 = tn_pow10(3.0f * pp.NEnvDcy - 2.0f);
    dur = a1 + d1; if (a2 + d2 > dur) dur = a2 + d2;
    dur = dur * TN_DUR_K + 0.05f;
    if (dur > 12.0f) dur = 12.0f;
#ifdef TN_FORCE_DUR
    dur = TN_FORCE_DUR;
#endif
#ifdef TN_DEBUG
    { extern int printf(const char*, ...); printf("  dur v%d = %.3f (a1 %.3f d1 %.3f a2 %.3f d2 %.3f)\n", v+1, dur, a1, d1, a2, d2); }
#endif
    po32_synth_voice_init(&t->vc[v], t->sr, &pp, vel, dur);
    t->active[v] = 1;
    t->kill[v] = 0;
    if (t->choke & (1u << v)) {                           /* wip328: CHOKE: this voice is in the group: the others of it fade out in 3 ms */
        int w, n = (int)(t->sr * 3u / 1000u);
        for (w = 0; w < TN_VOICES; w++)
            if (w != v && (t->choke & (1u << w)) && t->active[w] && t->kill[w] == 0)
                t->kill[w] = (int16_t)(n < 1 ? 1 : n);
    }
}

void tonic_set_choke(tonic_t *t, int mask) { t->choke = (uint8_t)mask; }

/* wip619: SIDECHAIN = a COMPRESSOR with an external sidechain (his Ableton settings): the input voice's sound (after its cuts, its choke fade and its level) is measured as
 * RMS (5 ms average of the power), in dB; the gain computer has a fixed ratio 4:1 and a soft knee of 6 dB round the THRESHOLD; the gain reduction is smoothed
 * with attack 1 ms and release 30 ms; no look-ahead, no makeup. A ducked voice is multiplied by that gain: its whole sound, so its reverb and delay sends too.
 * The only knob is the threshold: sc_int 0 = off (0 dB, nothing ducks), 126 = -59 dB, 127 = -INF (the range of the Ableton compressor's threshold).
 * A muted input makes no sound: no duck. */
#define SC_RATIO 4.0f
#define SC_KNEE  6.0f
float tonic_sc_thr_db(int sc_int)                    /* wip620: like the Ableton compressor: 0 = 0 dB (off), 1..126 = -0.5 .. -59 dB, 127 = -INF (counted as -120 dB: whatever sounds ducks all it can) */
{
    return sc_int >= 127 ? -120.0f : -59.0f * (float)sc_int * (1.0f / 126.0f);
}
void tonic_set_sidechain(tonic_t *t, int input, int intensity, int mask)
{
    t->sc_in = (uint8_t)(input & 7);
    t->sc_int = (uint8_t)(intensity < 0 ? 0 : intensity > 127 ? 127 : intensity);
    t->sc_mask = (uint8_t)(mask & 0xFF & ~(1 << t->sc_in));
    if (!t->sc_int || !t->sc_mask) t->sc_p = t->sc_pk = t->sc_e = 0.0f;
}
int tonic_duck_depth(const tonic_t *t)               /* wip352; wip619: the duck right now, 0-255 (0 = no reduction) */
{
    float d = t->sc_e;
    if (!t->sc_int || !t->sc_mask) return 0;
    d = d > 1.0f ? 1.0f : d;
    return (int)(d * 255.0f + 0.5f);
}
int tonic_duck_peak(tonic_t *t)                      /* wip356; wip619: the deepest duck since the last call (0-255), for the pulsing dot */
{
    float d = t->sc_pk > t->sc_e ? t->sc_pk : t->sc_e;
    t->sc_pk = t->sc_e;
    if (!t->sc_int || !t->sc_mask) return 0;
    d = d > 1.0f ? 1.0f : d;
    return (int)(d * 255.0f + 0.5f);
}
static void sc_follow(tonic_t *t, const float *x, float gain, int got, int m, float *duck)   /* the duck gain of the next m samples; x = the input's samples (got of them), 0 = silent */
{
    float p = t->sc_p, e = t->sc_e, thr = tonic_sc_thr_db((int)t->sc_int), tgt = e;
    float kp = 1.0f / (0.005f * (float)t->sr), ka = 1.0f / (0.001f * (float)t->sr), kr = 1.0f / (0.030f * (float)t->sr);
    const float slope = 1.0f - 1.0f / SC_RATIO;
    int k;
    for (k = 0; k < m; k++) {
        float s = (x && k < got) ? x[k] * gain : 0.0f;
        s *= s;
        p += kp * (s - p);                           /* RMS: the power, averaged over about 5 ms */
        if ((k & 3) == 0) {                          /* the gain computer every 4th sample (the log and the exp are the costly part) */
            float over = 3.0103f * fm_log2f(p + 1e-12f) - thr, gr;   /* the level in dB (10 log10 of the power) over the threshold */
            if (over <= -0.5f * SC_KNEE) gr = 0.0f;
            else if (over >= 0.5f * SC_KNEE) gr = over * slope;
            else { float q = over + 0.5f * SC_KNEE; gr = slope * q * q / (2.0f * SC_KNEE); }   /* the soft knee */
            tgt = 1.0f - fm_exp2f(-gr * 0.166096405f);                /* the depth: 1 - 10^(-gr / 20) */
        }
        e += (tgt > e ? ka : kr) * (tgt - e);        /* attack 1 ms down, release 30 ms back up */
        if (e > t->sc_pk) t->sc_pk = e;
        duck[k] = 1.0f - e;
    }
    t->sc_p = fm_flush(p);
    t->sc_e = fm_flush(e);
}

void tonic_set(tonic_t *t, int v, int i, uint16_t val)
{
    if (v >= 0 && v < TN_VOICES && i >= 0 && i < TN_NPARAMS) t->p[v][i] = val;
}
void tonic_set_pan(tonic_t *t, int v, int pot)
{
    if (v < 0 || v >= TN_VOICES) return;
    if (pot < 0) pot = 0;
    if (pot > 127) pot = 127;
    t->pan[v] = (uint8_t)pot;
    tn_pan_gains(pot, &t->pan_l[v], &t->pan_r[v]);
}
void tonic_set_gain_db(tonic_t *t, int v, float db)
{
    if (v < 0 || v >= TN_VOICES) return;
    t->vgain[v] = TN_TRIM * tn_exp2(db * 0.16609640f);     /* 10^(db/20) */
}
void tonic_nudge(tonic_t *t, int v, int i, int d)
{
    int n;
    if (v < 0 || v >= TN_VOICES || i < 0 || i >= TN_NPARAMS) return;
    if (i == TNP_OSCWAVE || i == TNP_MODMODE || i == TNP_NFILMOD || i == TNP_NENVMOD) {
        int pos = (int)((uint32_t)t->p[v][i] * 3u / 65536u);   /* 3 positions */
        pos += d; if (pos < 0) pos = 0; if (pos > 2) pos = 2;
        t->p[v][i] = (uint16_t)(pos == 2 ? 65535 : pos * 32768);
        return;
    }
    n = (int)t->p[v][i] + d * 512;
    if (n < 0) n = 0;
    if (n > 65535) n = 65535;
    t->p[v][i] = (uint16_t)n;
}

int tonic_active(const tonic_t *t)
{
    int v;
    for (v = 0; v < TN_VOICES; v++) if (t->active[v]) return 1;
    return 0;
}

void tonic_render_st(tonic_t *t, float *l, float *r, int n)
{
    tonic_render_sends(t, l, r, 0, 0, 0.0f, n);
}

void tonic_render_sends(tonic_t *t, float *l, float *r, float *rev, float *dly, float gain, int n)
{
    float tmp[TN_MAX_BLOCK], duck[TN_MAX_BLOCK];
    int v, vi, ord[TN_VOICES];
    while (n > 0) {
        int m = n > TN_MAX_BLOCK ? TN_MAX_BLOCK : n, k;
        int sc = t->sc_int > 0 && t->sc_mask != 0, in = t->sc_in;
        for (vi = 0; vi < TN_VOICES; vi++) ord[vi] = vi;
        if (sc) {                                            /* wip348: the input first (the others are ducked by what it sounds like in this very block) */
            int q = 1;
            ord[0] = in;
            for (vi = 0; vi < TN_VOICES; vi++) if (vi != in) ord[q++] = vi;
            if (!t->active[in]) sc_follow(t, 0, 0.0f, 0, m, duck);   /* (a silent input: the follower runs down) */
        }
        for (vi = 0; vi < TN_VOICES; vi++) {
            size_t got = 0;
            float gl, gr;
            v = ord[vi];
            if (!t->active[v]) continue;
            po32_synth_voice_render_f32(&t->vc[v], tmp, (size_t)m, &got);
            {                                                /* wip318: always on (no OFF: switching a filter in and out in the middle of the sound clicked); wip295: LOW CUT / HIGH CUT like Ableton's EQ Eight 12 dB: one 2nd-order Butterworth biquad each (12 dB / octave, -3 dB at the corner), low = high-pass, high = low-pass */
                const float *cl = t->sh_lo_c[v], *ch = t->sh_hi_c[v];
                float l1 = t->sh_lo_z[v][0], l2 = t->sh_lo_z[v][1];
                float h1 = t->sh_hi_z[v][0], h2 = t->sh_hi_z[v][1];
                for (k = 0; k < (int)got; k++) {
                    float x = tmp[k], y;
                    y = cl[0] * x + l1; l1 = -2.0f * cl[0] * x - cl[1] * y + l2; l2 = cl[0] * x - cl[2] * y; x = y;   /* high-pass: b = (1, -2, 1) * b0 */
                    y = ch[0] * x + h1; h1 = 2.0f * ch[0] * x - ch[1] * y + h2; h2 = ch[0] * x - ch[2] * y; x = y;    /* low-pass: b = (1, 2, 1) * b0 */
                    tmp[k] = x;
                }
                t->sh_lo_z[v][0] = fm_flush(l1); t->sh_lo_z[v][1] = fm_flush(l2);
                t->sh_hi_z[v][0] = fm_flush(h1); t->sh_hi_z[v][1] = fm_flush(h2);
            }
            if (t->kill[v] > 0) {                            /* wip328: being choked: a linear fade to silence over the rest of the 3 ms, then the voice stops */
                int n0 = (int)(t->sr * 3u / 1000u), kk = t->kill[v];
                if (n0 < 1) n0 = 1;
                for (k = 0; k < (int)got; k++) {
                    if (kk > 0) { tmp[k] *= (float)kk / (float)n0; kk--; }
                    else tmp[k] = 0.0f;
                }
                t->kill[v] = (int16_t)kk;
                if (kk == 0) t->active[v] = 0;
            }
            if (sc) {                                        /* wip348: SIDECHAIN */
                if (v == in) sc_follow(t, tmp, t->vgain[v], (int)got, m, duck);
                else if (t->sc_mask & (1u << v)) for (k = 0; k < (int)got; k++) tmp[k] *= duck[k];
            }
            gl = t->pan_l[v] * t->vgain[v];
            gr = t->pan_r[v] * t->vgain[v];
            for (k = 0; k < (int)got; k++) { l[k] += tmp[k] * gl; r[k] += tmp[k] * gr; }
            if (rev && dly) {                           /* wip242: this track's sends (a ramp over the block: no zipper when a knob turns) */
                float tr = (float)t->snd_rev[v] * (1.0f / 127.0f) * t->vgain[v] * gain, cr = t->snd_rev_f[v];
                float td = (float)t->snd_dly[v] * (1.0f / 127.0f) * t->vgain[v] * gain, cd = t->snd_dly_f[v];
                float sr_ = got ? (tr - cr) / (float)got : 0.0f, sd_ = got ? (td - cd) / (float)got : 0.0f;
                if (cr != 0.0f || tr != 0.0f || cd != 0.0f || td != 0.0f)
                    for (k = 0; k < (int)got; k++) {
                        cr += sr_; cd += sd_;
                        rev[k] += tmp[k] * cr;
                        dly[k] += tmp[k] * cd;
                    }
                t->snd_rev_f[v] = tr;
                t->snd_dly_f[v] = td;
            }
            if (po32_synth_voice_done(&t->vc[v])) t->active[v] = 0;
        }
        l += m; r += m; if (rev && dly) { rev += m; dly += m; } n -= m;
    }
}

/* ------------------------------------------------------------------ the knobs --- */
static const char *const SW_WAVE[3] = {"SINE", "TRI", "SAW"};
static const char *const SW_MMOD[3] = {"DECAY", "SINE", "RANDOM"};
static const char *const SW_NFIL[3] = {"LP", "BP", "HP"};
static const char *const SW_NENV[3] = {"EXP", "LIN", "CLAP"};
/* def: the pot that shows INIT[i] (checked by tests/host/tonic_pots_test.c) */
static const x0x_param_t PARAMS[TN_NPOTS] = {
    {"Wave", 2, 0, SW_WAVE},   {"Pitch", 127, 60, 0},  {"OscAtk", 127, 0, 0},  {"OscDcy", 127, 78, 0},
    {"ModMod", 2, 0, SW_MMOD}, {"ModRat", 127, 64, 0}, {"ModAmt", 127, 64, 0}, {"NFMode", 2, 0, SW_NFIL},
    {"NFFreq", 127, 127, 0},   {"NFQ", 127, 22, 0},    {"NEMode", 2, 0, SW_NENV}, {"NEAtk", 127, 0, 0},
    {"NEDcy", 127, 0, 0},      {"Mix", 127, 0, 0},     {"Dist", 127, 0, 0},    {"EQFreq", 127, 0, 0},
    {"EQGain", 127, 64, 0},    {"Level", 127, 101, 0}, {"VelOsc", 127, 0, 0},  {"VelNs", 127, 0, 0},
    {"VelMod", 127, 0, 0},     {"Pan", 127, 64, 0},
    {"Rev", 127, 0, 0},        {"Dly", 127, 0, 0}};   /* wip242: the track's reverb / delay sends */

static int is_switch(int i) { return i == TNP_OSCWAVE || i == TNP_MODMODE || i == TNP_NFILMOD || i == TNP_NENVMOD; }

int tonic_nparams(int v) { return (v >= 0 && v < TN_VOICES) ? TN_NPOTS : 0; }
const x0x_param_t *tonic_param(int v, int i)
{
    return (v >= 0 && v < TN_VOICES && i >= 0 && i < TN_NPOTS) ? &PARAMS[i] : 0;
}
int tonic_val_to_pot(int i, uint16_t val)
{
    if (is_switch(i)) return (int)((uint32_t)val * 3u / 65536u);     /* the thirds libpo32 switches on */
    return (int)(((uint32_t)val * 127u + 32767u) / 65535u);
}
uint16_t tonic_pot_to_val(int i, int pot)
{
    if (is_switch(i)) return (uint16_t)(pot >= 2 ? 65535 : pot <= 0 ? 0 : 32768);
    if (i == TNP_MODAMT && pot == 64) return 32768;                  /* MOD AMT: the middle is exactly 0.00 sm */
    if (pot < 0) pot = 0;
    if (pot > 127) pot = 127;
    return (uint16_t)(((uint32_t)pot * 65535u + 63u) / 127u);
}
int tonic_shelf_hz(int hi, int pot)
{
    if (pot < 0) pot = 0;
    if (pot > 127) pot = 127;
    if (hi) {                                          /* wip298: HIGH CUT: 127 = off, 126 = 20 kHz .. 0 = 1 kHz; steps fine at the top (24 Hz) and growing as it goes down (130 Hz at 18.8 kHz, 230 Hz at 13 kHz, 210 Hz at 8.5 kHz, 47 Hz at 1 kHz): f = 20 kHz * exp(-(0.001 u + 0.000181 u^2)), u = 126 - knob */
        float u;
        if (pot > 126) pot = 126;
        u = (float)(126 - pot);
        return (int)(20000.0f * fm_expf(-(0.001f * u + 0.000181f * u * u)) / 10.0f + 0.5f) * 10;   /* wip299: to the nearest 10 Hz, as shown */
    }
    return (int)(20.0f * fm_powf(33.1f, (float)pot / 127.0f) + 0.5f);   /* LOW CUT: 20 Hz .. 663 Hz */
}
static void shelf_coef(float *cf, int hi, int pot, uint32_t sr)   /* wip295: one Butterworth biquad (Q = 0.7071, like Ableton's EQ Eight Low Cut / High Cut 12 dB): b0, a1, a2 */
{
    float fc = (float)tonic_shelf_hz(hi, pot), K, n;
    if (fc > 0.45f * (float)sr) fc = 0.45f * (float)sr;
    K = fm_tanf(FM_PI * fc / (float)sr);
    n = 1.0f / (1.0f + 1.41421356f * K + K * K);
    cf[0] = (hi ? K * K : 1.0f) * n;
    cf[1] = 2.0f * (K * K - 1.0f) * n;
    cf[2] = (1.0f - 1.41421356f * K + K * K) * n;
}
void tonic_set_pot(tonic_t *t, int v, int i, int pot)
{
    if (v >= 0 && v < TN_VOICES && (i == TN_POT_LOSH || i == TN_POT_HISH)) {   /* wip289 */
        if (pot < 0) pot = 0;
        if (pot > 127) pot = 127;
        if (i == TN_POT_LOSH) { t->sh_lo[v] = (uint8_t)pot; shelf_coef(t->sh_lo_c[v], 0, pot, t->sr); }
        else { t->sh_hi[v] = (uint8_t)pot; shelf_coef(t->sh_hi_c[v], 1, pot, t->sr); }
        return;
    }
    if (v < 0 || v >= TN_VOICES || i < 0 || i >= TN_NPOTS) return;
    if (i == TN_POT_PAN) { tonic_set_pan(t, v, pot); return; }
    if (i == TN_POT_RSND || i == TN_POT_DSND) {          /* wip242: the track's own sends */
        if (pot < 0) pot = 0;
        if (pot > 127) pot = 127;
        if (i == TN_POT_RSND) t->snd_rev[v] = (uint8_t)pot; else t->snd_dly[v] = (uint8_t)pot;
        return;
    }
    if (tonic_val_to_pot(i, t->p[v][i]) == pot) return;      /* the pot still shows the stored value */
    t->p[v][i] = tonic_pot_to_val(i, pot);
}
