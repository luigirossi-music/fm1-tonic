/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X sequencer; see sequencer.h for the model. */
#include "sequencer.h"

#define NEVER 0xFFFFFFFFu

static const uint8_t TICKS_PER_STEP[NRATES] = {6, 4, 3, 8, 12};  /* 24 ppqn: 1/16, 1/16T, 1/32, 1/8T, 1/8 */
static const uint8_t GM_DRUM[NKIT][NDRUM] = {
    {36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51},   /* 909: BD SD LT MT HT RS CP CH OH CR RD */
    {36, 38, 41, 45, 50, 37, 39, 56, 49, 46, 42},   /* 808: BD SD LT MT HT RS CP CB CY OH CH */
};

void seq_init(seq_t *s, pattern_t *patterns)
{
    uint32_t i;
    uint8_t *p = (uint8_t *)s;
    for (i = 0; i < sizeof *s; i++)
        p[i] = 0;
    s->pat = patterns;
    for (i = 0; i < NTRACKS; i++)
        s->pcue[i] = 0xFF;
    s->bpm = 125.0f;
    s->ext_bpm = 125.0f;
    s->gacc = 30;   /* wip616: default +3 */
    s->accent_q7 = 88;                /* a 909 non-accented hit sits ~3 dB under an accent */
    s->send_clock = 1;
    {
        static const uint8_t CH[NTRACKS] = {9, 10, 3};   /* 909 10, 808 11, break 4 */
        for (i = 0; i < NTRACKS; i++)
            s->ch[i] = CH[i];
    }
    for (i = 0; i < NTRACKS; i++) {
        s->t[i].to_next = NEVER;
        s->t[i].pp_dir = 1;
        s->t[i].rng = 0x1234567u * (i + 1);
    }
    s->clk_to_next = NEVER;
    for (i = 0; i < NLATE; i++)
        s->late[i].off = 0;
    s->late_mask = 0;
}

float seq_tempo(const seq_t *s) { return s->ext ? s->ext_bpm : s->bpm; }

uint32_t seq_step_q8(const seq_t *s, int rate)
{
    float bpm = seq_tempo(s), q;
    if (bpm < 20.0f)
        bpm = 20.0f;
    if (bpm > 300.0f)
        bpm = 300.0f;
    q = (float)FS * 60.0f / bpm;                    /* samples per quarter */
    return (uint32_t)(q * 256.0f * (float)TICKS_PER_STEP[rate < NRATES ? rate : RATE_16] / 24.0f);
}

static const dpart_t *dpart(const seq_t *s, int k) { return &s->pat[s->ppat[k]].drum[k]; }
static int is_drum(int t) { return t < NKIT; }

int seq_part_len(const seq_t *s, int t)
{
    int n = is_drum(t) ? dpart(s, t)->len : 16;
    return n < 1 ? 1 : n > NSTEPS ? NSTEPS : n;
}

static int part_rate(const seq_t *s, int t)
{
    int r = is_drum(t) ? dpart(s, t)->rate : RATE_16;
    return r < NRATES ? r : RATE_16;
}

/* FILL RATE of the 909's pattern (the bar's): how many hits a FILL step plays, the first one included */
static uint32_t fill_times(const seq_t *s)
{
    uint8_t f = s->pat[s->ppat[TRK_DRUM]].fill;
    return f == 1 ? 2u : f == 2 ? 8u : 4u;
}

/* swing offset of a step, Q8: odd 16ths / 32nds come late; triplets are never swung */
static uint32_t swing_q8(const seq_t *s, int rate, uint32_t step_q8)
{
    uint32_t sw = s->pat[s->ppat[TRK_DRUM]].swing;   /* the 909 is the bar: its pattern's groove */
    if (rate == RATE_16T || rate == RATE_8T || !sw)
        return 0;
    if (sw > 100u)
        sw = 100u;
    return step_q8 / 200u * sw;                     /* 100 -> half a step late = 75 % (32-bit: no __udivdi3) */
}

static uint32_t rng_next(uint32_t *r)
{
    uint32_t x = *r ? *r : 1;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *r = x;
}

/* the step after p (forward: the drum parts and the break always play forward) */
static int next_pos(seq_t *s, int t, int p, int commit)
{
    (void)commit;
    return (p + 1) % seq_part_len(s, t);
}

static void midi(const seq_sink_t *o, uint8_t st, uint8_t d1, uint8_t d2)
{
    if (o->midi)
        o->midi(o->ctx, st, d1, d2);
}

/* one drum hit: an accent at full velocity, the rest at accent_q7; 0 = the sink did not play it (its chance).
 * (RECONSTRUCTED in wip40: it had been lost; the MIDI out is a GUESS: see HANDOFF) */
/* acc: 0 = a normal hit, 1 = accented (its own track row: full), 2 = wip612 accented only by the TOTAL row: the boost over a normal hit is scaled by GLOBAL ACCENT (100 % = full, as before) */
static int hit_drum(seq_t *s, int k, int v, int acc, const seq_sink_t *o)
{
    if (!o->drum)
        return 0;
    float vel = (float)s->accent_q7 / 127.0f, sv = 1.0f;
    if (acc == 1)
        vel = 1.0f;
    else if (acc == 2) {
        float g = (float)(s->gacc >= 10 && s->gacc < 100 ? s->gacc : 100) / 100.0f;
        vel += g * (1.0f - vel);
        sv = g < 1.0f ? 1.0f + g : 1.0f;           /* the sink's contract (wip612): above 1.0 = an accent, the part over 1.0 = the share (GLOBAL ACCENT) of the boost */
    }
    if (acc == 2 && sv > 1.0f ? !o->drum(o->ctx, k, v, sv) : !o->drum(o->ctx, k, v, vel))
        return 0;
    if (s->send_notes && s->ch[k] != SEQ_CH_OFF) {       /* MIDI out: the GM drum note, a trigger (on, then off) */
        uint8_t n = GM_DRUM[k][v], vl = (uint8_t)(vel * 127.0f + 0.5f);
        midi(o, (uint8_t)(0x90 | s->ch[k]), n, vl ? vl : 1);
        midi(o, (uint8_t)(0x80 | s->ch[k]), n, 0);
    }
    return 1;
}

static void fire_drum(seq_t *s, int k, int p, const seq_sink_t *o)
{
    const dpart_t *d = dpart(s, k);
    seq_track_t *tr = &s->t[k];
    uint32_t bit = 1u << (p & 31);
    int w = p >> 5;
    int v;
    tr->fill_n = 0;                                  /* the last step's retriggers end with it */
    tr->fill_mask = tr->fill_acc = tr->fill_gac = 0;
    for (v = 0; v < NDRUM; v++) {
        int acc, gonly;
        if (!(d->hit[v][w] & bit) || ((s->mute | s->vmute) & (1u << (k * NDRUM + v))) || (s->vsolo && !(s->vsolo & (1u << (k * NDRUM + v)))))
            continue;
        acc = ((d->accent[w] | d->vacc[v][w]) & bit) != 0;
        gonly = acc && !(d->vacc[v][w] & bit);                 /* wip612: accented by the TOTAL row alone */
        if (k == TRK_DRUM && !s->ext && v < 8) {   /* wip311: a late hit waits inside its step */
            int off = seq_late_get(s, s->ppat[k], v, p);
            if (off) {
                s->late_to[v] = (seq_step_q8(s, part_rate(s, k)) >> 8) * (uint32_t)off;
                s->late_mask |= (uint16_t)(1u << v);
                if (acc)
                    s->late_acc |= (uint16_t)(1u << v);
                if (gonly)
                    s->late_gac |= (uint16_t)(1u << v);
                continue;                            /* (a late hit has no FILL retriggers) */
            }
        }
        if (!hit_drum(s, k, v, acc ? (gonly ? 2 : 1) : 0, o))
            continue;
        if (d->vfill[v][w] & bit) {                  /* FILL: three more hits follow, a quarter step apart */
            tr->fill_mask |= (uint16_t)(1u << v);
            if (acc)
                tr->fill_acc |= (uint16_t)(1u << v);
            if (gonly)
                tr->fill_gac |= (uint16_t)(1u << v);
        }
    }
}

/* the FILL retriggers due now: every track marked on the step hits again (unless muted since) */
static void fire_fill(seq_t *s, int k, const seq_sink_t *o)
{
    seq_track_t *tr = &s->t[k];
    int v;
    for (v = 0; v < NDRUM; v++)
        if ((tr->fill_mask >> v & 1u) && !((s->mute | s->vmute) & (1u << (k * NDRUM + v))) && !(s->vsolo && !(s->vsolo & (1u << (k * NDRUM + v)))))
            hit_drum(s, k, v, (tr->fill_acc >> v & 1u) ? ((tr->fill_gac >> v & 1u) ? 2 : 1) : 0, o);
    tr->fill_n--;
    tr->fill_to = (tr->fill_to < 256u ? tr->fill_to : 0) + tr->fill_gap;
    if (!tr->fill_n)
        tr->fill_mask = tr->fill_acc = tr->fill_gac = 0;
}

/* wip311: the late hits due now (unless muted since) */
static void fire_late(seq_t *s, const seq_sink_t *o)
{
    int v;
    for (v = 0; v < 8; v++) {
        if (!(s->late_mask >> v & 1u) || s->late_to[v] >= 256u)
            continue;
        s->late_mask &= (uint16_t)~(1u << v);
        if (!((s->mute | s->vmute) & (1u << (TRK_DRUM * NDRUM + v))) && !(s->vsolo && !(s->vsolo & (1u << (TRK_DRUM * NDRUM + v)))))
            hit_drum(s, TRK_DRUM, v, (s->late_acc >> v & 1u) ? ((s->late_gac >> v & 1u) ? 2 : 1) : 0, o);
        s->late_acc &= (uint16_t)~(1u << v);
        s->late_gac &= (uint16_t)~(1u << v);
    }
}

int seq_part_muted(const seq_t *s, int p)
{
    uint32_t m = seq_part_mask(p);
    return (s->mute & m) == m;
}

/* the song's bar k into the parts and their mutes (into next[]: the caller switches) */
static void song_read(seq_t *s, int k, uint8_t *next)
{
    const song_bar_t *b = &s->song->bar[k];
    uint32_t mute = 0;                                  /* the bar decides every part's mute (built up, */
    int p;                                              /* not masked: ~mask reads as a ROM address) */
    for (p = 0; p < NTRACKS; p++) {
        next[p] = b->pat[p] < NPAT ? b->pat[p] : 0;
        if (b->mute >> p & 1u)
            mute |= seq_part_mask(p);
    }
    s->mute = mute;
}

/* the live state (next[], the mutes) into the song's bar k, growing it */
static void song_write(seq_t *s, int k, const uint8_t *next)
{
    song_bar_t *b = &s->song->bar[k];
    int p;
    b->mute = 0;
    for (p = 0; p < NTRACKS; p++) {
        b->pat[p] = next[p];
        if (seq_part_muted(s, p))
            b->mute |= (uint8_t)(1u << p);
    }
    if (s->song->len < k + 1)
        s->song->len = (uint16_t)(k + 1);
}

static int song_plays(const seq_t *s) { return s->song_on && s->song && (s->song_rec || s->song->len); }

/* the bar's end (the 909 wrapping), or the start: cues, the chain or the song decide each
 * part's next pattern. Returns the parts whose pattern changed (bit t). */
static uint32_t bar_end(seq_t *s, int start)
{
    uint8_t next[NTRACKS];
    uint32_t changed = 0;
    int t, cued = 0;
    for (t = 0; t < NTRACKS; t++)
        next[t] = s->ppat[t];
    if (song_plays(s) && !s->song_rec) {               /* the song: its next bar (looping), or this one again */
        int k = start ? s->song_start : s->song_pos + 1;
        if (!start && (s->song_hold || SONG_LOOP(&s->song->bar[s->song_pos < s->song->len ? s->song_pos : 0])) && s->song_pos < s->song->len) {   /* SONG OFF (wip211): not left, the advance is paused: this bar loops */
            k = s->song_pos;
        } else if (!start && s->song_rep > 1 && s->song_pos < s->song->len) {
            k = s->song_pos;
            s->song_rep--;
        } else {
            if (k >= s->song->len)
                k = 0;
            s->song_rep = (uint8_t)SONG_REP(&s->song->bar[k]);
        }
        s->song_pos = (uint16_t)k;
        song_read(s, k, next);
        for (t = 0; t < NTRACKS; t++)
            s->pcue[t] = 0xFF;
    } else {
        for (t = 0; t < NTRACKS; t++)
            if (s->pcue[t] < NPAT) {
                next[t] = s->pcue[t];
                s->pcue[t] = 0xFF;
                cued = 1;
            }
        if (!cued && !start && s->chain_a != s->chain_b) {
            int a = s->chain_a < s->chain_b ? s->chain_a : s->chain_b;
            int b = s->chain_a < s->chain_b ? s->chain_b : s->chain_a, c = s->ppat[TRK_DRUM];
            c = (c < a || c >= b) ? a : c + 1;
            for (t = 0; t < NTRACKS; t++)
                next[t] = (uint8_t)c;
        } else if (!cued && !start && s->chain_a == s->chain_b && s->chain_on) {
            /* FREE-PLAY CHAIN (wip122, option B), only with CHAIN ON: a pattern flagged CHAINED runs into the next one;
             * the first pattern WITHOUT the flag ends its chain and the next chain follows (1, 2, 3-4-5-6, 7-8, back to 1
             * after pattern 8). So the whole preset plays through, chain after chain, and loops. Patterns past 8 loop. */
            int c = s->ppat[TRK_DRUM];
            if (c < NPAT_TONIC) {
                c = c + 1 >= NPAT_TONIC ? 0 : c + 1;
                for (t = 0; t < NTRACKS; t++)
                    next[t] = (uint8_t)c;
            }
        } else if (!cued && start && s->chain_a == s->chain_b && s->chain_on && !song_plays(s)) {
            /* PLAY with CHAIN ON from the middle of a chain: it starts at the chain's first pattern */
            int c = s->ppat[TRK_DRUM];
            if (c > 0 && c < NPAT_TONIC) {
                while (c > 0 && s->pat[c - 1].drum[0].chain)
                    c--;
                for (t = 0; t < NTRACKS; t++)
                    next[t] = (uint8_t)c;
            }
        }
        if (song_plays(s)) {                           /* recording: what plays is the song */
            int k = start ? s->song_start : s->song_pos + 1;
            if (k >= NSONG) {
                s->song_rec = 0;                       /* full */
            } else {
                if (start && k < s->song->len)         /* overdub from where the song is */
                    song_read(s, k, next);
                s->song_pos = (uint16_t)k;
                song_write(s, k, next);
            }
        }
    }
    for (t = 0; t < NTRACKS; t++)
        if (next[t] != s->ppat[t]) {
            s->ppat[t] = next[t];
            changed |= 1u << t;
        }
    return changed;
}

int seq_rec_step(seq_t *s, int t, int nearest)
{
    seq_track_t *tr = &s->t[t];
    if (!nearest || !s->playing || tr->to_next == NEVER)
        return tr->pos;
    if (tr->to_next < seq_step_q8(s, part_rate(s, t)) / 2u)   /* less than half a step to the next one: that one is nearer */
        return next_pos(s, t, tr->pos, 0);
    return tr->pos;
}

int seq_rec_off(const seq_t *s, int t)
{
    const seq_track_t *tr = &s->t[t];
    uint32_t L = seq_step_q8(s, part_rate(s, t)) >> 8, e = tr->since >> 8;
    if (!s->playing || s->ext || tr->to_next == NEVER || !L)
        return 0;
    e = e * 256u / L;
    return e > 255u ? 255 : (int)e;
}

int seq_late_get(const seq_t *s, int pat, int v, int step)
{
    int i;
    for (i = 0; i < NLATE; i++)
        if (s->late[i].off && s->late[i].pat == pat && s->late[i].v == v && s->late[i].step == step)
            return s->late[i].off;
    return 0;
}

int seq_late_set(seq_t *s, int pat, int v, int step, int off)
{
    int i, free_i = -1;
    for (i = 0; i < NLATE; i++) {
        if (s->late[i].off && s->late[i].pat == pat && s->late[i].v == v && s->late[i].step == step) {
            s->late[i].off = (uint8_t)off;                /* one aligned byte: the ISR never sees half of it */
            return 1;
        }
        if (!s->late[i].off && free_i < 0)
            free_i = i;
    }
    if (!off)
        return 1;
    if (free_i < 0)
        return 0;
    s->late[free_i].pat = (uint8_t)pat;
    s->late[free_i].v = (uint8_t)v;
    s->late[free_i].step = (uint8_t)step;
    s->late[free_i].off = (uint8_t)off;                   /* last: the entry only counts once off is set */
    return 1;
}

void seq_late_clear_pat(seq_t *s, int pat)
{
    int i;
    for (i = 0; i < NLATE; i++)
        if (s->late[i].pat == pat)
            s->late[i].off = 0;
}

/* fire track t's next step now; returns the parts whose pattern changed (the 909 wrapping) */
static uint32_t fire_step(seq_t *s, int t, const seq_sink_t *o)
{
    seq_track_t *tr = &s->t[t];
    int rate = part_rate(s, t), p, wrapped;
    uint32_t changed = 0;
    uint32_t L = seq_step_q8(s, rate);
    uint32_t carry = tr->to_next < 256u ? tr->to_next : 0;   /* the step was due this far into the sample: keep it */
    if (!tr->started) {
        p = 0;
        tr->started = 1;
        wrapped = 0;
    } else {
        p = next_pos(s, t, tr->pos, 1);
        wrapped = p == 0;
    }
    if (wrapped && t == TRK_DRUM && (changed = bar_end(s, 0)) != 0) {
        rate = part_rate(s, t);                       /* the 909's new pattern */
        L = seq_step_q8(s, rate);
        p = 0;
    }
    if (wrapped)
        tr->bars++;
    if (changed & 1u)                                /* the 909's new pattern: its passes from 1 again, as */
        tr->bars = 0;                                /* restart() does for the others (probability cycles) */
    tr->pos = (uint8_t)p;
    tr->since = carry;                               /* wip311: this step starts now (UNQUANTIZED REC measures from here) */
    if (o->step)                                     /* first: the step's motion and p-locks, which the hit */
        o->step(o->ctx, t, p);                       /* below must hear (a drum takes its tune as it fires) */
    if (is_drum(t))
        fire_drum(s, t, p, o);
    else if (t == TRK_BRK) {
        if (o->brk)
            o->brk(o->ctx, p, tr->bars, (float)L / 256.0f,
                   (s->pat[s->ppat[TRK_BRK]].brk.steps >> p & 1u) && !(s->mute & (1u << MUTE_BRK)));
    }
    if (s->ext) {
        tr->to_next = NEVER;                         /* the clock arms the next one */
    } else {
        /* the step that follows p: swung if it is an odd one */
        int np = next_pos(s, t, p, 0);
        uint32_t sw = swing_q8(s, rate, L);
        tr->to_next = carry + ((np & 1) ? L + sw : L - sw);
        if (!(np & 1) && (p & 1) == 0)               /* even -> even (odd lengths, wraps): no swing */
            tr->to_next = carry + L;
    }
    if (is_drum(t) && tr->fill_mask) {               /* FILL: the retriggers split the step into 2 / 4 / 8 (FILL RATE) */
        uint32_t span = tr->to_next == NEVER ? L : tr->to_next - carry, x = fill_times(s);
        tr->fill_gap = span / x;
        tr->fill_to = carry + tr->fill_gap;
        tr->fill_n = (uint8_t)(x - 1u);
    }
    return changed;
}

static void restart(seq_t *s, uint32_t parts)  /* parts whose pattern switched: from step 1 (not the 909's) */
{
    int t;
    for (t = 1; t < NTRACKS; t++) {
        if (!(parts >> t & 1u))
            continue;
        s->t[t].started = 0;
        s->t[t].to_next = s->ext ? NEVER : 0;
        s->t[t].ext_ticks = (uint8_t)(TICKS_PER_STEP[part_rate(s, t)] - 1);
        s->t[t].bars = 0;
        s->t[t].pp_dir = 1;
    }
}

static void do_start(seq_t *s, int from_top, const seq_sink_t *o)
{
    int t;
    if (from_top)
        bar_end(s, 1);                                /* cues, or the song's start bar */
    for (t = 0; t < NTRACKS; t++) {
        seq_track_t *tr = &s->t[t];
        if (from_top) {
            tr->started = 0;
            tr->bars = 0;
            tr->pp_dir = 1;
        }
        tr->to_off = 0;
        tr->fill_n = 0;
        tr->fill_mask = tr->fill_acc = tr->fill_gac = 0;
        tr->to_next = s->ext ? NEVER : 0;
        tr->ext_ticks = (uint8_t)(TICKS_PER_STEP[part_rate(s, t)] - 1);   /* the first clock is the downbeat */
    }
    s->clk_to_next = 0;
    s->late_mask = s->late_acc = s->late_gac = 0;                  /* wip311 */
    s->playing = 1;
    if (!s->ext && s->send_clock)
        midi(o, from_top ? 0xFA : 0xFB, 0, 0);
}

static void do_stop(seq_t *s, const seq_sink_t *o)
{
    int t;
    s->playing = 0;
    if (o->brk_stop)
        o->brk_stop(o->ctx);
    for (t = 0; t < NTRACKS; t++) {
        s->t[t].to_next = NEVER;
        s->t[t].fill_n = 0;
        s->t[t].fill_mask = s->t[t].fill_acc = s->t[t].fill_gac = 0;
    }
    s->late_mask = s->late_acc = s->late_gac = 0;                  /* wip311 */
    if (!s->ext && s->send_clock)
        midi(o, 0xFC, 0, 0);
}

/* external clock ticks: arm the steps they complete */
static void ext_ticks(seq_t *s)
{
    int t;
    uint32_t n = s->ext_pending;
    s->ext_pending = 0;
    for (; n; n--) {
        if (s->ext_since > 0 && s->ext_since < FS) {  /* tempo: smoothed tick period */
            uint32_t q = s->ext_since << 8;
            s->ext_period = s->ext_period ? s->ext_period - (s->ext_period >> 3) + (q >> 3) : q;
            s->ext_bpm = (float)FS * 60.0f * 256.0f / (24.0f * (float)s->ext_period);
        }
        s->ext_since = 0;
        if (!s->playing)
            continue;
        for (t = 0; t < NTRACKS; t++) {
            seq_track_t *tr = &s->t[t];
            int rate = part_rate(s, t), tps = TICKS_PER_STEP[rate];
            if (++tr->ext_ticks < tps)
                continue;
            tr->ext_ticks = 0;
            {
                int np = tr->started ? next_pos(s, t, tr->pos, 0) : 0;
                uint32_t L = s->ext_period ? s->ext_period * (uint32_t)tps : seq_step_q8(s, rate);
                tr->to_next = (np & 1) ? swing_q8(s, rate, L) : 0;
            }
        }
    }
}

uint32_t seq_until_event(seq_t *s, uint32_t n)
{
    uint32_t m = n, e;
    int t;
    if (!s->playing)
        return n;
    for (t = 0; t < NTRACKS; t++) {
        if (s->t[t].to_next != NEVER && (e = s->t[t].to_next >> 8) < m)
            m = e;
        if (s->t[t].to_off && (e = s->t[t].to_off >> 8) < m)
            m = e;
        if (s->t[t].fill_n && (e = s->t[t].fill_to >> 8) < m)
            m = e;
    }
    for (t = 0; s->late_mask && t < 8; t++)           /* wip311: a late hit is due */
        if ((s->late_mask >> t & 1u) && (e = s->late_to[t] >> 8) < m)
            m = e;
    if (!s->ext && s->send_clock && (e = s->clk_to_next >> 8) < m)
        m = e;
    return m ? m : 1;
}

void seq_advance(seq_t *s, uint32_t k, const seq_sink_t *o)
{
    int t, guard;
    uint32_t dq = k << 8;
    s->ext_since += k;
    if (s->ext_pending)
        s->ext = 1;
    else if (s->ext && s->ext_since > FS / 2u) {    /* clock gone for 0.5 s: back to internal */
        s->ext = 0;
        for (t = 0; t < NTRACKS; t++)
            if (s->playing && s->t[t].to_next == NEVER)
                s->t[t].to_next = 0;
    }
    if (s->req_stop) {
        s->req_stop = 0;
        s->req_start = s->req_cont = 0;
        if (s->playing)
            do_stop(s, o);
    }
    if (s->req_start) {
        s->req_start = 0;
        do_start(s, 1, o);
    }
    if (s->req_cont) {
        s->req_cont = 0;
        do_start(s, 0, o);
    }
    if (s->ext_pending)
        ext_ticks(s);
    if (!s->playing) {
        if (!s->ext)
            s->clk_to_next = 0;
        for (t = 0; t < NTRACKS; t++)                /* stopped: a cue takes effect at once */
            if (s->pcue[t] < NPAT) {
                s->ppat[t] = s->pcue[t];
                s->pcue[t] = 0xFF;
            }
        return;
    }
    for (t = 0; t < NTRACKS; t++) {                 /* move time */
        seq_track_t *tr = &s->t[t];
        if (tr->to_next != NEVER)
            tr->to_next = tr->to_next > dq ? tr->to_next - dq : 0;
        if (tr->to_off)
            tr->to_off = tr->to_off > dq ? tr->to_off - dq : 1;   /* 1: due, still "set" */
        if (tr->fill_n)
            tr->fill_to = tr->fill_to > dq ? tr->fill_to - dq : 0;
        tr->since += dq;                             /* wip311 */
    }
    if (s->late_mask) {                              /* wip311: the late hits wait */
        int v;
        for (v = 0; v < 8; v++)
            if (s->late_mask >> v & 1u)
                s->late_to[v] = s->late_to[v] > dq ? s->late_to[v] - dq : 0;
    }
    if (!s->ext && s->send_clock)
        s->clk_to_next = s->clk_to_next > dq ? s->clk_to_next - dq : 0;
    /* fire what is due; the drum part first, since its wrap may switch the pattern */
    for (guard = 0; guard < 8; guard++) {
        int fired = 0;
        for (t = 0; t < NTRACKS; t++) {
            seq_track_t *tr = &s->t[t];
            if (tr->fill_n && tr->fill_to < 256u) {   /* a FILL retrigger (before a new step at the same time) */
                fire_fill(s, t, o);
                fired = 1;
            }
            if (t == TRK_DRUM && s->late_mask)        /* wip311: a late hit (before a new step at the same time) */
                fire_late(s, o);
            if (tr->to_next < 256u) {
                uint32_t ch = fire_step(s, t, o);
                if (ch)
                    restart(s, ch);                   /* a part's new pattern starts on step 1 */
                fired = 1;
            }
        }
        if (!fired)
            break;
    }
    if (!s->ext && s->send_clock && s->clk_to_next < 256u) {
        midi(o, 0xF8, 0, 0);
        s->clk_to_next += seq_step_q8(s, RATE_16) / 6u;           /* 24 ppqn */
    }
}

void seq_start(seq_t *s) { s->req_start = 1; }
void seq_stop(seq_t *s) { s->req_stop = 1; }
void seq_continue(seq_t *s) { s->req_cont = 1; }
void seq_ext_clock(seq_t *s) { s->ext_pending++; }
void seq_cue(seq_t *s, int p)
{
    int t;
    if (p >= 0 && p < NPAT)
        for (t = 0; t < NTRACKS; t++)
            s->pcue[t] = (uint8_t)p;
}
void seq_cue_part(seq_t *s, int t, int p)
{
    if (t >= 0 && t < NTRACKS && p >= 0 && p < NPAT)
        s->pcue[t] = (uint8_t)p;
}
int seq_cue_of(const seq_t *s, int t) { return s->pcue[t] < NPAT ? s->pcue[t] : -1; }

/* wip625: the pattern the song will play at the end of this bar (same rule as bar_end), or -1 when no song is playing (stopped, SONG OFF is a loop of
 * this bar, recording) or it is the pattern now. Reads only: nothing is changed. */
int seq_song_next(const seq_t *s)
{
    int k, n;
    if (!s->playing || !song_plays(s) || s->song_rec || !s->song || s->song_pos >= s->song->len)
        return -1;
    k = s->song_pos + 1;
    if (s->song_hold || SONG_LOOP(&s->song->bar[s->song_pos]) || s->song_rep > 1)
        k = s->song_pos;
    else if (k >= s->song->len)
        k = 0;
    n = s->song->bar[k].pat[TRK_DRUM];
    if (n >= NPAT || n == s->ppat[TRK_DRUM])
        return -1;
    return n;
}
int seq_next_step(seq_t *s, int t, int p) { return next_pos(s, t, p, 0); }
void seq_chain(seq_t *s, int a, int b)
{
    s->chain_a = (uint8_t)a;
    s->chain_b = (uint8_t)b;
}
