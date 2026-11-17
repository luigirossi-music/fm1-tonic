/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X pattern model: 32 patterns, each holding the 909, the 808 and the break.
 *
 * Plain data, no pointers, so a pattern copies with an assignment and the project
 * stores the bytes. The UI edits these in place from the main loop while the
 * sequencer reads them in the audio ISR: every field the sequencer reads is a
 * single byte or an aligned 32-bit word, so a read never sees half a write. A drum row is
 * two words (steps 1-32, 33-64): sm_get / sm_flip below. */
#pragma once
#include <stdint.h>

#define NPAT 32
#define NSTEPS 64
#define NSW (NSTEPS / 32)            /* 32-bit words in a row of steps */
#define NDRUM 11                     /* tracks per drum machine: the 11 black keys */
#define NKIT 2                       /* 0 = the Tonic, 1 = old 808 slot (unused, kept for the saved format) */

enum { RATE_16, RATE_16T, RATE_32, RATE_8T, RATE_8, NRATES };   /* step length (wip74: RATE_8 added at the end, so saved values stay) */

typedef struct {
    uint32_t hit[NDRUM][NSW];         /* step s (sm_get): the instrument plays */
    uint32_t accent[NSW];             /* the total-accent row: accents every track that hits on the step */
    uint32_t vacc[NDRUM][NSW];        /* per-track accents (Microtonic keeps one accent row a drum): a track hit
                                       * is accented when its own bit OR the total row's bit is set */
    uint32_t vfill[NDRUM][NSW];       /* per-track FILL steps: a hit there is retriggered 2 / 4 / 8 times over the step (pattern_t.fill) */
    uint8_t len;                      /* 1..64 */
    uint8_t rate;
    uint8_t chain;                    /* Tonic: 1 = CHAINED: when this pattern ends the next pattern plays (the last of
                                       * the 8 goes back to the first). Same byte as the old rsv[0]: no size change */
    uint8_t rsv;
} dpart_t;
#define NPAT_NOTE 16                  /* wip252: the Tonic patterns 1-16 keep a note per track per step (project_t.note) */
#define NTRK_NOTE 8                   /* ... for the Tonic's 8 tracks */
#define NPAT_TONIC 8                  /* the Tonic's patterns 1-8 (a-h): where a chain wraps (= TNP_NPAT_LOAD in tnp.h) */

/* BREAK: the generator's settings (dsp/breaks.h) and the steps it may play on */
enum { BRK_COMPLEX, BRK_ANCHOR, BRK_ROLL, BRK_FILL, BRK_R2, BRK_R3, BRK_R4, BRK_R8,
       BRK_PHRASE, BRK_BCHANCE, BRK_ALEN, BRK_BLEN, BRK_NSET };
typedef struct {
    uint32_t steps;                   /* bit s = the break may sound on 16th s of the bar */
    uint8_t set[BRK_NSET];            /* breaks.h param values, in BRK_* order */
    uint8_t slot_a, slot_b;           /* loop A / B: the built-in bank, then the user slots (engine_brk_slot_names) */
    uint8_t rsv[2];
    uint8_t slice[16];                /* per 16th: 0 = the generator chooses, 1..8 = loop A's slice 1..8 */
} brkpart_t;

typedef struct {
    dpart_t drum[NKIT];
    brkpart_t brk;
    uint8_t swing;                    /* 0..100: 0 = straight (50 %), 100 = 75 % shuffle */
    uint8_t fill;                     /* FILL RATE (wip74): 0 = 4x (the default, so old data keeps it), 1 = 2x, 2 = 8x */
    uint8_t rsv[2];
} pattern_t;

/* a row of steps (NSW words): step s */
static inline int sm_get(const uint32_t *m, int s) { return (int)((m[s >> 5] >> (s & 31)) & 1u); }
static inline void sm_flip(uint32_t *m, int s) { m[s >> 5] ^= 1u << (s & 31); }
static inline void sm_set(uint32_t *m, int s) { m[s >> 5] |= 1u << (s & 31); }
static inline int sm_any(const uint32_t *m)
{
    uint32_t a = 0;
    int k;
    for (k = 0; k < NSW; k++)
        a |= m[k];
    return a != 0;
}

/* default empty pattern: 16 steps, straight, the break silent */
void pattern_init(pattern_t *p);
