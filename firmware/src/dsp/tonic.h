/* SPDX-License-Identifier: GPL-3.0-only */
/* Tonic: the Microtonic-style drum engine. 8 voices, each a libpo32 (MIT, see po32/) voice with
 * its 21 normalised parameters, plus a per-voice level (dB) and pan.
 *
 * Unlike the X0X engines, the parameters are 16-bit (0..65535 = 0.0..1.0, the PO-32 transfer
 * encoding) so a loaded Microtonic / Patternarium preset keeps its full precision; the knobs move
 * them in coarse steps (tonic_nudge). Threading: trigger / set between render blocks.
 * No libm, no allocation. */
#pragma once
#include <stdint.h>
#include "po32/po32_synth.h"
#include "x0x_param.h"

#define TN_VOICES   8
#define TN_NPARAMS  21                 /* po32_patch_params_t, in its field order */
#define TN_MAX_BLOCK 256
/* velocities that reproduce Microtonic: an accented step plays at full velocity, a normal step at
 * about a third of it (fitted on the Rostoma export: accent-vs-normal level gaps of all 8 drums
 * agree within 1.2 dB rms at 42; libpo32's own velocity law is otherwise unchanged) */
#define TN_VEL_NORMAL 42
#define TN_VEL_ACCENT 127

enum { TNP_OSCWAVE, TNP_OSCFREQ, TNP_OSCATK, TNP_OSCDCY, TNP_MODMODE, TNP_MODRATE, TNP_MODAMT,
       TNP_NFILMOD, TNP_NFILFRQ, TNP_NFILQ, TNP_NENVMOD, TNP_NENVATK, TNP_NENVDCY, TNP_MIX,
       TNP_DIST, TNP_EQFREQ, TNP_EQGAIN, TNP_LEVEL, TNP_OSCVEL, TNP_NVEL, TNP_MODVEL };

typedef struct {
    po32_synth_voice_t vc[TN_VOICES];
    uint16_t p[TN_VOICES][TN_NPARAMS];  /* the sound: 0..65535 */
    int16_t  gain_q8[TN_VOICES];        /* extra per-voice gain, 1/256 steps (Microtonic's strip level) */
    uint8_t  pan[TN_VOICES];            /* 0..127, 64 = centre (x0x_pan_gains) */
    float    pan_l[TN_VOICES], pan_r[TN_VOICES];
    float    vgain[TN_VOICES];
    uint8_t  active[TN_VOICES];
    uint8_t  choke;                     /* wip328: the CHOKE group (bit v = voice v): a hit of one of them cuts the others, 3 ms fade */
    uint8_t  sc_in, sc_int, sc_mask;     /* wip348: the SIDECHAIN (one for the project): the input voice 0..7, the intensity 0..127 (0 = no ducking), the ducked voices (bit v) */
    float    sc_pk;                      /* wip356: the highest sc_p since the screen last looked (the dot pulses at any tempo) */
    float    sc_p;                       /* ... the follower of the input's sound (its power, smoothed) */
    int16_t  kill[TN_VOICES];           /* wip328: samples of the choke fade still to run (0 = not being choked) */
    uint8_t  snd_rev[TN_VOICES], snd_dly[TN_VOICES];   /* wip242: each track's own reverb / delay send, 0..127 (pots 22 / 23) */
    uint8_t  sh_lo[TN_VOICES], sh_hi[TN_VOICES];       /* wip289: the shelving cuts' knobs (lo 0 = 20 Hz, hi 127 = 20 kHz: never off, wip318) */
    float    sh_lo_c[TN_VOICES][3], sh_hi_c[TN_VOICES][3];   /* wip295: the cuts' biquad (b0, a1, a2; b1 = -/+ 2 b0, b2 = b0) */
    float    sh_lo_z[TN_VOICES][2], sh_hi_z[TN_VOICES][2];   /* wip295: their states (one biquad each = 12 dB / octave) */
    float    snd_rev_f[TN_VOICES], snd_dly_f[TN_VOICES];   /* the send as it is now in the render (it ramps across a block) */
    uint32_t sr;
} tonic_t;

void tonic_init(tonic_t *t, uint32_t sample_rate);
/* vel 1..127 (libpo32 velocity); the sequencer uses 96 normal / 127 accent */
void tonic_trigger(tonic_t *t, int voice, int vel);
/* wip251: the same, the pitch moved by semis semitones (-64..63) for this hit only; the stored OscFreq stays */
void tonic_trigger_note(tonic_t *t, int voice, int vel, int semis);
/* a new 16-bit value for one parameter (a preset load, a knob) */
void tonic_set(tonic_t *t, int voice, int i, uint16_t value);
void tonic_set_choke(tonic_t *t, int mask);          /* wip328: the choke group (bits 0..7) */
void tonic_set_sidechain(tonic_t *t, int input, int intensity, int mask);   /* wip348: the SIDECHAIN: what the input voice sounds like ducks the voices of the mask (the input itself is never ducked); intensity 0 = none */
int tonic_duck_peak(tonic_t *t);                     /* wip356: the deepest duck since the last call (0-255), then restarts from now: for the pulsing dot */
int tonic_duck_depth(const tonic_t *t);              /* wip352: the sidechain's duck right now, 0-255 */
void tonic_set_pan(tonic_t *t, int voice, int pan_pot);
/* ---- the knobs (X0X pots, 0..127; a 3-way switch 0..2) over the 16-bit store ---------------------
 * A pot is a coarse view of the 16-bit value. tonic_set_pot keeps the 16-bit value when the pot still
 * shows it (a loaded preset keeps its full precision) and takes the pot's value when it moved. The same
 * rule runs on the main loop's mirror and in the audio ISR, so both stay equal. Pot TN_POT_PAN = pan. */
#define TN_POT_PAN  TN_NPARAMS          /* pot 21: pan, 0..127, 64 = centre */
#define TN_POT_RSND (TN_NPARAMS + 1)    /* pot 22 (wip242): the track's reverb send, 0..127 (0 = none) */
#define TN_POT_DSND (TN_NPARAMS + 2)    /* pot 23 (wip242): the track's delay send */
#define TN_NPOTS    (TN_NPARAMS + 3)
/* wip289: the track's two SHELVING CUTS (HOME 1st press, K3 / K4 of MIX 2). Not in the 24-wide pot rows: tonic_set_pot takes them by number.
 * LOSH: 0 = 20 Hz (wip318: no OFF, the filter is always in the path) .. 127 = 663 Hz, below the corner the sound is cut (wip295: a real low cut, 12 dB / octave like Ableton's EQ Eight Low Cut 12 dB); HISH: 127 and 126 = 20 kHz (no OFF) .. 0 = 1 kHz, fine steps at the top (wip298),
 * above the corner the sound is cut (a real high cut, 12 dB / octave like EQ Eight's High Cut 12 dB). Both corners are exponential. */
#define TN_POT_LOSH TN_NPOTS            /* 24 */
#define TN_POT_HISH (TN_NPOTS + 1)      /* 25 */
int tonic_shelf_hz(int hi, int pot);    /* the corner in Hz of a knob position (hi = 0: LOW, 1: HIGH) */
int tonic_nparams(int voice);           /* TN_NPOTS for a voice 0..7, else 0 */
const x0x_param_t *tonic_param(int voice, int i);
int tonic_val_to_pot(int i, uint16_t val);
uint16_t tonic_pot_to_val(int i, int pot);
uint16_t tonic_init_val(int i);
void tonic_set_pot(tonic_t *t, int voice, int i, int pot);
void tonic_set_gain_db(tonic_t *t, int voice, float db);
/* a knob step: +-1 detent = about 1/128 of the range, for the discrete ones one position */
void tonic_nudge(tonic_t *t, int voice, int i, int detents);
/* mix n (<= TN_MAX_BLOCK per call; any n is accepted) stereo frames, ADDED to l / r */
void tonic_render_st(tonic_t *t, float *l, float *r, int n);
/* wip242: the same, and each track's own sends: ADDED to rev / dly (mono, after the track's level, before its pan), times gain
 * (the part's level). rev / dly 0 = no sends (tonic_render_st) */
void tonic_render_sends(tonic_t *t, float *l, float *r, float *rev, float *dly, float gain, int n);
int  tonic_active(const tonic_t *t);
