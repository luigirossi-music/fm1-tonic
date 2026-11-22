/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X audio engine; see engine.h. */
#include "engine.h"
#include "plat.h"
#include "../dsp/tonic.h"
#include "../dsp/breaks.h"
#include "../dsp/fxbus.h"
#include "../dsp/master.h"
#include "../dsp/fastmath.h"

#define POOL X0X_POOL

/* time per stage of the render, in plat_cycles() units (the CPU's cycles, or the 24 MHz timer):
 * running totals (wrapping; the PERF screen takes differences) and the frames rendered. */
volatile uint32_t eng_prof[ENG_PROF_N], eng_prof_frames;
#define PROF_T(v) uint32_t v = plat_cycles()
#define PROF_ADD(k, t0) (eng_prof[k] += plat_cycles() - (t0))

/* ---------------------------------------------------------------- state --- */
seq_t seq;
volatile uint8_t eng_step[NTRACKS];
volatile uint16_t eng_peak[NPARTS];
volatile uint8_t eng_hit[NKIT][NDRUM];
volatile uint8_t eng_hit_cnt[NDRUM];   /* wip584: Tonic hits counted per voice (wraps at 256): the UI's hit flash sees EVERY hit, however slow its frames are */
int16_t eng_scope[SCOPE_N];
volatile uint32_t eng_scope_w;

static tonic_t tn POOL;                /* kit 0: the Tonic (Microtonic-style) drums */
static breaks_t brk POOL;
static fxbus_t fx POOL;
static master_t mst;
#define DLY_LEN 88200
static int16_t dly_buf[DLY_LEN] POOL;
static volatile uint8_t brk_hold;      /* main loop is rewriting the break loops: ISR leaves it alone */
static motion_t mot;
static const sound_t *mot_base_snd;    /* the knobs' own values (the project's mirror) */
static uint8_t mot_playing;
static volatile uint8_t mot_on = 1;
static uint8_t mot_was_on = 1;
void engine_motion_enable(int on) { mot_on = (uint8_t)(on != 0); }   /* the ISR releases the knobs */
static volatile uint8_t mot_release_req;
void engine_motion_reset(void) { mot_release_req = 1; }   /* lanes changed under it (undo): knobs back */

/* overload guard: past the CPU (a block over 92%, or 85% on average) the
 * 808's voices end at
 * -50 dB instead of -90 (~2%); after 3 s back under 65% both return. The platform reports each
 * block's load after rendering it (engine_load, same context as engine_render); the change is
 * made at the start of the next block. Off during the PERF test, which measures the full cost. */
#define GUARD_ON_LAST 92u
#define GUARD_ON_AVG 85u
#define GUARD_OFF 65u
#define GUARD_CALM_FRAMES (3u * 44100u)
static uint8_t guard_on, guard_applied, guard_enabled = 1;
static uint32_t guard_avg_q8, guard_calm;
volatile uint32_t eng_guard_count;
void engine_guard_enable(int on) { guard_enabled = (uint8_t)(on != 0); }
int engine_guard_active(void) { return guard_on; }
void engine_load(uint32_t pct, uint32_t frames)
{
    guard_avg_q8 = (guard_avg_q8 * 15u + pct * 256u) / 16u;
    if (!guard_enabled) {
        guard_on = 0;
        guard_calm = 0;
    } else if (!guard_on) {
        if (pct > GUARD_ON_LAST || guard_avg_q8 > GUARD_ON_AVG * 256u) {
            guard_on = 1;
            guard_calm = 0;
            eng_guard_count++;
        }
    } else if (pct >= GUARD_OFF || guard_avg_q8 >= GUARD_OFF * 256u) {
        guard_calm = 0;
    } else if ((guard_calm += frames) >= GUARD_CALM_FRAMES) {
        guard_on = 0;
    }
}
static void guard_apply(void)
{
    if (guard_on == guard_applied)
        return;
    guard_applied = guard_on;
}

/* mixer: per part Level, Rev send, Dly send (a drum machine's add to its voices' own sends) */
static const x0x_param_t MIX_P[MX_NPARAMS] = {{"LEVEL", 127, 100, 0}, {"REV WET", 127, 0, 0}, {"DLY WET", 127, 0, 0},
                                               {"PAN", 127, 64, 0}};
static const char *const PART_NAMES[NPARTS] = {"TONIC", "808", "BREAK"};
static float mix_level[NPARTS], mix_rev[NPARTS], mix_dly[NPARTS];
/* pan: a balance law. Centre is full level on both sides (a centred mix is the mono one exactly);
 * turning toward a side fades the other out along a quarter cosine, the near side stays at 1 */
static float mix_pl[NPARTS] = {1, 1, 1}, mix_pr[NPARTS] = {1, 1, 1};

/* ------------------------------------------------------- command queue --- */
/* uint32 commands, single producer (main loop) / single consumer (audio ISR):
 * type:4 | a:8 | b:8 | c:12 */
enum { C_PARAM_T0 = 0, C_DRUM = 8, C_BRK, C_NOP, C_TNLO, C_TNHI, C_TNNOTE, C_TNCHOKE, C_TNSC };   /* C_TNNOTE (wip251): a = voice, b = semitones + 64, c = velocity x 4095; C_TNCHOKE (wip328): a = the choke group's mask */
/* a 16-bit Tonic value needs 16 bits, a command has 12: C_TNLO carries the low 12 (kept aside), C_TNHI the top 4 and applies */
#define CQ 1024u
static uint32_t cq[CQ];
static volatile uint32_t cq_w, cq_r;

static void cq_put(uint32_t type, uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t w = cq_w;
    if (w - cq_r >= CQ)
        return;                        /* full: drop (a param flood; the mirror still has it) */
    cq[w % CQ] = (type << 28) | ((a & 0xFFu) << 20) | ((b & 0xFFu) << 12) | (c & 0xFFFu);
    __asm__ volatile("" ::: "memory");
    cq_w = w + 1;
}

/* wip384 (MIDI remote / CC, step S1): the MIDI queue from the audio ISR to the UI main loop.
 * midi_in (ISR) is the only writer, engine_midi_pop (ui_frame) the only reader; a full queue drops the new packet.
 * It carries the 4-byte USB-MIDI packets the UI must act on (now: control change on the Tonic's channel). */
#define RQ 32u
static uint32_t rq[RQ];
static volatile uint32_t rq_w, rq_r;
static volatile uint8_t rem_ch = 0xFFu;                 /* wip385: the REMOTE PANEL's channel, 0-15, 0xFF = OFF */
void engine_remote_ch(int ch) { rem_ch = (uint8_t)(ch >= 0 && ch <= 15 ? ch : 0xFF); }
static volatile uint8_t cc_mode, cc_base;               /* wip386: CC MODE (0 = OFF: no control change reaches the UI) and the block's first channel (0-8) */
void engine_cc_cfg(int mode, int base) { cc_base = (uint8_t)(base >= 0 && base <= 8 ? base : 0); cc_mode = (uint8_t)(mode >= 0 && mode <= 3 ? mode : 0); }
static void rq_put(uint32_t pkt)
{
    uint32_t w = rq_w;
    if (w - rq_r >= RQ)
        return;
    rq[w % RQ] = pkt;
    __asm__ volatile("" ::: "memory");
    rq_w = w + 1;
}
int engine_midi_pop(uint32_t *pkt)
{
    uint32_t r = rq_r;
    if (r == rq_w)
        return 0;
    *pkt = rq[r % RQ];
    __asm__ volatile("" ::: "memory");
    rq_r = r + 1;
    return 1;
}

/* ------------------------------------------------------------ params --- */
int engine_nvoices(int t)
{
    switch (t) {
    case T_909: return TN_VOICES;
    case T_MIX: return NPARTS;
    default: return 1;
    }
}

int engine_nparams(int t, int v)
{
    switch (t) {
    case T_909: return tonic_nparams(v);
    case T_BRK: return 3;                                /* Level, Pitch, Filter (sends: the mixer's; the rest: the pattern's) */
    case T_FX: return fxbus_nparams();
    case T_MIX: return MX_NPARAMS;                       /* every part: Level, Rev send, Dly send */
    case T_MST: return master_nparams();
    default: return 0;
    }
}

const x0x_param_t *engine_param(int t, int v, int i)
{
    switch (t) {
    case T_909: return tonic_param(v, i);
    case T_BRK: return breaks_param(BRK_NSET + i);
    case T_FX: return fxbus_param(i);
    case T_MIX: return (i >= 0 && i < MX_NPARAMS) ? &MIX_P[i] : 0;
    case T_MST: return master_param(i);
    default: return 0;
    }
}

static const char *const N909[TN_VOICES] = {"TRACK 1", "TRACK 2", "TRACK 3", "TRACK 4", "TRACK 5", "TRACK 6", "TRACK 7", "TRACK 8"};   /* Microtonic's drums 1-8 */
static const char *const N909S[TN_VOICES] = {"1", "2", "3", "4", "5", "6", "7", "8"};   /* the same, short (where TRACK n does not fit) */

const char *engine_voice_short(int t, int v)
{
    if (t == T_909)
        return (v >= 0 && v < TN_VOICES) ? N909S[v] : "";
    return engine_voice_name(t, v);
}

const char *engine_voice_name(int t, int v)
{
    switch (t) {
    case T_909: return (v >= 0 && v < TN_VOICES) ? N909[v] : "";
    case T_MIX: return (v >= 0 && v < NPARTS) ? PART_NAMES[v] : "";
    case T_BRK: return "BREAK";
    case T_FX: return "FX";
    case T_MST: return "MASTER";
    default: return "";
    }
}

static void apply_param(int t, int v, int i, int val)    /* ISR */
{
    switch (t) {
    case T_909: tonic_set_pot(&tn, v, i, val); break;
    case T_BRK: breaks_set(&brk, BRK_NSET + i, val); break;
    case T_FX: fxbus_set(&fx, i, val); break;
    case T_MIX:
        if (v < 0 || v >= NPARTS)
            break;
        if (i == MX_LEVEL)
            mix_level[v] = (float)val * (float)val / (100.0f * 100.0f);     /* 100 = unity, square law */
        else if (i == MX_REV)
            mix_rev[v] = (float)val / 127.0f;
        else if (i == MX_DLY)
            mix_dly[v] = (float)val / 127.0f;
        else if (i == MX_PAN)
            x0x_pan_gains(val, &mix_pl[v], &mix_pr[v]);
        break;
    case T_MST: master_set(&mst, i, val); break;
    default: break;
    }
}

void engine_set(int t, int v, int i, int val) { cq_put((uint32_t)t, (uint32_t)v, (uint32_t)i, (uint32_t)val); }

/* the output's scale: Felucca's -6 dBFS ceiling of the 24-bit codec (2^22), or its full scale (LOUD); the
 * clamp keeps an over from wrapping (normal: the codec's own full scale, as before) */
static volatile int kit_fx_off;                       /* wip242: ALL FX OFF: the PERFORM reverb / delay sends (the kit's) are muted; each track's own sends are not */
void engine_fx_off(int off)
{
    kit_fx_off = off ? 1 : 0;
    fx.bypass = off ? 1 : 0;
    mst.fbypass = off ? 1u : 0u;
}

int engine_fx_is_off(void) { return fx.bypass && mst.fbypass; }

static volatile float out_scale = 4194303.0f, out_lim = 2.0f;
void engine_out_loud(int on)
{
    out_scale = on ? 8388607.0f : 4194303.0f;
    out_lim = on ? 1.0f : 2.0f;
}

/* USB AUDIO FULL (#12): the render at full level, for the USB tap, and the MASTER knob after it, for
 * the speaker, headphones and line out (with the master's ~10 ms glide). The limiter then works at
 * full level too: turned down, the FM-1 plays what it sends, quieter. */
static volatile int usb_full;
static float vol_after = -1.0f;
void engine_usb_full(int on) { usb_full = on; }

void engine_volume(int32_t *out_lr, uint32_t n)
{
    float vol = (float)plat_master() / 4096.0f, v = vol_after < 0.0f ? vol : vol_after;
    uint32_t i;
    if (!usb_full) {
        vol_after = -1.0f;
        return;
    }
    for (i = 0; i < n; i++) {
        if (v != vol) {
            v += (vol - v) * 0.0022651f;
            if (fm_fabsf(vol - v) < 1e-5f)
                v = vol;
        }
        out_lr[2u * i] = (int32_t)((float)out_lr[2u * i] * v);
        out_lr[2u * i + 1u] = (int32_t)((float)out_lr[2u * i + 1u] * v);
    }
    vol_after = v;
}

/* wip227: the compressor, reverb and delay group (T_FX rows 0-1, T_MST row 0, the REV / DLY sends of T_MIX) at its factory values:
 * what a preset without a saved FX block starts with, and what a fresh project has. */
void engine_fx_defaults(sound_t *s)
{
    int v, i, p;
    for (v = 0; v < 2; v++)
        for (i = 0; i < NPARAMS_MAX; i++) {
            const x0x_param_t *q = (v == 0 && i < engine_nparams(T_FX, 0)) ? engine_param(T_FX, 0, i) : 0;
            s->v[T_FX][v][i] = q ? q->def : 0;
        }
    for (i = 0; i < NPARAMS_MAX; i++) {
        const x0x_param_t *q = i < engine_nparams(T_MST, 0) ? engine_param(T_MST, 0, i) : 0;
        s->v[T_MST][0][i] = q ? q->def : 0;
    }
    for (p = 0; p < NPARTS; p++) {
        const x0x_param_t *q = engine_param(T_MIX, p, MX_REV), *d = engine_param(T_MIX, p, MX_DLY);
        s->v[T_MIX][p][MX_REV] = q ? q->def : 0;
        s->v[T_MIX][p][MX_DLY] = d ? d->def : 0;
    }
    s->v[T_MIX][PART_909][MX_REV] = 0;               /* the reverb send (REVERB D/W on PERFORM) is on the Tonic and starts at 0 %: no reverb until the knob is turned (wip109; it was 25) */
    s->v[T_FX][0][FX_DL_TYPE] = 1;                   /* tape delay, dotted eighths */
    /* wip121: the master compressor starts OFF in effect (threshold at 0 dB, no makeup, no pump: the signal is not touched),
     * but with the drum-bus voicing ready for when THRESHOLD is turned down (FX button, COMP page):
     * 4:1; ATTACK 30 ms lets each kick's front through (the gain falls over ~30 ms); RELEASE 120 ms = one 16th note at 120-130 BPM
     * (125 / 115 ms): the gain has come back about 95% (3 time constants, 360 ms) before the next kick of a 4-on-the-floor bar
     * (462-500 ms apart), yet it still holds the tail and the hats between the kicks. Measured on the 8 factory presets: threshold
     * -16 dB gives about 3 dB of gain reduction on average, 5-6 dB at the peaks, and makeup +4.5 dB brings the level back to
     * the uncompressed one (+-1 dB). */
    s->v[T_MST][0][MST_RATIO] = 4;                   /* 4:1 */
    s->v[T_MST][0][MST_THRESH] = 127;                /* 0 dB: nothing is compressed until it is turned down */
    s->v[T_MST][0][MST_MAKEUP] = 0;                  /* 0 dB (about +4.5 dB, pot 24, goes with a -16 dB threshold) */
    s->v[T_MST][0][MST_ATTACK] = 105;                /* 30 ms */
    s->v[T_MST][0][MST_RELEASE] = 63;                /* 120 ms */
    s->v[T_MST][0][MST_PUMP] = 0;                    /* no kick-keyed ducking */
    s->v[T_MST][0][MST_COMP1] = 1;                   /* the one-knob COMP's nearest to the threshold above (it is not reached by a knob now) */
    s->v[T_FX][1][0] = (uint8_t)(16 + s->v[T_FX][0][FX_DL_WEAR] * 111 / 127);   /* TAPE (ui.c R_TAPE): on, its wear */
}

void engine_sound_defaults(sound_t *s)
{
    int t, v, i;
    uint8_t *z = (uint8_t *)s;
    for (i = 0; i < (int)sizeof *s; i++)
        z[i] = 0;
    for (t = 0; t < NTARGETS; t++)
        for (v = 0; v < engine_nvoices(t) && v < NVOICES_MAX; v++)
            for (i = 0; i < engine_nparams(t, v) && i < NPARAMS_MAX; i++) {
                const x0x_param_t *p = engine_param(t, v, i);
                s->v[t][v][i] = p ? p->def : 0;
            }
    /* the factory mix: big beat (Chemical Brothers, Fatboy Slim), balanced by tools/mix_report.py
     * against a funk break, a 909 kick under it and the break. The break leads, the kick sits
     * under it; the master squeezes and pumps. Each engine's
     * own voicing (its pot defaults) is left as its original; only these are X0X's. */
    s->v[T_MIX][PART_909][MX_LEVEL] = 100;           /* the Tonic drums at unity (TN_TRIM in tonic.c is their level) */
    for (v = 0; v < TN_VOICES; v++)                  /* the 16-bit store behind the knobs */
        for (i = 0; i < TN_NPARAMS; i++)
            s->tn16[v][i] = tonic_init_val(i);
    s->v[T_BRK][0][0] = 127;                         /* the break: full level, part a little over unity */
    s->v[T_MIX][PART_BRK][MX_LEVEL] = 120;
    engine_fx_defaults(s);                            /* wip227: the reverb / delay sends, the delay and the master compressor (below) */
}

void engine_sound_centre_drum_pans(sound_t *s)
{
    int t, v;
    for (t = T_909; t <= T_909; t++)
        for (v = 0; v < NDRUM && v < NVOICES_MAX; v++) {
            int i = engine_nparams(t, v) - 1;           /* PAN is every voice's last pot */
            const x0x_param_t *p = engine_param(t, v, i);
            if (p && p->def == 64 && p->max == 127)
                s->v[t][v][i] = 64;
        }
}

void engine_sound_reconcile(sound_t *s)
{
    int v, i;
    for (v = 0; v < TN_VOICES; v++)
        for (i = 0; i < TN_NPARAMS; i++)
            if (tonic_val_to_pot(i, s->tn16[v][i]) != s->v[T_909][v][i])
                s->tn16[v][i] = tonic_pot_to_val(i, s->v[T_909][v][i]);
}

void engine_tonic_set16(int v, int i, uint16_t val)
{
    cq_put(C_TNLO, (uint32_t)v, (uint32_t)i, val & 0xFFFu);
    cq_put(C_TNHI, (uint32_t)v, (uint32_t)i, (uint32_t)(val >> 12));
}

void engine_apply_sound(const sound_t *s)
{
    int t, v, i;
    for (v = 0; v < TN_VOICES; v++)                               /* the Tonic's exact values first; its pots then change nothing */
        for (i = 0; i < TN_NPARAMS; i++) {
            uint16_t w = tonic_val_to_pot(i, s->tn16[v][i]) == s->v[T_909][v][i] ? s->tn16[v][i]
                                                                                  : tonic_pot_to_val(i, s->v[T_909][v][i]);
            cq_put(C_TNLO, (uint32_t)v, (uint32_t)i, w & 0xFFFu);
            cq_put(C_TNHI, (uint32_t)v, (uint32_t)i, (uint32_t)(w >> 12));
        }
    for (v = 0; v < TN_VOICES; v++) {                              /* wip289: the shelving cuts */
        engine_set(T_909, v, TN_POT_LOSH, s->v[T_909][SH_LO_ROW][v]);
        engine_set(T_909, v, TN_POT_HISH, 127 - s->v[T_909][SH_HI_ROW][v]);
    }
    engine_set(T_MST, 0, MST_COMP1, s->v[T_MST][0][MST_COMP1]);   /* the macros first: the pots they set */
    engine_set(T_MST, 0, MST_FTYPE, s->v[T_MST][0][MST_FTYPE]);
    engine_set(T_MST, 0, MST_DJF, 0);                             /* wip368: the PERFORM filter has no knob now: always open (its saved value is kept; put s->v[T_MST][0][MST_DJF] back here to bring the filter back) */
    engine_set(T_MST, 0, MST_LOCUT, s->v[T_MST][0][MST_LOCUT]);   /* wip368: the master LOW CUT */
    for (t = 0; t < NTARGETS; t++)
        for (v = 0; v < engine_nvoices(t) && v < NVOICES_MAX; v++)
            for (i = 0; i < engine_nparams(t, v) && i < NPARAMS_MAX; i++)
                if (t != T_MST || i < MST_COMP1)
                    engine_set(t, v, i, s->v[t][v][i]);
}

void engine_drum(int kit, int voice, float vel)
{
    cq_put(C_DRUM, (uint32_t)kit, (uint32_t)voice, (uint32_t)(vel * 4095.0f));
}
/* wip251: a Tonic voice played at a note: semis semitones above its own pitch (negative = below), for this hit only */
void engine_tonic_note(int voice, int semis, float vel)
{
    semis = semis < -64 ? -64 : semis > 63 ? 63 : semis;
    cq_put(C_TNNOTE, (uint32_t)voice, (uint32_t)(semis + 64), (uint32_t)(vel * 4095.0f));
}
static int8_t tn_last_semis;                          /* (host tests: the pitch offset of the last C_TNNOTE) */
int engine_last_note(void) { return tn_last_semis; }   /* host tests */
void engine_tonic_choke(int mask) { cq_put(C_TNCHOKE, (uint32_t)(mask & 0xFF), 0, 0); }   /* wip328 */
void engine_tonic_sidechain(int input, int intensity, int mask) { cq_put(C_TNSC, (uint32_t)(input & 7), (uint32_t)(intensity < 0 ? 0 : intensity > 127 ? 127 : intensity), (uint32_t)(mask & 0xFF)); }   /* wip348 */
int engine_tonic_duck(void) { return tonic_duck_peak(&tn); }   /* wip352 */
void engine_brk_live(int key, int down) { cq_put(C_BRK, (uint32_t)key, (uint32_t)(down != 0), 0); }

/* ------------------------------------------------------- sequencer sink --- */
/* probability: each part's (and drum track's) chance on the step playing now, set by its T_PROB lane
 * before the step fires (motion_step runs first); 100 = always */
static uint8_t prob[NPARTS][NDRUM];
static uint32_t prob_rng = 0x2545F491u;
static int prob_skip(int part, int v)                 /* ISR: 1 = this hit does not play */
{
    uint32_t p = prob[part][v];
    if (p > 100u && p < PROB_COND + PROB_NCOND) {      /* A:B: the A-th of every B passes (seq_track_t.bars) */
        int a, b;
        prob_cond((int)p - PROB_COND, &a, &b);
        return (int)(seq.t[part].bars % (uint32_t)b) != a - 1;
    }
    if (p >= 100u)
        return 0;
    prob_rng ^= prob_rng << 13;
    prob_rng ^= prob_rng >> 17;
    prob_rng ^= prob_rng << 5;
    return prob_rng % 100u >= p;
}
static int8_t tn_note_semis;                          /* wip251: the pitch offset the next s_drum hit plays at (0 = the voice's own) */
#ifdef X0X_HOST
uint32_t eng_hit_n[8];
#endif
static void s_drum(void *x, int kit, int v, float vel);
static const pattern_t *eng_note_pat;                 /* wip253 (ARP step 3): the patterns the notes below belong to (not the bench's) */
static const int8_t *eng_note;                        /* the project's note[NPAT_NOTE][NTRK_NOTE][NSTEPS] */
static int8_t tn_seq_semis;                           /* the note of the last sequencer hit (host test) */
void engine_notes(const pattern_t *pat, const int8_t *note) { eng_note_pat = pat; eng_note = note; }
int engine_last_seq_note(void) { return tn_seq_semis; }
static int s_drum_seq(void *x, int kit, int v, float vel)
{
    if (prob_skip(kit, v))
        return 0;
    tn_note_semis = 0;                                /* a step plays at its stored note (0 = the voice's own pitch; FILL retriggers too:\n                                                    * eng_step is still the step they belong to) */
    if (kit == 0 && eng_note && seq.pat == eng_note_pat && seq.ppat[kit] < NPAT_NOTE && v < NTRK_NOTE)
        tn_note_semis = eng_note[((int)seq.ppat[kit] * NTRK_NOTE + v) * NSTEPS + (eng_step[TRK_DRUM + kit] & (NSTEPS - 1))];
    tn_seq_semis = tn_note_semis;
    s_drum(x, kit, v, vel);
    tn_note_semis = 0;
    return 1;
}
static void s_drum(void *x, int kit, int v, float vel)
{
    (void)x;
    if (v == 0)                          /* BD (both kits' track 1): the master's PUMP key */
        master_key(&mst, kit, vel);
    if ((unsigned)kit < NKIT && (unsigned)v < NDRUM)   /* MIX's drum pages: the track's meter jumps */
        eng_hit[kit][v] = (uint8_t)(vel >= 1.0f ? 255 : (int)(vel * 255.0f));
    if (kit == 0 && (unsigned)v < NDRUM)
        eng_hit_cnt[v] = (uint8_t)(eng_hit_cnt[v] + 1u);
#ifdef X0X_HOST
    if (kit == 0 && v >= 0 && v < 8)
        eng_hit_n[v]++;                  /* (host tests: how many hits each Tonic voice got) */
#endif
    if (kit == 0) {                      /* Microtonic: an accented step at full velocity, a normal one about a third of it */
        if (v < TN_VOICES)
            tonic_trigger_note(&tn, v, vel >= 0.999f ? TN_VEL_ACCENT : (int)(vel * 127.0f * ((float)TN_VEL_NORMAL / 88.0f) + 0.5f), tn_note_semis);
    }                                      /* (kit 1, the old 808: no engine behind it any more, silent) */
}
static uint8_t brk_applied[BRK_NSET];  /* the pattern settings the break was last given */
static void s_brk(void *x, int s16, int bar, float spb, int en)
{
    const brkpart_t *bp = &seq.pat[seq.ppat[TRK_BRK]].brk;
    int i;
    if (en && prob_skip(PART_BRK, 0))
        en = 0;
    (void)x;
    if (brk_hold)
        return;
    breaks_set_quiet(&brk, bp->steps == 0 || (seq.mute & (1u << MUTE_BRK)));   /* unheard: stop decoding */
    for (i = 0; i < BRK_NSET; i++)       /* follow the playing pattern's settings (12 compares) */
        if (bp->set[i] != brk_applied[i]) {
            brk_applied[i] = bp->set[i];
            breaks_set(&brk, i, bp->set[i]);
        }
    breaks_pin(&brk, (int)bp->slice[s16 & 15] - 1);   /* the step's own slice, if it has one */
    breaks_step(&brk, s16, bar, spb, en);
}
static void s_brkstop(void *x) { (void)x; breaks_stop(&brk); }
static void s_midi(void *x, uint8_t st, uint8_t d1, uint8_t d2)
{
    uint32_t cin = st >= 0xF0 ? 0x0Fu : (uint32_t)(st >> 4);
    (void)x;
    plat_midi_out(cin | ((uint32_t)st << 8) | ((uint32_t)d1 << 16) | ((uint32_t)d2 << 24));
}
static void s_step(void *x, int t, int p)
{
    (void)x;
    eng_step[t] = (uint8_t)p;
    if (mot_on)
        motion_step(&mot, t, seq.ppat[t], p, seq_next_step(&seq, t, p), seq_part_len(&seq, t));
}

/* --------------------------------------------------------------- motion --- */
static int mot_base(void *x, int t, int v, int i)
{
    (void)x;
    if (t >= NTARGETS)
        return 100;                                   /* T_PROB: always */
    return mot_base_snd ? mot_base_snd->v[t][v][i] : 0;
}
static void mot_apply(void *x, int t, int v, int i, int val)
{
    (void)x;
    if (t == T_PROB) {
        if ((v >> 4) < NPARTS && (v & 15) < NDRUM)
            prob[v >> 4][v & 15] = (uint8_t)val;
    } else if (t < NTARGETS)
        apply_param(t, v, i, val);
}

int engine_motion_part(int t, int v)
{
    switch (t) {
    case T_909: return PART_909;
    case T_BRK: return PART_BRK;
    case T_MIX: return v < NPARTS ? v : PART_909;
    case T_PROB: return (v >> 4) < NPARTS ? v >> 4 : PART_909;
    default: return PART_909;                     /* FX, master: the bar */
    }
}
int engine_motion_value(int k) { return motion_value(&mot, k); }
void engine_motion_rec(int k) { motion_req_rec(&mot, k); }
void engine_motion_hold(int k) { motion_req_hold(&mot, k); }
static const seq_sink_t SINK = {s_drum_seq, s_brk, s_brkstop, s_midi, s_step, 0};

/* ---------------------------------------------------------------- MIDI in --- */

static void midi_in(uint32_t pkt)
{
    uint8_t st = (uint8_t)(pkt >> 8), d1 = (uint8_t)(pkt >> 16) & 0x7F, d2 = (uint8_t)(pkt >> 24) & 0x7F;
    uint8_t ch = st & 0x0F, ty = st & 0xF0;
    int k, v;
    switch (st) {
    case 0xF8: seq_ext_clock(&seq); return;
    case 0xFA: seq_start(&seq); return;
    case 0xFB: seq_continue(&seq); return;
    case 0xFC: seq_stop(&seq); return;
    default: break;
    }
    if (ty == 0x90 && d2 == 0)
        ty = 0x80;
    if (rem_ch == ch) {                                  /* wip385: the REMOTE PANEL: its notes are keys for the UI, they never play a voice; */
        if (ty == 0x90 || ty == 0x80 || (ty == 0xB0 && (d1 == 120 || d1 == 123))) {   /* all sound / notes off = every key up */
            rq_put(pkt);
            return;
        }
    }
    if (ty == 0xB0 && (((cc_mode & 1u) && seq.ch[0] == ch) ||            /* wip386: a control change goes to the UI loop only if CC MODE says so: SELECTED TRACK = the Tonic's channel */
                       ((cc_mode & 2u) && ch >= cc_base && ch < cc_base + 8u)))   /* (TONIC MIDI CH = OFF never matches), CHANNELS = the 8-channel block */
        rq_put(pkt);
    for (k = 0; k < NKIT; k++)                          /* each part on its own channel (GLO): the drum */
        if (seq.ch[k] == ch && ty == 0x90) {             /* machines on GM drum notes */
            static const uint8_t GM[NDRUM] = {36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51};
            static const uint8_t GM8[NDRUM] = {36, 38, 41, 45, 50, 37, 39, 56, 49, 46, 42};
            for (v = 0; v < NDRUM; v++)
                if ((k ? GM8 : GM)[v] == d1)
                    s_drum(0, k, v, (float)d2 / 127.0f);
        }
    if (seq.ch[TRK_BRK] == ch && (ty == 0x90 || ty == 0x80) && d1 >= 36 && d1 <= 43)   /* the break's slices */
        breaks_live(&brk, d1 - 36, ty == 0x90);
}

/* ---------------------------------------------------------------- render --- */
static uint16_t tn_lo;
static void drain(void)
{
    uint32_t r = cq_r, pkt;
    int guard = 64;
    while (r != cq_w) {
        uint32_t c = cq[r % CQ], type = c >> 28, a = (c >> 20) & 0xFFu, b = (c >> 12) & 0xFFu, v = c & 0xFFFu;
        r++;
        if (type < 8u)
            apply_param((int)type, (int)a, (int)b, (int)v);
        else if (type == C_TNLO)
            tn_lo = (uint16_t)v;
        else if (type == C_TNHI)
            tonic_set(&tn, (int)a, (int)b, (uint16_t)((v << 12) | tn_lo));
        else if (type == C_DRUM)
            s_drum(0, (int)a, (int)b, (float)v / 4095.0f);
        else if (type == C_TNNOTE) {                      /* wip251: the same hit, at a pitch */
            tn_note_semis = tn_last_semis = (int8_t)((int)b - 64);
            s_drum(0, 0, (int)a, (float)v / 4095.0f);
            tn_note_semis = 0;
        }
        else if (type == C_TNCHOKE)                       /* wip328: the CHOKE group of the Tonic voices */
            tonic_set_choke(&tn, (int)a);
        else if (type == C_TNSC)                          /* wip348: the SIDECHAIN: a = the input voice, b = the intensity, v = the ducked mask */
            tonic_set_sidechain(&tn, (int)a, (int)b, (int)v);
        else if (type == C_BRK && !brk_hold)
            breaks_live(&brk, (int)a, (int)b);
    }
    cq_r = r;
    while (guard-- && plat_midi_in(&pkt))
        midi_in(pkt);
}

static float part_peak[NPARTS];
__attribute__((noinline)) static void meter(int p, const float *x, float g, uint32_t n)
{
    uint32_t i;
    float pk = part_peak[p];
    for (i = 0; i < n; i++) {
        float a = fm_fabsf(x[i]) * g;
        if (a > pk)
            pk = a;
    }
    part_peak[p] = pk;
}

/* dry into L and R by the part's pan; the sends stay mono (the reverb and delay make the width) */
/* a mix gain that glides to its target (one pole, ~10 ms) instead of stepping each block, which
 * zippered on quick turns of LEVEL, PAN or a send. cur < 0: never used, takes the target at once
 * (no fade-in at boot); settled, cur IS the target, so the product is the unsmoothed one exactly */
typedef struct { float cur, tgt; } glide_t;
#define GLIDE_A 0.0022651f                                /* 1 - exp(-1 / (10 ms * 44.1 kHz)) */
static void glide_set(glide_t *g, float t)
{
    g->tgt = t;
    if (g->cur < 0.0f)
        g->cur = t;
}
__attribute__((noinline)) static void add_glide(float *dst, const float *src, glide_t *g, uint32_t n)
{
    uint32_t i;
    float c = g->cur, t = g->tgt;
    if (c == t) {
        if (c != 0.0f)
            for (i = 0; i < n; i++)
                dst[i] += src[i] * c;
        return;
    }
    for (i = 0; i < n; i++) {
        c += (t - c) * GLIDE_A;
        dst[i] += src[i] * c;
    }
    if (fm_fabsf(t - c) < 1e-5f)
        c = t;
    g->cur = c;
}
/* each part's gains: dry left and right, its reverb and delay sends, and (drums) the voices' own sends */
enum { GL_L, GL_R, GL_REV, GL_DLY, GL_VREV, GL_VDLY, GL_N };
static glide_t mix_glide[NPARTS][GL_N] = {
    {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},
    {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}};
static void mix_targets(int p, float g)                     /* g: the part level */
{
    glide_set(&mix_glide[p][GL_L], g * mix_pl[p]);
    glide_set(&mix_glide[p][GL_R], g * mix_pr[p]);
    glide_set(&mix_glide[p][GL_REV], p == PART_909 && kit_fx_off ? 0.0f : g * mix_rev[p]);
    glide_set(&mix_glide[p][GL_DLY], p == PART_909 && kit_fx_off ? 0.0f : g * mix_dly[p]);
    glide_set(&mix_glide[p][GL_VREV], g);
    glide_set(&mix_glide[p][GL_VDLY], g);
}

#if X0X_DEBUG
/* development: what one block cost, for the worst-block record (main_fm1.c render_block) */
volatile uint32_t eng_dbg_wait, eng_dbg_nsub;  /* 24 MHz ticks waiting for core 1, stretches */
#endif

/* the mix of one stretch between two events: dry left and right, the reverb and delay sends;
 * kbuf the break's, mono the 909's kit send */
static float dry_l[256], dry_r[256], rev[256], dly[256], kbuf[256], mono[256];

/* a glide applied in place to a buffer that started at zero: buf * c is 0 + buf * c exactly, so this
 * is add_glide(zeros, buf) to the bit (the same steps of c) */
__attribute__((noinline)) static void scale_glide(float *buf, glide_t *g, uint32_t n)
{
    uint32_t i;
    float c = g->cur, t = g->tgt;
    if (c == t) {
        for (i = 0; i < n; i++)
            buf[i] *= c;
        return;
    }
    for (i = 0; i < n; i++) {
        c += (t - c) * GLIDE_A;
        buf[i] *= c;
    }
    if (fm_fabsf(t - c) < 1e-5f)
        c = t;
    g->cur = c;
}

/* the second core's job (or the first's, after its own part, with one core): the break into kbuf
 * (the 808 is gone); arg bit 8: not timed (the cycle counter is the first core's). Nothing
 * here is mixed: the first core does that afterwards, in one core's order */
static int brk_silent_now = 1;
static void core2_job(uint32_t arg)
{
    uint32_t n = arg & 0xFFu ? arg & 0xFFu : 256u, timed = !(arg & 0x100u), pt = timed ? plat_cycles() : 0;
    if (!brk_hold) {
        brk_silent_now = breaks_silent(&brk);             /* the render writes zeros: nothing to mix */
        breaks_render(&brk, kbuf, (int)n);
    }
    if (timed)
        PROF_ADD(PR_BRK, pt);
}

/* a drum machine's mix, from its render (L, R, voice sends; mono, its kit send) */
__attribute__((noinline)) static void kit_sends(int b, const float *m, uint32_t n)
{
    add_glide(rev, m, &mix_glide[b][GL_REV], n);
    add_glide(dly, m, &mix_glide[b][GL_DLY], n);
    meter(b, m, mix_level[b], n);
}

/* one stretch: the 909 (into the mix, in place) here while the second core renders the
 * 808 and the break; then those are mixed in, in the order one core always used (909, 808,
 * break): the same sums either way */
static void render_sub(float *out_l, float *out_r, uint32_t n)
{
    uint32_t arg = n & 0xFFu, i, pt;
    int on2 = plat_cpu2_run(core2_job, arg | 0x100u);
    for (i = 0; i < n; i++)
        dry_l[i] = dry_r[i] = rev[i] = dly[i] = 0.0f;
    {   /* the Tonic drums, straight into the mix: L and R (it has no sends of its own; the part's sends take the kit) */
        PROF_T(p9);
        if (tonic_active(&tn)) {
            tonic_render_sends(&tn, dry_l, dry_r, rev, dly, mix_level[PART_909], (int)n);   /* wip242: and each track's own reverb / delay sends */
            mix_targets(PART_909, mix_level[PART_909]);
            for (i = 0; i < n; i++)                       /* the kit send: the whole machine, both sides */
                mono[i] = (dry_l[i] + dry_r[i]) * 0.5f;   /* (centred: L exactly) */
            scale_glide(dry_l, &mix_glide[PART_909][GL_L], n);   /* then the part's PAN over the kit */
            scale_glide(dry_r, &mix_glide[PART_909][GL_R], n);
            kit_sends(PART_909, mono, n);
        }
        PROF_ADD(PR_909, p9);
    }
    pt = plat_cycles();
#if X0X_DEBUG
    eng_dbg_nsub++;
    {
        uint32_t w0 = plat_ticks24();
        int failed = on2 && plat_cpu2_wait();
        eng_dbg_wait += plat_ticks24() - w0;
        if (failed)
            on2 = 0;
    }
    if (!on2)
#else
    if (!on2 || plat_cpu2_wait())
#endif
        core2_job(arg);                                   /* one core (or the second failed): here */
    else
        PROF_ADD(PR_808, pt);                             /* what the first core waited for the second */
    if (!brk_hold && !brk_silent_now) {
        float g = mix_level[PART_BRK];
        mix_targets(PART_BRK, g);
        add_glide(dry_l, kbuf, &mix_glide[PART_BRK][GL_L], n);
        add_glide(dry_r, kbuf, &mix_glide[PART_BRK][GL_R], n);
        add_glide(rev, kbuf, &mix_glide[PART_BRK][GL_REV], n);
        add_glide(dly, kbuf, &mix_glide[PART_BRK][GL_DLY], n);
        meter(PART_BRK, kbuf, g, n);
    }
    {
        PROF_T(pf);
        fxbus_process_st(&fx, dry_l, dry_r, rev, dly, out_l, out_r, (int)n);
        PROF_ADD(PR_FX, pf);
    }
}

void engine_render(int32_t *out_lr, uint32_t n)
{
    static float out_l[256], out_r[256];
    uint32_t done = 0, i;
    float vol = usb_full ? 1.0f : (float)plat_master() / 4096.0f;   /* (FULL: engine_volume) */
    int p;
    if (n > 256u)
        n = 256u;
    eng_prof_frames += n;
    if ((mot_was_on && !mot_on) || mot_release_req) {   /* motion off, or its lanes replaced: knobs back */
        mot_release_req = 0;
        motion_release(&mot);
    }
    mot_was_on = mot_on;
    guard_apply();
    {
        PROF_T(pt);
        drain();
        if (mot_on)
            motion_tick(&mot, n);
        fxbus_set_bpm(&fx, seq_tempo(&seq));
        seq_advance(&seq, 0, &SINK);
        PROF_ADD(PR_SEQ, pt);
    }
    while (done < n) {
        uint32_t k = seq_until_event(&seq, n - done);
        render_sub(out_l + done, out_r + done, k);
        {
            PROF_T(pt);
            seq_advance(&seq, k, &SINK);
            PROF_ADD(PR_SEQ, pt);
        }
        done += k;
    }
    if (mot_playing && !seq.playing)                     /* stopped: every knob back to its own value */
        motion_release(&mot);
    mot_playing = seq.playing;
    {
        PROF_T(pt);
        master_process_st(&mst, out_l, out_r, (int)n, vol);
        PROF_ADD(PR_MST, pt);
    }
    for (i = 0; i < n; i++) {
        float l = out_l[i], r = out_r[i];            /* (out_scale: NORMAL 2^22, Felucca's; LOUD 2^23) */
        l = l > out_lim ? out_lim : l < -out_lim ? -out_lim : l;
        r = r > out_lim ? out_lim : r < -out_lim ? -out_lim : r;
        out_lr[2u * i] = (int32_t)(l * out_scale);
        out_lr[2u * i + 1u] = (int32_t)(r * out_scale);
        if (i & 1u)
            eng_scope[eng_scope_w++ & (SCOPE_N - 1u)] = (int16_t)((out_l[i] + out_r[i]) * 16383.5f);
    }
    for (p = 0; p < NPARTS; p++) {                      /* meters: peak hold, ~300 ms decay at the UI */
        uint32_t q = (uint32_t)(fm_minf(part_peak[p], 1.0f) * 32767.0f);
        if (q > eng_peak[p])
            eng_peak[p] = (uint16_t)q;
        part_peak[p] = 0.0f;
    }
}

#ifdef X0X_WEB
/* the browser page's "download the 8 voices": one hit of a Tonic voice, alone and dry (its own EQ, drive,
 * level and pan; no sequencer, FX or master), from a copy of the live sound, so nothing playing is touched.
 * engine_hit_begin(v) starts it (full velocity), engine_hit_render(l, r, n) adds n frames (<= 1024) to l / r
 * and returns 1 while the hit is still sounding. Not built for the device (no RAM spent there). */
static tonic_t hit_tn;
void engine_hit_begin(int v)
{
    int k;
    hit_tn = tn;
    for (k = 0; k < TN_VOICES; k++)
        hit_tn.active[k] = 0;
    tonic_trigger(&hit_tn, v, TN_VEL_ACCENT);
}
int engine_hit_render(float *l, float *r, int n)
{
    int k, any = 0;
    tonic_render_st(&hit_tn, l, r, n);
    for (k = 0; k < TN_VOICES; k++)
        any |= hit_tn.active[k];
    return any;
}
#endif

/* -------------------------------------------------------------- loops --- */
/* a user loop's length in bars: 1, 2, 4 or 8, whichever puts its tempo nearest 120 BPM
 * inside 80..180 (breaks are recorded at their own tempo; the player stretches to ours) */
static uint8_t loop_bars(uint32_t ns, uint32_t rate)
{
    float sec = (float)ns / (float)(rate ? rate : 22050u), best = 1e9f;
    uint8_t bars = 1, b;
    for (b = 1; b <= 8; b = (uint8_t)(b * 2)) {
        float bpm = (float)b * 240.0f / sec, d = bpm > 120.0f ? bpm - 120.0f : 120.0f - bpm;
        if (bpm >= 80.0f && bpm <= 180.0f && d < best) {
            best = d;
            bars = b;
        }
    }
    return bars;
}

/* No built-in loops (wip474): the BREAK part only plays loops of the user sample slots; nothing is
 * sampled in the firmware itself. */
#define X0X_NBREAKS 0
#define X0X_BREAK_RATE 22050u

static const brk_loop_t *brk_builtin(int k)
{
    (void)k;
    return 0;                            /* an empty loop: the part stays silent */
}

/* the LOOP choices: the built-in bank, then every loop (zone) of the user slots, in order.
 * The list follows uploads: engine_brk_loops() rescans it. */
#define NLOOPS_MAX (X0X_NBREAKS + PLAT_NSLOTS * PLAT_SLOT_ZONES)
static const char *loop_names[NLOOPS_MAX];
static char user_names[PLAT_NSLOTS * PLAT_SLOT_ZONES][8];
static uint8_t loop_slot[NLOOPS_MAX], loop_zone[NLOOPS_MAX];
static int nloops;

static void scan_loops(void)
{
    int i, k, z, u = 0;
    nloops = 0;
    for (k = 0; k < PLAT_NSLOTS; k++) {                 /* yours: BR1, BR2 ... in order, whatever slot */
        int nz = plat_slot_zones(k);                    /* holds them (the break page packs the list) */
        for (z = 0; z < nz && z < PLAT_SLOT_ZONES; z++) {
            char *n = user_names[k * PLAT_SLOT_ZONES + z];
            int j = 2;
            u++;
            n[0] = 'B';
            n[1] = 'R';
            if (u >= 10)
                n[j++] = (char)('0' + u / 10);
            n[j++] = (char)('0' + u % 10);
            n[j] = 0;
            loop_slot[nloops] = (uint8_t)k;
            loop_zone[nloops] = (uint8_t)z;
            loop_names[nloops++] = n;
        }
    }
}

int engine_brk_nslots(void)
{
    if (!nloops)
        scan_loops();
    return nloops;
}
const char *const *engine_brk_slot_names(void)
{
    if (!nloops)
        scan_loops();
    return loop_names;
}

int engine_brk_slice(void) { return brk.running ? brk.st_slice : -1; }

void engine_brk_state(int *slice, int *bank, int *div, int *running)
{
    *slice = brk.st_slice;
    *bank = brk.st_bank;
    *div = brk.st_div;
    *running = brk.running;
}

/* the loop's outline for the screen: n columns of peak level 0..255 (main loop: decodes the
 * whole loop once, ~20k IMA steps; called when the loop changes, never per frame) */
static const int16_t ISTEP[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
    107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428,
    4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
    22385, 24623, 27086, 29794, 32767};
static const int8_t IIDX[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
int engine_brk_outline(int which, uint8_t *peaks, int n)
{
    const brk_bank_t *bk = &brk.bank[which & 1];
    uint32_t i, ns = bk->n;
    int32_t pred = 0, idx = 0, col = 0, pk = 0;
    uint32_t per;
    if (!bk->adpcm || !ns || n <= 0)
        return 0;
    per = (ns + (uint32_t)n - 1u) / (uint32_t)n;       /* samples per column (32-bit: no __udivdi3) */
    for (i = 0; i < ns; i++) {
        uint32_t code = (bk->adpcm[i >> 1] >> ((i & 1u) * 4u)) & 15u;
        int32_t st = ISTEP[idx], vd = st >> 3, c;
        if (code & 4u) vd += st;
        if (code & 2u) vd += st >> 1;
        if (code & 1u) vd += st >> 2;
        pred += (code & 8u) ? -vd : vd;
        pred = pred < -32768 ? -32768 : pred > 32767 ? 32767 : pred;
        idx += IIDX[code & 7u];
        idx = idx < 0 ? 0 : idx > 88 ? 88 : idx;
        c = (int32_t)(i / per);
        if (c != col) {
            peaks[col] = (uint8_t)(pk >> 7);
            col = c;
            pk = 0;
        }
        if ((pred < 0 ? -pred : pred) > pk)
            pk = pred < 0 ? -pred : pred;
    }
    peaks[col] = (uint8_t)(pk >> 7);
    for (col++; col < n; col++)
        peaks[col] = 0;
    return 1;
}
float engine_gr_db(void) { return mst.gr_view; }
void engine_master_format(int i, char *buf) { master_format(&mst, i, buf); }

void engine_brk_loops(void)
{
    const brkpart_t *bp = &seq.pat[seq.ppat[TRK_BRK]].brk;
    brk_loop_t L[2];
    int w;
    scan_loops();                                       /* an upload may have changed the user loops */
    brk_hold = 1;                                       /* the ISR skips the break while we rewrite it */
    for (w = 0; w < 2; w++) {
        int slot = w ? bp->slot_b : bp->slot_a;
        uint32_t ns = 0, rate = 0;
        char name[9];
        const uint8_t *d = (slot >= X0X_NBREAKS && slot < nloops)
                               ? plat_slot(loop_slot[slot], loop_zone[slot], &ns, &rate, name) : 0;
        if (d) {
            L[w].adpcm = d;
            L[w].nsamples = ns;
            L[w].rate = rate;
            L[w].bars = loop_bars(ns, rate);
            breaks_set_loop(&brk, w, &L[w]);
        } else {
            breaks_set_loop(&brk, w, brk_builtin(slot < X0X_NBREAKS ? slot : 0));   /* built-in (or an empty user slot: the first) */
        }
    }
    brk_hold = 0;
}

void engine_init(pattern_t *patterns, song_t *song, lane_t *lanes, const sound_t *base)
{
    int p;
    for (p = 0; p < NPARTS * NDRUM; p++)
        prob[p / NDRUM][p % NDRUM] = 100;             /* every step plays, until a lane says */
    tonic_init(&tn, 44100);
    breaks_init(&brk);
    fxbus_init(&fx, dly_buf, DLY_LEN);
    master_init(&mst);
    seq_init(&seq, patterns);
    seq.song = song;
    motion_init(&mot, lanes);
    mot.base = mot_base;
    mot.apply = mot_apply;
    mot_base_snd = base;
    for (p = 0; p < NPARTS; p++) {
        mix_level[p] = 1.0f;
        mix_rev[p] = mix_dly[p] = 0.0f;
    }
}
