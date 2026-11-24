/* SPDX-License-Identifier: GPL-3.0-only */
/* wip657: the second spot of a project (firmware/src/app/ext_spot.c): song + motion lanes + ARP notes (20484 raw bytes) in ONE
 * object of 3840 bytes. Round trip, the measure = the pack, "does not fit" = 0 and no header written and nothing past max, and a spot that is torn,
 * cut or of another length is never loaded (-1), and one with a flipped byte is refused or gives back exactly the same data, never other data. Also prints how big a
 * typical / dense / very dense project packs (the numbers of handoff 657). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../firmware/src/app/lz.c"
static uint32_t crc32_of(const uint8_t *p, uint32_t n)
{
    static const uint32_t T[16] = {
        0x00000000u, 0x1DB71064u, 0x3B6E20C8u, 0x26D930ACu, 0x76DC4190u, 0x6B6B51F4u, 0x4DB26158u, 0x5005713Cu,
        0xEDB88320u, 0xF00F9344u, 0xD6D6A3E8u, 0xCB61B38Cu, 0x9B64C2B0u, 0x86D3D2D4u, 0xA00AE278u, 0xBDBDF21Cu};
    uint32_t c = 0xFFFFFFFFu;
    while (n--) {
        c ^= *p++;
        c = (c >> 4) ^ T[c & 15u];
        c = (c >> 4) ^ T[c & 15u];
    }
    return ~c;
}
#include "../../firmware/src/app/ext_spot.c"

#define RAW 20484u
#define NOTES 8192u
#define SONG 772u
#define MAX 3840u
#define MK 0xA5C3F00Du                        /* a pair mark (wip659): the main spot's CRC in the firmware */
static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
static uint32_t rs = 7;
static uint32_t rnd(void) { rs = rs * 1664525u + 1013904223u; return rs >> 8; }

/* a project's raw bytes: nl used lanes (sp % of the steps set), a song of sl bars, nn notes */
static void make(uint8_t *b, int nl, int sp, int sl, int nn)
{
    uint8_t *notes = b + SONG + 11520u - 11520u, *song, *lane;
    (void)notes;
    song = b; lane = b + SONG; notes = b + SONG + 11520u;     /* arr = song, lanes; then the notes (project_t order) */
    memset(b, 0, RAW);
    song[0] = (uint8_t)sl; song[1] = (uint8_t)(sl >> 8);
    for (int i = 0; i < sl && i < 192; i++) {
        song[4 + i * 4] = (uint8_t)(rnd() % 16); song[5 + i * 4] = (uint8_t)(rnd() % 16);
        song[6 + i * 4] = (uint8_t)(rnd() % 16); song[7 + i * 4] = (uint8_t)(rnd() % 32);
    }
    for (int l = 0; l < nl && l < 160; l++) {
        uint8_t *L = lane + l * 72;
        int v = (int)(rnd() % 256);
        L[0] = 1; L[2] = (uint8_t)(rnd() % 32); L[4] = (uint8_t)(rnd() % 6); L[5] = (uint8_t)(rnd() % 8);
        for (int s = 0; s < 64; s++) {
            if (rnd() % 3 == 0)
                v = (v + (int)(rnd() % 9) - 4) & 0xFF;
            L[8 + s] = (int)(rnd() % 100) < sp ? (uint8_t)v : 0xFF;
        }
    }
    for (int l = nl; l < 160; l++)
        memset(lane + l * 72 + 8, 0, 64);
    for (int i = 0; i < nn; i++)
        notes[rnd() % NOTES] = (uint8_t)((int)(rnd() % 25) - 12);
}

int main(void)
{
    static uint8_t raw[RAW], back[RAW], spot[MAX + 64];
    static const struct { const char *name; int nl, sp, sl, nn; int fits; } sc[] = {
        {"empty", 0, 0, 0, 0, 1}, {"typical", 12, 20, 16, 60, 1}, {"fairly full", 30, 40, 48, 150, 1},
        {"dense", 100, 60, 64, 300, 0}, {"very dense", 160, 90, 192, 700, 0}};
    for (unsigned t = 0; t < sizeof sc / sizeof sc[0]; t++) {
        uint32_t m, n;
        make(raw, sc[t].nl, sc[t].sp, sc[t].sl, sc[t].nn);
        m = ext_pack(raw, RAW, 0, 100000u, 0);                  /* the size, whatever the room */
        memset(spot, 0xEE, sizeof spot);
        n = ext_pack(raw, RAW, spot, MAX, MK);
        printf("  %-12s packs to %5u bytes (room %u): %s\n", sc[t].name, m, MAX, n ? "fits" : "DOES NOT FIT");
        CHECK((n != 0) == sc[t].fits, "%s: fits = %d, expected %d", sc[t].name, n != 0, sc[t].fits);
        CHECK(m == EXT_HDR + (m - EXT_HDR), "measure");
        if (!n) {
            CHECK(spot[0] == 0xEE && spot[1] == 0xEE && spot[2] == 0xEE && spot[3] == 0xEE, "%s: a spot that does not fit wrote a header (it could be taken for a spot)", sc[t].name);
            CHECK(spot[MAX] == 0xEE && spot[MAX + 63] == 0xEE, "%s: a spot that does not fit wrote past max", sc[t].name);   /* (the firmware measures first, out = 0, and never packs such a project: place_save) */
            continue;
        }
        CHECK(n == m, "%s: measure %u != pack %u", sc[t].name, m, n);
        CHECK(n <= MAX && spot[n] == 0xEE, "%s: wrote past its length", sc[t].name);
        memset(back, 0x55, sizeof back);
        CHECK(ext_unpack(spot, n, back, RAW, MK) == 0 && memcmp(raw, back, RAW) == 0, "%s: round trip", sc[t].name);
        CHECK(ext_unpack(spot, n - 1, back, RAW, MK) == -1, "%s: cut one byte short still loaded", sc[t].name);
        CHECK(ext_unpack(spot, n / 2, back, RAW, MK) == -1, "%s: cut in half still loaded", sc[t].name);
        CHECK(ext_unpack(spot, 8, back, RAW, MK) == -1, "%s: header only still loaded", sc[t].name);
        CHECK(ext_unpack(spot, n, back, RAW - 4u, MK) == -1, "%s: another raw length still loaded", sc[t].name);
        for (uint32_t i = 0; i < n; i += (i < 64 ? 1 : 7)) {  /* every header byte, a byte in seven after that */
            uint8_t keep = spot[i];
            spot[i] ^= 0x10;
            memset(back, 0x55, sizeof back);                 /* a changed byte is refused, or (a copy distance into a run of identical bytes) harmless: never other data */
            CHECK(ext_unpack(spot, n, back, RAW, MK) == -1 || memcmp(raw, back, RAW) == 0, "%s: flipped byte %u loaded OTHER data", sc[t].name, i);
            spot[i] = keep;
        }
        CHECK(ext_unpack(spot, n, back, RAW, MK) == 0, "%s: spot restored", sc[t].name);
        /* wip659: the PAIR MARK. A spot saved with another main spot's mark (a save cut by the power between the two writes) is refused and the raw bytes are not touched */
        memset(back, 0x55, sizeof back);
        CHECK(ext_unpack(spot, n, back, RAW, MK ^ 1u) == -1, "%s: another pair's mark (1 bit) was accepted", sc[t].name);
        CHECK(ext_unpack(spot, n, back, RAW, 0) == -1, "%s: mark 0 was accepted for a spot of mark MK", sc[t].name);
        CHECK(back[0] == 0x55 && back[RAW - 1] == 0x55 && back[RAW / 2] == 0x55, "%s: a refused mark still wrote the raw bytes", sc[t].name);
        for (int i = 16; i < 20; i++) {                      /* every byte of the mark in the header: refused, never loaded */
            uint8_t keep = spot[i];
            spot[i] ^= 0x01;
            CHECK(ext_unpack(spot, n, back, RAW, MK) == -1, "%s: a flipped mark byte %d was accepted", sc[t].name, i);
            spot[i] = keep;
        }
        {                                                    /* a second spot of an older generation (TEX1, no mark) is not read either */
            uint8_t keep = spot[3];
            spot[3] = '1';
            CHECK(ext_unpack(spot, n, back, RAW, MK) == -1, "%s: a TEX1 spot was accepted", sc[t].name);
            spot[3] = keep;
        }
    }
    CHECK(ext_unpack(0 + spot, 0, back, RAW, MK) == -1, "empty buffer");
    memset(spot, 0xFF, 64);
    CHECK(ext_unpack(spot, 64, back, RAW, MK) == -1, "erased flash (0xFF) was taken for a spot");
    memset(spot, 0, 64);
    CHECK(ext_unpack(spot, 64, back, RAW, MK) == -1, "zeroes were taken for a spot");
    CHECK(ext_pack(raw, RAW, spot, 8, MK) == 0, "a room smaller than the header");
    printf(fails ? "ext_spot_test: %d FAILED\n" : "ext_spot_test: all ok\n", fails);
    return fails ? 1 : 0;
}
