/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X app: the project (everything that is saved) and the app-level entry points.
 * Included by the unity build after gfx.c, so drawing helpers are in scope. */
#pragma once
#include <stdint.h>
#include "plat.h"
#include "engine.h"
#include "../seq/pattern.h"
#include "../seq/sequencer.h"
#include "../seq/motion.h"
#include "../dsp/breaks.h"
#include "../dsp/fxbus.h"            /* FX_* param indices (the send pages) */

#ifndef X0X_VERSION                     /* a release build passes its own (tools/build.py --release) */
#define X0X_VERSION "DEV"
#endif
#ifndef TONIC_VERSION                   /* the version the welcome screen shows (one source for all of them: a later job) */
#define TONIC_VERSION "V0.9"
#endif
#define PROJ_MAGIC 0x50305830u           /* "0X0P" */
#define PROJ_FORMAT 1u
#define PROJ_STEREO 2u
#define PROJ_SND_REV 8u                  /* 1: the 909 SD has a DECAY pot (index 3; LEVEL.. moved up one);
                                          * 2: the master's one-knob COMP and FILTER, the delay's TAPE;
                                          * 3: the 909 BD has sends (REV, DLY at 8, 9; PAN moved to 10);
                                          * 4: the 808 BD too (REV, DLY at 7, 8; PAN moved to 9);
                                          * 5: the break's FILTER (pot 2, 64 = off; was unused, 0);
                                          * 6: Tonic replaces the 909 (22 pots a voice, sound_t has its 16-bit store;
                                          *    older sounds do not load: their size differs);
                                          * 7: the master FILTER has a TYPE (HPF / BP / LPF) and 0 = closed: the old 64 = off is reset;
                                          * 8 (wip227): the compressor is reset to THRESHOLD 0.0 dB, MAKEUP 0.0 dB once (clean-up) */

typedef struct {
    uint32_t magic, format;
    uint8_t palette, keyled, clk_out, notes_out;   /* keyled: 0 off, 1 keys and the buttons' glow, 2 keys only */
    uint16_t bpm_x10;
    uint8_t accent_q7, keysound;         /* black drum keys sound: 0 = only when stopped or recording, 1 = always */
    uint8_t autosave_off;                /* 0: save by itself while stopped and idle (ui.c autosave) */
    uint8_t rsv_bright;                  /* was the screen's brightness (removed: always full); unused */
    uint8_t stereo;                      /* the pans the sound has: 0 none (saved mono), 1 the parts', 2 the
                                          * drum voices' too (PROJ_STEREO); a load centres what is missing */
    uint8_t snd_rev;                     /* the sound's layout (PROJ_SND_REV): 0 = before the 909 SD's DECAY */
    uint8_t midi_ch[5];                  /* each part's MIDI channel (in and out): 0 = its default (an older
                                          * project), 1-16, 17 = OFF */
    uint8_t out_loud;                    /* 1: the output 6 dB hotter (the codec's full scale) */
    uint8_t usb_full;                    /* 1: USB audio at full level, the MASTER knob for the FM-1's own outputs */
    uint8_t one_core;                    /* 1: the second core stays off (GLO > 2ND CORE, or after it crashed
                                          * beside it); applies at the next power-on. Older projects: 0, two */
    uint8_t autoscroll;                  /* 1: SEQ > AUTOSCROLL: the step page follows the playhead (older projects: 0) */
    uint8_t zoom_off;                    /* 1: a turned knob shows no zoom (GLO > ZOOM KNOB = NO); 0 (also older projects): zoom */
    uint8_t beat_off;                    /* 1 (default since wip234, also older projects that never stored it): the 4 / 4 beat lights (white keys 1 5 9 13) are off (GLO > 4/4 LED = OFF); 0: on */
    uint8_t classic;                     /* wip302: GLO > SYSTEM > TONIC ENGINE: 0 = ENHANCED (default, also older projects), 1 = CLASSIC (only the 21 Microtonic parameters count: the extras get greyed out and switched off in a LATER step; today the flag is only stored) */
    uint8_t preset_cur;                  /* wip309: the preset place loaded last + 1 (1-8 factory, 9-40 user 1-32; 0 = none), kept over power-off so QUICK SAVE still knows its place (older projects: 0) */
    uint8_t name_manual;                 /* wip324: GLO > PROJECT > NAME PROJECTS: 0 = GENERATE (default, also older projects), 1 = MANUAL (the name screen is a letter editor) */
    uint8_t choke;                       /* wip335: the CHOKE group (bit v = Tonic track v), kept by the autosave too (older projects: 0 = none) */
    uint8_t sc_int, sc_in, sc_mask;      /* wip353: the SIDECHAIN (intensity 0-127, input track 0-7, ducked tracks bit v): kept by the autosave and the presets (older projects: 0, track 1, none) */
    uint8_t cc_mode, cc_base;            /* wip386: GLO > CC MODE (0 OFF default, also older projects; 1 SELECTED TRACK, 2 CHANNELS, 3 BOTH) and CC CH BASE (0-8 = the first channel of the 8-channel block, 1-9 on the screen) */
    uint8_t gacc;                        /* wip612: GLO > GLOBAL ACCENT: 0 = 100 % (default, also older projects), else the percent - 9 (1..91 = 10..100 %): how much of its boost the TOTAL accent row keeps */
} settings_t;

/* the song and the motion lanes: one blob, split across OBJ_SONG and OBJ_MOTION */
typedef struct {
    song_t song;
    lane_t lane[NLANE];
} arrange_t;

typedef struct {
    settings_t set;
    sound_t sound;
    pattern_t pat[NPAT];
    arrange_t arr;
    int8_t note[NPAT_NOTE][NTRK_NOTE][NSTEPS];   /* wip252 ARP: the semitones of the hit on step s of a Tonic track (0 = the voice's own pitch);
                                                  * last in the project so an older saved project (shorter) still loads, with no notes */
} project_t;

extern project_t proj;

void project_defaults(void);
int project_load(void);                  /* 0 = loaded; else defaults are in place */
int project_autosave(void);              /* the same, but the compressor, reverb and delay are not kept (only SAVE keeps them) */
int project_chain_count(void);   /* wip250: patterns 1-8 flagged CHAINED */
int project_chain_tidy(void);    /* wip250: drop the flags of patterns with no notes; returns how many flags are left */
int project_save(void);                  /* 0 = saved and verified; -7 = too much to fit (nothing written) */
int project_autosave_settings(void);     /* wip321: the settings alone, kept even with AUTOSAVE off: 0 = saved */
int project_save_settings(void);         /* the settings alone (proj.set as it is): 0 = saved */
void project_apply_io(void);             /* the settings into the sequencer and the engine: MIDI, the output */
int project_mem_pct(void);
void project_flash_changed(void);        /* a restore wrote the flash: the next save writes every object */               /* how full the room for patterns, song and motion is, % */

void ui_init(void);
void ui_frame(void);
int ui_midi_seen(int what);            /* wip384 (host tests): 0 = packets the UI took from the MIDI queue, 1 = the last controller number, 2 = its value */
void ui_input_only(void);
void ui_leds_tick(void);                 /* wip615: between the frames: the blinking keys by the clock */
int ui_dirty(void);
int ui_msmode(void);                  /* wip606: 0 = MUTE mode, 1 = SOLO mode */
int ui_lowbatt(void);                 /* wip605: the LOW BATTERY screen is up */
void ui_say(const char *a, const char *b);
void ui_warn(const char *a, const char *b);   /* ui_say for what did not happen: a screen reader says it whole */
/* backup and restore over USB (backup.c) */
void ui_flush(void);                     /* stopped: the unsaved changes saved now (a backup takes the flash's) */
void ui_hold_saves(int on);              /* 1: no save until 0 (a backup streams the flash) */
void ui_redraw(void);                    /* the whole screen drawn again at the next frame */
void app_restored(void);                 /* a restore wrote the project: loaded again, as at power-on */
int app_load_preset(const uint8_t *b, uint32_t n);   /* a TNP1 preset (tnp.h) as the working project: 0 = loaded, 10 = playing */
int project_preset_unpack(const uint8_t *b, uint32_t n);
uint32_t project_preset_pack(uint8_t *out, uint32_t max, const char *name, unsigned bpm_x10, unsigned mutes);   /* (mutes: bits 0-7 the muted voices, bits 8-15 the CHOKE group) */   /* the working Tonic project as a TNP1 (patterns 1-8): its length, 0 = no room */
void ui_save_now(void);                  /* tests: what SAVE used to do (the working project to the flash, stopped) */
