/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X sequencer: runs in the audio ISR, sample accurate.
 *
 * The engine renders a block like this:
 *     while (n) {
 *         k = seq_until_event(&seq, n);       // samples until the next due event (>= 1)
 *         render k samples;
 *         seq_advance(&seq, k, &sink);        // moves time, fires the events now due
 *         n -= k;
 *     }
 * so a step lands on its exact sample, and engine code never sees time.
 *
 * Five parts, each with its own step counter (lengths and rates may differ):
 * 909, 808, 303 A, 303 B, BREAK (always 16ths, 16 to the bar). Each part plays its own
 * pattern (ppat[part]); track t is part t. The 909 part's length defines the bar: cued
 * patterns (or the next one of a chain, or the song's next bar) start when it wraps, and a
 * part whose pattern changed restarts on step 1 while the others keep running.
 *
 * Song: one entry per bar, each part's pattern and the part mutes. song_on plays it (and
 * loops); song_on + song_rec writes the live state (cues, mutes) into it from song_start on.
 *
 * Clock: internal (BPM) or external (24 ppqn MIDI clock, fed with seq_ext_clock()).
 * Swing delays every second 16th (909 shuffle: 0 = straight, 100 = 75 %).
 *
 * 303 semantics (as the TB-303 and Open303's sequencer):
 *   NOTE  plays; the gate falls at half the step, unless the step slides (BS_SLIDE)
 *         or the next step is a TIE, in which case it is held through.
 *   TIE   continues the sounding note (no retrigger); same gate rule.
 *   slide a NOTE following a step with BS_SLIDE glides from the held note
 *         (note_on with slide = 1, envelopes not retriggered).
 *   REST  releases.
 */
#pragma once
#include <stdint.h>
#include "pattern.h"

#define FS 44100u
#define TRK_DRUM 0                    /* + kit: 0 = 909, 1 = 808 */
#define TRK_BRK NKIT
#define NTRACKS (NKIT + 1)
/* mute bits: kit k voice v = k * NDRUM + v; then BREAK */
#define MUTE_BRK (NKIT * NDRUM)

/* the mute bits of part p (a drum machine: all its voices) */
static inline uint32_t seq_part_mask(int p)
{
    return p < NKIT ? ((1u << NDRUM) - 1u) << (p * NDRUM) : 1u << MUTE_BRK;
}

#define NSONG 192
#define SEQ_CH_OFF 0xFFu
/* a step's chance (the T_PROB lanes): 1-100 = that percentage, PROB_COND + k = the cycle k in the order
 * 1:2 2:2 1:3 2:3 3:3 1:4 ... 8:8 (A:B: on the A-th of every B passes of the part's pattern) */
#define PROB_COND 101
#define PROB_NCOND 35                    /* 2 + 3 + ... + 8 */
static inline void prob_cond(int k, int *a, int *b)
{
    int n = 2;
    while (k >= n) {
        k -= n;
        n++;
    }
    *a = k + 1;
    *b = n;
}
#define SONG_LOOP(b) (((b)->mute >> 4) & 1u)   /* wip608: LOOP = the bar plays until stopped: a spare bit of mute (only parts 0-2 exist) */
#define SONG_REP(b) (((b)->mute >> 5) + 1)    /* a bar plays 1-8 times: mute's bits 5-7 (the parts use 0-4) */
typedef struct {
    uint8_t pat[NTRACKS];             /* each part's pattern */
    uint8_t mute;                     /* bit p = part p muted */
} song_bar_t;
typedef struct {
    uint16_t len;                     /* bars; 0 = empty */
    uint8_t rsv[2];
    song_bar_t bar[NSONG];
} song_t;

typedef struct {
    int (*drum)(void *ctx, int kit, int voice, float vel);      /* 0: the hit did not play (its chance) */
    void (*brk)(void *ctx, int step16, int bar, float samples_per_16th, int enabled);
    void (*brk_stop)(void *ctx);
    void (*midi)(void *ctx, uint8_t status, uint8_t d1, uint8_t d2);   /* clock / transport / notes out */
    void (*step)(void *ctx, int part, int step);                        /* UI playhead (optional) */
    void *ctx;
} seq_sink_t;

typedef struct {                      /* one part's runtime */
    uint8_t pos;                      /* current step */
    int8_t pp_dir;                    /* ping-pong direction */
    uint8_t started;                  /* the first step has fired since start */
    uint8_t sounding;                 /* 303: a note is held */
    uint8_t held_note;
    uint32_t to_next;                 /* samples to the next step (Q8 fraction) */
    uint32_t to_off;                  /* 303: samples to the gate falling, Q8; 0 = none */
    uint32_t rng;                     /* DIR_RANDOM and auto-mutate */
    uint16_t bars;                    /* wraps since the pattern started (auto-mutate) */
    uint8_t ext_ticks;                /* external clock: ticks into this step */
    uint8_t fill_n;                   /* drums: FILL retriggers still to play on this step (0..7: FILL RATE 2x / 4x / 8x) */
    uint16_t fill_mask;               /* ... for which tracks (bit v) */
    uint16_t fill_acc;                /* ... of those, the accented ones */
    uint16_t fill_gac;                /* wip612: ... of the accented ones, those accented only by the TOTAL row (the GLOBAL ACCENT level applies) */
    uint32_t fill_to;                 /* ... samples to the next one, Q8 */
    uint32_t fill_gap;                /* ... and between them (a quarter of the step), Q8 */
    uint32_t since;                   /* wip311: samples (Q8) since this step fired (UNQUANTIZED REC: how late in the step a hit came) */
} seq_track_t;

/* wip311 UNQUANTIZED REC: a hit recorded between the steps plays LATE inside its step: off / 256 of a step (1..255).
 * A small table, RAM only for now (not saved yet): off = 0 = a free entry. */
typedef struct { uint8_t pat, v, step, off; } late_t;
#define NLATE 96

typedef struct {
    pattern_t *pat;                   /* NPAT patterns (the project's) */
    song_t *song;                     /* the project's song (may be 0) */
    volatile uint8_t playing;         /* 1 = running */
    volatile uint8_t ppat[NTRACKS];   /* the pattern each part plays */
    volatile uint8_t pcue[NTRACKS];   /* each part's cued pattern (0xFF = none) */
    volatile uint8_t chain_a, chain_b;/* chain range (whole patterns); a == b = no chain */
    volatile uint8_t chain_on;        /* Tonic (wip111): SONG ON (K1 of the pattern page): 1 = the patterns flagged CHAINED run into the next one and the song advances; 0 = they loop (wip211: also pauses the song bars) */
    volatile uint8_t song_hold;       /* wip211: SONG OFF in song mode: the bar advance is paused, the current bar loops (the UI sets it from chain_on) */
    volatile uint8_t song_on;         /* 1 = the song decides the patterns, bar by bar */
    volatile uint8_t song_rec;        /* with song_on: write the live state into the song */
    volatile uint16_t song_pos;       /* bar playing */
    volatile uint16_t song_start;     /* bar PLAY starts at */
    volatile uint8_t ext;             /* 1 = following an external clock */
    volatile uint8_t send_clock;      /* 1 = emit MIDI clock + transport when internal */
    volatile uint8_t send_notes;      /* 1 = echo the sequence on MIDI, on each part's channel */
    volatile uint8_t ch[NTRACKS];     /* each part's MIDI channel, 0-15 (in and out); SEQ_CH_OFF = none */
    uint8_t song_rep;                 /* the song bar playing: its passes still to play (SONG_REP) */
    volatile uint32_t mute;           /* MUTE_* bits */
    volatile uint32_t vmute;          /* drum tracks muted on their own (bit k * NDRUM + v): kept apart from
                                       * the part mutes, which the song and HOME set as a whole */
    volatile uint32_t vsolo;          /* wip264: SOLO (the Tonic: FILL + black key 1-8): the one voice that plays (bit k * NDRUM + v), 0 = no solo;
                                       * the others are silent without touching vmute; not saved */
    volatile uint8_t choke;           /* wip328: the CHOKE group of the Tonic voices (bit v): a hit of one cuts the others; saved in the preset (byte 12), not in the project */
    volatile uint8_t accent_q7;       /* drum velocity of a non-accented hit, of 127 */
    volatile uint8_t gacc;            /* wip612: GLOBAL ACCENT level 10..100 (0 = 100): the share of the accent boost the total row gives */
    float bpm;                        /* internal tempo (UI writes; read per step) */
    float ext_bpm;                    /* tempo measured from the external clock */
    seq_track_t t[NTRACKS];           /* TRK_DRUM.., TRK_BRK */
    late_t late[NLATE];               /* wip311: the late hits (see late_t) */
    uint16_t late_mask;               /* wip311: Tonic voices (bit v) with a late hit waiting on this step ... */
    uint16_t late_acc;                /* ... of those, the accented ones */
    uint16_t late_gac;                /* wip612: ... of those, accented only by the TOTAL row */
    uint32_t late_to[NDRUM];          /* ... and the samples (Q8) until each one plays */
    uint32_t clk_to_next;             /* internal: samples to the next MIDI clock, Q8 */
    uint32_t ext_since;               /* samples since the last external clock (tempo estimate) */
    uint32_t ext_period;              /* smoothed samples per external clock, Q8 */
    volatile uint32_t ext_pending;    /* external clocks received, not yet consumed */
    volatile uint8_t req_start, req_stop, req_cont; /* transport requests from the UI / MIDI */
} seq_t;

void seq_init(seq_t *s, pattern_t *patterns);
uint32_t seq_until_event(seq_t *s, uint32_t n);
void seq_advance(seq_t *s, uint32_t k, const seq_sink_t *o);

/* main loop / MIDI side (all are flag writes picked up at the next block) */
void seq_start(seq_t *s);             /* from step 1 of the current (or cued) pattern */
void seq_stop(seq_t *s);
void seq_continue(seq_t *s);
void seq_ext_clock(seq_t *s);         /* one 24-ppqn tick (USB MIDI 0xF8) */
void seq_cue(seq_t *s, int pattern);  /* every part, at the end of the bar (now, if stopped) */
void seq_cue_part(seq_t *s, int part, int pattern);
void seq_chain(seq_t *s, int a, int b);
int seq_cue_of(const seq_t *s, int part);   /* the part's cued pattern, or -1 */
int seq_song_next(const seq_t *s);             /* wip625: the pattern the song plays next bar, or -1 */
int seq_part_muted(const seq_t *s, int part);

/* steps of one part's rate, in samples Q8 (no swing) at the current tempo */
uint32_t seq_step_q8(const seq_t *s, int rate);
/* wip292: the step a live-recorded hit goes to on track t: nearest = 1 rounds to the NEAREST step (a hit in the second half of a
 * step goes to the next one), nearest = 0 keeps the step now playing. On an external clock: the step now playing */
int seq_rec_step(seq_t *s, int t, int nearest);
/* wip311: how far into the step now playing on track t it is, 0..255 (1/256 of a step; 0 = on the beat or stopped / external clock) */
int seq_rec_off(const seq_t *s, int t);
int seq_late_get(const seq_t *s, int pat, int v, int step);              /* the offset of that hit (0 = none) */
int seq_late_set(seq_t *s, int pat, int v, int step, int off);           /* off 0 clears; 1 = stored, 0 = table full */
void seq_late_clear_pat(seq_t *s, int pat);                              /* every offset of a pattern */
float seq_tempo(const seq_t *s);      /* the tempo in effect (internal or measured) */
int seq_part_len(const seq_t *s, int t);         /* steps in part t's pattern */
int seq_next_step(seq_t *s, int t, int p);       /* the step after p (direction; no commit) */
