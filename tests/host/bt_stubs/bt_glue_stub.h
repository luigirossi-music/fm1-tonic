/* SPDX-License-Identifier: GPL-3.0-only */
/* HOST SYNTAX CHECK ONLY (tests/host/bt_syntax_check.sh). NOT the glue: a minimal stand-in for what fm1-base's own
 * glue gives the Classic stack (the build switches, the app's Bluetooth state "bta", the flash read, the audio
 * constants), only so bt_inq.c / bt_page.c / bt_acl.c / bt_audio.c can be compiled with -fsyntax-only. The real glue
 * (HANDOFF_155, NEXT) will decide these names and types for Tonic; when it exists this file goes. Field names and
 * types here are guessed from how the stack uses them. */
#pragma once
#include <stdint.h>
struct ble_vm_info { uint32_t rsv; };
struct ble_rf_trims { uint32_t x106, x107, x108, x187; };
#define BTA_DEVS 8u
#define BTA_MEMS 5u
#define BTA_KEPT 4u
#define BTA_PAGE_SLOTS 3200u
#define BTA_SCAN_S 10u
enum { BTA_OFF, BTA_AUTO, BTA_PICK };
enum { BTA_S_OFF, BTA_S_SCANNING, BTA_S_FOUND, BTA_S_NONE, BTA_S_PAGING, BTA_S_LINKED, BTA_S_SETUP, BTA_S_STREAMING,
       BTA_S_REFUSED, BTA_S_NO_LAST, BTA_S_FAILED, BTA_S_READY };
struct bta_mem { uint8_t addr[6], lk[16], bonded; char name[17]; };
struct bta_dev { uint8_t addr[6], audio; char name[17]; };
struct { uint8_t mode, n, cur, nmem, page_i, tried, key_new, own[6], own_set; struct bta_dev d[BTA_DEVS]; struct bta_mem mem[BTA_MEMS]; } bta;
static struct bta_mem *bta_mem_of(const uint8_t a[6]);
extern int flash_ok;
int st_read(uint32_t off, uint8_t *dst, uint32_t n);
extern volatile uint32_t fm1_ms_v;
#define fm1_ms fm1_ms_v
#define FELUCCA_CDC 0
#define HALF_FRAMES 0
#define SY_TPS_Q8 256
