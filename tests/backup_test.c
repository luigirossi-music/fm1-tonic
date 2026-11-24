/* SPDX-License-Identifier: GPL-3.0-only */
/* Host test of backup and restore over USB, the device's side (firmware/src/backup.c, after Polly's), on storage.c
 * over a simulated NOR flash (erase -> 0xFF, a program only clears bits):
 *   backup: the image streamed in frames that each fit the SysEx TX ring, one a main-loop pass, contiguous, the six
 *     project objects as stored (absent ones absent), the CRCs right; stopped, the unsaved saved first; changes not
 *     saved flagged; the saves held while it streams
 *   restore: refused while playing (nothing fetched, nothing written); the cable cut at every read, a changed byte
 *     (payload, record, head, tail), a wrong length, another version, Polly's backup, HOME at the question: nothing
 *     changed; SEL: everything as backed up, the app reloaded; the panel's calibration and the flash outside the
 *     objects never touched
 *   a power cut at every flash write of a restore (a program torn in half): after the next boot (rs_boot) the project
 *     is the old one or the new one, whole, never a mix
 *   backup_test IMAGE_OUT   (the image, for tests/x0x_backup_test.py) */
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define X0X_HOST 1
#include "../firmware/src/app/plat.h"
#define FELUCCA_ID "FM-1_900"

static uint8_t nor[0x100000];
static int fails, cut_at = -1, ops, fetch_left = -1, flip_at = -1, playing, play_after = -1, answer = 1;
static int reloads, flushes, holding, asks, begins, fetches, unsaved;
static jmp_buf cut;
static uint32_t notes[8], nnotes;

static int in_store(uint32_t off, uint32_t n);
static void op(void)                              /* a flash write: the power may go here */
{
    if (cut_at >= 0 && ops++ == cut_at)
        longjmp(cut, 1);
}
static int st_read(uint32_t off, void *dst, uint32_t n)
{
    if (off > sizeof nor || n > sizeof nor - off)
        return -1;
    memcpy(dst, nor + off, n);
    return 0;
}
static int st_erase(uint32_t off)
{
    if (!in_store(off, 4096) || (off & 0xFFFu)) {
        printf("erase outside the objects: %#x\n", off);
        exit(1);
    }
    op();
    memset(nor + off, 0xFF, 4096);
    return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    if (!in_store(off, n)) {
        printf("program outside the objects: %#x+%u\n", off, n);
        exit(1);
    }
    if (cut_at >= 0 && ops == cut_at) {           /* torn: half of it written */
        for (uint32_t i = 0; i < n / 2; i++)
            nor[off + i] &= s[i];
    }
    op();
    for (uint32_t i = 0; i < n; i++)
        nor[off + i] &= s[i];
    return 0;
}
#include "../firmware/src/storage.c"

/* ota.c for its pack7 (its hooks unused here) */
static int ota_wire_send(const uint8_t *p, uint32_t n) { (void)p; (void)n; return -1; }
static int ota_frame_get(const uint8_t **p, uint32_t *n) { (void)p; (void)n; return 0; }
static void ota_frame_done(void) {}
static uint32_t t_ms;                             /* the clock (the backup's stall limit) */
static uint32_t ota_now_ms(void) { return t_ms; }
static void ota_idle(void) {}
static int ota_erase(uint32_t off) { (void)off; return -1; }
static int ota_prog(uint32_t off, const void *p, uint32_t n) { (void)off; (void)p; (void)n; return -1; }
static int ota_fread(uint32_t off, void *p, uint32_t n) { (void)off; (void)p; (void)n; return -1; }
static void ota_show(uint32_t step, int32_t code) { (void)step; (void)code; }
static void ota_commit(const uint8_t *parm) { (void)parm; }
#include "../firmware/src/ota.c"

#include "../firmware/src/backup.c"

static int in_store(uint32_t off, uint32_t n)     /* the six objects' sectors, the journal record's */
{
    for (uint32_t i = 0; i < BU_NOBJ; i++) {
        uint32_t b = st_sector(bu_obj(i), 0);
        if (off >= b && off + n <= b + 2u * ST_SECTOR)
            return 1;
    }
    if (off >= ST_PRESET_BASE && off + n <= ST_PRESET_BASE + ST_PRESET_N * ST_SECTOR)   /* the user preset slots */
        return 1;
    if (off >= ST_PRESET2_BASE && off + n <= ST_PRESET2_BASE + 8u * ST_SECTOR)   /* wip668: places 1-8 (slots 32..39) */
        return 1;
    if ((off >= ST_EXT_BASE_A && off + n <= ST_EXT_BASE_A + 10u * ST_SECTOR) || (off >= ST_EXT_BASE_B && off + n <= ST_EXT_BASE_B + 20u * ST_SECTOR))   /* wip669: the second spots */
        return 1;
    return off >= RS_REC && off + n <= RS_REC + ST_SECTOR;
}

/* the host side: the image served, the notes kept */
static uint8_t img[BU_MAXIMG + 64];
static uint32_t img_n;
static int rs_fetch(uint32_t addr, uint8_t *dst, uint32_t n)
{
    fetches++;
    if (fetch_left == 0)
        return -1;
    if (fetch_left > 0)
        fetch_left--;
    if (play_after >= 0 && fetches > play_after)
        playing = 1;
    memset(dst, 0, n);
    if (addr < img_n)
        memcpy(dst, img + addr, addr + n <= img_n ? n : img_n - addr);
    if (flip_at >= 0 && (uint32_t)flip_at >= addr && (uint32_t)flip_at < addr + n)
        dst[flip_at - (int)addr] ^= 0x04;
    return 0;
}
static int rs_note(uint32_t addr)
{
    if (nnotes < 8)
        notes[nnotes++] = addr;
    return 0;
}
static int rs_playing(void) { return playing; }
static void rs_begin(void) { begins++; }
static void rs_show(uint32_t pct) { (void)pct; }
static int rs_ask(void) { asks++; return answer; }
static void rs_reload(void) { reloads++; }
static int rs_rec_erase(void) { return st_erase(RS_REC); }
static int rs_rec_prog(uint32_t off, const void *src, uint32_t n) { return st_prog(RS_REC + off, src, n); }
static int rs_rec_read(uint32_t off, void *dst, uint32_t n) { return st_read(RS_REC + off, dst, n); }

/* the backup's frames: decoded and put together as the host does */
static int room = 1, frames_this_pass;
static uint8_t got[BU_MAXIMG + 64];
static uint32_t got_n, end_len, end_crc, max_wire, frames;
static int end_seen, end_status, end_flags, gap;
static uint8_t pg_list_body[4 + 2 * 32], pg_got[4096], pg_got_slot, pg_end_slot, pg_end_status;
static uint32_t pg_got_n, pg_end_len, pg_end_crc;
static int pg_list_n, pg_end_n;
static uint32_t pg_list_len;                          /* wip667: the LIST frame's body length (must stay 4 + 2 x 32: the web page needs 32 entries) */
static int bu_room(void)                          /* a frame in the ring: full until the next pass; room -1: USB gone */
{
    return room < 0 ? -1 : room && !frames_this_pass;
}
static int bu_send(const uint8_t *p, uint32_t n)
{
    uint8_t u[400];
    uint32_t d, len, s = 0;
    frames_this_pass++;
    frames++;
    if (n > max_wire)
        max_wire = n;
    d = ota_unpack7(p + 1, n - 2, u, sizeof u);
    len = u[3] | u[4] << 8 | (uint32_t)u[5] << 16;
    if (p[0] != 0xF0 || p[n - 1] != 0xF7 || u[0] != 0 || u[1] != 0x59 || len + 7u != d)
        gap = 1;
    for (uint32_t i = 0; i < len; i++)
        s += u[6 + i];
    if ((uint8_t)~s != u[6 + len])
        gap = 1;
    if (u[2] == BU_REQ) {
        uint32_t off = bu_rd32(u + 6);
        if (off != got_n)
            gap = 1;
        memcpy(got + got_n, u + 10, len - 4);
        got_n += len - 4;
    } else if (u[2] == PG_LIST) {
        pg_list_n++;
        pg_list_len = len;
        memcpy(pg_list_body, u + 6, len < sizeof pg_list_body ? len : sizeof pg_list_body);
    } else if (u[2] == PG_DATA) {
        if (u[6] != pg_got_slot || bu_rd32(u + 7) != pg_got_n || len - 5u > PG_CHUNK)
            gap = 1;
        memcpy(pg_got + pg_got_n, u + 11, len - 5);
        pg_got_n += len - 5;
    } else if (u[2] == PG_END) {
        pg_end_n++;
        pg_end_slot = u[6];
        pg_end_len = bu_rd32(u + 7);
        pg_end_crc = bu_rd32(u + 11);
        pg_end_status = u[15];
    } else if (u[2] == BU_END) {
        end_seen = 1;
        end_len = bu_rd32(u + 6);
        end_crc = bu_rd32(u + 10);
        end_status = u[14];
        end_flags = u[15];
    }
    return 0;
}
static void bu_flush(void) { flushes++; }
static int bu_unsaved(void) { return unsaved; }
static void bu_hold(int on) { holding = on; }

static int check(const char *what, int ok)
{
    printf("%-66s %s\n", what, ok ? "ok" : "FAIL");
    fails += !ok;
    return ok;
}

/* the objects as st_load gives them */
typedef struct { int n[BU_NOBJ]; uint8_t d[BU_NOBJ][ST_PAYLOAD_MAX]; } set_t;
static set_t set_a, set_b, now;
static void snap(set_t *s)
{
    for (uint32_t i = 0; i < BU_NOBJ; i++)
        s->n[i] = st_load(bu_obj(i), s->d[i], ST_PAYLOAD_MAX);
}
static int same(const set_t *a, const set_t *b)
{
    for (uint32_t i = 0; i < BU_NOBJ; i++)
        if (a->n[i] != b->n[i] || (a->n[i] > 0 && memcmp(a->d[i], b->d[i], (size_t)a->n[i])))
            return 0;
    return 1;
}
static uint8_t panel_was[2 * ST_SECTOR];
static int panel_same(void) { return !memcmp(nor + st_sector(OBJ_PANEL, 0), panel_was, sizeof panel_was); }

static void fill(uint8_t *p, uint32_t n, uint32_t seed)
{
    for (uint32_t i = 0; i < n; i++)
        p[i] = (uint8_t)((i * 31u + seed * 7u) ^ (i >> 5));
}
static void save(uint32_t obj, uint32_t n, uint32_t seed)
{
    static uint8_t b[ST_PAYLOAD_MAX];
    fill(b, n, seed);
    if (st_save(obj, b, n))
        printf("save failed\n");
}

static int backup(int play)
{
    int passes = 0;
    got_n = 0;
    end_seen = gap = 0;
    frames = max_wire = 0;
    playing = play;
    bu_start();
    playing = 0;
    while (bu.on && passes < 100000) {
        frames_this_pass = 0;
        room = passes % 3 != 1;                   /* the ring busy now and then */
        bu_step();
        if (frames_this_pass > 1)
            gap = 1;
        passes++;
    }
    return passes;
}
static int restore(void)
{
    nnotes = 0;
    fetches = 0;
    return restore_rx(img_n, bu_crc(0, img, img_n), 0);
}

int main(int argc, char **argv)
{
    FILE *f;
    int rc;
    memset(nor, 0x5A, sizeof nor);                /* not erased anywhere: a write outside would show */
    for (uint32_t i = 0; i < BU_NOBJ; i++) {
        st_erase(st_sector(bu_obj(i), 0));
        st_erase(st_sector(bu_obj(i), 1));
    }
    st_erase(RS_REC);
    memset(nor + st_sector(OBJ_PANEL, 0), 0x33, sizeof panel_was);   /* the calibration: never touched */
    memcpy(panel_was, nor + st_sector(OBJ_PANEL, 0), sizeof panel_was);

    /* set A: the settings, the sound (whole), the stream's first two and its last; the song absent */
    save(OBJ_SET, 120, 1);
    save(OBJ_SOUND, ST_PAYLOAD_MAX, 2);
    save(OBJ_SOUND, ST_PAYLOAD_MAX, 3);           /* (its B copy current) */
    save(OBJ_PAT0, ST_PAYLOAD_MAX, 10);
    save(OBJ_PAT1, 1777, 11);
    save(OBJ_MOTION, 16, 12);
    snap(&set_a);

    /* ---- backup */
    flushes = 0;
    backup(0);
    check("backup: an end frame, status 0, the frames contiguous", end_seen && end_status == 0 && !gap);
    check("... each frame fits the SysEx ring whole (<= 64 packets)", max_wire <= 64u * 3u);
    check("... stopped: the unsaved saved first; the autosave held, then free", flushes == 1 && holding == 0);
    check("... the length and the CRC-32 of the whole", end_len == got_n && end_crc == bu_crc(0, got, got_n));
    {
        uint32_t pos = BU_HEAD, ok = bu_rd32(got) == BU_MAGIC && bu_rd16(got + 6) == BU_NOBJ &&
                                      bu_rd32(got + 12) == bu_crc(0, got, 12) && bu_rd32(got + 8) == 0;
        for (uint32_t i = 0; i < BU_NOBJ && ok; i++) {
            uint32_t n = bu_rd16(got + pos + 2);
            ok = bu_rd16(got + pos) == bu_obj(i) && (int)n == (set_a.n[i] < 0 ? 0 : set_a.n[i]) &&
                 bu_rd32(got + pos + 4) == bu_crc(0, got + pos + 8, n) &&
                 (!n || !memcmp(got + pos + 8, set_a.d[i], n));
            pos += BU_REC + n;
        }
        check("... the image: the head, 6 records as stored (absent: 0 bytes)", ok);
        check("... the tail: the CRC-32 of all before", pos + 4u == got_n && bu_rd32(got + pos) == bu_crc(0, got, pos));
        printf("  %u bytes in %u frames (largest %u wire bytes)\n", got_n, frames, max_wire);
    }
    memcpy(img, got, got_n);
    img_n = got_n;
    if (argc > 1 && (f = fopen(argv[1], "wb"))) {
        fwrite(img, 1, img_n, f);
        fclose(f);
    }
    flushes = 0;
    unsaved = 1;
    backup(1);
    unsaved = 0;
    check("backup while playing: nothing saved first, unsaved changes flagged", flushes == 0 && end_flags == 1 &&
          bu_rd32(got + 8) == 1 && !gap && end_len == img_n && !memcmp(got + 16, img + 16, img_n - 20));
    backup(1);
    check("... nothing unsaved: not flagged", end_flags == 0 && got_n == img_n && !memcmp(got, img, img_n));

    {                                             /* a cut cable: the ring never drains / USB gone */
        int passes = 0, held_long;
        got_n = 0;
        end_seen = 0;
        t_ms = 1000;
        playing = 1;
        bu_start();
        playing = 0;
        for (int k = 0; k < 5; k++) {             /* a few frames out */
            frames_this_pass = 0;
            room = 1;
            bu_step();
            t_ms += 10;
        }
        room = 0;                                 /* then the ring full for good */
        while (bu.on && passes < 1000) {
            frames_this_pass = 0;
            bu_step();
            t_ms += 10;
            passes++;
        }
        held_long = passes * 10 >= (int)BU_STALL_MS && passes * 10 <= (int)BU_STALL_MS + 20;
        check("backup stalled (the ring never drains): given up after ~2 s, autosave free",
              !bu.on && !holding && !end_seen && held_long);
        bu_start();
        frames_this_pass = 0;
        room = 1;
        bu_step();
        room = -1;                                /* unplugged / suspended */
        frames_this_pass = 0;
        bu_step();
        check("... USB gone: given up at once, autosave free", !bu.on && !holding && !end_seen);
        room = 1;
        t_ms = 0;
    }

    /* set B: other contents, the absent song filled, settings of another size */
    save(OBJ_SET, 140, 4);
    save(OBJ_SOUND, 3000, 5);
    save(OBJ_SONG, 2000, 6);                      /* absent in A */
    save(OBJ_PAT1, 100, 7);
    save(OBJ_MOTION, 3840, 8);
    snap(&set_b);
    check("set B differs from A", !same(&set_a, &set_b));

    /* ---- refused, failed: nothing changes */
    playing = 1;
    ops = 0;
    cut_at = -1;
    rc = restore();
    playing = 0;
    snap(&now);
    check("playing: STOP FIRST (RS_E_PLAYING), told, nothing read", rc == RS_E_PLAYING && fetches == 0 && nnotes == 1 &&
                                                                    notes[0] == (RS_FAIL_ADDR | RS_E_PLAYING));
    check("... nothing changed", same(&now, &set_b));
    {
        int all = 1, n_fetch;
        fetch_left = -1;
        answer = 0;                               /* (to count the reads; answered no) */
        restore();
        n_fetch = fetches;
        for (int k = 0; k < n_fetch; k++) {       /* the cable cut at every read */
            fetch_left = k;
            asks = 0;
            rc = restore();
            snap(&now);
            all &= rc == RS_E_FETCH && asks == 0 && same(&now, &set_b) && nnotes == 1;
        }
        fetch_left = -1;
        printf("  %d reads a restore\n", n_fetch);
        check("the cable cut at each read: RS_E_FETCH, never asked, nothing changed", all);
    }
    answer = 1;
    {
        int all = 1;
        uint32_t spots[] = {20, BU_HEAD + BU_REC + 5, BU_HEAD + BU_REC + 119, 2000, img_n - 300, img_n - 10};
        for (uint32_t k = 0; k < sizeof spots / sizeof spots[0]; k++) {
            flip_at = (int)spots[k];
            rc = restore();
            snap(&now);
            all &= rc != 0 && same(&now, &set_b);
        }
        flip_at = (int)(img_n - 2);               /* the tail */
        rc = restore();
        all &= rc == RS_E_TOTAL;
        flip_at = 3;                              /* the head */
        rc = restore();
        all &= rc == RS_E_HEAD;
        flip_at = BU_HEAD + 1;                    /* the first record's id */
        rc = restore();
        all &= rc == RS_E_OBJ;
        flip_at = BU_HEAD + BU_REC + 30;          /* the project's payload */
        rc = restore();
        all &= rc == RS_E_CRC;
        flip_at = -1;
        snap(&now);
        check("a byte changed (payload, record, head, tail): refused, nothing changed", all && same(&now, &set_b));
    }
    rc = restore_rx(img_n + 4, bu_crc(0, img, img_n), 0);
    snap(&now);
    check("a wrong length: refused, nothing changed", rc != 0 && same(&now, &set_b));
    {   /* another version, Polly's backup ("PLYB"): their heads whole (CRCs right), refused */
        uint8_t keep[BU_HEAD + 4];
        uint32_t tail = img_n - 4u;
        int all = 1;
        memcpy(keep, img, BU_HEAD);
        memcpy(keep + BU_HEAD, img + tail, 4);
        for (int k = 0; k < 3; k++) {
            memcpy(img, keep, BU_HEAD);
            if (k == 0)
                bu_wr16(img + 4, 2);
            else if (k == 1)
                bu_wr16(img + 6, 18);
            else
                bu_wr32(img, 0x42594C50u);
            bu_wr32(img + 12, bu_crc(0, img, 12));
            bu_wr32(img + tail, bu_crc(0, img, tail));
            rc = restore();
            snap(&now);
            all &= rc == RS_E_HEAD && same(&now, &set_b) && fetches == 1;
        }
        check("another version, another count, Polly's backup: RS_E_HEAD, nothing changed", all && BU_VERSION == 1u);
        memcpy(img, keep, BU_HEAD);
        memcpy(img + tail, keep + BU_HEAD, 4);
    }
    rc = restore_rx(img_n, bu_crc(0, img, img_n) ^ 1u, 0);
    snap(&now);
    check("the host's CRC-32 not the image's: RS_E_TOTAL, nothing changed", rc == RS_E_TOTAL && same(&now, &set_b));
    play_after = 5;                               /* PLAY (MIDI start) during the transfer */
    rc = restore();
    play_after = -1;
    playing = 0;
    snap(&now);
    check("started playing meanwhile: RS_E_PLAYING, nothing changed", rc == RS_E_PLAYING && same(&now, &set_b));
    answer = 0;
    asks = reloads = 0;
    rc = restore();
    snap(&now);
    check("HOME at the question: RS_E_NO, asked once, the host told first", rc == RS_E_NO && asks == 1 && nnotes == 2 &&
                                                                         notes[0] == RS_ASK_ADDR &&
                                                                         notes[1] == (RS_FAIL_ADDR | RS_E_NO));
    check("... nothing changed, nothing reloaded", same(&now, &set_b) && reloads == 0);

    /* ---- SEL: restored */
    answer = 1;
    begins = 0;
    rc = restore();
    snap(&now);
    check("SEL: restored (0), the host told, the app reloaded", rc == 0 && nnotes == 2 && notes[1] == RS_OK_ADDR &&
                                                               reloads == 1 && begins == 1);
    check("... every object as backed up (the song absent in A: absent)", same(&now, &set_a) && now.n[4] < 0);
    check("... the panel's calibration untouched", panel_same());
    check("... the journal record erased", nor[RS_REC] == 0xFF && nor[RS_REC + 299] == 0xFF);
    backup(0);
    check("... a backup of it is the image again", got_n == img_n && !memcmp(got, img, img_n));
    save(OBJ_SONG, 50, 9);
    {
        uint8_t b[64];
        fill(b, 50, 9);
        check("... saving after it works (the next seq)", st_load(OBJ_SONG, now.d[0], 64) == 50 &&
                                                         !memcmp(now.d[0], b, 50));
    }

    /* ---- the user preset slots over USB: list and get one (step B1) */
    {
        static uint8_t blob[ST_PRESET_N][ST_PAYLOAD_MAX];
        static const int slots[] = {0, 5, 21, 12};
        static const uint32_t lens[] = {3840, 1, 777, 120};
        uint32_t i, mask, ok_all = 1, hold_seen = 0;
        int rc, passes;
        for (i = 0; i < 4; i++) {
            fill(blob[i], lens[i], 100 + i);
            rc = st_preset_save(slots[i], blob[i], lens[i]);
            if (rc)
                ok_all = 0;
        }
        check("presets: 4 slots saved (0, 5, 12, 21; 1 to 3840 bytes)", ok_all);
        /* slot 12 damaged: one payload byte flipped (a program only clears bits) */
        nor[ST_PRESET_BASE + 12 * ST_SECTOR + ST_PAYLOAD_OFF + 3] = 0x00;
        pg_list_n = 0;
        pg_start(1, 0);
        for (passes = 0; passes < 5 && bpg.kind; passes++) {
            frames_this_pass = 0;
            room = 1;
            pg_step();
        }
        mask = bu_rd32(pg_list_body);
        check("list: one frame; used slots 0, 5, 21 (the damaged 12 is not)", pg_list_n == 1 && mask == ((1u << 0) | (1u << 5) | (1u << 21)));
        check("... the lengths", bu_rd16(pg_list_body + 4) == 3840 && bu_rd16(pg_list_body + 4 + 10) == 1 &&
                                 bu_rd16(pg_list_body + 4 + 42) == 777 && bu_rd16(pg_list_body + 4 + 24) == 0 &&
                                 bu_rd16(pg_list_body + 4 + 2) == 0);
        {   /* wip667: the frame is always 32 entries (the page rejects a shorter one); slots 22..31 are empty, the mask has no bit there */
            uint32_t z = 1;
            for (i = ST_PRESET_N; i < 32; i++)
                if (bu_rd16(pg_list_body + 4 + 2 * i) != 0)
                    z = 0;
            check("list: the frame keeps 32 entries (4 + 64 bytes), slots 22..31 are 0 and not in the mask",
                  pg_list_len == 4u + 2u * 32u && z && (mask >> ST_PRESET_N) == 0);
        }
        for (i = 0; i < 3; i++) {
            static const int gs[] = {0, 5, 21};
            uint32_t want = lens[i == 0 ? 0 : i == 1 ? 1 : 2];
            pg_got_n = 0;
            pg_got_slot = (uint8_t)gs[i];
            pg_end_n = 0;
            gap = 0;
            pg_start(2, (uint32_t)gs[i]);
            hold_seen = holding;
            for (passes = 0; passes < 100000 && bpg.kind; passes++) {
                frames_this_pass = 0;
                room = passes % 3 != 1;           /* the ring busy now and then */
                pg_step();
                if (frames_this_pass > 1)
                    gap = 1;
            }
            check("get: the blob as saved, frames contiguous, an end frame ok",
                  pg_end_n == 1 && pg_end_status == 0 && pg_end_slot == gs[i] && pg_end_len == want &&
                  pg_got_n == want && !memcmp(pg_got, blob[i == 0 ? 0 : i == 1 ? 1 : 2], want) &&
                  pg_end_crc == bu_crc(0, pg_got, pg_got_n) && !gap);
            check("... the saves held while it streams, free after", hold_seen == 1 && holding == 0);
        }
        pg_end_n = 0;
        pg_got_n = 0;
        pg_got_slot = 12;
        pg_start(2, 12);
        for (passes = 0; passes < 10 && bpg.kind; passes++) {
            frames_this_pass = 0;
            room = 1;
            pg_step();
        }
        check("get of the damaged slot: status 1, no data", pg_end_n == 1 && pg_end_status == 1 && pg_got_n == 0 && holding == 0);
        pg_end_n = 0;
        pg_start(2, 7);
        for (passes = 0; passes < 10 && bpg.kind; passes++) {
            frames_this_pass = 0;
            room = 1;
            pg_step();
        }
        check("get of an empty slot: status 1", pg_end_n == 1 && pg_end_status == 1 && pg_got_n == 0);
        pg_end_n = 0;
        pg_start(2, 22);
        for (passes = 0; passes < 10 && bpg.kind; passes++) {
            frames_this_pass = 0;
            room = 1;
            pg_step();
        }
        check("get of slot 22 (not a place): status 1", pg_end_n == 1 && pg_end_status == 1);
        /* wip668: places 1-8 (slots 32..39) over USB: list (sub 4) and get */
        {
            static uint8_t ob[1500];
            uint32_t k;
            fill(ob, sizeof ob, 77);
            check("places 1-8: slot 34 (place 3) saved", st_preset_save(34, ob, sizeof ob) == 0);
            pg_list_n = 0;
            pg_start(PG_SUB_LISTX, 0);
            for (passes = 0; passes < 5 && bpg.kind; passes++) {
                frames_this_pass = 0;
                room = 1;
                pg_step();
            }
            check("list of places 1-8: one frame of 17 bytes, only place 3 used, its length 1500",
                  pg_list_n == 1 && pg_list_len == 17u && pg_list_body[0] == (1u << 2) && bu_rd16(pg_list_body + 1 + 4) == 1500 &&
                  bu_rd16(pg_list_body + 1) == 0);
            pg_end_n = 0;
            pg_got_n = 0;
            pg_got_slot = 34;
            gap = 0;
            pg_start(2, 34);
            for (passes = 0; passes < 100000 && bpg.kind; passes++) {
                frames_this_pass = 0;
                room = 1;
                pg_step();
            }
            check("get of slot 34: the blob byte for byte, end frame ok",
                  pg_end_n == 1 && pg_end_status == 0 && pg_got_n == 1500 && !memcmp(pg_got, ob, 1500) && !gap);
            pg_end_n = 0;
            pg_start(2, 40);
            for (passes = 0; passes < 10 && bpg.kind; passes++) {
                frames_this_pass = 0;
                room = 1;
                pg_step();
            }
            check("get of slot 40 (past the places): status 1", pg_end_n == 1 && pg_end_status == 1);
            k = (uint32_t)preset_erase_rx(34);
            check("erase slot 34: status 0, the slot empty again", k == 0 && st_preset_load(34, ob, sizeof ob) < 0);
        }
        /* busy: a backup streaming */
        pg_end_n = 0;
        got_n = 0;
        frames_this_pass = 0;
        room = 1;
        bu_start();
        pg_start(2, 0);
        check("get while a backup streams: status 3, the backup goes on", pg_end_n == 1 && pg_end_status == 3 && bu.on);
        bu_stop();
        /* busy: another get running */
        pg_end_n = 0;
        frames_this_pass = 0;
        room = 1;
        pg_got_n = 0;
        pg_got_slot = 0;
        pg_start(2, 0);
        frames_this_pass = 0;
        pg_start(2, 5);
        check("get while another get runs: status 3", pg_end_n == 1 && pg_end_status == 3 && bpg.slot == 0);
        pg_stop();
        check("... stopped: the saves free", holding == 0 && bpg.kind == PG_K_NONE);
        /* the cable cut / USB gone */
        pg_start(2, 0);
        room = -1;
        pg_step();
        check("USB gone: the get given up, the saves free", bpg.kind == PG_K_NONE && holding == 0);
        pg_start(2, 0);
        room = 0;
        t_ms += 100;
        pg_step();
        check("the ring never drains: waits, then gives up after the stall limit", bpg.kind == PG_K_GET);
        t_ms += BU_STALL_MS + 1;
        pg_step();
        check("... given up, the saves free", bpg.kind == PG_K_NONE && holding == 0);
        room = 1;
    }

    /* ---- the user presets over USB: put one, erase one (step B2) */
    {
        static uint8_t keep_img[sizeof img], blobs[3][ST_PAYLOAD_MAX], t[ST_PAYLOAD_MAX], t2[ST_PAYLOAD_MAX];
        static uint8_t start2[sizeof nor];
        uint32_t keep_n = img_n, i;
        volatile int k, total, mixed = 0, n_old = 0, n_new = 0, n_empty = 0;
        int rc;
        static const uint32_t npats[3] = {8, 1, 12};
        memcpy(keep_img, img, sizeof img);
        for (i = 0; i < 3; i++) {
            uint32_t n = 392u + 196u * npats[i];
            fill(blobs[i], n, 200 + i);
            memcpy(blobs[i], "TNP1", 4);
            bu_wr16(blobs[i] + 4, n);
            blobs[i][9] = (uint8_t)npats[i];
        }
#define SERVE(b, n) do { memset(img, 0, sizeof img); memcpy(img, (b), (n)); img_n = (n); } while (0)
#define PUTB(slot, b, n, crc) (nnotes = 0, fetches = 0, preset_put_rx((slot), (n), (crc)))
#define OTHERS_OK() (st_preset_load(0, t, sizeof t) == 3840 && st_preset_load(5, t, sizeof t) == 1 && \
                     st_preset_load(21, t, sizeof t) == 777)
        SERVE(blobs[0], 1960u);
        rc = PUTB(3, blobs[0], 1960u, bu_crc(0, blobs[0], 1960u));
        check("put: a 1960-byte preset into slot 3: stored (0), the host told OK",
              rc == 0 && nnotes == 1 && notes[0] == RS_OK_ADDR);
        check("... slot 3 loads as sent; the other slots untouched",
              st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960) && OTHERS_OK());
        SERVE(blobs[0], 1960u);                           /* wip668: a put into place 4 (slot 35) */
        rc = PUTB(35, blobs[0], 1960u, bu_crc(0, blobs[0], 1960u));
        check("put: a preset into slot 35 (place 4): stored, loads as sent, slot 3 untouched",
              rc == 0 && st_preset_load(35, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960) && st_preset_load(3, t, sizeof t) == 1960);
        st_preset_erase(35);
        {   /* wip669: the second spots over USB (slot byte bit 7) */
            static uint8_t xb[900], xg[900];
            uint32_t xi, xz = 1;
            int xp;
            fill(xb, sizeof xb, 55);
            xb[0] = 'T'; xb[1] = 'E'; xb[2] = 'X'; xb[3] = '2';
            check("second spots: slot 3 and place 3 (slot 34) saved", st_ext_save(3, xb, sizeof xb) == 0 && st_ext_save(34, xb, 300) == 0);
            pg_list_n = 0;
            pg_start(PG_SUB_XLIST, 0);
            for (xp = 0; xp < 5 && bpg.kind; xp++) { frames_this_pass = 0; room = 1; pg_step(); }
            check("second spots list (sub 5): 68 bytes, only slot 3 used, length 900",
                  pg_list_n == 1 && pg_list_len == 68u && bu_rd32(pg_list_body) == (1u << 3) && bu_rd16(pg_list_body + 4 + 6) == 900);
            pg_list_n = 0;
            pg_start(PG_SUB_XLISTX, 0);
            for (xp = 0; xp < 5 && bpg.kind; xp++) { frames_this_pass = 0; room = 1; pg_step(); }
            check("second spots of places 1-8 (sub 6): 17 bytes, only place 3, length 300",
                  pg_list_n == 1 && pg_list_len == 17u && pg_list_body[0] == (1u << 2) && bu_rd16(pg_list_body + 1 + 4) == 300);
            pg_end_n = 0; pg_got_n = 0; pg_got_slot = 0x83; gap = 0;
            pg_start(2, 0x83);
            for (xp = 0; xp < 100000 && bpg.kind; xp++) { frames_this_pass = 0; room = 1; pg_step(); }
            check("get of the second spot of slot 3 (0x83): byte for byte, the end frame says 0x83",
                  pg_end_n == 1 && pg_end_status == 0 && pg_end_slot == 0x83 && pg_got_n == 900 && !memcmp(pg_got, xb, 900) && !gap);
            SERVE(xb, 900u);
            rc = PUTB(0x85, xb, 900u, bu_crc(0, xb, 900u));
            for (xi = 0; xi < 900; xi++) xg[xi] = 0;
            check("put of a second spot into slot 5 (0x85): stored, the main spot of slot 5 untouched",
                  rc == 0 && st_ext_load(5, xg, sizeof xg) == 900 && !memcmp(xg, xb, 900) && st_preset_load(5, t, sizeof t) == 1);
            SERVE(blobs[0], 1960u);
            rc = PUTB(0x86, blobs[0], 1960u, bu_crc(0, blobs[0], 1960u));
            check("put of a main blob as a second spot: RS_E_HEAD, nothing written", rc == RS_E_HEAD && st_ext_load(6, xg, sizeof xg) < 0);
            SERVE(xb, 900u);
            rc = PUTB(0x80 | 22, xb, 900u, bu_crc(0, xb, 900u));
            check("put of a second spot into slot 22: RS_E_OBJ", rc == RS_E_OBJ);
            rc = preset_erase_rx(0x83);
            for (xi = 0; xi < 1; xi++) xz = (rc == 0 && pg_end_slot == 0x83 && st_ext_load(3, xg, sizeof xg) < 0 && st_preset_load(5, t, sizeof t) == 1);
            check("erase of the second spot of slot 3: status 0, only that spot gone, the presets untouched", xz);
            st_ext_erase(5);
            st_ext_erase(34);
        }
        playing = 1;
        SERVE(blobs[1], 588u);
        rc = PUTB(3, blobs[1], 588u, bu_crc(0, blobs[1], 588u));
        playing = 0;
        check("put while playing: refused, nothing fetched, the slot as it was",
              rc == RS_E_PLAYING && fetches == 0 && notes[0] == (RS_FAIL_ADDR | RS_E_PLAYING) &&
              st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960));
        rc = PUTB(3, blobs[1], 588u, bu_crc(0, blobs[1], 588u) ^ 1u);
        check("put with a wrong CRC-32: refused before any write, the slot as it was",
              rc == RS_E_CRC && st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960));
        SERVE(blobs[1], 588u);
        img[1] = 'X';                              /* not TNP1 (the CRC given is of the changed bytes) */
        rc = PUTB(3, img, 588u, bu_crc(0, img, 588u));
        check("put of a blob that is not TNP1: RS_E_HEAD, the slot as it was",
              rc == RS_E_HEAD && st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960));
        SERVE(blobs[1], 588u);
        bu_wr16(img + 4, 589u);                    /* the head's total disagrees with the length */
        rc = PUTB(3, img, 588u, bu_crc(0, img, 588u));
        check("put with a head whose total disagrees: RS_E_HEAD, the slot as it was",
              rc == RS_E_HEAD && st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960));
        SERVE(blobs[1], 588u);
        rc = PUTB(22, blobs[1], 588u, bu_crc(0, blobs[1], 588u));
        check("put into slot 22: RS_E_OBJ", rc == RS_E_OBJ);
        rc = PUTB(3, blobs[1], 100u, bu_crc(0, blobs[1], 100u));
        check("put of 100 bytes: RS_E_SIZE", rc == RS_E_SIZE);
        rc = PUTB(3, blobs[1], 3841u, 0);
        check("put of 3841 bytes: RS_E_SIZE", rc == RS_E_SIZE);
        fetch_left = 1;                            /* the cable cut in the first pass */
        rc = PUTB(3, blobs[1], 588u, bu_crc(0, blobs[1], 588u));
        fetch_left = -1;
        check("put with the cable cut in the first pass: RS_E_FETCH, the slot as it was",
              rc == RS_E_FETCH && st_preset_load(3, t, sizeof t) == 1960 && !memcmp(t, blobs[0], 1960));
        fetch_left = 2 + 0;                        /* 588 bytes = 2 pieces: the second pass fails at its first */
        rc = PUTB(3, blobs[1], 588u, bu_crc(0, blobs[1], 588u));
        fetch_left = -1;
        check("put with the cable cut in the second pass: RS_E_FETCH, THAT slot lost, the others untouched",
              rc == RS_E_FETCH && st_preset_load(3, t, sizeof t) < 0 && OTHERS_OK());
        SERVE(blobs[2], 2744u);
        rc = PUTB(3, blobs[2], 2744u, bu_crc(0, blobs[2], 2744u));
        check("put of a 12-pattern preset (2744 bytes) into the lost slot: stored",
              rc == 0 && st_preset_load(3, t, sizeof t) == 2744 && !memcmp(t, blobs[2], 2744) && OTHERS_OK());
        SERVE(blobs[1], 588u);
        rc = PUTB(3, blobs[1], 588u, bu_crc(0, blobs[1], 588u));
        check("put over a used slot: the new preset, whole",
              rc == 0 && st_preset_load(3, t, sizeof t) == 588 && !memcmp(t, blobs[1], 588));
        /* a power cut at every flash write of a put: that slot is the old preset, the new one or empty; the others whole */
        SERVE(blobs[0], 1960u);
        memcpy(start2, nor, sizeof nor);
        ops = 0;
        cut_at = 1 << 30;
        PUTB(3, blobs[0], 1960u, bu_crc(0, blobs[0], 1960u));
        total = ops;
        for (k = 0; k < total; k++) {
            int l;
            memcpy(nor, start2, sizeof nor);
            ops = 0;
            cut_at = k;
            if (!setjmp(cut)) {
                PUTB(3, blobs[0], 1960u, bu_crc(0, blobs[0], 1960u));
                mixed++;
                continue;
            }
            cut_at = -1;
            l = st_preset_load(3, t2, sizeof t2);
            if (l == 588 && !memcmp(t2, blobs[1], 588))
                n_old++;
            else if (l == 1960 && !memcmp(t2, blobs[0], 1960))
                n_new++;
            else if (l < 0)
                n_empty++;
            else
                mixed++;
            if (!OTHERS_OK())
                mixed++;
        }
        cut_at = -1;
        printf("  %d flash writes a put: the old preset %d, the new %d, empty %d\n", total, n_old, n_new, n_empty);
        check("a power cut at each write of a put: the slot old, new or empty, never a mix; the others whole",
              !mixed && n_empty && total > 4);
        memcpy(nor, start2, sizeof nor);
        /* erase */
        pg_end_n = 0;
        frames_this_pass = 0;
        room = 1;
        rc = preset_erase_rx(5);
        check("erase slot 5: status 0 in one frame, the slot empty, the others untouched",
              rc == 0 && pg_end_n == 1 && pg_end_slot == 5 && pg_end_status == 0 && st_preset_load(5, t, sizeof t) < 0 &&
              st_preset_load(0, t, sizeof t) == 3840 && st_preset_load(21, t, sizeof t) == 777);
        pg_end_n = 0;
        frames_this_pass = 0;
        playing = 1;
        rc = preset_erase_rx(21);
        playing = 0;
        check("erase while playing: status 4, the slot kept", rc == 4 && pg_end_status == 4 && st_preset_load(21, t, sizeof t) == 777);
        pg_end_n = 0;
        frames_this_pass = 0;
        rc = preset_erase_rx(22);
        check("erase slot 22: status 1", rc == 1 && pg_end_status == 1);
        rc = preset_erase_rx(5);
        check("erase of an empty slot: ok again (idempotent)", rc == 0);
        /* put the preset slots back as the later tests expect: slot 5 as it was (1 byte) is not needed again */
        memcpy(img, keep_img, sizeof img);
        img_n = keep_n;
        (void)i;
#undef SERVE
#undef PUTB
#undef OTHERS_OK
    }

    /* ---- a power cut at every flash write: old or new after the boot, whole */
    {
        static uint8_t start[sizeof nor];
        volatile int k, n_old = 0, n_new = 0, mixed = 0, total;
        save(OBJ_SET, 140, 4);
        save(OBJ_SOUND, 3000, 5);
        save(OBJ_SONG, 2000, 6);
        save(OBJ_PAT1, 100, 7);
        save(OBJ_MOTION, 3840, 8);
        snap(&set_b);
        memcpy(start, nor, sizeof nor);
        ops = 0;
        cut_at = 1 << 30;                         /* counting only */
        restore();
        total = ops;
        for (k = 0; k < total; k++) {
            memcpy(nor, start, sizeof nor);
            ops = 0;
            cut_at = k;
            if (!setjmp(cut)) {
                restore();
                printf("cut %d: not reached\n", k);
                mixed++;
                continue;
            }
            cut_at = -1;
            rs_boot();                            /* the next power-on */
            snap(&now);
            if (same(&now, &set_a))
                n_new++;
            else if (same(&now, &set_b))
                n_old++;
            else
                mixed++;
            if (nor[RS_REC] != 0xFF)
                mixed++;
        }
        cut_at = -1;
        printf("  %d flash writes a restore: cut before the record %d (old), after %d (new, finished at boot)\n", total,
               n_old, n_new);
        check("a power cut at each write: the old set or the new, whole, never a mix", !mixed && n_old && n_new);
        check("... the panel's calibration untouched through all of it", panel_same());
    }
    {
        uint8_t t[ST_PAYLOAD_MAX];
        check("the user presets untouched by backups, restores and the power cuts",
              st_preset_load(0, t, sizeof t) == 3840 && st_preset_load(5, t, sizeof t) < 0 &&
              st_preset_load(21, t, sizeof t) == 777);
    }
    printf("%s\n", fails ? "BACKUP TEST FAILED" : "backup test passed");
    return fails != 0;
}
