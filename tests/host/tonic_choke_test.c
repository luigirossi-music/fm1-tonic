/* SPDX-License-Identifier: GPL-3.0-only */
/* wip328: the CHOKE group: a hit of a voice in the group stops the others of the group (3 ms fade); a voice outside the group and
 * a hit without a group are not touched; the choked voice can play again at once. */
#include <stdio.h>
#include <string.h>
#include "dsp/tonic.h"

static float l[4800], r[4800];
static void run(tonic_t *t, int n)
{
    memset(l, 0, sizeof l);
    memset(r, 0, sizeof r);
    tonic_render_st(t, l, r, n);
}
static void long_voice(tonic_t *t, int v)
{
    t->p[v][TNP_OSCDCY] = 60000;                       /* a long decay: it would ring for seconds */
    t->p[v][TNP_NENVDCY] = 60000;
    t->p[v][TNP_LEVEL] = 52000;
}
int main(void)
{
    static tonic_t t;
    int bad = 0, v, k;
    float peak;
    tonic_init(&t, 48000);
    for (v = 0; v < 3; v++)
        long_voice(&t, v);
    /* no group: voice 0 keeps ringing when voice 1 plays */
    tonic_trigger(&t, 0, 127); run(&t, 2400);
    tonic_trigger(&t, 1, 127); run(&t, 2400);
    if (!t.active[0]) { printf("FAIL: no group, but voice 1 stopped voice 0\n"); bad = 1; }
    /* the group {0, 1}: voice 1 stops voice 0, voice 2 (outside) is not touched */
    tonic_init(&t, 48000);
    for (v = 0; v < 3; v++)
        long_voice(&t, v);
    tonic_set_choke(&t, 3);
    tonic_trigger(&t, 0, 127);
    tonic_trigger(&t, 2, 127);
    run(&t, 2400);
    tonic_trigger(&t, 1, 127);
    run(&t, 480);                                      /* 10 ms: the 3 ms fade is over */
    if (t.active[0]) { printf("FAIL: voice 0 was not choked\n"); bad = 1; }
    if (!t.active[2]) { printf("FAIL: voice 2 (outside the group) was choked\n"); bad = 1; }
    if (!t.active[1]) { printf("FAIL: the hitting voice stopped\n"); bad = 1; }
    /* the fade: the choked voice alone is silent after 3 ms (no click: it ramps) */
    tonic_init(&t, 48000);
    long_voice(&t, 0); long_voice(&t, 1);
    tonic_set_choke(&t, 3);
    tonic_trigger(&t, 0, 127);
    run(&t, 2400);
    t.active[1] = 0;
    t.kill[0] = 144;                                   /* what a hit of voice 1 sets */
    run(&t, 144);
    peak = 0.0f;
    for (k = 0; k < 144; k++) { float a = l[k] < 0 ? -l[k] : l[k]; if (a > peak) peak = a; }
    run(&t, 480);
    for (k = 0; k < 480; k++) if (l[k] != 0.0f) { printf("FAIL: not silent after the fade\n"); bad = 1; break; }
    if (t.active[0]) { printf("FAIL: voice 0 still active after the fade\n"); bad = 1; }
    /* played again at once after a choke */
    tonic_trigger(&t, 0, 127);
    run(&t, 480);
    peak = 0.0f;
    for (k = 0; k < 480; k++) { float a = l[k] < 0 ? -l[k] : l[k]; if (a > peak) peak = a; }
    if (!t.active[0] || peak < 0.001f) { printf("FAIL: voice 0 does not play again after a choke (peak %.4f)\n", peak); bad = 1; }
    printf(bad ? "tonic_choke: FAIL\n" : "tonic_choke: ok\n");
    return bad;
}
