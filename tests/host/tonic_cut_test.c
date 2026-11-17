/* SPDX-License-Identifier: GPL-3.0-only
 * wip318: LOW CUT / HIGH CUT have no OFF: the filters are always in the path, so turning a cut from its end position
 * in the middle of a hit makes no step (before: the high cut switched in with a jump), and the default positions
 * (20 Hz / 20 kHz) leave the sound as it was (same level within 2 %). */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "dsp/tonic.h"
static tonic_t T;
static float L[256], R[256];
static void run(int lo, int hi, int midlo, int midhi, double *rms, float *jump)
{
    int i, b;
    double e = 0;
    float prev = 0;
    *jump = 0;
    tonic_init(&T, 44100);
    tonic_set_pot(&T, 0, TN_POT_LOSH, lo);
    tonic_set_pot(&T, 0, TN_POT_HISH, hi);
    tonic_trigger(&T, 0, 127);
    for (b = 0; b < 16; b++) {
        memset(L, 0, sizeof L);
        memset(R, 0, sizeof R);
        if (b == 4) {
            if (midlo >= 0) tonic_set_pot(&T, 0, TN_POT_LOSH, midlo);
            if (midhi >= 0) tonic_set_pot(&T, 0, TN_POT_HISH, midhi);
        }
        tonic_render_st(&T, L, R, 256);
        for (i = 0; i < 256; i++) {
            e += (double)L[i] * L[i];
            if ((b || i) && fabsf(L[i] - prev) > *jump) *jump = fabsf(L[i] - prev);
            prev = L[i];
        }
    }
    *rms = sqrt(e / 4096.0);
}
int main(void)
{
    double r0, r1;
    float j0, j1;
    int bad = 0;
    run(0, 127, -1, -1, &r0, &j0);
    if (r0 < 0.01) { printf("FAIL: the default sound is silent (rms %.5f)\n", r0); bad = 1; }
    run(0, 127, 1, -1, &r1, &j1);
    if (j1 > j0 * 1.3f + 1e-4f) { printf("FAIL: low cut 0 -> 1 steps: %.5f vs %.5f\n", j1, j0); bad = 1; }
    run(0, 127, -1, 126, &r1, &j1);
    if (j1 > j0 * 1.3f + 1e-4f) { printf("FAIL: high cut 127 -> 126 steps: %.5f vs %.5f\n", j1, j0); bad = 1; }
    run(0, 127, -1, 120, &r1, &j1);
    if (j1 > j0 * 1.3f + 1e-4f) { printf("FAIL: high cut 127 -> 120 steps: %.5f vs %.5f\n", j1, j0); bad = 1; }
    run(0, 127, -1, -1, &r1, &j1);
    if (fabs(r1 - r0) > 0.02 * r0) { printf("FAIL: level moved\n"); bad = 1; }
    if (tonic_shelf_hz(0, 0) != 20 || tonic_shelf_hz(1, 127) != 20000) { printf("FAIL: the ends are %d Hz / %d Hz, not 20 / 20000\n", tonic_shelf_hz(0, 0), tonic_shelf_hz(1, 127)); bad = 1; }
    if (!bad) printf("tonic cut test: ok (default rms %.4f, step %.4f)\n", r0, j0);
    return bad;
}
