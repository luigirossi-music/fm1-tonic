/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X on the host: the whole app — UI, project, engine, sequencer, every DSP part — run
 * against a simulated FM-1 driven by a script. Audio goes to a WAV, the screen to PNGs,
 * the key / button lights to a text dump, MIDI out to a log. The device-only parts
 * (HAL, USB, OTA, flash driver) are replaced by plat.h implemented here.
 *
 *   x0x_host SCRIPT [OUTDIR]
 *
 * Script, one command per line (# comments):
 *   wait MS                      run the device for MS milliseconds
 *   press BTN | release BTN | tap BTN     (BTN: FX SEL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCT- OCT+)
 *   key K down|up | tapkey K     (K: 0..26 = F3..G5, or w0..w15 white keys, b0..b10 black keys)
 *   turn ENC N [MS]              (ENC: SELECT ALGO PRESET K1 K2 K3 K4; N detents, signed, MS apart, 80 = slow)
 *   spin ENC N                   N detents at once (1 ms), exactly N (no knob acceleration)
 *   master N                     MASTER pot 0..4096
 *   param T V I VALUE            a sound pot (engine.h target, voice, index), as its knob would
 *   slot K NAME A.wav [B.wav..]  user sample slot K (0..2): one loop (zone) per WAV, encoded here
 *   slotimg K F.hdr F.bin       slot K from tools/upload_breaks.py --dry-run's image (the device format)
 *   midi B0 B1 B2                incoming USB MIDI message (hex bytes)
 *   remch N                      the remote panel's MIDI channel 1-16 (0 = OFF)
 *   wav FILE | wavstop           start / stop recording the output
 *   load N                       the overload guard sees every block cost N% (-1: as measured)
 *   peakreset                    restart the output peak (expect peak_db_min / peak_db_max)
 *   shot FILE.png                save the screen
 *   leds                         print the lit buttons and keys
 *   savenow                      the project saved to the flash (what the SAVE button did before SAVE AS)
 *   expect WHAT VALUE            check state (playing, pattern, cue, view-part, ...): exit 1 on mismatch;
 *                                VALUE "<N" / ">N" is a bound
 *   reboot                       save nothing; re-run boot from the simulated flash (persistence test)
 *   projimg FILE                 a project as the FM-1 holds it in RAM (fm1_debug peek of proj): saved
 *                                to the simulated flash and booted, as if it had been made here
 *   backup FILE                  a backup over USB (firmware/src/backup.c's image: the six project objects) written
 *                                to FILE (in OUTDIR), as tools/x0x_backup.py writes it (stopped: the unsaved saved
 *                                first)
 *   restore FILE                 a backup (FILE in OUTDIR, or a path with a /) put back as the device does once SEL
 *                                answers RESTORE FROM USB?: checked whole first; refused while playing (STOP FIRST)
 *                                or when damaged: nothing changes (expect restore_rc: 0, 10 playing, else damaged)
 *   backup_damage FILE [head]    a byte of a backup file changed (its middle; head: made Polly's, CRCs right)
 *   listen                       a screen reader listens (announce.c's HELLO, again every 2 s); each SAY is printed
 *                                ("said: ..."; "said!:" cuts off what was being said)
 *   where                        the reader asks for the whole screen
 *   said TEXT                    some SAY since the last "said" (or "listen") has TEXT in it: else a failure
 *   said_none                    nothing said since the last check (no reader: nothing is sent)
 */
#define X0X_HOST 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#ifdef __APPLE__
#include <libproc.h>
#include <unistd.h>
#endif
#define __attribute__(x)

/* ------------------------------------------------------------ screen --- */
static uint16_t fb[240 * 240];          /* RGB565, byte-swapped as the panel takes it (gfx.c) */
static void lcd_sync(void) {}
static void lcd_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c)
{
    uint32_t i, j;
    uint16_t sc = (uint16_t)(((c >> 8) & 0xFFu) | ((c & 0xFFu) << 8));
    for (j = y; j < y + h && j < 240u; j++)
        for (i = x; i < x + w && i < 240u; i++)
            fb[j * 240u + i] = sc;
}
static uint32_t blits;
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *px)
{
    uint32_t i, j;
    blits++;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (x + i < 240u && y + j < 240u)
                fb[(y + j) * 240u + x + i] = px[j * w + i];
}
#include "../firmware/src/gfx.c"

/* ---------------------------------------------------------- platform --- */
#include "../firmware/src/app/plat.h"

static uint32_t now_ms, held_btn, held_keys, master = 2048;
static int32_t enc_acc[NE];
static uint32_t lit_btn, lit_keys, dim_keys, faint_keys;

uint32_t plat_ms(void) { return now_ms; }
uint32_t plat_buttons(void) { return held_btn; }
uint32_t plat_keys(void) { return held_keys; }
int32_t plat_enc(int role)
{
    int32_t v = enc_acc[role];
    enc_acc[role] = 0;
    return v;
}
uint32_t plat_master(void) { return master; }
int plat_battery(void) { return 3; }    /* the simulator: a full battery */
void plat_leds(uint32_t b, uint32_t k)
{
    lit_btn = b;
    lit_keys = k;
}
static int glow_on = 1;                 /* the device's button glow (plat_glow); the lights printed are the bright ones */
void plat_glow(int on) { glow_on = on; }
void plat_play_red(int red) { (void)red; }
void plat_keys_faint(uint32_t k) { faint_keys = k; }   /* faint keys print as 'o' too */
void plat_keys_dim(uint32_t k) { dim_keys = k; }      /* dim keys: "o" in the leds print */      /* the device's PLAY colour: green, red while recording */

#define MQ 256
static uint32_t min_q[MQ], mi_w, mi_r;
static FILE *midi_log;
int plat_midi_in(uint32_t *pkt)
{
    if (mi_r == mi_w)
        return 0;
    *pkt = min_q[mi_r++ % MQ];
    return 1;
}
static uint32_t midi_out_count, clock_out_count, note_ons;
void plat_midi_out(uint32_t pkt)
{
    midi_out_count++;
    if ((((pkt >> 8) & 0xF0) == 0x90) && ((pkt >> 24) & 0x7F))
        note_ons++;                                  /* the notes the sequence sent (expect noteons) */
    if (((pkt >> 8) & 0xFF) == 0xF8)
        clock_out_count++;
    else if (midi_log)
        fprintf(midi_log, "%u ms: %02X %02X %02X\n", now_ms, (pkt >> 8) & 0xFF, (pkt >> 16) & 0xFF, (pkt >> 24) & 0xFF);
}

/* flash: one buffer per object and the one before (storage.c's A/B copies), kept across a simulated
 * reboot; "tearsave N": the next save stops after N more object writes (a power cut mid-save) */
static uint8_t store[OBJ_NOBJ][PLAT_STORE_MAX], store_prev[OBJ_NOBJ][PLAT_STORE_MAX];
static int store_len[OBJ_NOBJ] = {-1, -1, -1, -1, -1, -1, -1}, store_prev_len[OBJ_NOBJ] = {-1, -1, -1, -1, -1, -1, -1};
static uint32_t store_writes;
static int store_tear = -1;                          /* object writes left before the cut; -1 none */
int plat_store_load(uint32_t obj, void *dst, uint32_t max)
{
    int n = store_len[obj];
    if (obj >= OBJ_NOBJ || n < 0)
        return -1;
    if ((uint32_t)n > max)
        n = (int)max;
    memcpy(dst, store[obj], (size_t)n);
    return n;
}
int plat_store_save(uint32_t obj, const void *src, uint32_t len)
{
    if (obj >= OBJ_NOBJ || len > PLAT_STORE_MAX)
        return -1;
    if (store_tear == 0)
        return -1;                                   /* the power is off: nothing more is written */
    if (store_tear > 0)
        store_tear--;
    memcpy(store_prev[obj], store[obj], PLAT_STORE_MAX);
    store_prev_len[obj] = store_len[obj];
    memcpy(store[obj], src, len);
    store_len[obj] = (int)len;
    store_writes++;
    return 0;
}
static uint8_t preset_slot[PLAT_NPRESET_ALL][PLAT_STORE_MAX];      /* the user preset slots (SAVE AS), kept across a reboot */
static int preset_len[PLAT_NPRESET_ALL];
int plat_preset_load(int slot, void *dst, uint32_t max)
{
    if (slot < 0 || slot >= PLAT_NPRESET_ALL || preset_len[slot] <= 0)
        return -1;
    memcpy(dst, preset_slot[slot], (uint32_t)preset_len[slot] < max ? (uint32_t)preset_len[slot] : max);
    return preset_len[slot];
}
int plat_preset_save(int slot, const void *src, uint32_t len)
{
    if (slot < 0 || slot >= PLAT_NPRESET_ALL || !len || len > PLAT_STORE_MAX)
        return -1;
    memcpy(preset_slot[slot], src, len);
    preset_len[slot] = (int)len;
    return 0;
}
int plat_store_load_prev(uint32_t obj, void *dst, uint32_t max)
{
    int n = obj < OBJ_NOBJ ? store_prev_len[obj] : -1;
    if (n < 0)
        return -1;
    if ((uint32_t)n > max)
        n = (int)max;
    memcpy(dst, store_prev[obj], (size_t)n);
    return n;
}

/* sample slots, two ways in:
 *  - slotimg: a slot image exactly as tools/upload_breaks.py writes it, read by the firmware's own
 *    slots.c (the device path: what the upload tool writes is what the firmware parses);
 *  - slot: WAVs encoded here, one zone each (quick, for scenarios that do not test the format). */
static uint8_t slot_img[3][0x14000];
static int slot_is_img[3];
#define SMP_USER_XIP(k) ((const uint8_t *)slot_img[k])
static uint32_t st_crc32(const void *p, uint32_t n)
{
    const uint8_t *b = p;
    uint32_t c = 0xFFFFFFFFu, k;
    while (n--) {
        c ^= *b++;
        for (k = 0; k < 8; k++)
            c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
    }
    return ~c;
}
#include "../firmware/src/app/slots.c"

/* WAV slots */
static uint8_t *slot_data[PLAT_NSLOTS][PLAT_SLOT_ZONES];
static uint32_t slot_n[PLAT_NSLOTS][PLAT_SLOT_ZONES], slot_rate[PLAT_NSLOTS][PLAT_SLOT_ZONES];
static int slot_nz[PLAT_NSLOTS];
static char slot_name[PLAT_NSLOTS][9];
int plat_slot_zones(int k)
{
    if (k < 0 || k >= PLAT_NSLOTS)
        return 0;
    return slot_is_img[k] ? usr_nz[k] : slot_nz[k];
}
const uint8_t *plat_slot(int k, int z, uint32_t *ns, uint32_t *rate, char name[9])
{
    if (k >= 0 && k < PLAT_NSLOTS && slot_is_img[k]) {    /* plat_fm1.c's plat_slot, verbatim */
        const smp_user_hdr_t *h;
        uint32_t i;
        if (z < 0 || z >= usr_nz[k])
            return 0;
        h = (const smp_user_hdr_t *)smp_user_xip((uint32_t)k);
        for (i = 0; i < 8u; i++)
            name[i] = h->name[i] >= 32 && h->name[i] < 127 ? h->name[i] : 0;
        name[8] = 0;
        *ns = h->zone[z].n;
        *rate = (h->zone[z].rate * 44100u + 32768u) >> 16;
        return smp_user_xip((uint32_t)k) + SMP_USER_DATA + h->zone[z].off;
    }
    if (k < 0 || k >= PLAT_NSLOTS || z < 0 || z >= slot_nz[k])
        return 0;
    *ns = slot_n[k][z];
    *rate = slot_rate[k][z];
    memcpy(name, slot_name[k], 9);
    return slot_data[k][z];
}

static uint32_t cpu_pct, cpu_peak;
static int load_force = -1;                      /* script "load N": the guard sees N%, -1 measured */
/* the second core, simulated: a job runs at once, in order (the split path, the same sums) */
static int cpu2_on;
int plat_cpu2_set(int on) { cpu2_on = on; return 0; }
int plat_cpu2_on(void) { return cpu2_on; }
int plat_cpu2_run(void (*fn)(uint32_t), uint32_t arg)
{
    if (!cpu2_on)
        return 0;
    fn(arg);
    return 1;
}
int plat_cpu2_wait(void) { return 0; }
int plat_cpu2_gave_up(void) { return 0; }
uint32_t plat_cpu2_bad(void) { return 0; }
int plat_cpu2_barred(void) { return 0; }
int plat_vdc14_mv(void) { return 1600; }
int plat_sysvdd_mv(void) { return 1380; }
static int host_crash;                            /* "fakecrash": PERF shows a crash record */
int plat_last_crash(uint32_t v[5])
{
    static const uint32_t r[5] = {1, 0x03000470u, 0, 0x00001000u, 0x02007D80u};   /* fm1-x0x#21's first */
    for (int i = 0; i < 5; i++)
        v[i] = r[i];
    return host_crash;
}
uint32_t plat_cpu_pct(void) { return cpu_pct; }
uint32_t plat_xruns(void) { return 0; }
/* performance: no CPU counter here: the "cycles" are nanoseconds (plat_cycles_hz) */
static uint64_t host_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}
void plat_perf_init(void) {}
uint32_t plat_cycles(void) { return (uint32_t)host_ns(); }
uint32_t plat_cycles_hz(void) { return 1000000000u; }
int plat_cycles_cpu(void) { return 0; }
uint32_t plat_ticks24(void) { return (uint32_t)(host_ns() * 24u / 1000u); }
uint32_t plat_irq_stack_used(void) { return 0; }
uint32_t plat_irq_stack_size(void) { return 0; }
uint32_t plat_cpu_peak_pct(void)
{
    uint32_t p = cpu_peak;
    cpu_peak = 0;
    return p;
}
int plat_stalls(uint32_t s[3]) { (void)s; return -1; }
void plat_stalls_enable(int on) { (void)on; }

/* -------------------------------------------------------------- app --- */
#include "../firmware/src/app/x0x.h"
#define ANN_NAME "X0X " X0X_VERSION
#include "ann_words.h"                                  /* the words file's link (tools/gen_words.py) */
#include "../firmware/src/announce.c"
/* what the reader is told: the last SAY (expect said), every one to stdout with "said:" (listen) */
static char said_last[ANN_MAX + 1], said_log[16384];   /* said_log: every SAY since the last "said" check */
static uint32_t said_count, said_len;
static int said_print = 1, sim_listen;
static FILE *said_log_f;                                /* X0X_SAID_LOG=FILE: every SAY, for fm1-reader's audit */
#ifdef X0X_WEB
static void web_said(const uint8_t *p, uint32_t n);
#endif
static int ann_wire_send(const uint8_t *p, uint32_t n)
{
#ifdef X0X_WEB
    web_said(p, n);
#endif
    if (n >= 7u && p[4] == ANN_SAY) {
        uint32_t i, k = 0;
        for (i = 6; i + 1u < n && k < ANN_MAX; i++)
            said_last[k++] = (char)p[i];
        said_last[k] = 0;
        said_count++;
        if (said_len + k + 2u < sizeof said_log) {
            memcpy(said_log + said_len, said_last, k);
            said_len += k;
            said_log[said_len++] = '\n';
            said_log[said_len] = 0;
        }
#ifndef X0X_WEB
        if (said_print)
            printf("said%s: %s\n", (p[5] & ANN_NOW) ? "!" : "", said_last);
        if (said_log_f)
            fprintf(said_log_f, "%s\t%s\n", (p[5] & ANN_KEEP) ? "WARN" : "MSG", said_last);
#endif
    }
    return 0;
}
static int preset_rc = -99;
#include "../firmware/src/app/project.c"
#include "../firmware/src/app/ui.c"

/* ---------------------------------------------------------- outputs --- */
static void png_write(const char *path)
{
    /* RGB PNG with stored (uncompressed) deflate blocks: no zlib needed */
    static uint8_t raw[240 * (1 + 240 * 3)];
    FILE *f = fopen(path, "wb");
    uint32_t crc_t[256], i, j, k, n = 0, a = 1, b = 0, pos, len = sizeof raw;
    uint8_t hdr[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (!f) {
        fprintf(stderr, "cannot write %s\n", path);
        return;
    }
    for (i = 0; i < 256; i++) {
        uint32_t c = i;
        for (k = 0; k < 8; k++)
            c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        crc_t[i] = c;
    }
    for (j = 0; j < 240; j++) {
        raw[n++] = 0;
        for (i = 0; i < 240; i++) {
            uint16_t p = fb[j * 240 + i];
            p = (uint16_t)((p >> 8) | (p << 8));
            raw[n++] = (uint8_t)(((p >> 11) & 31) * 255 / 31);
            raw[n++] = (uint8_t)(((p >> 5) & 63) * 255 / 63);
            raw[n++] = (uint8_t)((p & 31) * 255 / 31);
        }
    }
    for (i = 0; i < len; i++) {
        a = (a + raw[i]) % 65521;
        b = (b + a) % 65521;
    }
#define PUT32(v) do { uint8_t q_[4] = {(uint8_t)((v) >> 24), (uint8_t)((v) >> 16), (uint8_t)((v) >> 8), (uint8_t)(v)}; fwrite(q_, 1, 4, f); } while (0)
    fwrite(hdr, 1, 8, f);
    {
        uint8_t ih[17] = {'I', 'H', 'D', 'R', 0, 0, 0, 240, 0, 0, 0, 240, 8, 2, 0, 0, 0};
        uint32_t c = 0xFFFFFFFFu;
        PUT32(13u);
        fwrite(ih, 1, 17, f);
        for (i = 0; i < 17; i++)
            c = crc_t[(c ^ ih[i]) & 255] ^ (c >> 8);
        PUT32(~c);
    }
    {
        uint32_t nblk = (len + 65534) / 65535, zlen = 2 + len + nblk * 5 + 4, c = 0xFFFFFFFFu;
        uint8_t *z = malloc(zlen + 4), *q = z;
        memcpy(q, "IDAT", 4);
        q += 4;
        *q++ = 0x78;
        *q++ = 0x01;
        for (pos = 0; pos < len;) {
            uint32_t m = len - pos > 65535 ? 65535 : len - pos;
            *q++ = pos + m >= len ? 1 : 0;
            *q++ = (uint8_t)m;
            *q++ = (uint8_t)(m >> 8);
            *q++ = (uint8_t)~m;
            *q++ = (uint8_t)(~m >> 8);
            memcpy(q, raw + pos, m);
            q += m;
            pos += m;
        }
        *q++ = (uint8_t)(b >> 8);
        *q++ = (uint8_t)b;
        *q++ = (uint8_t)(a >> 8);
        *q++ = (uint8_t)a;
        PUT32(zlen);
        fwrite(z, 1, zlen + 4, f);
        for (i = 0; i < zlen + 4; i++)
            c = crc_t[(c ^ z[i]) & 255] ^ (c >> 8);
        PUT32(~c);
        free(z);
    }
    {
        uint8_t ie[4] = {'I', 'E', 'N', 'D'};
        uint32_t c = 0xFFFFFFFFu;
        PUT32(0u);
        fwrite(ie, 1, 4, f);
        for (i = 0; i < 4; i++)
            c = crc_t[(c ^ ie[i]) & 255] ^ (c >> 8);
        PUT32(~c);
    }
    fclose(f);
}

static FILE *wav;
static uint32_t wav_frames;
static float peak_out;
static void wav_open(const char *path)
{
    static const uint8_t h[44] = {0};
    wav = fopen(path, "wb");
    if (wav)
        fwrite(h, 1, 44, wav);
    wav_frames = 0;
}
static void wav_close(void)
{
    uint32_t bytes = wav_frames * 4, v;
    if (!wav)
        return;
    fseek(wav, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, wav);
    v = 36 + bytes;
    fwrite(&v, 4, 1, wav);
    fwrite("WAVEfmt ", 1, 8, wav);
    v = 16;
    fwrite(&v, 4, 1, wav);
    {
        uint16_t fmt[2] = {1, 2};
        uint32_t r[2] = {44100, 44100 * 4};
        uint16_t al[2] = {4, 16};
        fwrite(fmt, 2, 2, wav);
        fwrite(r, 4, 2, wav);
        fwrite(al, 2, 2, wav);
    }
    fwrite("data", 1, 4, wav);
    fwrite(&bytes, 4, 1, wav);
    fclose(wav);
    wav = 0;
}

/* --------------------------------------------------------- the device --- */
static double audio_due_ms;
static uint32_t ui_due;
static uint8_t ui_hold_ver;                       /* the browser: after the welcome screen the version is shown big for 500 ms (wip215)  */
static uint32_t ui_hold_until;                    /* the browser: the welcome screen stays until this time (0 = no hold) */
static double render_ns_total, render_budget_ns_total;
/* instructions the kernel counted in engine_render (Felucca's tests/regress.c measure: the same
 * on every run, unlike wall time): per sample overall, and the worst block */
static uint64_t ins_total, ins_frames, ins_block_max;
/* the engine's stage timing (eng_prof, in plat_cycles() units: here nanoseconds), summed per block
 * while playing; printed with X0X_PROFILE=1 in the environment */
static uint64_t prof_tot[ENG_PROF_N];
static uint64_t instr_now(void)
{
#ifdef __APPLE__
    struct rusage_info_v4 ri;
    if (!proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ri))
        return ri.ri_instructions;
#endif
    return 0;
}

#include <time.h>
static double now_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

static void boot(void)
{
    engine_init(proj.pat, &proj.arr.song, proj.arr.lane, &proj.sound);
    engine_notes(proj.pat, &proj.note[0][0][0]);
    if (project_load() != 0)
        project_defaults();
    engine_apply_sound(&proj.sound);
    seq.bpm = (float)proj.set.bpm_x10 / 10.0f;
    seq.accent_q7 = proj.set.accent_q7;
    project_apply_io();                             /* clock and notes out, the MIDI channels, the output */
    engine_brk_loops();
    lcd_fill(0, 0, 240, 240, C_BLACK);
    ui_init();
}

static uint8_t last_pat = 0xFF;
static void run_ms(uint32_t ms)
{
    static int32_t blk[512];
    uint32_t i;
    for (i = 0; i < ms; i++) {
        now_ms++;
        while (audio_due_ms <= (double)now_ms) {      /* 256-frame half buffers, as the I2S DMA */
            double t0 = now_ns(), dt;
            uint32_t k;
            uint64_t i0 = instr_now(), di;
            uint32_t pr0[ENG_PROF_N];
            for (k = 0; k < ENG_PROF_N; k++)
                pr0[k] = eng_prof[k];
            engine_render(blk, 256);
            engine_volume(blk, 256);
#ifdef X0X_WEB
            web_audio(blk, 256);                         /* the browser build: to the AudioWorklet */
#endif
            di = instr_now() - i0;
            dt = now_ns() - t0;
            if (seq.playing) {                           /* the cost that matters: while it plays */
                for (k = 0; k < ENG_PROF_N; k++)
                    prof_tot[k] += (uint32_t)(eng_prof[k] - pr0[k]);
                ins_total += di;
                ins_frames += 256;
                if (di > ins_block_max)
                    ins_block_max = di;
            }
            render_ns_total += dt;
            render_budget_ns_total += 256.0 * 1e9 / 44100.0;
            cpu_pct = (uint32_t)(100.0 * dt / (256.0 * 1e9 / 44100.0));
            if (cpu_pct > cpu_peak)
                cpu_peak = cpu_pct;
#ifdef X0X_WEB
            engine_load(load_force >= 0 ? (uint32_t)load_force : 0u, 256);   /* a browser's timing is not the FM-1's */
#else
            engine_load(load_force >= 0 ? (uint32_t)load_force : cpu_pct, 256);
#endif
            for (k = 0; k < 256; k++) {
                float l = (float)blk[2 * k] / 8388608.0f, r = (float)blk[2 * k + 1] / 8388608.0f;
                int16_t s[2];
                if (fabsf(l) > peak_out)
                    peak_out = fabsf(l);
                if (fabsf(r) > peak_out)
                    peak_out = fabsf(r);
                s[0] = (int16_t)(l * 32767.0f);
                s[1] = (int16_t)(r * 32767.0f);
                if (wav) {
                    fwrite(s, 2, 2, wav);
                    wav_frames++;
                }
            }
            audio_due_ms += 256.0 * 1000.0 / 44100.0;
        }
        if (seq.ppat[TRK_BRK] != last_pat) {
            last_pat = seq.ppat[TRK_BRK];
            engine_brk_loops();
        }
        if (sim_listen && now_ms % 2000u == 0) {        /* the reader's HELLO, as web/reader sends it */
            static const uint8_t HELLO[] = {ANN_H0, ANN_H1, ANN_H2, ANN_HELLO, ANN_VER};
            ann_frame(HELLO, sizeof HELLO, now_ms);
        }
        if (ui_hold_until && now_ms >= ui_hold_until) {
            if (ui_hold_ver) {
                ui_hold_ver = 0;
                welcome_version(TONIC_VERSION);         /* the version, big, half a second more */
                ui_hold_until = now_ms + 500u;
            } else {
                ui_hold_until = 0;
                ui_redraw();                            /* the welcome screen is over: the whole screen again */
            }
        }
        if (ui_hold_until) {
            /* the welcome screen stays (the browser build only) */
        } else if (now_ms >= ui_due) {
            ui_due = now_ms + 16;
            ui_frame();
        } else {
            ui_input_only();
        }
    }
}

/* ---------------------------------------------------------- script --- */
static const char *const BTN_N[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                      "PLAY", "REC", "OCT-", "OCT+"};
static const char *const ENC_N[NE] = {"SELECT", "ALGO", "PRESET", "K1", "K2", "K3", "K4"};
static const int WHITE_K[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
static const int BLACK_K[11] = {1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25};

static int find_name(const char *const *t, int n, const char *s)
{
    int i;
    for (i = 0; i < n; i++)
        if (!strcmp(t[i], s))
            return i;
    return -1;
}

static int key_of(const char *s)
{
    if (s[0] == 'w')
        return WHITE_K[atoi(s + 1) & 15];
    if (s[0] == 'b')
        return BLACK_K[atoi(s + 1) % 11];
    return atoi(s);
}

/* IMA ADPCM encoder (tools/sampleio.py's) */
static const int16_t IMA_STEP_T[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
    107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428,
    4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
    22385, 24623, 27086, 29794, 32767};
static const int8_t IMA_IDX_T[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
static void ima_encode(const int16_t *x, uint32_t n, uint8_t *out)
{
    int pred = 0, idx = 0;
    uint32_t i;
    for (i = 0; i < n; i++) {
        int step = IMA_STEP_T[idx], diff = x[i] - pred, code = 0, vd = step >> 3;
        if (diff < 0) {
            code = 8;
            diff = -diff;
        }
        if (diff >= step) {
            code |= 4;
            diff -= step;
            vd += step;
        }
        if (diff >= step >> 1) {
            code |= 2;
            diff -= step >> 1;
            vd += step >> 1;
        }
        if (diff >= step >> 2) {
            code |= 1;
            vd += step >> 2;
        }
        pred = code & 8 ? pred - vd : pred + vd;
        pred = pred > 32767 ? 32767 : pred < -32768 ? -32768 : pred;
        idx += IMA_IDX_T[code & 7];
        idx = idx < 0 ? 0 : idx > 88 ? 88 : idx;
        if (i & 1)
            out[i >> 1] |= (uint8_t)(code << 4);
        else
            out[i >> 1] = (uint8_t)code;
    }
}

static int load_slot(int k, int z, const char *path)
{
    FILE *f = fopen(path, "rb");
    uint8_t h[12];
    uint32_t rate = 44100, n = 0;
    uint16_t ch = 1, bits = 16;
    int16_t *x = 0;
    if (!f)
        return -1;
    if (fread(h, 1, 12, f) != 12 || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) {
        fclose(f);
        return -1;
    }
    for (;;) {
        uint8_t c[8];
        uint32_t sz;
        if (fread(c, 1, 8, f) != 8)
            break;
        sz = (uint32_t)c[4] | (uint32_t)c[5] << 8 | (uint32_t)c[6] << 16 | (uint32_t)c[7] << 24;
        if (!memcmp(c, "fmt ", 4)) {
            uint8_t fm[16];
            if (fread(fm, 1, 16, f) != 16)
                break;
            ch = (uint16_t)(fm[2] | fm[3] << 8);
            rate = (uint32_t)fm[4] | (uint32_t)fm[5] << 8 | (uint32_t)fm[6] << 16 | (uint32_t)fm[7] << 24;
            bits = (uint16_t)(fm[14] | fm[15] << 8);
            fseek(f, (long)(sz - 16 + (sz & 1)), SEEK_CUR);
        } else if (!memcmp(c, "data", 4)) {
            uint32_t frames = sz / (ch * (bits / 8u));
            uint8_t *raw = malloc(sz);
            if (fread(raw, 1, sz, f) != sz) {
                free(raw);
                break;
            }
            /* to mono 22050 (the slots' rate): average channels, decimate by 2 when 44.1k */
            {
                uint32_t step = rate >= 44100 ? 2 : 1, j;
                x = malloc(sizeof(int16_t) * (frames / step + 1));
                for (j = 0; j + step <= frames; j += step) {
                    int32_t acc = 0;
                    uint32_t s, cc;
                    for (s = 0; s < step; s++)
                        for (cc = 0; cc < ch; cc++) {
                            const uint8_t *p = raw + ((j + s) * ch + cc) * (bits / 8u);
                            int32_t v = bits == 16 ? (int16_t)(p[0] | p[1] << 8) : (int32_t)((p[0] | p[1] << 8 | p[2] << 16) << 8) >> 16;
                            acc += v;
                        }
                    x[n++] = (int16_t)(acc / (int32_t)(step * ch));
                }
                rate /= step;
            }
            free(raw);
            break;
        } else {
            fseek(f, (long)(sz + (sz & 1)), SEEK_CUR);
        }
    }
    fclose(f);
    if (!x || !n)
        return -1;
    if (n > 2u * (0x14000u - 512u))
        n = 2u * (0x14000u - 512u);                  /* an 80 KiB slot */
    free(slot_data[k][z]);
    slot_data[k][z] = calloc(1, (n + 1) / 2 + 1);
    ima_encode(x, n, slot_data[k][z]);
    slot_n[k][z] = n;
    slot_rate[k][z] = rate;
    if (z + 1 > slot_nz[k])
        slot_nz[k] = z + 1;
    free(x);
    return 0;
}

extern uint32_t eng_hit_n[8];                    /* engine.c (host): hits per Tonic voice */
static pattern_t pat_snap[NPAT];                  /* "snapproj": the patterns, for "expect projsame" */
/* ------------------------------------------- backup / restore over USB --- */
/* backup.c's image, made and taken from the simulated store (the device's side and its safety: tests/backup_test.c) */
static int restore_rc = -1;
static uint32_t sim_crc32(uint32_t c, const uint8_t *p, size_t n)
{
    c = ~c;
    while (n--) {
        int k;
        c ^= *p++;
        for (k = 0; k < 8; k++)
            c = c & 1u ? 0xEDB88320u ^ (c >> 1) : c >> 1;
    }
    return ~c;
}
static void sim_wr32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v, p[1] = (uint8_t)(v >> 8), p[2] = (uint8_t)(v >> 16), p[3] = (uint8_t)(v >> 24); }
static uint32_t sim_rd32(const uint8_t *p) { return p[0] | p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static const uint8_t SIM_OBJ[6] = {OBJ_SET, OBJ_SOUND, OBJ_PAT0, OBJ_PAT1, OBJ_SONG, OBJ_MOTION};
static uint8_t sim_img[16 + 6 * (8 + PLAT_STORE_MAX) + 4];
static int sim_backup(const char *path)
{
    size_t n = 16;
    FILE *f;
    int i;
    if (!seq.playing)
        ui_flush();
    memcpy(sim_img, "X0XB", 4);
    sim_img[4] = 1, sim_img[5] = 0, sim_img[6] = 6, sim_img[7] = 0;
    sim_wr32(sim_img + 8, (uint32_t)ui_dirty());
    sim_wr32(sim_img + 12, sim_crc32(0, sim_img, 12));
    for (i = 0; i < 6; i++) {
        uint32_t o = SIM_OBJ[i], len = store_len[o] > 0 ? (uint32_t)store_len[o] : 0;
        uint8_t *r = sim_img + n;
        r[0] = (uint8_t)o, r[1] = 0, r[2] = (uint8_t)len, r[3] = (uint8_t)(len >> 8);
        memcpy(r + 8, store[o], len);
        sim_wr32(r + 4, sim_crc32(0, r + 8, len));
        n += 8 + len;
    }
    sim_wr32(sim_img + n, sim_crc32(0, sim_img, n));
    n += 4;
    if (!(f = fopen(path, "wb")))
        return -1;
    fwrite(sim_img, 1, n, f);
    fclose(f);
    return 0;
}
static int sim_restore(const char *path)
{
    size_t n, pos = 16;
    FILE *f = fopen(path, "rb");
    int i;
    if (!f)
        return 1;
    n = fread(sim_img, 1, sizeof sim_img, f);
    fclose(f);
    if (seq.playing || seq.req_stop)
        return 10;                                /* STOP FIRST */
    if (n < 16 + 6 * 8 + 4 || memcmp(sim_img, "X0XB", 4) || sim_img[4] != 1 || sim_img[5] || sim_img[6] != 6 ||
        sim_img[7] || sim_rd32(sim_img + 12) != sim_crc32(0, sim_img, 12))
        return 3;
    for (i = 0; i < 6; i++) {                     /* all checked before anything changes */
        const uint8_t *r = sim_img + pos;
        uint32_t len = r[2] | r[3] << 8;
        if (pos + 8 + len + 4 > n || r[0] != SIM_OBJ[i] || r[1] || len > PLAT_STORE_MAX)
            return 4;
        if (sim_rd32(r + 4) != sim_crc32(0, r + 8, len))
            return 5;
        pos += 8 + len;
    }
    if (pos + 4 != n || sim_rd32(sim_img + pos) != sim_crc32(0, sim_img, pos))
        return 6;
    pos = 16;
    for (i = 0; i < 6; i++) {
        const uint8_t *r = sim_img + pos;
        uint32_t o = SIM_OBJ[i], len = r[2] | r[3] << 8;
        memcpy(store_prev[o], store[o], PLAT_STORE_MAX);   /* (the copy before: the project as it was) */
        store_prev_len[o] = store_len[o];
        memcpy(store[o], r + 8, len);
        store_len[o] = len ? (int)len : -1;       /* (an empty object: none, as storage.c's st_load) */
        pos += 8 + len;
    }
    app_restored();
    return 0;
}

static int last_pot;                               /* wip386: the pot read by the last `potget` */
static int expect(const char *what, const char *val)
{
    int got;
    if (!strcmp(what, "playing"))
        got = seq.playing;
    else if (!strcmp(what, "restore_rc"))          /* the last restore: 0 done, 10 playing (STOP FIRST), else refused */
        got = restore_rc;
    else if (!strcmp(what, "dirty"))
        got = ui_dirty();
    else if (!strcmp(what, "pattern"))
        got = seq.ppat[PART_909] + 1;
    else if (!strcmp(what, "cue"))
        got = seq_cue_of(&seq, PART_909) + 1;
    else if (!strncmp(what, "ppat", 4))            /* ppatP: part P's pattern, 1-based */
        got = seq.ppat[atoi(what + 4) % NPARTS] + 1;
    else if (!strcmp(what, "perftest"))            /* -1 idle, 0..2 running, 3 done */
        got = perf.test;
    else if (!strncmp(what, "perfres", 7))         /* perfresK: scenario K's share of the CPU x 10 */
        got = perf.res_load[atoi(what + 7) % 3];
    else if (!strcmp(what, "grdb10"))              /* the master compressor's gain reduction on the screen's meter, dB x 10 */
        got = (int)(engine_gr_db() * 10.0f + 0.5f);
    else if (!strcmp(what, "guard"))               /* the overload guard: 1 engaged */
        got = engine_guard_active();
    else if (!strcmp(what, "guardcount"))
        got = (int)eng_guard_count;
    else if (!strcmp(what, "patptr"))              /* 1: the sequencer plays the project's patterns */
        got = seq.pat == proj.pat;
    else if (!strcmp(what, "songon"))
        got = seq.song_on;
    else if (!strcmp(what, "songlen"))
        got = proj.arr.song.len;
    else if (!strcmp(what, "songpos"))
        got = seq.song_pos + 1;
    else if (!strncmp(what, "songbar", 7)) {       /* songbarK.P: bar K (1-based) part P's pattern, 1-based; P 5 = mutes */
        int k, q;
        sscanf(what, "songbar%d.%d", &k, &q);
        got = q < NPARTS ? proj.arr.song.bar[k - 1].pat[q] + 1 : proj.arr.song.bar[k - 1].mute;
    } else if (!strcmp(what, "lanes"))
        got = motion_count(proj.arr.lane);
    else if (!strncmp(what, "motion", 6)) {        /* motionT.V.I: what that knob's lane plays now (-1 none) */
        int t, v, i, part;
        sscanf(what, "motion%d.%d.%d", &t, &v, &i);
        part = engine_motion_part(t, v);
        got = engine_motion_value(motion_find(proj.arr.lane, seq.ppat[part], part, t, v, i));
    } else if (!strncmp(what, "laneval", 7)) {     /* lanevalT.V.I.S: the lane's value on step S (255 none, -1 no lane) */
        int t, v, i, st, part, k;
        sscanf(what, "laneval%d.%d.%d.%d", &t, &v, &i, &st);
        part = engine_motion_part(t, v);
        k = motion_find(proj.arr.lane, seq.ppat[part], part, t, v, i);
        got = k < 0 ? -1 : proj.arr.lane[k].val[st];
        if (k >= 0 && getenv("X0X_LANEDUMP")) {
            int j;
            for (j = 0; j < 16; j++)
                printf(" %d", proj.arr.lane[k].val[j]);
            printf("  (lane %d)\n", k);
        }
    }
    else if (!strncmp(what, "playhead", 8))        /* playheadT: track T's step (1-based) */
        got = eng_step[atoi(what + 8)] + 1;
    else if (!strncmp(what, "brkslice", 8))        /* brksliceS: step S's own slice (1-8), 0 = the generator's */
        got = proj.pat[seq.ppat[PART_BRK]].brk.slice[atoi(what + 8)];
    else if (!strcmp(what, "brknow"))               /* the slice sounding now, 1-8, 0 none */
        got = engine_brk_slice() + 1;
    else if (!strcmp(what, "mempct"))
        got = project_mem_pct();
    else if (!strcmp(what, "part"))
        got = ui.part;
    else if (!strcmp(what, "help"))                 /* the help card's button + 1, 0 none */
        got = ui.help;
    else if (!strcmp(what, "helpfits"))
        got = help_fits();
    else if (!strcmp(what, "rec"))
        got = ui.rec;
    else if (!strcmp(what, "spage"))                /* wip240: the step page the white keys show, 0 = 1-16 .. 3 = 49-64 */
        got = ui.spage;
    else if (!strcmp(what, "autoscroll"))
        got = proj.set.autoscroll;
    else if (!strcmp(what, "arp"))                  /* wip251: 1 = the SEQ view is the ARP page */
        got = ui.arp;
    else if (!strcmp(what, "arpoct"))               /* the ARP keyboard's octave, -3..+3 */
        got = ui.arp_oct;
    else if (!strcmp(what, "seqnote"))              /* wip253: the note of the last hit the sequencer played */
        got = engine_last_seq_note();
    else if (!strcmp(what, "arpnote"))              /* the semitone offset of the last ARP key played */
        got = engine_last_note();
    else if (!strcmp(what, "sel"))                  /* the selected track of the part on screen */
        got = ui.sel[ui.part];
    else if (!strncmp(what, "knobdef", 7)) {        /* knobdefK: knob K (1-4) on this page at its power-on value */
        pref_t r = pg.r[cur_page()][(atoi(what + 7) - 1) & 3];
        const x0x_param_t *d = pref_desc(r);
        got = d && pref_get(r) == d->def;
    } else if (!strcmp(what, "projsame"))           /* 1: the patterns are as at the last snapproj */
        got = memcmp(proj.pat, pat_snap, sizeof pat_snap) == 0;
    else if (!strcmp(what, "zoomoff"))              /* wip321: proj.set.zoom_off (a settings row) */
        got = proj.set.zoom_off;
    else if (!strcmp(what, "namemanual"))           /* wip324: GLO > NAME PROJECTS: 0 GENERATE, 1 MANUAL */
        got = proj.set.name_manual;
    else if (!strcmp(what, "nmlen"))                /* wip324: MANUAL name: the letters accepted so far */
        got = ui.nm_len;
    else if (!strcmp(what, "nmcand"))               /* ... and the blinking letter (its character code) */
        got = (unsigned char)NM_SET[ui.nm_ch];
    else if (!strncmp(what, "svname", 6))           /* svnameN: the character code at place N of the name being typed (0 = the end) */
        got = (unsigned char)ui.sv_name[atoi(what + 6) & 7];
    else if (!strcmp(what, "svhere"))               /* wip319: SAVE HERE?: the place it would save into (1-40) */
        got = ui.sv_here;
    else if (!strcmp(what, "svstage"))              /* SAVE AS: the screen 0-2, -1 = not open */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_SAVEAS) ? (int)ui.ask_arg : -1;
    else if (!strcmp(what, "seqpat"))                /* SEQ 2nd press: the pattern number screen is on */
        got = ui.seqpat;
    else if (!strcmp(what, "songmode"))              /* SONG MODE on = 1 */
        got = ui.songmode;
    else if (!strcmp(what, "cpm"))                   /* wip362: COPY-PASTE MODE (FUNC + ACCENT) on = 1 */
        got = ui.cpm & 1;
    else if (!strcmp(what, "cpsticky"))              /* wip377: the mode was kept with FUNC (many pastes) = 1 */
        got = (ui.cpm >> 2) & 1;
    else if (!strncmp(what, "tn16.", 5)) {           /* tn16.V.I: a Tonic voice's exact 16-bit value */
        int v_ = 0, i_ = 0;
        sscanf(what, "tn16.%d.%d", &v_, &i_);
        got = proj.sound.tn16[v_ & 7][i_ % TN_NPARAMS];
    } else if (!strcmp(what, "songask"))               /* the ENTER SONG MODE question: 1 = open */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_SONGMODE) ? 1 : (ui.overlay == O_ASK && ui.ask_act == ACT_SONGEXIT) ? 2 : (ui.overlay == O_ASK && ui.ask_act == ACT_SONGKILL) ? 3 : 0;
    else if (!strcmp(what, "clearask"))             /* FUNC + PLAY's question: 1 = CLEAR PATTERN?, 2 = CLEAR TRACK PATTERN?, 0 = none */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_CLEAR_TNPAT) ? 1 : (ui.overlay == O_ASK && ui.ask_act == ACT_CLEAR_TNTRK) ? 2 : 0;
    else if (!strcmp(what, "pasteask"))             /* wip416: FUNC + FILL on the pattern: PASTE PATTERN? 1 = open */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_PASTE_TNPAT) ? 1 : 0;
    else if (!strcmp(what, "autoask"))               /* wip367: FUNC + EDIT's question DELETE AUTOMATION?: 1 = open */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_AUTODEL) ? 1 : 0;
    else if (!strcmp(what, "clip"))                 /* FUNC + SEQ: the pattern copied, 1-based, 0 = none yet */
        got = ui.clip + 1;
    else if (!strcmp(what, "tcsrc"))                /* FUNC + black key + SEQ: the track copied, 1-based, 0 = none yet */
        got = ui.tc_src + 1;
    else if (!strcmp(what, "pcpage"))               /* FUNC + SEL: the page copied, 1-based, 0 = none yet */
        got = ui.pc_view < 0 ? 0 : ui.pc_page + 1;
    else if (!strcmp(what, "curpage"))              /* the page on the screen, 1-based */
        got = cur_page() + 1;
    else if (!strcmp(what, "pasteblink"))           /* FUNC + REC + white key: the slot blinking fast now, 1-16, 0 = none */
        got = (ui.blink_pat >= 0 && plat_ms() < ui.blink_until) ? ui.blink_pat + 1 : 0;
    else if (!strcmp(what, "chainn"))               /* CHAIN screen: the patterns chosen so far, -1 = not open */
        got = (ui.overlay == O_ASK && ui.ask_act == ACT_CHAIN) ? (int)ui.ch_n : -1;
    else if (!strncmp(what, "chainp", 6))           /* chainpK: the K-th (1-based) chosen pattern, 1-16 */
        got = ui.ch_pat[atoi(what + 6) - 1] + 1;
    else if (!strcmp(what, "svslot"))               /* SAVE AS: the chosen slot, 0-31 */
        got = ui.sv_slot;
    else if (!strncmp(what, "slotused", 8))         /* slotusedN: user preset slot N (0-31) holds something */
        got = preset_len[atoi(what + 8)] > 0;
    else if (!strncmp(what, "plmatch", 7)) {         /* wip317: plmatchN: place N (0-39: 0-7 factory, 8-39 user) holds exactly the name in ui.sv_name (the last made-up / renamed one) */
        uint32_t bn;
        const uint8_t *bb = preset_blob(atoi(what + 7), &bn);
        got = bb && !strncmp((const char *)bb + 16, ui.sv_name, TNP_NAME_LEN) && ui.sv_name[0];
    } else if (!strncmp(what, "plfirst", 7)) {       /* wip317: plfirstN: the first letter of the name in place N, 0 = empty */
        uint32_t bn;
        const uint8_t *bb = preset_blob(atoi(what + 7), &bn);
        got = bb ? bb[16] : 0;
    } else if (!strncmp(what, "slotun", 6)) {           /* wip308: slotunN: user slot N holds "UNTITLED nn": nn, else 0 */
        uint32_t bn;
        const uint8_t *bb = preset_blob(NFACT + atoi(what + 6), &bn);
        got = (bb && !strncmp((const char *)bb + 16, "UNTITLED ", 9)) ? atoi((const char *)bb + 16 + 9) : 0;
    } else if (!strncmp(what, "hits", 4))               /* wip264: hitsN = how many hits Tonic voice N (0-7) got since start */
        got = (int)eng_hit_n[atoi(what + 4) & 7];
    else if (!strcmp(what, "choke"))                 /* wip328: the CHOKE group's mask (bit v = Tonic voice v) */
        got = (int)seq.choke;
    else if (!strcmp(what, "chkscreen"))             /* wip328: the CHOKE screen is open */
        got = chk_page() ? 1 : 0;
    else if (!strcmp(what, "mutes"))                 /* wip284: how many Tonic voices (1-8) are muted now */
        got = __builtin_popcount((unsigned)(seq.vmute & 0xFFu));
    else if (!strcmp(what, "solo"))                  /* wip264: the soloed Tonic voice, 1-8, 0 = none */
        got = seq.vsolo ? __builtin_ctz(seq.vsolo) + 1 : 0;
    else if (!strcmp(what, "presetcur"))            /* the preset loaded last: 0 none, 1-8 factory, 9-40 user slot */
        got = ui.preset_cur;
    else if (!strcmp(what, "tn_name0"))             /* the first letter of the SAVE AS name (it is made up) */
        got = (unsigned char)ui.sv_name[0];
    else if (!strcmp(what, "view"))
        got = ui.view;
    else if (!strcmp(what, "fxoff"))                 /* ALL FX OFF (1) / ON (0), K4 of PERFORM */
        got = engine_fx_is_off();
    else if (!strcmp(what, "chainon"))               /* wip111: CHAIN ON (1) / OFF (0), K1 of the pattern page */
        got = seq.chain_on;
    else if (!strcmp(what, "scint"))                 /* wip349: the sidechain intensity 0-127 */
        got = proj.set.sc_int;
    else if (!strcmp(what, "scduck"))                /* wip581: the dot's level 0-255 = the input's peak since the last call (the screens call it every frame: read it from a page with no dot) */
        got = engine_tonic_duck();
    else if (!strcmp(what, "scmask"))                /* wip350: the sidechain's ducked tracks (bit v) */
        got = proj.set.sc_mask;
    else if (!strcmp(what, "dck"))                   /* wip350: the DUCK screen is open */
        got = ui.dck;
    else if (!strcmp(what, "scin"))                  /* wip349: the sidechain input track 0-7 */
        got = proj.set.sc_in;
    else if (!strcmp(what, "classic"))               /* wip354: TONIC ENGINE: 0 ENHANCED, 1 CLASSIC */
        got = proj.set.classic;
    else if (!strcmp(what, "fxpage"))                /* wip101: the page of the FX view (0 PERFORM, 1 REVERB, 2 DELAY) */
        got = ui.page[V_FX][0];
    else if (!strcmp(what, "swing"))                 /* wip74: the 909 pattern's swing 0-100, step grid (RATE_*), fill code */
        got = proj.pat[seq.ppat[PART_909]].swing;
    else if (!strcmp(what, "len0"))                  /* wip85: the 909 pattern's length */
        got = proj.pat[seq.ppat[PART_909]].drum[PART_909].len;
    else if (!strcmp(what, "grid"))
        got = proj.pat[seq.ppat[PART_909]].drum[PART_909].rate;
    else if (!strcmp(what, "fillcode"))
        got = proj.pat[seq.ppat[PART_909]].fill;
    else if (!strcmp(what, "tempo"))
        got = (int)(seq.bpm + 0.5f);
    else if (!strcmp(what, "clocks_out"))
        got = (int)clock_out_count;
    else if (!strcmp(what, "noteons"))
        got = (int)note_ons;
    else if (!strcmp(what, "ukeys"))                /* wip385: the keys held now, real and remote (bit k = key k) */
        got = (int)ui.keys;
    else if (!strcmp(what, "lastpot"))               /* wip386: the pot read by the last `potget V I` */
        got = last_pot;
    else if (!strcmp(what, "d_autosaveoff")) got = proj.set.autosave_off;     /* wip437: the factory settings, read back as stored */
    else if (!strcmp(what, "d_outloud")) got = proj.set.out_loud;
    else if (!strcmp(what, "d_beatoff")) got = proj.set.beat_off;
    else if (!strcmp(what, "d_clkout")) got = proj.set.clk_out;
    else if (!strcmp(what, "d_notesout")) got = proj.set.notes_out;
    else if (!strcmp(what, "d_usbfull")) got = proj.set.usb_full;
    else if (!strcmp(what, "d_onecore")) got = proj.set.one_core;
    else if (!strcmp(what, "d_zoomoff")) got = proj.set.zoom_off;
    else if (!strcmp(what, "d_namemanual")) got = proj.set.name_manual;
    else if (!strcmp(what, "d_palette")) got = proj.set.palette;
    else if (!strcmp(what, "d_tonicch")) got = seq.ch[0] + 1;                /* the Tonic's MIDI channel as the screen shows it */
    else if (!strcmp(what, "d_remch")) got = proj.set.midi_ch[4];            /* 0 = OFF */
    else if (!strcmp(what, "ccmode"))                /* wip386: CC MODE 0-3 and CC CH BASE 0-8 */
        got = proj.set.cc_mode;
    else if (!strcmp(what, "ccbase"))
        got = proj.set.cc_base;
    else if (!strcmp(what, "mq_n"))                  /* wip384: packets the UI took from the MIDI queue, the last CC number and value */
        got = ui_midi_seen(0);
    else if (!strcmp(what, "mq_cc"))
        got = ui_midi_seen(1);
    else if (!strcmp(what, "mq_val"))
        got = ui_midi_seen(2);
    else if (!strcmp(what, "store_writes"))
        got = (int)store_writes;
    else if (!strcmp(what, "peak_db_max")) {
        double db = 20.0 * log10(peak_out > 1e-9f ? peak_out : 1e-9f);
        if (db > atof(val)) {
            printf("FAIL expect peak <= %s dBFS, got %.1f\n", val, db);
            return 1;
        }
        printf("  ok peak %.1f dBFS\n", db);
        return 0;
    } else if (!strcmp(what, "peak_db_min")) {
        double db = 20.0 * log10(peak_out > 1e-9f ? peak_out : 1e-9f);
        if (db < atof(val)) {
            printf("FAIL expect peak >= %s dBFS, got %.1f\n", val, db);
            return 1;
        }
        printf("  ok peak %.1f dBFS\n", db);
        return 0;
    } else if (!strcmp(what, "preset_rc"))
        got = preset_rc;
    else if (!strncmp(what, "vacc", 4)) {          /* vaccP.V.S: pattern P, Tonic track V (0-7), step S: its own accent */
        int p, v, s;
        sscanf(what, "vacc%d.%d.%d", &p, &v, &s);
        got = sm_get(proj.pat[p].drum[0].vacc[v], s);
    } else if (!strcmp(what, "acsel"))              /* wip287: 1 = the SEQ page's voice selector is on AC */
        got = ui.ac;
    else if (!strncmp(what, "gacc", 4)) {          /* gaccP.S: pattern P, step S: the global accent */
        int p, s;
        sscanf(what, "gacc%d.%d", &p, &s);
        got = sm_get(proj.pat[p].drum[0].accent, s);
    } else if (!strncmp(what, "phit", 4)) {        /* phitP.V.S: pattern P (any, not only the playing one), track V, step S: a hit */
        int p, v, s;
        sscanf(what, "phit%d.%d.%d", &p, &v, &s);
        got = sm_get(proj.pat[p].drum[0].hit[v], s);
    } else if (!strncmp(what, "pswing", 6)) {      /* pswingP: pattern P's swing byte */
        int p;
        sscanf(what, "pswing%d", &p);
        got = proj.pat[p].swing;
    } else if (!strncmp(what, "vfill", 5)) {       /* vfillP.V.S: pattern P, Tonic track V (0-7), step S: its FILL */
        int p, v, s;
        sscanf(what, "vfill%d.%d.%d", &p, &v, &s);
        got = sm_get(proj.pat[p].drum[0].vfill[v], s);
    } else if (!strncmp(what, "pchain", 6)) {      /* pchainP: pattern P's CHAINED flag */
        int p;
        sscanf(what, "pchain%d", &p);
        got = proj.pat[p].drum[0].chain;
    } else if (!strncmp(what, "plen", 4)) {        /* plenP: pattern P's Tonic length */
        int p;
        sscanf(what, "plen%d", &p);
        got = proj.pat[p].drum[0].len;
    } else if (!strncmp(what, "tn16_", 5)) {       /* tn16_V.I: a Tonic value at full precision */
        int v, i;
        sscanf(what, "tn16_%d.%d", &v, &i);
        got = proj.sound.tn16[v][i];
    } else if (!strncmp(what, "nhit", 4)) {        /* wip236 nhitK.V: how many steps of kit K voice V are set */
        int k, v, s, n = 0;
        sscanf(what, "nhit%d.%d", &k, &v);
        for (s = 0; s < NSTEPS; s++)
            n += sm_get(proj.pat[seq.ppat[k]].drum[k].hit[v], s);
        got = n;
    } else if (!strncmp(what, "notesum", 7)) {      /* wip252: notesumP.V: the sum of the notes of pattern P track V (all 64 steps) */
        int p, v, s;
        sscanf(what, "notesum%d.%d", &p, &v);
        got = 0;
        for (s = 0; s < NSTEPS; s++)
            got += proj.note[p & 15][v & 7][s];
    } else if (!strncmp(what, "note", 4)) {         /* wip252: noteP.V.S: the ARP note of pattern P track V step S */
        int p, v, s;
        sscanf(what, "note%d.%d.%d", &p, &v, &s);
        got = proj.note[p & 15][v & 7][s & 63];
    } else if (!strncmp(what, "hit", 3)) {         /* hitK.V.S: kit K voice V step S set */
        int k, v, s;
        sscanf(what, "hit%d.%d.%d", &k, &v, &s);
        got = sm_get(proj.pat[seq.ppat[k]].drum[k].hit[v], s);
    } else if (!strncmp(what, "late", 4)) {        /* wip311: lateP.V.S = the late offset (0-255) of Tonic voice V on step S of pattern P */
        int p, v, s;
        if (!strncmp(what, "latehas", 7)) {        /* latehasP.V.S = 1 when there is an offset (its size depends on the timing) */
            sscanf(what, "latehas%d.%d.%d", &p, &v, &s);
            got = seq_late_get(&seq, p, v, s) != 0;
        } else {
            sscanf(what, "late%d.%d.%d", &p, &v, &s);
            got = seq_late_get(&seq, p, v, s);
        }
    } else if (!strncmp(what, "brk", 3)) {
        int s;
        sscanf(what, "brk%d", &s);
        got = (int)((proj.pat[seq.ppat[PART_BRK]].brk.steps >> s) & 1u);
    } else if (!strncmp(what, "sound", 5)) {       /* soundT.V.I */
        int t, v, i;
        sscanf(what, "sound%d.%d.%d", &t, &v, &i);
        got = proj.sound.v[t][v][i];
    } else if (!strcmp(what, "mute"))
        got = (int)seq.mute;
    else if (!strcmp(what, "vmute"))                /* drum tracks muted on their own (bit kit * 11 + track) */
        got = (int)seq.vmute;
    else if (!strcmp(what, "loops"))
        got = engine_brk_nslots();
    else if (!strcmp(what, "loop_a"))
        got = proj.pat[seq.ppat[PART_BRK]].brk.slot_a;
    else {
        printf("FAIL unknown expect %s\n", what);
        return 1;
    }
    if (val[0] == '<' || val[0] == '>') {          /* a bound: "<30", ">0" */
        int lim = (int)strtol(val + 1, 0, 0);
        if (val[0] == '<' ? got >= lim : got <= lim) {
            printf("FAIL expect %s %s, got %d\n", what, val, got);
            return 1;
        }
        printf("  ok %s == %d (%s)\n", what, got, val);
        return 0;
    }
    if (got != (int)strtol(val, 0, 0)) {
        printf("FAIL expect %s == %s, got %d\n", what, val, got);
        return 1;
    }
    printf("  ok %s == %d\n", what, got);
    return 0;
}

#ifndef X0X_WEB
int main(int argc, char **argv)
{
    FILE *sc;
    char line[512], out[400];
    const char *dir = argc > 2 ? argv[2] : ".";
    int lineno = 0, fails = 0;
    if (argc < 2) {
        fprintf(stderr, "usage: x0x_host SCRIPT [OUTDIR]\n");
        return 2;
    }
    sc = fopen(argv[1], "r");
    if (!sc) {
        perror(argv[1]);
        return 2;
    }
    snprintf(out, sizeof out, "%s/midi_out.txt", dir);
    midi_log = fopen(out, "w");
    if (getenv("X0X_SAID_LOG")) {                        /* a reader listens from the start, quietly: the log only */
        said_log_f = fopen(getenv("X0X_SAID_LOG"), "a");
        sim_listen = 1;
        said_print = 0;
    }
    boot();
    if (sim_listen) {
        static const uint8_t HELLO[] = {ANN_H0, ANN_H1, ANN_H2, ANN_HELLO, ANN_VER};
        ann_frame(HELLO, sizeof HELLO, now_ms);
    }
    while (fgets(line, sizeof line, sc)) {
        char cmd[32] = {0}, a[256] = {0}, b[64] = {0}, c[64] = {0}, line_d[64] = {0};
        lineno++;
        if (line[0] == '#' || sscanf(line, "%31s %255s %63s %63s %63s", cmd, a, b, c, line_d) < 1)
            continue;
        if (!strcmp(cmd, "wait"))
            run_ms((uint32_t)atoi(a));
        else if (!strcmp(cmd, "press") || !strcmp(cmd, "release") || !strcmp(cmd, "tap")) {
            int i = find_name(BTN_N, NB, a);
            if (i < 0) {
                printf("line %d: no button %s\n", lineno, a);
                return 2;
            }
            if (cmd[0] != 'r')
                held_btn |= 1u << i;
            if (cmd[0] == 't')
                run_ms(40);
            if (cmd[0] != 'p')
                held_btn &= ~(1u << i);
            run_ms(cmd[0] == 't' ? 40 : 2);
        } else if (!strcmp(cmd, "key") || !strcmp(cmd, "tapkey")) {
            int k = key_of(a);
            if (cmd[0] == 't') {
                held_keys |= 1u << k;
                run_ms(60);
                held_keys &= ~(1u << k);
                run_ms(30);
            } else {
                if (!strcmp(b, "down"))
                    held_keys |= 1u << k;
                else
                    held_keys &= ~(1u << k);
                run_ms(2);
            }
        } else if (!strcmp(cmd, "songmode")) {           /* songmode 0/1: set SONG MODE directly (older scenarios) */
            ui.songmode = (uint8_t)atoi(a);
        } else if (!strcmp(cmd, "turn")) {
            int e = find_name(ENC_N, NE, a), n = atoi(b), s = n > 0 ? 1 : -1;
            if (e < 0) {
                printf("line %d: no encoder %s\n", lineno, a);
                return 2;
            }
            {
                uint32_t gap = c[0] ? (uint32_t)atoi(c) : 80u;   /* turn ENC N [MS]: a detent every MS */
                for (; n; n -= s) {                   /* (80 ms, the default: no acceleration) */
                    enc_acc[e] += s;
                    run_ms(gap);
                }
            }
        } else if (!strcmp(cmd, "spin")) {              /* spin ENC N: all N detents at once, exactly N */
            int e = find_name(ENC_N, NE, a);
            if (e < 0) {
                printf("line %d: no encoder %s\n", lineno, a);
                return 2;
            }
            enc_acc[e] += atoi(b);
            accel_off = 1;                              /* a test's exact values, not a hand's speed */
            run_ms(1);
            accel_off = 0;
        } else if (!strcmp(cmd, "palette")) {           /* palette N: the skin (0 = the old black one, 6 = GRIGIOTONIC), for a shot */
            proj.set.palette = (uint8_t)atoi(a);
            palette_set((uint32_t)proj.set.palette);
            run_ms(50);
        } else if (!strcmp(cmd, "param")) {             /* param T V I VALUE: a sound pot, as a knob would set it */
            int t = atoi(a), v = atoi(b), i = atoi(c), val = atoi(line_d);
            if (t < 0 || t >= NTARGETS || v < 0 || v >= NVOICES_MAX || i < 0 || i >= NPARAMS_MAX) {
                printf("line %d: no param %d %d %d\n", lineno, t, v, i);
                return 2;
            }
            proj.sound.v[t][v][i] = (uint8_t)val;
            engine_set(t, v, i, val);
            run_ms(1);
        } else if (!strcmp(cmd, "savenow")) {           /* savenow: the working project to the flash, as the SAVE button used to (SAVE now opens SAVE AS) */
            ui_save_now();
        } else if (!strcmp(cmd, "preset")) {            /* preset FILE.tnp: a TNP1 preset as the working project (preset_rc: 0 = loaded) */
            static uint8_t pbuf[8192];
            FILE *pf = fopen(a, "rb");
            size_t pn = pf ? fread(pbuf, 1, sizeof pbuf, pf) : 0;
            if (pf)
                fclose(pf);
            else {
                printf("line %d: cannot read %s\n", lineno, a);
                return 2;
            }
            preset_rc = app_load_preset(pbuf, (uint32_t)pn);
            run_ms(50);
        } else if (!strcmp(cmd, "master"))
            master = (uint32_t)atoi(a);
        else if (!strcmp(cmd, "slot")) {                /* slot K NAME a.wav [b.wav ...]: one zone each */
            char *tok, *rest = line + 4;
            int k, z = 0;
            strtok(rest, " \t\n");                          /* K */
            k = atoi(a);
            tok = strtok(0, " \t\n");                      /* NAME */
            snprintf(slot_name[k], sizeof slot_name[k], "%s", tok ? tok : "");
            slot_nz[k] = 0;
            while ((tok = strtok(0, " \t\n")) && z < PLAT_SLOT_ZONES) {
                if (load_slot(k, z++, tok)) {
                    printf("line %d: cannot load %s\n", lineno, tok);
                    return 2;
                }
            }
            engine_brk_loops();
        } else if (!strcmp(cmd, "slotimg")) {          /* slotimg K FILE.hdr FILE.bin: as uploaded */
            int k = atoi(a);
            FILE *fh = fopen(b, "rb"), *fd = fopen(c, "rb");
            size_t nh, nd;
            if (!fh || !fd || k < 0 || k > 2) {
                printf("line %d: cannot read the slot image\n", lineno);
                return 2;
            }
            memset(slot_img[k], 0xFF, sizeof slot_img[k]);       /* erased flash */
            nh = fread(slot_img[k], 1, sizeof(smp_user_hdr_t), fh);   /* SMP_END writes the header, */
            nd = fread(slot_img[k] + SMP_USER_DATA, 1, sizeof slot_img[k] - SMP_USER_DATA, fd);   /* SMP_WRITE the data */
            fclose(fh);
            fclose(fd);
            (void)nh;
            {
                const smp_user_hdr_t *h = (const smp_user_hdr_t *)slot_img[k];
                if (st_crc32(slot_img[k] + SMP_USER_DATA, h->data_len) != h->crc || nd < h->data_len) {
                    printf("line %d: slot image CRC mismatch (the device would refuse it)\n", lineno);
                    return 2;
                }
            }
            slot_is_img[k] = 1;
            smp_user_scan((uint32_t)k);
            printf("  slot %d: %d zone(s) from the image\n", k, usr_nz[k]);
            engine_brk_loops();
        } else if (!strcmp(cmd, "ccmode")) {           /* ccmode M [B]: CC MODE 0 OFF, 1 SELECTED, 2 CHANNELS, 3 BOTH, and CC CH BASE 0-8 (as GLO > CC MODE / CC CH BASE) */
            proj.set.cc_mode = (uint8_t)atoi(a);
            proj.set.cc_base = (uint8_t)(b[0] ? atoi(b) : 0);
            engine_cc_cfg(proj.set.cc_mode, proj.set.cc_base);
        } else if (!strcmp(cmd, "autosave")) {          /* wip437: autosave 1 = AUTOSAVE ON, 0 = OFF (as GLO > AUTOSAVE; the factory default is OFF since wip437, the older scenarios switch it on) */
            proj.set.autosave_off = (uint8_t)!atoi(a);
        } else if (!strcmp(cmd, "outloud")) {           /* wip437: outloud 1 = OUTPUT LOUD, 0 = NORMAL (as GLO > OUTPUT; the factory default is LOUD since wip437, the level tests use NORMAL) */
            proj.set.out_loud = (uint8_t)(atoi(a) != 0);
            engine_out_loud(proj.set.out_loud);
        } else if (!strcmp(cmd, "potget")) {           /* potget V I: the Tonic track V's pot I into `expect lastpot` (the shelving cuts: I = 24 low, 25 high, as the knob shows them) */
            int v = atoi(a), i = atoi(b);
            last_pot = i == 24 ? proj.sound.v[T_909][SH_LO_ROW][v & 7] : i == 25 ? 127 - proj.sound.v[T_909][SH_HI_ROW][v & 7] : proj.sound.v[T_909][v & 7][i];
        } else if (!strcmp(cmd, "remch")) {            /* remch N: the remote panel's MIDI channel 1-16, 0 = OFF (as GLO > REMOTE MIDI CH) */
            int n = atoi(a);
            proj.set.midi_ch[4] = (uint8_t)(n ? n : 17);
            engine_remote_ch(n ? n - 1 : -1);
        } else if (!strcmp(cmd, "midi")) {
            uint32_t s = (uint32_t)strtoul(a, 0, 16), d1 = (uint32_t)strtoul(b, 0, 16), d2 = (uint32_t)strtoul(c, 0, 16);
            uint32_t cin = s >= 0xF0 ? 0x0F : s >> 4;
            min_q[mi_w++ % MQ] = cin | s << 8 | d1 << 16 | d2 << 24;
        } else if (!strcmp(cmd, "wav")) {
            snprintf(out, sizeof out, "%s/%s", dir, a);
            wav_open(out);
        } else if (!strcmp(cmd, "load"))              /* load N: every block costs N% (the guard's input) */
            load_force = atoi(a);
        else if (!strcmp(cmd, "peakreset"))
            peak_out = 0.0f;
        else if (!strcmp(cmd, "wavstop"))
            wav_close();
        else if (!strcmp(cmd, "shot")) {
            snprintf(out, sizeof out, "%s/%s", dir, a);
            png_write(out);
        } else if (!strcmp(cmd, "welcome")) {           /* welcome: the boot screen (gfx.c welcome_draw), for a shot */
            welcome_draw();
        } else if (!strcmp(cmd, "welcomever")) {        /* welcomever: the big version screen that follows it (wip215) */
            welcome_version(TONIC_VERSION);
        } else if (!strcmp(cmd, "leds")) {
            int i;
            printf("  leds: buttons");
            for (i = 0; i < NB; i++)
                if (lit_btn >> i & 1u)
                    printf(" %s", BTN_N[i]);
            printf(" | white");
            for (i = 0; i < 16; i++)
                printf("%c", (lit_keys >> WHITE_K[i] & 1u) ? '#' : ((dim_keys | faint_keys) >> WHITE_K[i] & 1u) ? 'o' : '.');
            printf(" black");
            for (i = 0; i < 11; i++)
                printf("%c", (lit_keys >> BLACK_K[i] & 1u) ? '#' : ((dim_keys | faint_keys) >> BLACK_K[i] & 1u) ? 'o' : '.');
            printf("\n");
        } else if (!strcmp(cmd, "expect"))
            fails += expect(a, b);
        else if (!strcmp(cmd, "projimg")) {
            FILE *f;
            snprintf(out, sizeof out, "%s", a);
            if (!(f = fopen(out, "rb")) || fread(&proj, 1, sizeof proj, f) != sizeof proj) {
                fprintf(stderr, "projimg: %s is not a %zu-byte project image\n", a, sizeof proj);
                return 1;
            }
            fclose(f);
            project_save();
            memset(&proj, 0, sizeof proj);
            boot();
        } else if (!strcmp(cmd, "storedump") || !strcmp(cmd, "storeload")) {   /* the simulated flash, whole */
            FILE *f;
            int o, ld = cmd[5] == 'l';
            if (!(f = fopen(a, ld ? "rb" : "wb"))) {
                fprintf(stderr, "%s: cannot open %s\n", cmd, a);
                return 1;
            }
            for (o = 0; o < OBJ_NOBJ; o++) {
                int32_t n = store_len[o];
                if (ld ? fread(&n, 4, 1, f) != 1 || fread(store[o], 1, PLAT_STORE_MAX, f) != PLAT_STORE_MAX
                       : fwrite(&n, 4, 1, f) != 1 || fwrite(store[o], 1, PLAT_STORE_MAX, f) != PLAT_STORE_MAX) {
                    fprintf(stderr, "%s: %s is short\n", cmd, a);
                    return 1;
                }
                store_len[o] = n;
            }
            fclose(f);
            if (ld) {                                    /* boot on it */
                memset(&proj, 0, sizeof proj);
                boot();
            }
        } else if (!strcmp(cmd, "noise")) {             /* noise SEED: every step and lane random (no room to save) */
            uint32_t r = (uint32_t)atoi(a) | 1u, i;
            uint8_t *raw = (uint8_t *)proj.pat;
            for (i = 0; i < sizeof proj.pat + sizeof proj.arr; i++) {
                r ^= r << 13;
                r ^= r >> 17;
                r ^= r << 5;
                raw[i] = (uint8_t)r;
            }
            for (i = 0; i < NPAT; i++) {
                int k;
                for (k = 0; k < NKIT; k++) {
                    proj.pat[i].drum[k].len = 64;
                    proj.pat[i].drum[k].rate = 0;
                }
            }
            proj.arr.song.len = 0;
            for (i = 0; i < NLANE; i++)
                proj.arr.lane[i].used = 0;
        } else if (!strcmp(cmd, "cpu2")) {              /* cpu2 0|1: the simulated second core */
            plat_cpu2_set(atoi(a));
        } else if (!strcmp(cmd, "fillrandom")) {        /* fillrandom SEED: every pattern's drum hits at random
                                                         * (they pack badly: the stream spans several objects) */
            uint32_t r = (uint32_t)atoi(a) * 2654435761u + 1u, p, k, v, w;
            for (p = 0; p < NPAT; p++)
                for (k = 0; k < NKIT; k++)
                    for (v = 0; v < NDRUM; v++)
                        for (w = 0; w < NSW; w++) {
                            r ^= r << 13, r ^= r >> 17, r ^= r << 5;
                            proj.pat[p].drum[k].hit[v][w] = r;
                        }
            mark_dirty();
        } else if (!strcmp(cmd, "notepoke")) {          /* wip252: notepoke P V S N: the note of pattern P track V step S */
            proj.note[atoi(a) & 15][atoi(b) & 7][atoi(c) & 63] = (int8_t)atoi(line_d);
            mark_dirty();
        } else if (!strcmp(cmd, "presetmove")) {        /* wip255 presetmove A B: flash preset slot A moved to slot B (B 32-39 = presets 1-8 written over) */
            int sa = atoi(a), sb = atoi(b);
            if (sa >= 0 && sa < PLAT_NPRESET_ALL && sb >= 0 && sb < PLAT_NPRESET_ALL && sa != sb) {
                memcpy(preset_slot[sb], preset_slot[sa], PLAT_STORE_MAX);
                preset_len[sb] = preset_len[sa];
                preset_len[sa] = 0;
            }
        } else if (!strcmp(cmd, "snapproj")) {
            memcpy(pat_snap, proj.pat, sizeof pat_snap);
        } else if (!strcmp(cmd, "fakecrash")) {
            host_crash = 1;
        } else if (!strcmp(cmd, "tearsave")) {          /* tearsave N: the next save stops after N object writes */
            store_tear = atoi(a);
        } else if (!strcmp(cmd, "reboot")) {
            store_tear = -1;                            /* the power back */
            memset(&proj, 0, sizeof proj);
            boot();
        } else if (!strcmp(cmd, "backup") || !strcmp(cmd, "restore")) {
            if (strchr(a, '/'))
                snprintf(out, sizeof out, "%s", a);
            else
                snprintf(out, sizeof out, "%s/%s", dir, a);
            if (cmd[0] == 'b') {
                if (sim_backup(out)) {
                    printf("line %d: cannot write %s\n", lineno, out);
                    return 2;
                }
            } else {                                       /* (the device's message after it: main_fm1.c) */
                restore_rc = sim_restore(out);
                ui_say(!restore_rc ? "RESTORED FROM USB" : restore_rc == 10 ? "STOP FIRST, THEN RESTORE"
                       : "RESTORE FAILED", 0);
            }
        } else if (!strcmp(cmd, "backup_damage")) {     /* a byte of a backup file changed; head: made Polly's */
            FILE *bf;
            snprintf(out, sizeof out, "%s/%s", dir, a);
            if ((bf = fopen(out, "r+b"))) {
                long n;
                fseek(bf, 0, SEEK_END);
                n = ftell(bf);
                if (!strcmp(b, "head") && n <= (long)sizeof sim_img) {
                    fseek(bf, 0, SEEK_SET);
                    if (fread(sim_img, 1, (size_t)n, bf) == (size_t)n) {
                        memcpy(sim_img, "PLYB", 4);
                        sim_wr32(sim_img + 12, sim_crc32(0, sim_img, 12));
                        sim_wr32(sim_img + n - 4, sim_crc32(0, sim_img, (size_t)n - 4));
                        fseek(bf, 0, SEEK_SET);
                        fwrite(sim_img, 1, (size_t)n, bf);
                    }
                } else {
                    int c;
                    fseek(bf, n / 2, SEEK_SET);
                    c = fgetc(bf);
                    fseek(bf, n / 2, SEEK_SET);
                    fputc(c ^ 1, bf);
                }
                fclose(bf);
            }
        } else if (!strcmp(cmd, "listen")) {
            static const uint8_t HELLO[] = {ANN_H0, ANN_H1, ANN_H2, ANN_HELLO, ANN_VER};
            sim_listen = 1;
            said_len = 0;
            said_log[0] = 0;
            ann_frame(HELLO, sizeof HELLO, now_ms);
            run_ms(40);
        } else if (!strcmp(cmd, "speakdump")) {        /* every screen's text for a screen reader, to stdout */
            speak_dump();
        } else if (!strcmp(cmd, "where")) {
            static const uint8_t WHERE[] = {ANN_H0, ANN_H1, ANN_H2, ANN_WHERE};
            ann_frame(WHERE, sizeof WHERE, now_ms);
            run_ms(40);
        } else if (!strcmp(cmd, "said_none")) {
            if (said_count) {
                printf("line %d: said_none: %u said\n", lineno, said_count);
                fails++;
            }
        } else if (!strcmp(cmd, "said")) {
            char *t = line + 4, *e;
            while (*t == ' ')
                t++;
            e = t + strlen(t);
            while (e > t && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' '))
                *--e = 0;
            if (!strstr(said_log, t)) {
                printf("line %d: said \"%s\": not said (since the last check: %s)\n", lineno, t,
                       said_len ? said_log : "nothing\n");
                fails++;
            }
            said_len = 0;
            said_log[0] = 0;
        } else if (!strcmp(cmd, "echo"))
            printf("%s", line + 5);
        else {
            printf("line %d: unknown command %s\n", lineno, cmd);
            return 2;
        }
    }
    wav_close();
    if (midi_log)
        fclose(midi_log);
    printf("host: %u ms simulated, %u blits, render %.1f%% of real time (host CPU)%s\n", now_ms, blits,
           render_budget_ns_total > 0 ? 100.0 * render_ns_total / render_budget_ns_total : 0.0,
           fails ? ", EXPECTATIONS FAILED" : "");
    if (getenv("X0X_PROFILE")) {
        static const char *const N[ENG_PROF_N] = {"909", "808", "BREAK", "FX", "MASTER", "SEQ"};
        uint64_t tot = 0;
        int k;
        for (k = 0; k < ENG_PROF_N; k++)
            tot += prof_tot[k];
        printf("profile (share of the render, ns per sample):");
        for (k = 0; k < ENG_PROF_N; k++)
            printf(" %s %.1f%% (%.1f)", N[k], tot ? 100.0 * (double)prof_tot[k] / (double)tot : 0.0,
                   ins_frames ? (double)prof_tot[k] / (double)ins_frames : 0.0);
        printf("\n");
    }
    if (ins_frames && ins_total)
        printf("host: playing, %.0f instructions / sample (mean), %.0f in the worst block\n",
               (double)ins_total / (double)ins_frames, (double)ins_block_max / 256.0);
    return fails ? 1 : 0;
}
#endif /* X0X_WEB */
