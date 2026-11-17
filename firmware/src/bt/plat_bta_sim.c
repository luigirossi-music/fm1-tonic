/* SPDX-License-Identifier: MPL-2.0 */
/* A stand-in for Bluetooth audio out, for the HOST simulator only (never part of the firmware image): it implements the
 * plat_bta_* calls of plat_bta.h with a fake radio, so scenarios and tests can run the scan / connect / stream path
 * without hardware. Its behaviour is LOOM-8's (https://github.com/charlesvestal/fm1-loom, app/bt.c, MPL-2.0, Copyright (C)
 * 2026 Charles Vestal and the fm1-loom authors; rewritten here as the whole set of calls, with a clock the host sets):
 *   - one device, "Speaker", is found 0.5 s into a scan; a scan lasts 10 s;
 *   - a connect (pick or reconnect) streams 0.3 s later;
 *   - four devices are remembered, the last one first.
 * Beyond Loom's (made up here, for tests): a link can be cut by plat_bta_sim_drop(), which sets plat_bta_lost(); a
 * pick can be made to be refused with plat_bta_sim_refuse(1); the stream's latency is a fixed 30000 us.
 * Time: the host calls plat_bta_sim_time(ms) as its clock goes on (the host simulator's plat_ms()). Under MPL-2.0 a
 * change to this file is published under MPL-2.0. */
#include <stdint.h>
#include <string.h>
#include "plat_bta.h"

#define SIM_NAME "Speaker"
#define SIM_SCAN_FIND_MS 500u
#define SIM_SCAN_END_MS 10000u
#define SIM_CONNECT_MS 300u
#define SIM_LATENCY_US 30000

static struct {
    uint8_t on, st, found, nk, to_dev, refuse, lost, timed_out;
    uint32_t t, now;
    char k[4][17];
} sb = { .to_dev = 1 };

void plat_bta_sim_time(uint32_t ms) { sb.now = ms; }
void plat_bta_sim_refuse(int refuse) { sb.refuse = (uint8_t)(refuse != 0); }
void plat_bta_sim_drop(int why)                      /* the device leaves (a PLAT_BTA_LOST_* reason, 0: LINK) */
{
    if (sb.on && sb.st >= PLAT_BTA_PAGING && sb.st <= PLAT_BTA_STREAMING) {
        sb.st = PLAT_BTA_NONE;
        sb.lost = (uint8_t)(why > 0 ? why : PLAT_BTA_LOST_LINK);
    }
}

static void put_name(char *dst, const char *src)
{
    if (!dst)
        return;
    memset(dst, 0, 17);
    strncpy(dst, src, 16);
}

void plat_bta_on(int on)
{
    if (!on == !sb.on)
        return;
    sb.on = (uint8_t)(on != 0);
    sb.st = on ? PLAT_BTA_NONE : PLAT_BTA_OFF;
    sb.found = 0;
    sb.lost = 0;
    sb.timed_out = 0;
}

void plat_bta_scan(int on)
{
    if (!sb.on)
        return;
    if (on) {
        if (on == 1)
            sb.found = 0;                            /* 2: again, the list kept */
        sb.st = PLAT_BTA_SCANNING;
        sb.t = sb.now;
    } else if (sb.st == PLAT_BTA_SCANNING)
        sb.st = sb.found ? PLAT_BTA_FOUND : PLAT_BTA_NONE;
}

int plat_bta_state(void)
{
    uint32_t dt = sb.now - sb.t;
    if (sb.st == PLAT_BTA_SCANNING) {
        if (dt >= SIM_SCAN_FIND_MS)
            sb.found = 1;
        if (dt >= SIM_SCAN_END_MS)
            sb.st = PLAT_BTA_NONE;
    }
    if (sb.st == PLAT_BTA_PAGING && dt >= SIM_CONNECT_MS) {
        if (sb.refuse) {
            sb.st = PLAT_BTA_REFUSED;
            sb.lost = PLAT_BTA_LOST_REFUSED;
        } else
            sb.st = PLAT_BTA_STREAMING;
    }
    return sb.st;
}

int plat_bta_found(int i, char *name)
{
    (void)plat_bta_state();
    if (i != 0 || !sb.found)
        return 0;
    put_name(name, SIM_NAME);
    return 1;
}

int plat_bta_known(int i, char *name)
{
    if (i < 0 || i >= sb.nk)
        return 0;
    put_name(name, sb.k[i]);
    return 1;
}

static void connect_now(void)
{
    sb.st = PLAT_BTA_PAGING;
    sb.t = sb.now;
    sb.lost = 0;
    sb.timed_out = 0;
    sb.to_dev = 1;
}

void plat_bta_pick(int i)
{
    if (!sb.on || !plat_bta_found(i, 0))
        return;
    if (!sb.nk || strcmp(sb.k[0], SIM_NAME) != 0) {
        for (int j = sb.nk < 4 ? sb.nk++ : 3; j > 0; j--)
            memcpy(sb.k[j], sb.k[j - 1], sizeof sb.k[0]);
        put_name(sb.k[0], SIM_NAME);
    }
    connect_now();
}

int plat_bta_reconnect(int i)
{
    if (!sb.on || i < 0 || i >= sb.nk)
        return -1;
    connect_now();
    return 0;
}

void plat_bta_forget(int i)
{
    if (i < 0 || i >= sb.nk)
        return;
    for (sb.nk--; i < sb.nk; i++)
        memcpy(sb.k[i], sb.k[i + 1], sizeof sb.k[0]);
    memset(sb.k[sb.nk], 0, sizeof sb.k[0]);
}

void plat_bta_hangup(void)
{
    if (sb.on && sb.st != PLAT_BTA_SCANNING)
        sb.st = PLAT_BTA_NONE;
}

void plat_bta_output(int to_device) { sb.to_dev = (uint8_t)(to_device != 0); }
int plat_bta_out_device(void) { return sb.to_dev; }

int plat_bta_lost(void)                              /* once, then NONE */
{
    int l = sb.lost;
    sb.lost = 0;
    return l;
}

int plat_bta_timed_out(void) { return sb.timed_out; }   /* the stand-in never times out */

int32_t plat_bta_latency_us(void) { return plat_bta_state() == PLAT_BTA_STREAMING ? SIM_LATENCY_US : -1; }

uint32_t plat_bta_mem_bytes(void) { return 0; }      /* static buffers: no block to lend */
int plat_bta_mem(void *p) { return p ? -1 : 0; }     /* a block is refused; taking back nothing is done */

int plat_bta_streams(void) { return plat_bta_state() == PLAT_BTA_STREAMING && sb.to_dev; }
