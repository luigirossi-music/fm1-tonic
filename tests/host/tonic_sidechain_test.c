/* SPDX-License-Identifier: GPL-3.0-only */
/* wip348: the SIDECHAIN. The input voice's SOUND ducks the voices of the mask: a short kick note = a short duck, a long one = a long duck;
 * intensity 0 = nothing changes (bit for bit); a voice outside the mask is not touched; the input itself is never ducked; a silent input does not duck.
 * Layout: the kick (input) is panned hard left, the bass (ducked) hard right, so the right channel is the bass alone. */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "dsp/tonic.h"

#define N 24000                                        /* 0.5 s */
static float l[N], r[N];
static void run(tonic_t *t, int n)
{
    memset(l, 0, sizeof l);
    memset(r, 0, sizeof r);
    tonic_render_st(t, l, r, n);
}
static void setup(tonic_t *t, int kick_dcy)
{
    int v;
    tonic_init(t, 48000);
    for (v = 0; v < 3; v++) {
        t->p[v][TNP_LEVEL] = 52000;
        t->p[v][TNP_OSCDCY] = 60000;                   /* the bass and the outside voice ring all the time */
        t->p[v][TNP_NENVDCY] = 60000;
    }
    t->p[0][TNP_OSCFREQ] = 12000;                      /* voice 0 = the kick: low, its length is the parameter */
    t->p[0][TNP_OSCDCY] = (uint16_t)kick_dcy;
    t->p[0][TNP_NENVDCY] = (uint16_t)kick_dcy;
    t->p[1][TNP_OSCFREQ] = 30000;                      /* voice 1 = the bass */
    t->p[2][TNP_OSCFREQ] = 36000;                      /* voice 2 = outside the mask, hard right too */
    tonic_set_pan(t, 0, 0);
    tonic_set_pan(t, 1, 127);
    tonic_set_pan(t, 2, 127);
}
static float rms(const float *x, int a, int b)
{
    double s = 0;
    int k;
    for (k = a; k < b; k++) s += (double)x[k] * x[k];
    return (float)sqrt(s / (double)(b - a));
}
static int bad;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL: " __VA_ARGS__); printf("\n"); bad = 1; } } while (0)
int main(void)
{
    static tonic_t t, u;
    static float r_free[N], l_free[N];
    float free_rms, duck_rms, after_rms, dur_short, dur_long;
    int k;
    /* the bass alone, no sidechain: the reference (kick not played) */
    setup(&t, 60000);
    tonic_trigger(&t, 1, 127);
    run(&t, 4800);                                     /* past the attack */
    run(&t, N);
    memcpy(r_free, r, sizeof r);
    free_rms = rms(r_free, 0, 2400);
    CHECK(free_rms > 0.001f, "the bass is silent (%.5f)", free_rms);
    /* intensity 0 with an input and a mask = the same as no sidechain, bit for bit */
    setup(&t, 3000); setup(&u, 3000);
    tonic_set_sidechain(&u, 0, 0, 6);
    tonic_trigger(&t, 0, 127); tonic_trigger(&t, 1, 127); tonic_trigger(&u, 0, 127); tonic_trigger(&u, 1, 127);
    run(&t, N); memcpy(l_free, l, sizeof l); memcpy(r_free, r, sizeof r);
    run(&u, N);
    CHECK(!memcmp(l, l_free, sizeof l) && !memcmp(r, r_free, sizeof r), "intensity 0 changed the sound");
    /* the kick plays: the bass (right) drops while the kick sounds and comes back after (compared with the same bass without the sidechain) */
    setup(&t, 3000);                                   /* a short kick */
    tonic_trigger(&t, 1, 127);
    run(&t, 4800);
    tonic_trigger(&t, 0, 127);
    run(&t, N); memcpy(r_free, r, sizeof r);           /* the reference: no sidechain */
    setup(&t, 3000);
    tonic_trigger(&t, 1, 127);
    run(&t, 4800);
    tonic_trigger(&t, 0, 127);
    tonic_set_sidechain(&t, 0, 127, 2);
    run(&t, N);
    duck_rms = 9.0f;                                   /* the deepest 5 ms window in the first 40 ms of the kick */
    for (k = 0; k < 1920; k += 240) { float q = rms(r, k, k + 240) / rms(r_free, k, k + 240); if (q < duck_rms) duck_rms = q; }
    after_rms = rms(r, N - 4800, N) / rms(r_free, N - 4800, N);
    printf("  the bass at the deepest = %.2f of its level, 0.4 s later %.2f\n", duck_rms, after_rms);
    CHECK(duck_rms < 0.6f, "the bass is not ducked enough while the kick sounds (%.2f)", duck_rms);
    CHECK(after_rms > 0.95f, "the bass does not come back (%.2f)", after_rms);
    /* the duck is as long as the kick: how many ms the bass stays under 70 % of its level */
    {
        int kd[2] = {2000, 50000}, w, ms[2];
        for (w = 0; w < 2; w++) {
            setup(&t, kd[w]);
            tonic_trigger(&t, 1, 127);
            run(&t, 4800);
            tonic_trigger(&t, 0, 127);
            run(&t, N); memcpy(r_free, r, sizeof r);
            setup(&t, kd[w]);
            tonic_trigger(&t, 1, 127);
            run(&t, 4800);
            tonic_trigger(&t, 0, 127);
            tonic_set_sidechain(&t, 0, 127, 2);
            run(&t, N);
            ms[w] = 0;
            for (k = 0; k + 480 <= N; k += 480)
                if (rms(r, k, k + 480) < 0.7f * rms(r_free, k, k + 480)) ms[w] = (k + 480) / 48;
            printf("  kick decay %d: the bass is ducked for ~%d ms\n", kd[w], ms[w]);
        }
        dur_short = (float)ms[0]; dur_long = (float)ms[1];
        CHECK(dur_long > 2.0f * dur_short, "a long kick does not duck longer (%.0f vs %.0f ms)", dur_long, dur_short);
    }
    /* a voice outside the mask is not touched; the input is not ducked */
    setup(&t, 3000); setup(&u, 3000);
    tonic_trigger(&t, 2, 127); tonic_trigger(&t, 0, 127);
    tonic_trigger(&u, 2, 127); tonic_trigger(&u, 0, 127);
    tonic_set_sidechain(&u, 0, 127, 2);                /* only voice 1 is in the mask: voice 2 and the kick must be as without */
    run(&t, N); memcpy(l_free, l, sizeof l); memcpy(r_free, r, sizeof r);
    run(&u, N);
    CHECK(!memcmp(l, l_free, sizeof l) && !memcmp(r, r_free, sizeof r), "a voice outside the mask (or the input) was ducked");
    /* a silent input (not played) does not duck */
    setup(&t, 3000);
    tonic_set_sidechain(&t, 0, 127, 2);
    tonic_trigger(&t, 1, 127);
    run(&t, 4800);
    run(&t, N);
    duck_rms = rms(r, 0, 2400);
    CHECK(duck_rms > 0.001f, "a silent input ducked the bass (%.5f)", duck_rms);
    /* the input inside its own mask is not ducked (the setter drops it) */
    tonic_set_sidechain(&t, 3, 127, 8);
    CHECK(t.sc_mask == 0, "the input stayed in the mask (%d)", t.sc_mask);
    printf(bad ? "tonic_sidechain: FAIL\n" : "tonic_sidechain: ok\n");
    return bad;
}
