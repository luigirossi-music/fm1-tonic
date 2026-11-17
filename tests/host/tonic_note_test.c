/* SPDX-License-Identifier: GPL-3.0-only */
/* wip251: tonic_trigger_note: a hit at +12 semitones sounds an octave up (twice the frequency), -12 an octave down, 0 is the plain
 * tonic_trigger, and the stored OscFreq is not changed by any of them. Frequency = zero crossings of the first 0.2 s. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsp/tonic.h"

static float hz_of(tonic_t *t, int semis)
{
    static float l[9600], r[9600];
    int i, n = 0, N = 9600;
    memset(l, 0, sizeof l);
    memset(r, 0, sizeof r);
    tonic_trigger_note(t, 0, 127, semis);
    tonic_render_st(t, l, r, N);
    for (i = 1; i < N; i++)
        if ((l[i - 1] < 0.0f) != (l[i] < 0.0f))
            n++;
    return (float)n / 2.0f / ((float)N / (float)t->sr);
}

int main(void)
{
    static tonic_t t;
    float f0, fu, fd, fp;
    uint16_t before;
    int bad = 0;
    tonic_init(&t, 48000);
    before = t.p[0][TNP_OSCFREQ];
    f0 = hz_of(&t, 0);
    fu = hz_of(&t, 12);
    fd = hz_of(&t, -12);
    fp = hz_of(&t, 7);
    printf("0: %.1f Hz  +12: %.1f  -12: %.1f  +7: %.1f\n", f0, fu, fd, fp);
    if (f0 < 100.0f || fu / f0 < 1.95f || fu / f0 > 2.05f) { printf("FAIL: +12 is not an octave up\n"); bad = 1; }
    if (fd / f0 < 0.48f || fd / f0 > 0.52f) { printf("FAIL: -12 is not an octave down\n"); bad = 1; }
    if (fp / f0 < 1.46f || fp / f0 > 1.53f) { printf("FAIL: +7 is not a fifth\n"); bad = 1; }   /* 2^(7/12) = 1.498 */
    if (t.p[0][TNP_OSCFREQ] != before) { printf("FAIL: the stored pitch changed\n"); bad = 1; }
    tonic_trigger_note(&t, 0, 127, 63);                 /* the top of the range: clamped, no crash */
    tonic_trigger_note(&t, 0, 127, -64);
    return bad;
}
