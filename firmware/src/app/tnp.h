/* SPDX-License-Identifier: GPL-3.0-only */
/* TNP1: the FM1 Tonic preset, the binary a Microtonic .mtpreset is packed into (tools/tnp_pack.c, later the
 * transfer webapp) and the firmware loads (project.c: project_preset_unpack). All numbers little-endian.
 * The firmware never parses text and uses no floating point for this: the computer does the conversion.
 *
 *   header   40 bytes   "TNP1", total length u16, tempo_x10 u16, swing u8 (0..100, the X0X swing byte),
 *                       npat u8 (1..12 = patterns a..l), step rate u8 (RATE_* of pattern.h), mute bits u8
 *                       (bit v = voice v+1 muted in the preset), fill rate x100 u16, master volume x10 i16 (dB),
 *                       name 24 bytes (NUL padded)
 *   voices   8 x 44     p[21] u16 (0..65535, the libpo32 order = TNP_* of tonic.h), pan pot u8 (0..127, 64 centre), flags u8 (wip328: bit 0 = the voice is in the CHOKE group)
 *   patterns npat x 196 len u8, chained u8, rsv 2, then 8 voices x { trig[2], acc[2], fill[2] } u32 (step s = bit s of
 *                       word s/32): the hits, the voice's own accents, the voice's fill steps
 * The firmware plays the voices' own accents, fill steps (4 retriggers per step), the mute bits and the chain flag (a
 * chained pattern runs into the next one when it ends; after the 8th it goes back to the 1st). */
#pragma once
#define TNP_MAGIC "TNP1"
#define TNP_HDR 40u
#define TNP_VOICE 44u
#define TNP_PAT_HDR 4u
#define TNP_PAT (TNP_PAT_HDR + 8u * 24u)
#define TNP_NPAT_MAX 12u
#define TNP_SIZE(npat) (TNP_HDR + 8u * TNP_VOICE + (npat) * TNP_PAT)
#define TNP_NAME_LEN 24u
#define TNP_NPAT_SAVE 8u            /* SAVE AS keeps patterns a-h (1-8), the ones the firmware loads (user slots; 1960 bytes < PLAT_STORE_MAX) */
#define TNP_NPAT_LOAD 8u            /* the firmware keeps patterns a-h (1-8); a file's others are not loaded, 9-32 stay empty */

/* wip227: OPTIONAL TRAILER after the patterns (a preset made by SAVE AS has it; a Microtonic conversion and the factory presets do not): the
 * compressor, reverb and delay as the user left them. The u16 total at 4 then is TNP_SIZE(npat) + TNP_FX_LEN. Without it the preset starts
 * with these at their factory values (compressor 0.0 dB / 0.0 dB, reverb and delay off).
 *   0..3 "TFX1", 4..27 T_FX voice 0 (24 pots), 28..51 T_FX voice 1, 52..75 T_MST voice 0, 76 reverb send (T_MIX Tonic), 77 delay send, 78 the SIDECHAIN's intensity (0-127), 79 its input track (0-7) (wip353; older presets: 0 / 0). The voice record's spare byte (offset 43): bit 0 = in the CHOKE group, bit 1 = ducked by the sidechain */
#define TNP_FX_TAG "TFX1"
#define TNP_FX_LEN 80u
/* wip242: the same trailer with the TRACKS' OWN sends, "TFX2": the 80 bytes as above (tag TFX2), then 80..87 the 8 tracks' reverb sends, 88..95 their
 * delay sends (pots 22 / 23 of each voice). The total is TNP_SIZE(npat) + TNP_FX2_LEN. SAVE AS writes this one; TFX1 and no trailer still load
 * (the track sends then start at 0). */
#define TNP_FX2_TAG "TFX2"
#define TNP_FX2_LEN 96u
/* wip289: "TFX3" = TFX2 + the tracks' shelving cuts: 96..103 the 8 tracks' low cut (0 = off), 104..111 their high cut as STORED (127 - the knob: 0 = off).
 * Total TNP_SIZE(npat) + TNP_FX3_LEN. SAVE AS writes this one; TFX2, TFX1 and no trailer still load (no shelving cut then). */
#define TNP_FX3_TAG "TFX3"
#define TNP_FX3_LEN 112u
