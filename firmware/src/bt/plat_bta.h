/* SPDX-License-Identifier: MPL-2.0 */
/* The calls of Bluetooth audio out (an A2DP source to headphones and speakers), copied from fm1-base's include/plat.h
 * (https://github.com/charlesvestal/fm1-base, MPL-2.0, Copyright (C) 2026 Charles Vestal and the fm1-base authors) as it
 * stands on 6 Nov 2026, with this header added and nothing else changed. THIS FILE IS A DECLARATION ONLY: nothing in
 * FM1 Tonic includes it or implements these calls yet (HANDOFF_152, step 2). Under MPL-2.0 a change to this file is
 * published under MPL-2.0.
 * (The comments below speak of fm1.toml, FM1B_* and OBJ_BLE: they are fm1-base's build and store; Tonic will have its own.) */
#pragma once
#include <stdint.h>

/* Bluetooth audio out (fm1.toml bt_audio = true, with ble_stack = "own": FM1B_BT_AUDIO 1): an A2DP source to headphones
 * and speakers (SBC), beside Bluetooth MIDI on the one radio. While it streams, the render goes to the device and the
 * FM-1's own output is muted. Four devices are remembered with their keys (in OBJ_BLE), the last one first. Expect
 * 150-300 ms from a key to the ear (the device's own buffer): for listening, not for playing in time. */
#ifndef FM1B_BT_AUDIO
#define FM1B_BT_AUDIO 0
#endif
enum { PLAT_BTA_OFF, PLAT_BTA_SCANNING, PLAT_BTA_FOUND, PLAT_BTA_NONE, PLAT_BTA_PAGING, PLAT_BTA_LINKED, PLAT_BTA_SETUP,
       PLAT_BTA_STREAMING, PLAT_BTA_REFUSED, PLAT_BTA_NO_LAST, PLAT_BTA_FAILED, PLAT_BTA_READY };
void plat_bta_on(int on);              /* on: the radio up (Bluetooth MIDI's start, if not yet), ready to scan / connect */
void plat_bta_scan(int on);            /* look for devices (about 10 s): 1 a fresh list, 2 again with the list kept
                                        * (only when nothing else is under way: a page goes on), 0 stop */
int plat_bta_found(int i, char *name); /* found device i: its name (17 bytes) -> 0: none */
int plat_bta_known(int i, char *name); /* remembered device i (0 the last one) -> 0: none */
void plat_bta_pick(int i);             /* connect found device i (pairing if new; it becomes the last one) */
int plat_bta_reconnect(int i);         /* connect remembered device i -> 0 paging */
void plat_bta_forget(int i);
void plat_bta_hangup(void);
int plat_bta_state(void);              /* PLAT_BTA_* */
/* while it streams, where the sound goes: 1 the device, 0 the FM-1's own output (the stream suspended, the link
 * kept: back to the device at once, without a new connection) */
void plat_bta_output(int to_device);
int plat_bta_out_device(void);         /* 1: the sound goes to the device (the default) */
/* a link that ended by itself (the device left, or refused): where it had got to, once (then PLAT_BTA_LOST_NONE) */
enum { PLAT_BTA_LOST_NONE, PLAT_BTA_LOST_LINK, PLAT_BTA_LOST_PAIRING, PLAT_BTA_LOST_ENCRYPTION, PLAT_BTA_LOST_SETUP,
       PLAT_BTA_LOST_STREAMING, PLAT_BTA_LOST_REFUSED };
int plat_bta_lost(void);
int plat_bta_timed_out(void);          /* PLAT_BTA_REFUSED because the set-up timed out (the device never said no) */
int32_t plat_bta_latency_us(void);     /* what the stream adds on the FM-1's side (its queue), us; -1: not streaming.
                                        * An app that follows a clock may play that much earlier (the device's own
                                        * buffer comes on top and is not known) */
/* Bluetooth audio's big buffers in the app's memory (fm1.toml bt_audio_mem = "app": FM1B_BTA_MEM 1): the audio packets'
 * queue and the receptions' ring, which only a run of the Classic radio uses (a scan, a page, a link), are a block the
 * app lends while an AUDIO mode is on, instead of static RAM: an app that switches Bluetooth audio off has that RAM
 * back. Without it (FM1B_BTA_MEM 0) they are static and these say so (bytes 0, the block refused).
 *   plat_bta_mem_bytes()  the block's size (4-aligned), 0: static buffers
 *   plat_bta_mem(p)       p: the block lent (before plat_bta_on(1), which does nothing without one) -> 0;
 *                         0: taken back -> 0 done (the radio's code and interrupts no longer read it), -1 still in use
 *                         (an AUDIO mode on, or its run not over yet: plat_bta_on(0) first, then again later) */
#ifndef FM1B_BTA_MEM
#define FM1B_BTA_MEM 0
#endif
uint32_t plat_bta_mem_bytes(void);
int plat_bta_mem(void *p);
int plat_bta_streams(void);            /* 1: the last render went to Bluetooth (the FM-1's own output muted for it); for the
                                        * render (the audio interrupt) to plan its block by. 0 without Bluetooth audio */
