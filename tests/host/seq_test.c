/* SPDX-License-Identifier: GPL-3.0-only */
/* Sequencer tests on the host: event times to the sample, swing, rates, polymeter, pattern cue + chain,
 * external clock, MIDI clock out, the break clock. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../firmware/src/seq/sequencer.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("  FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* ---- event log */
enum { E_DRUM, E_ON, E_OFF, E_BRK, E_BRKSTOP, E_MIDI };
typedef struct { uint32_t t; int kind, a, b, c, d; } ev_t;
static ev_t evs[100000];
static int nev;
static uint32_t now;
static void log_ev(int k, int a, int b, int c, int d)
{
    if (nev < (int)(sizeof evs / sizeof evs[0]))
        evs[nev++] = (ev_t){now, k, a, b, c, d};
}
static int s_drum(void *x, int k, int v, float vel) { (void)x; log_ev(E_DRUM, k * 100 + v, (int)(vel * 127.0f + 0.5f), 0, 0); return 1; }
static void s_brk(void *x, int s16, int bar, float spb, int en) { (void)x; log_ev(E_BRK, s16, bar, (int)spb, en); }
static void s_brkstop(void *x) { (void)x; log_ev(E_BRKSTOP, 0, 0, 0, 0); }
static void s_midi(void *x, uint8_t st, uint8_t d1, uint8_t d2) { (void)x; log_ev(E_MIDI, st, d1, d2, 0); }
static const seq_sink_t SINK = {s_drum, s_brk, s_brkstop, s_midi, 0, 0};

static seq_t S;
static pattern_t P[NPAT];

/* render like the engine: block of 256, split at events */
static void run(uint32_t samples)
{
    while (samples) {
        uint32_t n = samples < 256 ? samples : 256;
        seq_advance(&S, 0, &SINK);
        while (n) {
            uint32_t k = seq_until_event(&S, n);
            now += k;
            seq_advance(&S, k, &SINK);
            n -= k;
            samples -= k;
        }
    }
}

static void reset(void)
{
    int i;
    for (i = 0; i < NPAT; i++)
        pattern_init(&P[i]);
    seq_init(&S, P);
    S.send_clock = 0;
    nev = 0;
    now = 0;
}

static int find(int kind, int a, int from)
{
    int i;
    for (i = from; i < nev; i++)
        if (evs[i].kind == kind && (a < 0 || evs[i].a == a))
            return i;
    return -1;
}

static void test_drum_timing(void)
{
    int i, k, n = 0;
    double step = 44100.0 * 60.0 / 120.0 / 4.0;      /* 5512.5 samples */
    reset();
    S.bpm = 120.0f;
    for (i = 0; i < 16; i += 4)
        P[0].drum[0].hit[0][0] |= 1u << i;                 /* BD on the quarters */
    P[0].drum[0].hit[7][0] = 0xAAAAu;                      /* CH on the off-16ths */
    P[0].drum[0].accent[0] = 1u;                           /* accent on step 1 */
    seq_start(&S);
    run(44100 * 4 - 1);                              /* 2 bars, not the downbeat of the 3rd */
    for (i = 0, k = 0; (i = find(E_DRUM, 0, i)) >= 0; i++, k++) {
        double want = k * 4 * step;
        CHECK(abs((int)evs[i].t - (int)(want + 0.5)) <= 1, "BD %d at %u, want %.1f", k, evs[i].t, want);
        if (k % 4 == 0)
            CHECK(evs[i].b == 127, "BD %d accented vel %d", k, evs[i].b);
        else
            CHECK(evs[i].b == 88, "BD %d vel %d, want 88", k, evs[i].b);
        n++;
    }
    CHECK(n == 8, "BD hits %d, want 8 in 2 bars", n);
    for (i = 0, k = 0; (i = find(E_DRUM, 7, i)) >= 0; i++, k++)
        CHECK(abs((int)evs[i].t - (int)((2 * k + 1) * step + 0.5)) <= 1, "CH %d at %u", k, evs[i].t);
    CHECK(k == 16, "CH hits %d", k);
    printf("  drum timing: %d BD, %d CH on the sample\n", n, k);
}

/* wip311: a late hit (UNQUANTIZED REC) plays off / 256 of a step after its step; seq_rec_off measures it */
static void test_late_hit(void)
{
    int i, n = 0;
    double step = 44100.0 * 60.0 / 120.0 / 4.0;
    reset();
    S.bpm = 120.0f;
    P[0].drum[0].hit[0][0] |= 1u << 4;                    /* BD on step 5 ... */
    P[0].drum[0].hit[1][0] |= 1u << 8;                    /* ... SD on step 9, accented */
    P[0].drum[0].accent[0] |= 1u << 8;
    CHECK(seq_late_set(&S, 0, 0, 4, 128) == 1 && seq_late_set(&S, 0, 1, 8, 64) == 1, "late_set");
    CHECK(seq_late_get(&S, 0, 0, 4) == 128 && seq_late_get(&S, 0, 1, 8) == 64 && seq_late_get(&S, 0, 0, 5) == 0, "late_get");
    seq_start(&S);
    run((uint32_t)(step * 4.0 + step * 0.25));           /* a quarter into step 5: nothing yet */
    CHECK(find(E_DRUM, 0, 0) < 0, "BD must wait");
    CHECK(abs(seq_rec_off(&S, 0) - 64) <= 3, "rec_off %d, want about 64", seq_rec_off(&S, 0));
    run(44100 * 2);
    i = find(E_DRUM, 0, 0);
    CHECK(i >= 0 && abs((int)evs[i].t - (int)(4 * step + step * 128.0 / 256.0 + 0.5)) <= 2, "late BD at %u, want %.1f", i >= 0 ? evs[i].t : 0, 4 * step + step / 2);
    i = find(E_DRUM, 1, 0);
    CHECK(i >= 0 && abs((int)evs[i].t - (int)(8 * step + step * 64.0 / 256.0 + 0.5)) <= 2, "late SD at %u", i >= 0 ? evs[i].t : 0);
    CHECK(i >= 0 && evs[i].b == 127, "late SD keeps its accent");
    for (i = 0; (i = find(E_DRUM, 0, i)) >= 0; i++)
        n++;
    CHECK(n == 1, "BD played %d times in the first bar, want 1", n);
    seq_late_set(&S, 0, 0, 4, 0);                         /* cleared: back on the grid */
    CHECK(seq_late_get(&S, 0, 0, 4) == 0, "late cleared");
    seq_stop(&S);
    run(256);
    nev = 0;
    now = 0;
    seq_start(&S);
    run((uint32_t)(step * 4.0 + 100));
    i = find(E_DRUM, 0, 0);
    CHECK(i >= 0 && abs((int)evs[i].t - (int)(4 * step + 0.5)) <= 2, "cleared BD on the grid at %u", i >= 0 ? evs[i].t : 0);
    printf("  late hit: BD and SD play late by their offsets\n");
}

static void test_swing_and_rates(void)
{
    int i, k;
    double step = 44100.0 * 60.0 / 120.0 / 4.0;
    reset();
    S.bpm = 120.0f;
    P[0].swing = 100;                                /* 75 %: odd 16ths half a step late */
    P[0].drum[0].hit[7][0] = 0xFFFFu;
    seq_start(&S);
    run(44100 * 2);
    for (i = 0, k = 0; (i = find(E_DRUM, 7, i)) >= 0 && k < 16; i++, k++) {
        double want = k * step + ((k & 1) ? 0.5 * step : 0.0);
        CHECK(abs((int)evs[i].t - (int)(want + 0.5)) <= 1, "swung CH %d at %u want %.1f", k, evs[i].t, want);
    }
    /* triplets: 1/16T = 1/6 quarter, never swung */
    reset();
    S.bpm = 120.0f;
    P[0].swing = 100;
    P[0].drum[0].rate = RATE_16T;
    P[0].drum[0].len = 12;
    P[0].drum[0].hit[7][0] = 0xFFFu;
    seq_start(&S);
    run(44100);
    for (i = 0, k = 0; (i = find(E_DRUM, 7, i)) >= 0 && k < 12; i++, k++) {
        double want = k * 44100.0 * 60.0 / 120.0 / 6.0;
        CHECK(abs((int)evs[i].t - (int)(want + 0.5)) <= 1, "triplet %d at %u want %.1f", k, evs[i].t, want);
    }
    printf("  swing 75%%, 1/16T unswung: checked\n");
}

static void test_polymeter_cue_chain(void)
{
    int i, n, last;
    double step = 44100.0 * 60.0 / 120.0 / 4.0;
    reset();
    S.bpm = 120.0f;
    P[0].drum[1].len = 3;                                           /* the 808: a 3-step BD loop (polymeter) */
    P[0].drum[1].hit[0][0] = 1u;
    P[0].drum[0].hit[0][0] = 1u;
    P[1].drum[0].hit[1][0] = 1u;                                    /* SD only in pattern 2 */
    seq_start(&S);
    run((uint32_t)(step * 15) - 1);                         /* not the hit due exactly at step 15 */
    for (i = 0, n = 0; (i = find(E_DRUM, 100, i)) >= 0; i++, n++)
        CHECK(abs((int)evs[i].t - (int)(n * 3 * step + 0.5)) <= 1, "3-step 808 loop %d at %u", n, evs[i].t);
    CHECK(n == 5, "3-step loop over 15 steps: %d hits", n);
    seq_cue(&S, 1);
    nev = 0;
    run((uint32_t)(step * 17));                               /* to the bar line and one more */
    i = find(E_DRUM, 1, 0);
    CHECK(i >= 0 && abs((int)evs[i].t - (int)(16 * step + 0.5)) <= 1, "cued pattern starts on the bar (%u)", i >= 0 ? evs[i].t : 0);
    CHECK(S.ppat[0] == 1 && S.ppat[TRK_BRK] == 1, "now playing pattern 2");
    i = find(E_DRUM, 100, 0);
    /* pattern 2's 808 is empty: no hits after the switch */
    for (n = 0; (i = find(E_DRUM, 100, i < 0 ? 0 : i)) >= 0; i++)
        if (evs[i].t >= (uint32_t)(16 * step))
            n++;
    CHECK(n == 0, "808 restarts with the new (empty) pattern");
    /* chain 3..5 */
    reset();
    S.bpm = 120.0f;
    for (i = 0; i < NPAT; i++)
        P[i].drum[0].hit[0][0] = 1u;
    for (i = 0; i < NTRACKS; i++)
        S.ppat[i] = 3;
    seq_chain(&S, 3, 5);
    seq_start(&S);
    for (i = 0, last = -1; i < 7; i++) {
        uint8_t c = S.ppat[0];
        run((uint32_t)(16 * step));
        if (i < 6)
            CHECK(c == 3 + (i % 3), "chain bar %d plays pattern %d (got %d)", i, 3 + i % 3, c + 1 - 1);
        last = c;
    }
    (void)last;
    printf("  polymeter (808 loop), cue on the bar, chain 4-6: checked\n");
}

/* one part's cue: it switches on the bar and restarts; the others keep their place */
static void test_part_cue(void)
{
    int i, n;
    double step = 44100.0 * 60.0 / 120.0 / 4.0;
    reset();
    S.bpm = 120.0f;
    P[0].drum[1].len = 3;                                       /* the 808: a 3-step BD loop (the 909 is the bar) */
    P[0].drum[1].hit[0][0] = 1u;
    P[1].drum[0].hit[0][0] = 1u;                                /* the 909 in pattern 2: a BD on step 1 */
    seq_start(&S);
    run((uint32_t)(step * 5));
    seq_cue_part(&S, 0, 1);
    run((uint32_t)(step * 11) - 1);                             /* to just before the bar */
    CHECK(S.ppat[0] == 0, "a part's cue waits for the bar");
    nev = 0;
    run((uint32_t)(step * 4));
    CHECK(S.ppat[0] == 1 && S.ppat[1] == 0 && S.ppat[TRK_BRK] == 0, "only the 909 switched");
    i = find(E_DRUM, 0, 0);
    CHECK(i >= 0 && abs((int)evs[i].t - (int)(16 * step + 0.5)) <= 1, "the 909's new line on the bar");
    for (i = 0, n = 0; (i = find(E_DRUM, 100, i)) >= 0; i++, n++)   /* the 808: steps 18 (its 7th loop), not 16 */
        CHECK(abs((int)evs[i].t - (int)(18 * step + 0.5)) <= 1, "the 808 kept its place (hit at %u)", evs[i].t);
    CHECK(n == 1, "the 808: one hit in the window, %d", n);
    printf("  a part's own cue: checked\n");
}

static void test_song(void)
{
    static song_t sg;
    double bar = 44100.0 * 60.0 / 120.0 * 4.0;
    int i;
    /* playback: three bars, looping; a bar's mutes */
    reset();
    S.bpm = 120.0f;
    S.song = &sg;
    sg.len = 3;
    for (i = 0; i < NTRACKS; i++) {
        sg.bar[0].pat[i] = 0;
        sg.bar[1].pat[i] = 1;
        sg.bar[2].pat[i] = 2;
    }
    sg.bar[1].pat[TRK_BRK] = 7;
    sg.bar[0].mute = sg.bar[1].mute = 0;
    sg.bar[2].mute = 1u << 1;                                   /* the 808 out in bar 3 */
    S.song_on = 1;
    S.song_start = 1;
    seq_start(&S);
    run(1);
    CHECK(S.ppat[0] == 1 && S.ppat[TRK_BRK] == 7 && S.song_pos == 1, "starts at the start bar");
    run((uint32_t)bar);
    CHECK(S.ppat[0] == 2 && S.song_pos == 2 && seq_part_muted(&S, 1) && !seq_part_muted(&S, 0), "bar 3, 808 muted");
    run((uint32_t)bar);
    CHECK(S.ppat[0] == 0 && S.song_pos == 0 && !seq_part_muted(&S, 1), "loops to bar 1, 808 back");
    seq_cue(&S, 9);
    run((uint32_t)bar);
    CHECK(S.ppat[0] == 1, "playing the song, a cue does not override it");
    /* recording: from an empty song, the live cues and mutes are written bar by bar */
    reset();
    S.bpm = 120.0f;
    S.song = &sg;
    sg.len = 0;
    S.song_on = S.song_rec = 1;
    S.song_start = 0;
    seq_start(&S);
    run(1);
    CHECK(sg.len == 1 && sg.bar[0].pat[0] == 0, "record: bar 1 written at the start");
    seq_cue_part(&S, 1, 5);
    S.mute |= seq_part_mask(TRK_BRK);
    run((uint32_t)bar);
    CHECK(sg.len == 2 && sg.bar[1].pat[1] == 5 && sg.bar[1].pat[0] == 0 && sg.bar[1].mute == 1u << TRK_BRK,
          "record: bar 2 has the cue and the mute (len %d, pat %d, mute %d)", sg.len, sg.bar[1].pat[1], sg.bar[1].mute);
    run((uint32_t)bar * 2);
    CHECK(sg.len == 4 && sg.bar[3].pat[1] == 5, "record: the song grows while it runs (%d)", sg.len);
    /* overdub from bar 2 plays bar 2 as it is, then writes on */
    seq_stop(&S);
    run(1);
    S.song_start = 1;
    sg.bar[1].pat[0] = 6;
    seq_start(&S);
    run(1);
    CHECK(S.ppat[0] == 6 && sg.len == 4, "overdub starts from the song's bar");
    printf("  song: playback, loop, mutes, record, overdub: checked\n");
}

static void test_ext_clock_and_midi_out(void)
{
    int i, k, clocks;
    uint32_t per_tick = 919;                                  /* ~120 BPM: 44100*60/120/24 = 918.75 */
    reset();
    P[0].drum[0].hit[0][0] = 0xFFFFu;
    for (i = 0; i < 2; i++) {                                 /* clocks before start: tempo only */
        seq_ext_clock(&S);
        run(per_tick);
    }
    seq_start(&S);
    nev = 0;
    {
    uint32_t t_start = now;
    for (i = 0; i < 24 * 4; i++) {                            /* one bar of ticks */
        uint32_t t0 = now;
        seq_ext_clock(&S);
        run(per_tick);
        (void)t0;
    }
    CHECK(S.ext == 1, "following the external clock");
    for (i = 0, k = 0; (i = find(E_DRUM, 0, i)) >= 0; i++, k++)
        CHECK((evs[i].t - t_start) == (uint32_t)k * 6u * per_tick, "ext step %d at +%u, want tick %d", k, evs[i].t - t_start, 6 * k);
    }
    CHECK(k == 16, "16 steps in 96 ticks, got %d", k);
    CHECK(S.ext_bpm > 119.0f && S.ext_bpm < 121.0f, "tempo estimate %.2f", (double)S.ext_bpm);
    run(44100);                                               /* clock gone: back to internal */
    CHECK(S.ext == 0, "internal again after the clock stops");
    /* MIDI clock out: 24 per quarter */
    reset();
    S.send_clock = 1;
    S.bpm = 120.0f;
    seq_start(&S);
    run(44100 * 2 - 1);                                       /* 4 quarters, not the next downbeat */
    for (i = 0, clocks = 0; i < nev; i++)
        if (evs[i].kind == E_MIDI && evs[i].a == 0xF8)
            clocks++;
    CHECK(clocks == 96, "MIDI clocks in 2 s at 120 BPM: %d, want 96", clocks);
    CHECK(find(E_MIDI, 0xFA, 0) == 0, "Start sent first");
    seq_stop(&S);
    run(256);
    CHECK(find(E_MIDI, 0xFC, 0) > 0, "Stop sent");
    printf("  external clock follow + tempo, MIDI clock out: checked\n");
}

static void test_break_clock(void)
{
    int i, k;
    reset();
    S.bpm = 170.0f;
    P[0].drum[0].rate = RATE_16T;                                /* the break stays on 16ths regardless */
    P[0].drum[0].len = 12;
    P[0].brk.steps = 0x00FFu;                                 /* first half of the bar */
    seq_start(&S);
    run(44100 * 3);
    for (i = 0, k = 0; (i = find(E_BRK, -1, i)) >= 0; i++, k++) {
        CHECK(evs[i].a == k % 16, "break step %d is %d", k, evs[i].a);
        CHECK(evs[i].b == k / 16, "break bar %d", evs[i].b);
        CHECK(evs[i].d == (k % 16 < 8), "break step %d enabled=%d", k, evs[i].d);
    }
    CHECK(k > 16 * 2, "break clocked %d times", k);
    seq_stop(&S);
    run(256);
    CHECK(find(E_BRKSTOP, -1, 0) >= 0, "break told to stop");
    printf("  break clock: %d 16ths, bars counted, step mask honoured\n", k);
}


int main(void)
{
    test_drum_timing();
    test_late_hit();
    test_swing_and_rates();
    test_polymeter_cue_chain();
    test_part_cue();
    test_song();
    test_ext_clock_and_midi_out();
    test_break_clock();
    printf("seq: %s\n", fails ? "FAIL" : "ok");
    return fails != 0;
}
