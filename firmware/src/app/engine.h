/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X audio engine: the three parts, the mixer, the send FX and the master, driven by
 * the sequencer. engine_render() runs in the audio ISR; everything else is called
 * from the main loop and reaches the ISR only through the command queue, so the
 * engines' state is only ever touched by one context.
 *
 * Parameters are addressed as (target, voice, index) -> pot value, the same triple
 * the UI pages, the project file and the command queue use:
 *   T_909 / T_808   voice = track 0..10, or DR_KIT / D8_KIT (= 11) for the kit
 *   T_BRK           voice 0: breaks.h params (the per-pattern ones live in the pattern)
 *   T_FX            voice 0: fxbus.h params (reverb, delay, master drive / comp)
 *   T_MIX           voice = part 0..2: Level, Rev send, Dly send (drums: on top of the per-voice sends)
 *   T_MST           voice 0: master filter + limiter
 */
#pragma once
#include <stdint.h>
#include "../dsp/x0x_param.h"
#include "../dsp/tonic.h"
#include "../seq/sequencer.h"
#include "../seq/motion.h"

enum { T_909, T_808, T_BRK, T_FX, T_MIX, T_MST, NTARGETS };
#define T_PROB NTARGETS                /* a lane's pseudo-target: a step's chance, 5..100 % (not in sound_t);
                                        * voice = part << 4 | drum track (0 on the break) */
enum { PART_909, PART_808, PART_BRK, NPARTS };
/* T_MIX, per part: level, reverb send, delay send, pan (64 = centre) */
enum { MX_LEVEL, MX_REV, MX_DLY, MX_PAN, MX_NPARAMS };
#define NVOICES_MAX 12                 /* a drum machine: 11 tracks + the kit */
/* wip289: the Tonic's two shelving cuts live in the T_909 rows 8 and 9 (the Tonic has 8 voices: rows 8-11 were unused), one byte per track, so the saved
 * sound keeps its size: SH_LO_ROW = the low cut (0 = off), SH_HI_ROW = the high cut as stored (127 - the knob: 0 = off, so an older save reads as off) */
#define SH_LO_ROW 8
#define SH_HI_ROW 9
#define NPARAMS_MAX 24                 /* params per (target, voice): the Tonic has 22 */

/* the sound of a project: every pot of every engine, mirrored here by the main loop */
typedef struct {
    uint8_t v[NTARGETS][NVOICES_MAX][NPARAMS_MAX];
    uint16_t tn16[TN_VOICES][TN_NPARAMS];   /* the Tonic's exact values: the T_909 pots are their coarse view
                                             * (tonic.h); engine_sound_reconcile before a save */
} sound_t;

extern seq_t seq;                      /* the sequencer (its pattern pointer is the project's) */
/* render stages (the PERF screen): 909, 808, BREAK, FX, MASTER, SEQ */
enum { PR_909, PR_808, PR_BRK, PR_FX, PR_MST, PR_SEQ, ENG_PROF_N };
extern volatile uint32_t eng_prof[ENG_PROF_N], eng_prof_frames;
void engine_motion_enable(int on);                          /* off: lanes do not play (the PERF test) */
void engine_motion_reset(void);                             /* lanes replaced (undo): knobs to their own values */
/* overload guard (engine.c): the platform reports each block's load after rendering it */
void engine_load(uint32_t pct, uint32_t frames);           /* ISR, after engine_render */
void engine_guard_enable(int on);                           /* off: full quality always (the PERF test) */
int engine_guard_active(void);
extern volatile uint32_t eng_guard_count;                   /* times it engaged (PERF screen) */
extern volatile uint8_t eng_step[NTRACKS];   /* playheads for the UI */
extern volatile uint16_t eng_peak[NPARTS];   /* part meters, Q15, decaying (UI) */
extern volatile uint8_t eng_hit[NKIT][NDRUM]; /* each drum track's last hit, 255 = full velocity (the UI lets it fall) */
#define SCOPE_N 256u
extern int16_t eng_scope[SCOPE_N];     /* the last output samples, for the screen */
extern volatile uint32_t eng_scope_w;

void engine_init(pattern_t *patterns, song_t *song, lane_t *lanes, const sound_t *base);
void engine_render(int32_t *out_lr, uint32_t n);   /* ISR: n stereo frames, 24-bit in int32 */
#ifdef X0X_WEB
void engine_hit_begin(int v);                        /* browser page: one dry hit of Tonic voice v (0..7), alone */
int engine_hit_render(float *l, float *r, int n);   /* adds n (<= 1024) frames to l / r; 1 while still sounding */
#endif

/* parameter descriptors (UI) */
int engine_nvoices(int target);
int engine_nparams(int target, int voice);
const x0x_param_t *engine_param(int target, int voice, int i);
const char *engine_voice_name(int target, int voice);
const char *engine_voice_short(int target, int voice);   /* the track's short label: "1".."8" for the Tonic (the grid and the mixer have no room for TRACK n) */

/* main loop -> ISR */
void engine_set(int target, int voice, int i, int value);   /* queued; the sound_t mirror is the caller's */
void engine_fx_off(int off);           /* 1: ALL FX OFF: the PERFORM sends (the kit's reverb / delay D/W), distortion and the PERFORM filter are off; the tracks' own sends (MIX PAGE 2) and the compressor are not touched; not saved */
int engine_fx_is_off(void);            /* the state engine_fx_off() set (tests) */
void engine_out_loud(int on);          /* 1: the output at the codec's full scale (6 dB hotter than Felucca's) */
void engine_usb_full(int on);          /* 1: engine_render at full level, engine_volume the MASTER knob after */
void engine_volume(int32_t *out_lr, uint32_t n);   /* ISR, after engine_render (and the USB tap) */
void engine_sound_reconcile(sound_t *s);                    /* tn16 follows the pots the knobs moved (before a save) */
void engine_tonic_set16(int voice, int i, uint16_t val);    /* a Tonic value at full precision (a preset load) */
void engine_apply_sound(const sound_t *s);                  /* queue every value (load, init) */
void engine_fx_defaults(sound_t *s);                        /* only the compressor / reverb / delay group (T_FX, T_MST, the T_MIX sends) at its power-on values */
void engine_sound_defaults(sound_t *s);                     /* each engine's power-on values */
void engine_sound_centre_drum_pans(sound_t *s);             /* every drum voice's PAN (its last pot) to centre */
void engine_drum(int kit, int voice, float vel);            /* play now (keys) */
void engine_tonic_sidechain(int input, int intensity, int mask);   /* wip348: the SIDECHAIN (one for the project): the input voice 0-7, the intensity 0-127 (0 = none), the ducked voices (bit v) */
#define REMOTE_NOTE0 53                                     /* wip385: the REMOTE PANEL: panel key k (0 = F3 .. 26 = G5) is MIDI note 53 + k (F3 = 53, C4 = 60) */
void engine_remote_ch(int ch);                              /* wip385: the remote panel's MIDI channel 0-15, -1 = OFF */
void engine_cc_cfg(int mode, int base);                      /* wip386: CC MODE (0 OFF, 1 SELECTED TRACK = Tonic channel, 2 CHANNELS = block of 8 from channel base + 1, 3 BOTH) */
int engine_midi_pop(uint32_t *pkt);                         /* wip384: the next packet for the UI (control change on the Tonic's channel), 1 = got one; main loop only */
int engine_tonic_duck(void);                                /* wip352: the sidechain's duck right now, 0-255 (the UI's pulsing dot) */
void engine_tonic_choke(int mask);                          /* wip328: the CHOKE group (bit v = Tonic voice v): a hit of one cuts the others */
void engine_tonic_note(int voice, int semis, float vel);
void engine_notes(const pattern_t *pat, const int8_t *note);   /* wip253: where the sequencer reads each step's note (project_t.note) */
int engine_last_seq_note(void);                                /* the note of the last hit the sequencer played (host test) */
int engine_last_note(void);                                 /* host tests: the semitone offset of the last note played */    /* wip251: a Tonic voice at a note: semis semitones from its own pitch, this hit only */
void engine_brk_live(int key, int down);
void engine_brk_loops(void);                                /* (re)read the A / B loops of the current pattern */
int engine_brk_slice(void);                                 /* slice sounding now, -1 none (UI) */
void engine_brk_state(int *slice, int *bank, int *div, int *running);
int engine_brk_outline(int which, uint8_t *peaks, int n);   /* the loop's peaks for the screen (main loop) */
/* motion (seq/motion.h): lanes play into the engine, never into the sound_t mirror */
int engine_motion_part(int target, int voice);              /* the part a parameter's lane follows */
int engine_motion_value(int lane);                          /* what the lane plays now, -1 none */
void engine_motion_rec(int lane);                           /* record a pass from the next step */
void engine_motion_hold(int lane);                          /* the knob wins for a pass */
float engine_gr_db(void);                                   /* master compressor gain reduction (UI meter) */
void engine_master_format(int i, char *buf);                /* a master pot in its unit */
int engine_brk_nslots(void);                                /* built-in loops + user slots */
const char *const *engine_brk_slot_names(void);
