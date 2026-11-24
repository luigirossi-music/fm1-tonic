/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X project: the settings, the sound, the 32 patterns, the song and the motion lanes.
 *
 * The settings and the sound are one storage object each, as they are. The patterns, the song and
 * the lanes (~30 KB, mostly empty steps) are one stream packed with lz.c across the four objects
 * OBJ_PAT0, OBJ_PAT1, OBJ_SONG, OBJ_MOTION (PROJ_STREAM). The first carries the raw length and a
 * CRC of the raw bytes, so a stream torn by a power cut between two objects, or one that does not
 * unpack, is never loaded as garbage: the objects' previous copies (storage.c keeps two) are tried
 * instead, which give the save before; only when no mix of copies checks out, the defaults. A save packs once to measure (nothing is
 * written if it would not fit: "MEMORY FULL") and once to write; an object whose bytes did not
 * change is not written again.
 *
 * Every object carries its format number. A project saved before the stream (format 1: 16
 * patterns of 32 steps in OBJ_PAT0/1, the song and 32-step lanes in OBJ_SONG/OBJ_MOTION) is
 * converted as it loads: its steps land on steps 1-32, its patterns on 1-16. */
#include "x0x.h"
#define REV1_VOICE 1                      /* voice 1 (was the 909 snare, DR_SD of drum909.h): the sound revision below (wip115) */
#include "../dsp/master.h"             /* MST_* (the sound's revisions) */
static void fx_take(void);               /* wip227: the compressor / reverb / delay that only SAVE keeps (below) */
#include "lz.c"
#include "tnp.h"
#include "tonic_start.h"                    /* tools/gen_preset_bank.py --start: the starting project (or none) */
int project_preset_unpack(const uint8_t *b, uint32_t n);

project_t proj;

#define PROJ_STREAM 2u                  /* the format word of the packed objects */
static const uint8_t STREAM_OBJ[] = {OBJ_PAT0, OBJ_PAT1, OBJ_SONG, OBJ_MOTION};
#define NSTREAM ((int)sizeof STREAM_OBJ)
#define STREAM_HDR 16u                  /* in the first object: format, raw length, raw CRC, packed length */
#define STREAM_CAP (NSTREAM * (PLAT_STORE_MAX - 4u) - (STREAM_HDR - 4u))
#define RAW_OFF __builtin_offsetof(project_t, pat)
#define RAW_LEN (sizeof(project_t) - RAW_OFF)   /* the patterns, the arrangement and (wip252) the notes, contiguous */
#define RAW_LEN_OLD (__builtin_offsetof(project_t, note) - RAW_OFF)   /* a project saved before the notes: they load empty */
typedef char raw_fits[(RAW_LEN < 0xFFFFu) ? 1 : -1];
typedef char raw_contiguous[(__builtin_offsetof(project_t, arr) == RAW_OFF + sizeof proj.pat &&
                             __builtin_offsetof(project_t, note) == __builtin_offsetof(project_t, arr) + sizeof proj.arr) ? 1 : -1];

static void arrange_defaults(void);
static void notes_clear(void)                  /* wip252: no step has a note */
{
    uint32_t i;
    int8_t *z = &proj.note[0][0][0];
    for (i = 0; i < sizeof proj.note; i++)
        z[i] = 0;
}
static uint32_t packed_len;             /* the last save's (or load's) packed stream, bytes */

void project_defaults(void)
{
    int i;
    proj.set.magic = PROJ_MAGIC;
    proj.set.format = PROJ_FORMAT;
    proj.set.palette = 6;                /* wip544: GRIGIOTONIC (the default skin); 5 = TONIC */
    proj.set.keyled = 1;
    proj.set.keysound = 0;
    proj.set.beat_off = 0;               /* wip437: the 4 / 4 beat lights are ON by default (GLO > 4/4 LED; wip234 had them off) */
    proj.set.autosave_off = 1;           /* wip437: AUTOSAVE = OFF by default (the settings alone are still kept: wip321; SAVE keeps the rest) */
    proj.set.preset_cur = 0;
    proj.set.name_manual = 0;            /* wip324: GENERATE */
    proj.set.choke = 0;                  /* wip335 */
    proj.set.sc_int = 0; proj.set.sc_in = 0; proj.set.sc_mask = 0;   /* wip353: no sidechain */
    proj.set.cc_mode = 0; proj.set.cc_base = 0;      /* wip386: CC MODE = OFF, the channel block starts at channel 1 */
    proj.set.clk_out = 0;                /* wip437: CLOCK OUT = OFF by default */
    proj.set.notes_out = 0;
    /* wip437: the factory settings, written out so that a factory reset in a running unit also gives them all
     * (TONIC MIDI CH 10 = the default of 0, REMOTE MIDI CH OFF, CC MODE OFF, CC CH BASE 1-8, OUTPUT LOUD,
     * USB AUDIO FOLLOWS KNOB, ZOOM KNOB YES, NAME PROJECTS GENERATE, TONIC ENGINE ENHANCED, 2ND CORE ON, theme TONIC) */
    { int k; for (k = 0; k < 5; k++) proj.set.midi_ch[k] = 0; }
    proj.set.out_loud = 1;
    proj.set.usb_full = 0;
    proj.set.one_core = 0;
    proj.set.autoscroll = 0;
    proj.set.zoom_off = 0;
    proj.set.classic = 0;
    proj.set.bpm_x10 = 1250;
    proj.set.accent_q7 = 88;
    proj.set.stereo = PROJ_STEREO;
    proj.set.snd_rev = PROJ_SND_REV;
    engine_sound_defaults(&proj.sound);
    fx_take();                           /* wip227: the kept compressor / reverb / delay = the defaults */
    arrange_defaults();
    notes_clear();
    /* the 303 lines start empty, like the drums: OCT+ on TB-3PO (or keyboard mode) writes one */
    for (i = 0; i < NPAT; i++)
        pattern_init(&proj.pat[i]);
    if (TONIC_START_N)                   /* the starting point: a preset (tools/gen_preset_bank.py --start) */
        project_preset_unpack(TNS, sizeof TNS);
}

/* how full the stream's room is, in % (GLO > ABOUT) */
int project_mem_pct(void) { return (int)((packed_len * 100u + STREAM_CAP - 1u) / STREAM_CAP); }

/* object payload: format word + body */
static uint8_t io_buf[PLAT_STORE_MAX] __attribute__((aligned(4)));

/* what each object last held on the flash (a hash of its payload), so a save writes only the
 * objects that changed: an autosave usually erases one or two sectors, not six */
static uint32_t stored_hash[OBJ_NOBJ];
static uint8_t stored_known[OBJ_NOBJ];
static uint32_t hash_of(const uint8_t *p, uint32_t n)        /* FNV-1a */
{
    uint32_t h = 2166136261u;
    while (n--)
        h = (h ^ *p++) * 16777619u;
    return h;
}

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t crc32_of(const uint8_t *p, uint32_t n)       /* zlib CRC-32, 4 bits per step */
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

/* the object is the n bytes of io_buf, format word included; written only if they changed */
static int save_buf(uint32_t obj, uint32_t n)
{
    uint32_t h = hash_of(io_buf, n);
    int rc;
    if (stored_known[obj] && stored_hash[obj] == h)
        return 0;                                    /* the flash already holds exactly this */
    rc = plat_store_save(obj, io_buf, n);
    stored_known[obj] = rc == 0;
    stored_hash[obj] = h;
    return rc;
}

static int load_obj(uint32_t obj, void *dst, uint32_t len)
{
    int n = plat_store_load(obj, io_buf, sizeof io_buf);
    uint32_t i;
    if (n < 0 || (uint32_t)n != len + 4u || get32(io_buf) != PROJ_FORMAT)
        return -1;
    for (i = 0; i < len; i++)
        ((uint8_t *)dst)[i] = io_buf[4u + i];
    stored_known[obj] = 1;
    stored_hash[obj] = hash_of(io_buf, len + 4u);
    return 0;
}

/* the settings may have grown since they were saved: a shorter payload of this format loads into
 * the front of dst, and the rest keeps what dst held (zeros: the new settings' defaults) */
static int load_settings(settings_t *dst)
{
    int n = plat_store_load(OBJ_SET, io_buf, sizeof io_buf);
    uint32_t i;
    if (n < 20 || (uint32_t)n > sizeof *dst + 4u || io_buf[0] != (uint8_t)PROJ_FORMAT || io_buf[1] || io_buf[2] || io_buf[3])
        return -1;
    for (i = 0; i + 4u < (uint32_t)n; i++)
        ((uint8_t *)dst)[i] = io_buf[4u + i];
    if ((uint32_t)n == sizeof *dst + 4u) {           /* exactly what a save of these settings writes */
        stored_known[OBJ_SET] = 1;
        stored_hash[OBJ_SET] = hash_of(io_buf, (uint32_t)n);
    }
    return 0;
}

static int save_obj(uint32_t obj, const void *src, uint32_t len)
{
    uint32_t i;
    if (len + 4u > sizeof io_buf)
        return -1;
    put32(io_buf, PROJ_FORMAT);
    for (i = 0; i < len; i++)
        io_buf[4u + i] = ((const uint8_t *)src)[i];
    return save_buf(obj, len + 4u);
}

typedef char sound_fits[(sizeof(sound_t) + 4u <= PLAT_STORE_MAX) ? 1 : -1];

static void arrange_defaults(void)
{
    uint32_t i;
    uint8_t *z = (uint8_t *)&proj.arr;
    for (i = 0; i < sizeof proj.arr; i++)
        z[i] = 0;
}

/* ---- a Tonic preset (tnp.h) onto the working project -------------------------------------------- */

static uint32_t rd16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }

/* The preset's sound (8 voices x 21 values + pans), its tempo and swing and its patterns a-l (slots 1-12) replace
 * the working ones; the other 20 slots, the song and the lanes are emptied. Settings, MIX and FX stay.
 * Validated whole first: 0 = loaded; -1 not a TNP1 file / too short; -2 sizes disagree; -3 a pattern does not fit.
 * The caller (ui.c app_load_preset) refuses while playing and brings the engine and the screen along. */
/* wip250: the CHAIN flags follow the notes. A Tonic pattern (1-8) with no hit on any track cannot be chained: its flag is taken
 * off (a preset that flags an empty pattern, an empty user slot, a pattern the user cleared). project_chain_count() = how many
 * patterns 1-8 are still flagged; project_chain_tidy() drops the flags of the empty ones and returns how many are left. */
int project_chain_count(void)
{
    int i, n = 0;
    for (i = 0; i < NPAT_TONIC; i++)
        n += proj.pat[i].drum[0].chain ? 1 : 0;
    return n;
}
int project_chain_tidy(void)
{
    int i, v, k, n = 0;
    for (i = 0; i < NPAT_TONIC; i++) {
        dpart_t *d = &proj.pat[i].drum[0];
        int notes = 0;
        for (v = 0; v < TN_VOICES && !notes; v++)
            for (k = 0; k < NSW; k++)
                if (d->hit[v][k]) {
                    notes = 1;
                    break;
                }
        if (d->chain && !notes)
            d->chain = 0;
        n += d->chain ? 1 : 0;
    }
    return n;
}

int project_preset_unpack(const uint8_t *b, uint32_t n)
{
    uint32_t npat, i, k;
    int v, j;
    const uint8_t *fxb = 0;
    int fx2 = 0, fx3 = 0;
    if (n < TNP_SIZE(1) || b[0] != 'T' || b[1] != 'N' || b[2] != 'P' || b[3] != '1')
        return -1;
    npat = b[9];
    if (npat < 1 || npat > TNP_NPAT_MAX || n < TNP_SIZE(npat))
        return -2;
    if (rd16(b + 4) == TNP_SIZE(npat) + TNP_FX3_LEN) {  /* wip289: with the tracks' shelving cuts too */
        const uint8_t *f = b + TNP_SIZE(npat);
        if (n < TNP_SIZE(npat) + TNP_FX3_LEN || f[0] != 'T' || f[1] != 'F' || f[2] != 'X' || f[3] != '3')
            return -2;
        fxb = f;
        fx2 = 1;
        fx3 = 1;
    } else if (rd16(b + 4) == TNP_SIZE(npat) + TNP_FX2_LEN) {  /* wip242: with the tracks' own sends too */
        const uint8_t *f = b + TNP_SIZE(npat);
        if (n < TNP_SIZE(npat) + TNP_FX2_LEN || f[0] != 'T' || f[1] != 'F' || f[2] != 'X' || f[3] != '2')
            return -2;
        fxb = f;
        fx2 = 1;
    } else if (rd16(b + 4) == TNP_SIZE(npat) + TNP_FX_LEN) {   /* wip227: with the compressor / reverb / delay trailer */
        const uint8_t *f = b + TNP_SIZE(npat);
        if (n < TNP_SIZE(npat) + TNP_FX_LEN || f[0] != 'T' || f[1] != 'F' || f[2] != 'X' || f[3] != '1')
            return -2;
        fxb = f;
    } else if (rd16(b + 4) != TNP_SIZE(npat))
        return -2;
    for (i = 0; i < npat; i++) {
        const uint8_t *pp = b + TNP_HDR + 8u * TNP_VOICE + i * TNP_PAT;
        if (pp[0] < 1 || pp[0] > NSTEPS)
            return -3;
    }
    for (i = 0; i < NPAT; i++)
        pattern_init(&proj.pat[i]);
    arrange_defaults();
    notes_clear();                                     /* wip252: a preset has no notes */
    for (v = 0; v < TN_VOICES; v++) {
        const uint8_t *o = b + TNP_HDR + (uint32_t)v * TNP_VOICE;
        for (j = 0; j < TN_NPARAMS; j++) {
            uint16_t val = (uint16_t)rd16(o + 2 * j);
            proj.sound.tn16[v][j] = val;
            proj.sound.v[T_909][v][j] = (uint8_t)tonic_val_to_pot(j, val);
        }
        proj.sound.v[T_909][v][TN_POT_PAN] = o[42] > 127 ? 127 : o[42];
        proj.sound.v[T_909][v][TN_POT_RSND] = 0;         /* wip242: the tracks' own sends: the preset's (TFX2, below) or none */
        proj.sound.v[T_909][v][TN_POT_DSND] = 0;
        proj.sound.v[T_909][SH_LO_ROW][v] = 0;           /* wip289: the shelving cuts: the preset's (TFX3) or none */
        proj.sound.v[T_909][SH_HI_ROW][v] = 0;
    }
    proj.set.bpm_x10 = (uint16_t)rd16(b + 6);
    for (i = 0; i < NPAT; i++) {                       /* the tempo feel is the preset's in every slot ... */
        proj.pat[i].drum[0].rate = b[10] < NRATES ? b[10] : RATE_16;
        proj.pat[i].swing = b[8] > 100 ? 100 : b[8];
    }
    if (npat > TNP_NPAT_LOAD)                          /* ... but only patterns 1-16 carry steps (wip651): 17-32 stay empty */
        npat = TNP_NPAT_LOAD;
    for (i = 0; i < npat; i++) {
        const uint8_t *pp = b + TNP_HDR + 8u * TNP_VOICE + i * TNP_PAT;
        dpart_t *d = &proj.pat[i].drum[0];
        d->len = pp[0];
        d->chain = (i < NPAT_TONIC && pp[1]) ? 1 : 0;  /* the preset's "Chained" flag (wip651: only patterns 1-8 chain) */
        for (v = 0; v < TN_VOICES; v++) {
            const uint8_t *r = pp + TNP_PAT_HDR + 24u * (uint32_t)v;
            for (k = 0; k < NSW; k++) {
                d->hit[v][k] = (uint32_t)rd16(r + 4 * k) | (uint32_t)rd16(r + 4 * k + 2) << 16;
                d->vacc[v][k] = (uint32_t)rd16(r + 8 + 4 * k) | (uint32_t)rd16(r + 8 + 4 * k + 2) << 16;
                d->vfill[v][k] = (uint32_t)rd16(r + 16 + 4 * k) | (uint32_t)rd16(r + 16 + 4 * k + 2) << 16;
            }
        }
    }
    project_chain_tidy();                               /* wip250: a flag on a pattern with no notes is forgotten */
    if (fxb) {                                          /* wip227: the preset's own compressor, reverb and delay */
        for (j = 0; j < NPARAMS_MAX; j++) {
            proj.sound.v[T_FX][0][j] = fxb[4 + j];
            proj.sound.v[T_FX][1][j] = fxb[28 + j];
            proj.sound.v[T_MST][0][j] = fxb[52 + j];
        }
        proj.sound.v[T_MIX][PART_909][MX_REV] = fxb[76];
        proj.sound.v[T_MIX][PART_909][MX_DLY] = fxb[77];
        if (fx2)
            for (v = 0; v < TN_VOICES; v++) {            /* wip242 */
                proj.sound.v[T_909][v][TN_POT_RSND] = fxb[80 + v] > 127 ? 127 : fxb[80 + v];
                proj.sound.v[T_909][v][TN_POT_DSND] = fxb[88 + v] > 127 ? 127 : fxb[88 + v];
            }
        if (fx3)
            for (v = 0; v < TN_VOICES; v++) {            /* wip289 */
                proj.sound.v[T_909][SH_LO_ROW][v] = fxb[96 + v] > 127 ? 127 : fxb[96 + v];
                proj.sound.v[T_909][SH_HI_ROW][v] = fxb[104 + v] > 127 ? 127 : fxb[104 + v];
            }
    } else
        engine_fx_defaults(&proj.sound);                /* none saved: the factory compressor (0.0 dB / 0.0 dB), reverb and delay */
    fx_take();                                          /* what the project keeps = what was loaded (autosave keeps it; only SAVE changes it) */
    proj.set.choke = 0;                                 /* wip335: the preset's CHOKE group (bit 0 of each voice's spare byte) is the project's group now */
    proj.set.sc_mask = 0;                               /* wip353: the preset's SIDECHAIN: ducked tracks = bit 1 of each voice's spare byte, intensity / input = the trailer's reserved bytes 78 / 79 (none in an older preset = 0, track 1) */
    for (v = 0; v < TN_VOICES && v < 8; v++) {
        proj.set.choke |= (uint8_t)((b[TNP_HDR + (uint32_t)v * TNP_VOICE + 43u] & 1u) << v);
        proj.set.sc_mask |= (uint8_t)(((b[TNP_HDR + (uint32_t)v * TNP_VOICE + 43u] >> 1) & 1u) << v);
    }
    proj.set.sc_int = fxb ? (uint8_t)(fxb[78] > 127 ? 127 : fxb[78]) : 0;
    proj.set.sc_in = fxb ? (uint8_t)(fxb[79] & 7u) : 0;
    return 0;
}

/* The working Tonic project as a TNP1 preset of patterns 1-16 (TNP_NPAT_SAVE, wip651) with their chain flags (what SAVE AS stores in a user slot). The caller gives the
 * tempo and the per-voice mute bits (they live in the sequencer, not the project). Returns the length, 0 = no room. */
static void wr16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
uint32_t project_preset_pack(uint8_t *out, uint32_t max, const char *name, unsigned bpm_x10, unsigned mutes)
{
    uint32_t base = TNP_SIZE(TNP_NPAT_SAVE), total = base + TNP_FX3_LEN, i, k;
    int v, j;
    if (max < total)
        return 0;
    for (i = 0; i < total; i++)
        out[i] = 0;
    out[0] = 'T'; out[1] = 'N'; out[2] = 'P'; out[3] = '1';
    wr16(out + 4, total);
    wr16(out + 6, bpm_x10);
    out[8] = proj.pat[0].swing;
    out[9] = (uint8_t)TNP_NPAT_SAVE;
    out[10] = proj.pat[0].drum[0].rate;
    out[11] = (uint8_t)mutes;
    for (i = 0; i < TNP_NAME_LEN - 1u && name[i]; i++)
        out[16 + i] = (uint8_t)name[i];
    for (v = 0; v < TN_VOICES; v++) {
        uint8_t *o = out + TNP_HDR + (uint32_t)v * TNP_VOICE;
        for (j = 0; j < TN_NPARAMS; j++)
            wr16(o + 2 * j, proj.sound.tn16[v][j]);
        o[42] = proj.sound.v[T_909][v][TN_POT_PAN];
        o[43] = (uint8_t)(((mutes >> (8 + v)) & 1u) | (((proj.set.sc_mask >> v) & 1u) << 1));       /* wip328: the voice's spare byte: bit 0 = the voice is in the CHOKE group (bits 8-15 of the argument); an older preset has 0 = no group */
    }
    for (i = 0; i < TNP_NPAT_SAVE; i++) {
        uint8_t *pp = out + TNP_HDR + 8u * TNP_VOICE + i * TNP_PAT;
        const dpart_t *d = &proj.pat[i].drum[0];
        pp[0] = d->len;
        pp[1] = (i < NPAT_TONIC && d->chain) ? 1 : 0;      /* wip651: a chain flag only exists on patterns 1-8 */
        for (v = 0; v < TN_VOICES; v++) {
            uint8_t *r = pp + TNP_PAT_HDR + 24u * (uint32_t)v;
            for (k = 0; k < NSW; k++) {
                wr16(r + 4 * k, d->hit[v][k] & 0xFFFFu);
                wr16(r + 4 * k + 2, d->hit[v][k] >> 16);
                wr16(r + 8 + 4 * k, d->vacc[v][k] & 0xFFFFu);
                wr16(r + 8 + 4 * k + 2, d->vacc[v][k] >> 16);
                wr16(r + 16 + 4 * k, d->vfill[v][k] & 0xFFFFu);
                wr16(r + 16 + 4 * k + 2, d->vfill[v][k] >> 16);
            }
        }
    }
    {                                                  /* wip227: the compressor, reverb and delay as they are now */
        uint8_t *f = out + base;
        f[0] = 'T'; f[1] = 'F'; f[2] = 'X'; f[3] = '3';   /* wip242: TFX2 = TFX1 + the tracks' own sends; wip289: TFX3 = TFX2 + the shelving cuts */
        for (j = 0; j < NPARAMS_MAX; j++) {
            f[4 + j] = proj.sound.v[T_FX][0][j];
            f[28 + j] = proj.sound.v[T_FX][1][j];
            f[52 + j] = proj.sound.v[T_MST][0][j];
        }
        f[76] = proj.sound.v[T_MIX][PART_909][MX_REV];
        f[77] = proj.sound.v[T_MIX][PART_909][MX_DLY];
        f[78] = proj.set.sc_int;                         /* wip353: the sidechain's intensity and input (the trailer's reserved bytes) */
        f[79] = proj.set.sc_in;
        for (v = 0; v < TN_VOICES; v++) {
            f[80 + v] = proj.sound.v[T_909][v][TN_POT_RSND];
            f[88 + v] = proj.sound.v[T_909][v][TN_POT_DSND];
            f[96 + v] = proj.sound.v[T_909][SH_LO_ROW][v];
            f[104 + v] = proj.sound.v[T_909][SH_HI_ROW][v];
        }
    }
    return total;
}

/* ---- the packed stream ---------------------------------------------------------------------- */

/* the writer: fills io_buf after the object's header and writes it when full (measure: counts only) */
typedef struct {
    int measure, k, err;
    uint32_t fill, total;
} st_out_t;

static int st_flush(st_out_t *o)
{
    int rc = save_buf(STREAM_OBJ[o->k], o->fill);
    o->k++;
    o->fill = 4u;
    put32(io_buf, PROJ_STREAM);
    return rc;
}

static int st_put(void *ctx, const uint8_t *p, uint32_t n)
{
    st_out_t *o = ctx;
    o->total += n;
    if (o->measure)
        return o->total > STREAM_CAP ? -1 : 0;
    while (n) {
        if (o->fill == PLAT_STORE_MAX) {
            if (o->k + 1 >= NSTREAM || st_flush(o)) {
                o->err = 1;
                return -1;
            }
        }
        io_buf[o->fill++] = *p++;
        n--;
    }
    return 0;
}

static int save_stream(void)
{
    const uint8_t *raw = (const uint8_t *)&proj + RAW_OFF;
    uint32_t crc = crc32_of(raw, RAW_LEN);
    st_out_t o = {1, 0, 0, 0, 0};
    lz_sink_t sink = {st_put, &o};
    if (lz_compress(raw, RAW_LEN, &sink))
        return -7;                                    /* would not fit: nothing written */
    packed_len = o.total;
    o.measure = 0;
    o.total = 0;
    o.fill = STREAM_HDR;
    put32(io_buf, PROJ_STREAM);
    put32(io_buf + 4, RAW_LEN);
    put32(io_buf + 8, crc);
    put32(io_buf + 12, packed_len);
    if (lz_compress(raw, RAW_LEN, &sink) || o.err || st_flush(&o))
        return -3;
    while (o.k < NSTREAM)                             /* the objects past the stream: empty */
        if (st_flush(&o))
            return -3;
    return 0;
}

/* the reader: the objects in turn, from io_buf */
typedef struct {
    int k, n;
    uint32_t pos, left, prev;                         /* prev: bit k, object k from its previous copy */
} st_in_t;

static int stream_obj_load(int k, uint32_t prev)      /* object k of the stream into io_buf, its copy per prev */
{
    return (prev >> k) & 1u ? plat_store_load_prev(STREAM_OBJ[k], io_buf, sizeof io_buf)
                            : plat_store_load(STREAM_OBJ[k], io_buf, sizeof io_buf);
}

static int st_get(void *ctx)
{
    st_in_t *s = ctx;
    if (!s->left)
        return -1;
    while (s->pos >= (uint32_t)s->n) {
        if (++s->k >= NSTREAM)
            return -1;
        s->n = stream_obj_load(s->k, s->prev);
        if (s->n < 4 || get32(io_buf) != PROJ_STREAM)
            return -1;
        stored_known[STREAM_OBJ[s->k]] = !((s->prev >> s->k) & 1u);   /* a previous copy: written again next save */
        stored_hash[STREAM_OBJ[s->k]] = hash_of(io_buf, (uint32_t)s->n);
        s->pos = 4u;
    }
    s->left--;
    return io_buf[s->pos++];
}

/* 0 = loaded; -1 = not there or not good (proj's patterns and arrangement then hold garbage) */
static int load_stream(int n, uint32_t prev)
{
    uint8_t *raw = (uint8_t *)&proj + RAW_OFF;
    uint32_t crc = get32(io_buf + 8), plen = get32(io_buf + 12), rlen = get32(io_buf + 4);
    st_in_t s = {0, n, STREAM_HDR, plen, prev};
    lz_src_t src = {st_get, &s};
    if (n < (int)STREAM_HDR || (rlen != RAW_LEN && rlen != RAW_LEN_OLD) || plen > STREAM_CAP)
        return -1;                                    /* (wip252: the length before the notes is good too) */
    notes_clear();
    stored_known[STREAM_OBJ[0]] = !(prev & 1u);
    stored_hash[STREAM_OBJ[0]] = hash_of(io_buf, (uint32_t)n);
    if (lz_decompress(&src, raw, rlen) || crc32_of(raw, rlen) != crc)
        return -1;
    packed_len = plen;
    return 0;
}

/* ---- format 1: 16 patterns of 32 steps, 32-step lanes ---------------------------------------- */

#define NSTEPS1 32
#define NPAT1 16
typedef struct {
    uint32_t hit[NDRUM];
    uint32_t accent;
    uint8_t len, rate, rsv[2];
} dpart1_t;
#define NBASS1 2                                      /* format 1 still stores the two (removed) 303 lines: skipped on load */
typedef struct {
    uint32_t raw[(NSTEPS1 * 2 + 4 + 12) / 4];
} bpart1_t;
typedef struct {
    uint32_t steps;
    uint8_t set[BRK_NSET];
    uint8_t slot_a, slot_b, rsv[2];
} brkpart1_t;
typedef struct {
    dpart1_t drum[NKIT];
    bpart1_t bass[NBASS1];
    brkpart1_t brk;
    uint8_t swing, rsv[3];
} pattern1_t;
typedef struct {
    uint8_t used, gen, pat, part, t, v, i, rsv;
    uint8_t val[NSTEPS1];
} lane1_t;
#define ARR1_LEN (sizeof(song_t) + NLANE * sizeof(lane1_t))
#define ARR1_A (PLAT_STORE_MAX - 4u)                 /* format 1's arrangement bytes in OBJ_SONG; the rest in OBJ_MOTION */
typedef char arr1_room[(ARR1_LEN <= sizeof(arrange_t)) ? 1 : -1];   /* it unpacks in proj.arr */

static void pattern_from1(pattern_t *p, const pattern1_t *o)
{
    int k, v;
    for (k = 0; k < NKIT; k++) {
        for (v = 0; v < NDRUM; v++)
            p->drum[k].hit[v][0] = o->drum[k].hit[v];
        p->drum[k].accent[0] = o->drum[k].accent;
        p->drum[k].len = o->drum[k].len;
        p->drum[k].rate = o->drum[k].rate;
    }
    p->brk.steps = o->brk.steps;
    for (k = 0; k < BRK_NSET; k++)
        p->brk.set[k] = o->brk.set[k];
    p->brk.slot_a = o->brk.slot_a;
    p->brk.slot_b = o->brk.slot_b;
    p->swing = o->swing;
}

/* OBJ_PAT0 + OBJ_PAT1, format 1 (eight patterns each) onto patterns 1-16 (the defaults are in place) */
static int load_patterns1(void)
{
    int h, i;
    for (h = 0; h < 2; h++) {
        int n = plat_store_load(h ? OBJ_PAT1 : OBJ_PAT0, io_buf, sizeof io_buf);
        if (n != (int)(8u * sizeof(pattern1_t) + 4u) || get32(io_buf) != PROJ_FORMAT)
            return -1;
        for (i = 0; i < 8; i++) {
            pattern1_t o;
            uint32_t b;
            for (b = 0; b < sizeof o; b++)
                ((uint8_t *)&o)[b] = io_buf[4u + (uint32_t)i * sizeof o + b];
            pattern_from1(&proj.pat[h * 8 + i], &o);
        }
    }
    return 0;
}

/* OBJ_SONG + OBJ_MOTION, format 1: the song as it is, the lanes widened to 64 steps (in place, last
 * first: lane k's new place never reaches below its old one) */
static int load_arrange1(void)
{
    uint8_t *a = (uint8_t *)&proj.arr;
    int k, i;
    if (load_obj(OBJ_SONG, a, ARR1_A) || load_obj(OBJ_MOTION, a + ARR1_A, ARR1_LEN - ARR1_A))
        return -1;
    for (k = NLANE - 1; k >= 0; k--) {
        lane1_t o;
        lane_t *l = &proj.arr.lane[k];
        uint32_t b;
        for (b = 0; b < sizeof o; b++)
            ((uint8_t *)&o)[b] = a[sizeof(song_t) + (uint32_t)k * sizeof o + b];
        l->used = o.used;
        l->gen = o.gen;
        l->pat = o.pat;
        l->part = o.part;
        l->t = o.t;
        l->v = o.v;
        l->i = o.i;
        l->rsv = 0;
        for (i = 0; i < NSTEPS; i++)
            l->val[i] = i < NSTEPS1 ? o.val[i] : MOT_NONE;
    }
    stored_known[OBJ_SONG] = stored_known[OBJ_MOTION] = 0;   /* the next save writes the stream */
    return 0;
}

/* ---- load, save ----------------------------------------------------------------------------- */

/* the 909 SD gained DECAY at pot 3: what was 3.. (LEVEL, DRIVE, DIST, REV, DLY, PAN) moves up one,
 * the knobs' motion with it, and DECAY takes its default (9W9's 340 ms) */
static void sound_rev1(void)
{
    uint8_t *sd = proj.sound.v[T_909][REV1_VOICE];
    int i, n = engine_nparams(T_909, REV1_VOICE);
    for (i = n - 1; i > 3; i--)
        sd[i] = sd[i - 1];
    sd[3] = engine_param(T_909, REV1_VOICE, 3)->def;
    for (i = 0; i < NLANE; i++) {
        lane_t *l = &proj.arr.lane[i];
        if (l->used && l->t == T_909 && l->v == REV1_VOICE && l->i >= 3)
            l->i++;
    }
}

/* the master's one-knob COMP and FILTER and the delay's TAPE arrived: each set to its nearest to what
 * the details say (the details themselves stay, and are applied after: the sound is as it was) */
static void sound_rev2(void)
{
    uint8_t *m = proj.sound.v[T_MST][0];
    int th = m[MST_THRESH], cut = m[MST_CUTOFF];
    float thr = (float)th * (48.0f / 127.0f) - 48.0f, x = (-thr - 4.0f) / 28.0f;
    m[MST_COMP1] = (uint8_t)(!m[MST_RATIO] ? 0 : x <= 0.0f ? 1 : x >= 1.0f ? 127 : (int)(x * 127.0f + 0.5f));
    m[MST_DJF] = (uint8_t)(m[MST_MODE] == 0 ? 64 : m[MST_MODE] == 3 ? 69 + cut * 58 / 120
                           : (cut < 8 ? 0 : (cut - 8) * 59 / 119));
    proj.sound.v[T_FX][1][0] = (uint8_t)(proj.sound.v[T_FX][0][FX_DL_TYPE] ? 16 + proj.sound.v[T_FX][0][FX_DL_WEAR] * 111 / 127 : 0);
}

/* a BD gained REV and DLY before its PAN (909: 8 -> 10, 808: 7 -> 9): the pan and its motion move, the sends start
 * at none (as the kick had) */
static void sound_bd_sends(int t, int pan)            /* (rev 3: the 909's at 8; rev 4: the 808's at 7) */
{
    uint8_t *bd = proj.sound.v[t][0];
    int i;
    bd[pan + 2] = bd[pan];
    bd[pan] = bd[pan + 1] = 0;
    for (i = 0; i < NLANE; i++) {
        lane_t *l = &proj.arr.lane[i];
        if (l->used && l->t == t && l->v == 0 && l->i == pan)
            l->i = (uint8_t)(pan + 2);
    }
}

static void patterns_defaults(void)
{
    int i;
    for (i = 0; i < NPAT; i++)
        pattern_init(&proj.pat[i]);
    notes_clear();
}

/* the flash was written behind project.c's back (a restore over USB, backup.c): what it knew the objects held is gone,
 * so the next save writes each one */
void project_flash_changed(void)
{
    int i;
    for (i = 0; i < OBJ_NOBJ; i++)
        stored_known[i] = 0;
}

/* each object is read straight into its place in proj: a load copies only a valid payload */
int project_load(void)
{
    int bad = 0, i, sound_ok, n;
    settings_t set;
    project_defaults();
    packed_len = 0;
    set = proj.set;
    if (load_settings(&set) == 0 && set.magic == PROJ_MAGIC)
        proj.set = set;
    else
        bad = 1;
    sound_ok = load_obj(OBJ_SOUND, &proj.sound, sizeof proj.sound) == 0;
    if (!sound_ok)
        bad = 1;
    if (sound_ok && proj.set.snd_rev < 3)             /* before the 909 BD's sends: its pan moves up two */
        sound_bd_sends(T_909, 8);                     /* (first: the pans' own fixes below see the new place) */
    if (sound_ok && proj.set.snd_rev < 4)             /* the 808 BD's, the same */
        sound_bd_sends(T_808, 7);
    if (proj.set.stereo < 1)                          /* saved mono: no pans there (zeros = hard left) */
        for (i = 0; i < NPARTS; i++)
            proj.sound.v[T_MIX][i][MX_PAN] = 64;
    if (proj.set.stereo < 2)                          /* saved before the drum voices had pans */
        engine_sound_centre_drum_pans(&proj.sound);
    proj.set.stereo = PROJ_STEREO;
    n = plat_store_load(OBJ_PAT0, io_buf, sizeof io_buf);
    if (n >= 4 && get32(io_buf) == PROJ_STREAM) {
        if (load_stream(n, 0)) {                      /* torn (a power cut mid-save) or broken: the previous
                                                       * copies, fewest first (the newest save that checks out) */
            uint32_t prev = 0;
            int ok = 0, b;
            for (b = 1; b <= NSTREAM && !ok; b++)
                for (prev = 1; prev < (1u << NSTREAM) && !ok; prev++) {
                    if ((uint32_t)__builtin_popcount(prev) != (uint32_t)b)
                        continue;
                    n = stream_obj_load(0, prev);
                    ok = n >= 4 && get32(io_buf) == PROJ_STREAM && load_stream(n, prev) == 0;
                }
            if (!ok) {                                /* nothing checks out: the defaults, not garbage */
                patterns_defaults();
                arrange_defaults();
                bad = 1;
            }
        }
    } else {
        if (load_patterns1()) {                       /* half a set is no set: the defaults */
            patterns_defaults();
            bad = 1;
        }
        /* the song and the lanes: absent in a project saved before they existed, which is no fault */
        if (load_arrange1())
            arrange_defaults();
        stored_known[OBJ_PAT0] = stored_known[OBJ_PAT1] = 0;
    }
    if (proj.arr.song.len > NSONG)
        proj.arr.song.len = NSONG;
    if (sound_ok && proj.set.snd_rev < 1)             /* saved before the 909 SD's DECAY: its pots move up one */
        sound_rev1();
    if (sound_ok && proj.set.snd_rev < 2)             /* before the one-knob COMP, FILTER, TAPE */
        sound_rev2();
    if (sound_ok && proj.set.snd_rev < 5)             /* before the break's FILTER: off (its pot held a 0) */
        proj.sound.v[T_BRK][0][2] = engine_param(T_BRK, 0, 2)->def;
    if (sound_ok && proj.set.snd_rev < 7) {           /* before the filter TYPE: closed, HPF */
        proj.sound.v[T_MST][0][MST_DJF] = 0;
        proj.sound.v[T_MST][0][MST_FTYPE] = 0;
    }
    if (sound_ok && proj.set.snd_rev < 8) {           /* wip227 clean-up: the compressor back to THRESHOLD 0.0 dB, MAKEUP 0.0 dB */
        proj.sound.v[T_MST][0][MST_THRESH] = 127;
        proj.sound.v[T_MST][0][MST_MAKEUP] = 0;
    }
    proj.set.snd_rev = PROJ_SND_REV;
    fx_take();                                        /* what was loaded is what the flash holds (the kept values stay as saved) */
    /* wip234: the PERFORM page starts clean at every power-on (and after a restore): reverb D/W 0 %, delay D/W 0 %, the
     * filter OFF (open). Only the LIVE values: keep_* still holds what SAVE stored, so an autosave leaves the flash alone and a
     * preset load brings its own saved values back. The compressor is not touched. ALL FX is a session switch (ON at power-on). */
    for (i = 0; i < NPARTS; i++) {
        proj.sound.v[T_MIX][i][MX_REV] = 0;
        proj.sound.v[T_MIX][i][MX_DLY] = 0;
    }
    proj.sound.v[T_MST][0][MST_DJF] = 0;
    proj.sound.v[T_MST][0][MST_FTYPE] = 0;
    return bad ? -1 : 0;
}

void project_apply_io(void)
{
    static const uint8_t DEF[NTRACKS] = {9, 10, 3};   /* 909 10, 808 11, break 4 */
    int p;
    seq.send_clock = proj.set.clk_out;
    seq.send_notes = proj.set.notes_out;
    engine_cc_cfg(proj.set.cc_mode, proj.set.cc_base);   /* wip386: CC MODE / CC CH BASE */
    seq.gacc = (proj.set.gacc >= 1 && proj.set.gacc <= 10) ? (uint8_t)(proj.set.gacc * 10) : 30;   /* wip616: GLOBAL ACCENT +1..+10, default +3 */
    engine_remote_ch(proj.set.midi_ch[4] >= 1 && proj.set.midi_ch[4] <= 16 ? proj.set.midi_ch[4] - 1 : -1);   /* wip385: midi_ch[4] = the REMOTE PANEL's channel (never written by older firmware: 0 = OFF) */
    for (p = 0; p < NTRACKS; p++) {
        int v = proj.set.midi_ch[p];
        seq.ch[p] = (uint8_t)(!v ? DEF[p] : v > 16 ? SEQ_CH_OFF : v - 1);
    }
    engine_out_loud(proj.set.out_loud);
    engine_usb_full(proj.set.usb_full);
    seq.choke = proj.set.choke;                        /* wip335: the CHOKE group comes back at power-on, after a restore and a preset load */
    engine_tonic_choke(seq.choke);
    engine_tonic_sidechain(proj.set.sc_in, proj.set.classic ? 0 : proj.set.sc_int, proj.set.classic ? 0 : proj.set.sc_mask);   /* wip353: the SIDECHAIN comes back with the project (CLASSIC = none in the sound) */
}

/* wip227: the master compressor (T_MST), the reverb and the delay (T_FX, and the REV / DLY sends of T_MIX) are kept
 * ONLY by the SAVE button. keep_* is what the flash holds of them (loaded at power-on, or taken at a SAVE); the
 * autosave writes the sound with these values in place of the live ones, so an autosave never keeps what was
 * turned (the live values themselves are not touched). */
static uint8_t keep_fx[2][NPARAMS_MAX], keep_mst[NPARAMS_MAX], keep_snd[3][2];

static void fx_take(void)                                /* the live values become the kept ones */
{
    int i, p;
    for (i = 0; i < NPARAMS_MAX; i++) {
        keep_fx[0][i] = proj.sound.v[T_FX][0][i];
        keep_fx[1][i] = proj.sound.v[T_FX][1][i];
        keep_mst[i] = proj.sound.v[T_MST][0][i];
    }
    for (p = 0; p < 3; p++) {
        keep_snd[p][0] = proj.sound.v[T_MIX][p][MX_REV];
        keep_snd[p][1] = proj.sound.v[T_MIX][p][MX_DLY];
    }
}

static void fx_swap(void)                                /* live <-> kept (twice = as it was) */
{
    int i, p;
    for (i = 0; i < NPARAMS_MAX; i++) {
        uint8_t t;
        t = proj.sound.v[T_FX][0][i]; proj.sound.v[T_FX][0][i] = keep_fx[0][i]; keep_fx[0][i] = t;
        t = proj.sound.v[T_FX][1][i]; proj.sound.v[T_FX][1][i] = keep_fx[1][i]; keep_fx[1][i] = t;
        t = proj.sound.v[T_MST][0][i]; proj.sound.v[T_MST][0][i] = keep_mst[i]; keep_mst[i] = t;
    }
    for (p = 0; p < 3; p++) {
        uint8_t t;
        t = proj.sound.v[T_MIX][p][MX_REV]; proj.sound.v[T_MIX][p][MX_REV] = keep_snd[p][0]; keep_snd[p][0] = t;
        t = proj.sound.v[T_MIX][p][MX_DLY]; proj.sound.v[T_MIX][p][MX_DLY] = keep_snd[p][1]; keep_snd[p][1] = t;
    }
}

static int project_save_x(int manual);

/* 0 = saved and verified; -7 = the patterns, song and motion do not fit (nothing was written).
 * The SAVE button: everything is kept, the compressor, reverb and delay too. */
int project_save(void) { return project_save_x(1); }

/* the autosave (and the flush before a backup): everything but the compressor, reverb and delay */
int project_autosave(void) { return project_save_x(0); }

static int project_save_x(int manual)
{
    int p, rc;
    for (p = 0; p < NTRACKS; p++)
        proj.set.midi_ch[p] = (uint8_t)(seq.ch[p] == SEQ_CH_OFF ? 17 : seq.ch[p] + 1);
    proj.set.bpm_x10 = (uint16_t)(seq.bpm * 10.0f + 0.5f);
    proj.set.accent_q7 = seq.accent_q7;
    proj.set.choke = seq.choke;                        /* wip335 */
    proj.set.clk_out = seq.send_clock;
    proj.set.notes_out = seq.send_notes;
    if (save_obj(OBJ_SET, &proj.set, sizeof proj.set))
        return -1;
    engine_sound_reconcile(&proj.sound);               /* the Tonic's exact values follow the knobs */
    if (manual)
        fx_take();                                     /* SAVE: the flash now holds these compressor / reverb / delay values */
    else
        fx_swap();                                     /* autosave: the flash keeps the ones it has */
    rc = save_obj(OBJ_SOUND, &proj.sound, sizeof proj.sound);
    if (!manual)
        fx_swap();                                     /* the live values back */
    if (rc)
        return -2;
    return save_stream();
}

int project_save_settings(void) { return save_obj(OBJ_SET, &proj.set, sizeof proj.set); }

/* wip321: the settings alone, with the live MIDI / clock / tempo values put into proj.set first, as a full save does (0 = saved; nothing is written if the flash holds the same) */
int project_autosave_settings(void)
{
    int p;
    for (p = 0; p < NTRACKS; p++)
        proj.set.midi_ch[p] = (uint8_t)(seq.ch[p] == SEQ_CH_OFF ? 17 : seq.ch[p] + 1);
    proj.set.bpm_x10 = (uint16_t)(seq.bpm * 10.0f + 0.5f);
    proj.set.accent_q7 = seq.accent_q7;
    proj.set.choke = seq.choke;                        /* wip335 */
    proj.set.clk_out = seq.send_clock;
    proj.set.notes_out = seq.send_notes;
    return save_obj(OBJ_SET, &proj.set, sizeof proj.set);
}
