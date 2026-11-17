/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Host test of firmware/src/storage.c against a simulated NOR flash:
 * erase -> 0xFF, program can only clear bits, page writes must not wrap. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t nor[0x100000];
static int fail_after = -1;            /* torn-write injection: stop after N programs */

static int st_read(uint32_t off, void *dst, uint32_t n) { memcpy(dst, nor + off, n); return 0; }
static int st_erase(uint32_t off) { memset(nor + off, 0xFF, 4096); return 0; }
static int st_prog(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    uint32_t i;
    if (fail_after == 0)
        return -9;
    if (fail_after > 0)
        fail_after--;
    if ((off & 0xFFu) + n > 256u) {
        printf("page wrap at %#x\n", off);
        exit(1);
    }
    for (i = 0; i < n; i++)
        nor[off + i] &= s[i];
    return 0;
}
#include "../firmware/src/app/plat.h"     /* X0X: the object map */
#include "../firmware/src/storage.c"
#define OBJ_PROJECT0 OBJ_SOUND                 /* the test's three data objects: SOUND, PAT0, PAT1 */
#define OBJ_SETTINGS OBJ_SET

static int check(const char *what, int ok)
{
    printf("%-46s %s\n", what, ok ? "ok" : "FAIL");
    return ok ? 0 : 1;
}

int main(void)
{
    char a[600], b[600], got[600];
    int bad = 0, n;
    memset(nor, 0xFF, sizeof nor);
    memset(a, 'A', sizeof a);
    memset(b, 'B', sizeof b);
    bad += check("empty flash loads nothing", st_load(OBJ_PROJECT0, got, sizeof got) < 0);
    bad += check("save A", st_save(OBJ_PROJECT0, a, sizeof a) == 0);
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("load returns A", n == (int)sizeof a && !memcmp(got, a, sizeof a));
    bad += check("save B", st_save(OBJ_PROJECT0, b, sizeof b) == 0);
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("load returns B (newer seq)", n == (int)sizeof b && !memcmp(got, b, sizeof b));
    fail_after = 1;                                  /* payload page 1 written, the rest torn */
    st_save(OBJ_PROJECT0, a, sizeof a);
    fail_after = -1;
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("torn save keeps B", n == (int)sizeof b && !memcmp(got, b, sizeof b));
    fail_after = 3;                                  /* all payload pages, header torn */
    st_save(OBJ_PROJECT0, a, sizeof a);
    fail_after = -1;
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("header not written keeps B", n == (int)sizeof b && !memcmp(got, b, sizeof b));
    bad += check("save A again", st_save(OBJ_PROJECT0, a, sizeof a) == 0);
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("load returns A", n == (int)sizeof a && !memcmp(got, a, sizeof a));
    n = st_load_prev(OBJ_PROJECT0, got, sizeof got);
    bad += check("the copy before A is B", n == (int)sizeof b && !memcmp(got, b, sizeof b));
    bad += check("no copy before on an empty object", st_load_prev(OBJ_MOTION, got, sizeof got) < 0);
    nor[st_sector(OBJ_PROJECT0, 0) + ST_PAYLOAD_OFF + 10] ^= 0x01;   /* bit rot in one copy */
    nor[st_sector(OBJ_PROJECT0, 1) + ST_PAYLOAD_OFF + 10] ^= 0x01;
    n = st_load(OBJ_PROJECT0, got, sizeof got);
    bad += check("both copies corrupt -> nothing", n < 0);
    bad += check("other objects untouched", st_load(OBJ_PROJECT0 + 1, got, sizeof got) < 0);
    bad += check("settings save/load",
                 st_save(OBJ_SETTINGS, "hello", 5) == 0 && st_load(OBJ_SETTINGS, got, 5) == 5 && !memcmp(got, "hello", 5));
    bad += check("CRC-32 is zlib's (check value 0xCBF43926)", st_crc32("123456789", 9) == 0xCBF43926u);
    memset(nor, 0xFF, sizeof nor);
    st_save(OBJ_PROJECT0 + 2, a, sizeof a);
    st_save(OBJ_PROJECT0 + 2, b, sizeof b);              /* B is newer, in the other copy */
    {
        st_hdr_t h;
        int cur = st_current(OBJ_PROJECT0 + 2, &h);
        nor[st_sector(OBJ_PROJECT0 + 2, (uint32_t)cur) + ST_PAYLOAD_OFF + 300] ^= 0x10;   /* rot in the newer copy */
    }
    n = st_load(OBJ_PROJECT0 + 2, got, sizeof got);
    bad += check("newer copy rotten -> the older one loads", n == (int)sizeof a && !memcmp(got, a, sizeof a));
    bad += check("save over the rotten copy", st_save(OBJ_PROJECT0 + 2, b, sizeof b) == 0);
    n = st_load(OBJ_PROJECT0 + 2, got, sizeof got);
    bad += check("... loads the new data", n == (int)sizeof b && !memcmp(got, b, sizeof b));
    nor[st_sector(OBJ_PROJECT0 + 2, 0) + 8] ^= 0x01;    /* both headers broken */
    nor[st_sector(OBJ_PROJECT0 + 2, 1) + 8] ^= 0x01;
    bad += check("both headers broken -> nothing", st_load(OBJ_PROJECT0 + 2, got, sizeof got) < 0);
    {   /* X0X map: every object in a data region, none in the sample slots (0xA0000..0xDBFFF), no overlaps */
        uint32_t o, p, ok = 1;
        for (o = 0; o < OBJ_NOBJ; o++) {
            uint32_t a0 = st_sector(o, 0), a1 = st_sector(o, 1) + 4096;
            int in_data = (a0 >= 0x97000 && a1 <= 0xA0000) || (a0 >= 0xDC000 && a1 <= 0xE0000) ||
                          (a0 >= 0xFC000 && a1 <= 0xFF000);
            if (!in_data || a1 - a0 != 8192)
                ok = 0;
            for (p = 0; p < o; p++)
                if (a0 < st_sector(p, 1) + 4096 && st_sector(p, 0) < a1)
                    ok = 0;
        }
        bad += check("objects in data regions, clear of the sample slots", (int)ok);
    }
    printf("%s\n", bad ? "STORAGE TEST FAILED" : "storage test passed");
    /* wip255 (option B, B1): the preset slots: 0-31 the user slots, 32-39 the presets 1-8 written over (0xC0000..), 40 does not exist */
    {
        char p31[300], p32[300], p39[300];
        int i;
        memset(p31, 'x', sizeof p31);
        memset(p32, 'y', sizeof p32);
        memset(p39, 'z', sizeof p39);
        bad += check("preset slot 31 saves", st_preset_save(31, p31, sizeof p31) == 0);
        bad += check("preset slot 32 saves (0xC0000)", st_preset_save(32, p32, sizeof p32) == 0);
        bad += check("preset slot 39 saves (0xC7000)", st_preset_save(39, p39, sizeof p39) == 0);
        bad += check("preset slot 40 refused", st_preset_save(40, p39, sizeof p39) < 0);
        n = st_preset_load(31, got, sizeof got);
        bad += check("slot 31 intact (32 did not touch it)", n == (int)sizeof p31 && !memcmp(got, p31, sizeof p31));
        n = st_preset_load(32, got, sizeof got);
        bad += check("slot 32 loads", n == (int)sizeof p32 && !memcmp(got, p32, sizeof p32));
        n = st_preset_load(39, got, sizeof got);
        bad += check("slot 39 loads", n == (int)sizeof p39 && !memcmp(got, p39, sizeof p39));
        bad += check("slot 33 is empty", st_preset_load(33, got, sizeof got) < 0);
        bad += check("slot 40 loads nothing", st_preset_load(40, got, sizeof got) < 0);
        bad += check("slot 32 header at 0xC0000", nor[0xC0000] != 0xFF && nor[0xC1000] == 0xFF);
        i = st_preset_erase(32);
        bad += check("slot 32 erased, 31 and 39 stay", i == 0 && st_preset_load(32, got, sizeof got) < 0 &&
                                                         st_preset_load(31, got, sizeof got) > 0 && st_preset_load(39, got, sizeof got) > 0);
    }
    return bad != 0;
}
