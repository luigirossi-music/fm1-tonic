/* SPDX-License-Identifier: GPL-3.0-only */
/* The Bluetooth audio stand-in (firmware/src/bt/plat_bta_sim.c): scan, found, connect, stream, lost, remembered. */
#include <stdio.h>
#include <string.h>
#include "../../firmware/src/bt/plat_bta.h"
void plat_bta_sim_time(uint32_t ms);
void plat_bta_sim_refuse(int refuse);
void plat_bta_sim_drop(int why);
static int bad;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); bad = 1; } } while (0)
int main(void)
{
    char n[17];
    plat_bta_sim_time(1000);
    CHECK(plat_bta_state() == PLAT_BTA_OFF);
    plat_bta_scan(1);                                   /* radio off: nothing happens */
    CHECK(plat_bta_state() == PLAT_BTA_OFF);
    plat_bta_on(1);
    CHECK(plat_bta_state() == PLAT_BTA_NONE);
    plat_bta_scan(1);
    CHECK(plat_bta_state() == PLAT_BTA_SCANNING && !plat_bta_found(0, n));
    plat_bta_sim_time(1499);
    CHECK(!plat_bta_found(0, n));                       /* 0.499 s: not yet */
    plat_bta_sim_time(1500);
    CHECK(plat_bta_found(0, n) && !strcmp(n, "Speaker") && !plat_bta_found(1, n));
    plat_bta_sim_time(10999);
    CHECK(plat_bta_state() == PLAT_BTA_SCANNING);
    plat_bta_sim_time(11000);
    CHECK(plat_bta_state() == PLAT_BTA_NONE);           /* the scan is over after 10 s */
    CHECK(plat_bta_known(0, n) == 0);
    plat_bta_scan(2);                                   /* again, list kept: found at once */
    CHECK(plat_bta_found(0, n));
    plat_bta_pick(0);
    CHECK(plat_bta_state() == PLAT_BTA_PAGING && plat_bta_latency_us() == -1 && !plat_bta_streams());
    plat_bta_sim_time(11299);
    CHECK(plat_bta_state() == PLAT_BTA_PAGING);
    plat_bta_sim_time(11300);
    CHECK(plat_bta_state() == PLAT_BTA_STREAMING && plat_bta_streams() && plat_bta_latency_us() > 0);
    CHECK(plat_bta_known(0, n) && !strcmp(n, "Speaker") && !plat_bta_known(1, n));
    plat_bta_output(0);                                 /* the FM-1's own output: link kept */
    CHECK(plat_bta_state() == PLAT_BTA_STREAMING && !plat_bta_streams() && !plat_bta_out_device());
    plat_bta_output(1);
    CHECK(plat_bta_streams());
    plat_bta_sim_drop(0);                               /* the device leaves */
    CHECK(plat_bta_state() == PLAT_BTA_NONE && plat_bta_lost() == PLAT_BTA_LOST_LINK && plat_bta_lost() == PLAT_BTA_LOST_NONE);
    CHECK(plat_bta_reconnect(1) == -1 && plat_bta_reconnect(0) == 0 && plat_bta_state() == PLAT_BTA_PAGING);
    plat_bta_sim_time(11700);
    CHECK(plat_bta_state() == PLAT_BTA_STREAMING);
    plat_bta_hangup();
    CHECK(plat_bta_state() == PLAT_BTA_NONE && plat_bta_lost() == PLAT_BTA_LOST_NONE);
    plat_bta_sim_refuse(1);                             /* refused */
    CHECK(plat_bta_reconnect(0) == 0);
    plat_bta_sim_time(12000);
    CHECK(plat_bta_state() == PLAT_BTA_REFUSED && plat_bta_lost() == PLAT_BTA_LOST_REFUSED && !plat_bta_timed_out());
    plat_bta_sim_refuse(0);
    plat_bta_forget(0);
    CHECK(!plat_bta_known(0, n) && plat_bta_reconnect(0) == -1);
    plat_bta_on(0);
    CHECK(plat_bta_state() == PLAT_BTA_OFF && !plat_bta_streams());
    CHECK(plat_bta_mem_bytes() == 0 && plat_bta_mem((void *)n) == -1 && plat_bta_mem(0) == 0);
    puts(bad ? "bta_sim: FAILED" : "bta_sim: all ok");
    return bad;
}
