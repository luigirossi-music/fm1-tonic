/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X user interface v2 (docs/plans/2026-10-03-ui.md): panel input, screens, the knob strip,
 * lists, questions, the readout, and the key lights.
 *
 * Grammar: ALGORITHM = part, PRESETS = pattern, SELECT = move (page / list row), KNOB 1-4 =
 * the four values in the knob strip (on every screen), SEL = open / run / yes, HOME = back.
 *
 * The screen is four bands (header, main top, main bottom, knob strip), each drawn into
 * Felucca's canvas and blitted only when its pixels changed: SPI time is the scarce thing.
 *
 * Keys: F3..G5; the 16 white keys are steps (HOME: patterns), the 11 black keys a drum
 * machine's tracks or the break's slice pads . */
#include "x0x.h"
#include "../dsp/fastmath.h"
#include "../dsp/master.h"
#include "tonic_bank.h"                     /* tools/gen_preset_bank.py: the factory presets */
#include "undo.c"

/* ================================================================ model === */
static const int8_t KEY_WHITE[NKEYS] = {0, -1, 1, -1, 2, -1, 3, 4, -1, 5, -1, 6, 7, -1, 8, -1, 9, -1, 10, 11, -1, 12, -1, 13, 14, -1, 15};
static const int8_t KEY_BLACK[NKEYS] = {-1, 0, -1, 1, -1, 2, -1, -1, 3, -1, 4, -1, -1, 5, -1, 6, -1, 7, -1, -1, 8, -1, 9, -1, -1, 10, -1};
static uint8_t WHITE_KEY[16], BLACK_KEY[11];

enum { V_HOME, V_PART, V_FX, V_MIX, V_GLO, V_SONG, V_PERF, V_SEQ, V_MIX2, V_MIX1, V_GLOB, V_PMOD, V_OSC, V_NOISE, V_NEV, V_VEL, NVIEWS };
static const char *const VIEW_N[NVIEWS] = {"HOME", "", "FX", "MIX", "GLOBAL", "SONG", "PERF", "SEQ", "MIX PAGE 2", "MIX", "GLOBAL PARAMETERS", "PITCH MOD", "OSCILLATOR", "NOISE FILTER", "NOISE ENVELOPE", "VEL"};
static const uint16_t PART_COL[NPARTS] = {RGB(169, 219, 241), RGB(120, 190, 225), RGB(190, 200, 208)};   /* TONIC: blues and greys */
static const char *const PART_N[NPARTS] = {"TONIC", "808", "BREAK"};
static const char *const PART_S[NPARTS] = {"TNC", "808", "BRK"};   /* 4 chars: lane labels */

/* overlays drawn over the main area */
enum { O_NONE, O_LIST, O_ASK, O_MEM };   /* O_MEM (wip267): GLO 3rd press, the project memory as a vertical battery */
enum { ACT_NONE, ACT_SAVE, ACT_CLEAR_PAT, ACT_RESET, ACT_ABOUT, ACT_SONG_INS,
       ACT_SONG_DEL, ACT_SONG_CLR, ACT_PERF, ACT_PERF_TEST, ACT_PRESET, ACT_SAVEAS, ACT_CHAIN, ACT_SONGMODE, ACT_SONGEXIT,
       ACT_CLEAR_TNPAT, ACT_CLEAR_TNTRK, ACT_PRESETGO, ACT_STOPFIRST, ACT_QSAVE, ACT_SONGKILL, ACT_SONGBAR, ACT_AUTODEL, ACT_PASTE_TNPAT, ACT_CLEAR_ALL };   /* ACT_CLEAR_ALL (wip632): FUNC + PLAY held 3 s: CLEAR ALL PATTERNS? */   /* ACT_PASTE_TNPAT (wip416): FUNC + FILL on the pattern: PASTE PATTERN? yes / no */   /* ACT_AUTODEL (wip367): FUNC + EDIT let go: DELETE AUTOMATION? */   /* ACT_SONGBAR (wip357): FILL on the SONG screen: DELETE BAR? */   /* ACT_SONGKILL (wip314): FUNC + ARP: DELETE SONG? */   /* ACT_QSAVE (wip265): FUNC + SAVE on a used place: OVERWRITE PROJECT? */   /* ACT_PRESETGO: a preset chosen while playing: change it (the playing stops)? */   /* the last two: FUNC + PLAY on the Tonic (the pattern / one track) */
#define CH_MAX 64                     /* the CHAIN screen: patterns in a chain at most */

typedef struct {
    uint8_t view, part, prev_view;
    uint8_t sel[NKIT];
    uint8_t page[NVIEWS][NPARTS];
    uint8_t spage;                     /* the 16 steps the white keys show: 0 = 1-16 .. 3 = 49-64 */
    uint8_t pbank;                     /* the 16 patterns HOME's white keys choose: 0 = 1-16, 1 = 17-32 */
    uint8_t mixsel;                    /* MIX's PARTS: the part the knobs set (black keys 1-5) */
    int8_t oct;
    uint8_t rec;
    int8_t held_step;
    uint8_t step_edited;
    uint32_t step_t0;                  /* when the held 303 step went down: a tap toggles, a hold does not */
    uint8_t step_preview;              /* the held step is sounding (only while stopped) */
    uint8_t step_created;              /* the press turned the held step on (its release keeps it) */
    int8_t brk_held;                   /* BREAK: the white key (16th) held, -1 none */
    int8_t hold_step;                  /* a drum machine: the step (white key) held, -1 none */
    uint8_t hold_created, hold_used;   /* drums / BREAK: the press turned the step on; a knob or a slice used it */
    uint8_t hold_mod;                  /* Tonic: ACCENT / FILL was used on the held step (its release keeps the step) */
    uint8_t preset_cur;                /* the preset loaded last + 1: 1-8 factory, 9-40 user slot 1-32 (0 = none since power on) */
    uint8_t preset_pend;               /* a preset waiting for the sequencer to stop (id + 1), 0 = none */
    uint8_t sv_fixed;                  /* wip265: SAVE AS opened by FUNC + SAVE in an empty place: the slot is the loaded one, no slot screen */
    uint8_t sv_rename;                 /* wip317: FUNC + HOME = RENAME PROJECT: the name screen alone, saved into the loaded place (1-8 included) */
    char sv_name[10];                  /* SAVE AS: the name (8 letters at most) */
    uint8_t nm_len, nm_ch;             /* wip324: MANUAL names: the letters accepted so far, and the blinking letter's place in NM_SET */
    char sv_occ[TNP_NAME_LEN + 1];     /* ... and who sits in the chosen slot now ("" = empty) */
    uint8_t sv_slot;                   /* ... the chosen slot 0-31 */
    uint8_t sv_here;                   /* wip319: SAVE HERE?: the place it will save into (1-40; ALGORITHM changes it, starts as the loaded place) */
    char sv_pn[TNP_NAME_LEN + 1];      /* ... and the project's name = the loaded place's name (what is saved) */
    uint8_t sv_newname;                /* wip708: SAVE HERE?: ALGORITHM made a new name (sv_pn) */
    uint8_t ch_n, ch_pat[CH_MAX];      /* CHAIN screen: the patterns chosen so far (0-15) */
    uint8_t songmode;                  /* Tonic: SONG MODE on (FUNC + SEQ, confirmed): PLAY plays the song; not saved */
    int8_t clip;                       /* Tonic: the pattern FUNC + ACCENT copied (0-31), -1 = none yet; not saved */
    uint8_t ca_hold;                   /* wip632: FUNC + PLAY just asked CLEAR PATTERN?: still held after 3 s it becomes CLEAR ALL PATTERNS? */
    uint8_t mute_pend;                 /* Tonic: FUNC + black key 1-8 held (bit = track): the mute happens when it is let go, */
                                       /* unless the key was used with PLAY / ACCENT / FILL meanwhile */
    uint8_t pst_pg;                    /* Tonic: ... a page key (EDIT / LFO) was held: the release pastes the copied page */
    uint8_t pst_trk;                   /* Tonic: FUNC + FILL went down with black key n held (n + 1): the release pastes that track's sound, 0 = the pattern */
    int8_t tc_src;                     /* Tonic: the track FUNC + black key + ACCENT copied (0-7), -1 none yet; not saved */
    uint8_t tc_pot[22], tc_mute;       /* ... its pots (the 22 knobs, pan last) and its mute */
    uint16_t tc16[21];                 /* ... and the exact 16-bit values */
    int8_t pc_view, pc_page;           /* Tonic: the page FUNC + EDIT + ACCENT copied (the screen and its page), pc_view -1 none yet; not saved */
    uint8_t pc_i[4], pc_pot[4];        /* ... its four knobs: the parameter number (255 = no knob) and the pot */
    uint16_t pc16[4];
    uint8_t pst_sh;                    /* Tonic: FUNC + FILL (black key 10) went down: 1 = a paste waits for FILL's release (or a white key), 2 = a white key used it */
    uint8_t cpexit;                    /* wip422: SEQ was pressed in the COPY-PASTE MODE: when it is let go it goes to the SEQUENCER page (never the pattern page) */
    uint8_t lowbatt;                   /* wip605: the LOW BATTERY screen is up (the battery fell to stage 1): HOME leaves it */
    uint8_t cpm;                       /* wip362: COPY-PASTE MODE (FUNC + ACCENT): the white keys blink, a white key pastes the copy there; any button leaves */
    uint8_t cpf;                       /* wip377: FUNC went down inside the COPY-PASTE MODE and nothing else was pressed since (a tap keeps the mode on) */
    uint16_t cpdone;                   /* wip365: the keys already pasted into in this mode: they stop blinking (steady) */
    int8_t cpsrc;                      /* wip362: the key (0-15 of the bank shown) chosen as the source with FUNC + ACCENT + white key, -1 = the playing pattern */
    int8_t blink_pat;                  /* Tonic: the slot just pasted into (0-15 of the bank shown), -1 none: its key blinks fast */
    uint32_t blink_until;
    uint8_t mod_held;                  /* Tonic: black key 9 (bit 0, ACCENT), 10 (bit 1, FILL) and 11 (bit 2, FUNC) held now */
    uint8_t adel;                      /* wip367: EDIT pressed with FUNC held, nothing else used yet: letting EDIT go asks DELETE AUTOMATION? */
    uint8_t fdel;                      /* wip356: FILL pressed on the SONG screen, nothing else used yet: letting it go deletes the bar */
    uint8_t chk;                       /* wip328: the CHOKE screen (EDIT, K2) is shown; not saved */
    uint8_t msp, msmode;               /* wip271: the MUTE / SOLO page (PERFORM, FX again): msp = shown, msmode 0 = MUTE (FUNC lit), 1 = SOLO (FILL lit); not saved */
    uint32_t hold_t0;                  /* when it went down (a tap takes an old step off) */
    uint16_t brk_pinkeys;              /* BREAK: black keys used to give a held step its slice (their release is not live) */
    uint32_t btn, keys, btn_used;
    uint32_t enc_t[NE];
    uint8_t enc_run[NE];               /* quick detents in a row (acceleration waits for a spin) */
    char msg[96];                      /* wip340: was 28 (a two-part warning: the big line, a newline, the small lines) */
    uint32_t msg_until;
    uint8_t fx_mem;                    /* wip200: the last page of the FX button's group: 0 MIX (V_MIX1), 1 an effects page (V_FX): FX comes back to it */
    uint8_t msg_warn;                  /* wip199: the message is a warning (something did not happen): it takes the whole screen */
    int8_t touched;                    /* knob cell being turned (4 = tempo), -1 none */
    uint32_t touch_until;
    uint8_t overlay;
    uint8_t list_sel, list_top, list_n;
    uint8_t ask_act;
    uint8_t seqpat;                    /* Tonic: SEQ pressed a 2nd time: a black screen with the pattern number, white keys 1-16 = the next pattern */
    uint16_t ask_arg;
    char ask_q[2][32];
    uint32_t frame;
    uint32_t dirty;
    uint32_t act_t;                    /* the last time anything was touched (autosave waits for quiet) */
    uint32_t saved_t;                  /* the last autosave */
    uint8_t last_autosave;             /* AUTOSAVE was just switched off: one more, which keeps that */
    uint8_t set_dirty;                 /* wip321: a setting of the GLO settings list changed: it is kept even with AUTOSAVE off (settings alone, stopped, after a short quiet) */
    uint8_t help;                      /* the help card on screen: its button + 1, 0 none */
    uint32_t down_t[NB];               /* when each button went down */
    uint32_t turn_t;                   /* the last encoder turn */
    uint8_t outline[232];              /* the break loop's outline */
    uint8_t outline_ok, outline_slot;
    uint8_t song_sel;                  /* the SONG screen's bar */
    uint8_t ac;                        /* wip287: the SEQ page's voice selector (ALGO) is on AC: the white keys put trigs on the global accent */
    uint8_t arp;                       /* wip251: the SEQ view is the ARP page (the keys are a piano) */
    int8_t arp_oct;                    /* the ARP keyboard's octave, -3..+3 */
    uint8_t dck;                       /* wip350: the DUCK screen (SENDS, ALGORITHM right) is open */
    uint8_t unq;                       /* wip292: FUNC + REC: 0 = QUANTIZED REC (default), 1 = UNQUANTIZED REC */
} ui_t;
static ui_t ui;
static uint8_t any_button;             /* a button has been pressed since power-on (the footer's first line) */
#define FOOT_GAP 7                     /* between the footer's parts */
static uint16_t footer_text(const char **a, const char **b, const char **c);

/* something the project keeps changed: SAVE (or the autosave) has work to do */
static void mark_dirty(void) { ui.dirty = 1; }

/* the encoders, read for the UI: a turn counts as being touched (the autosave waits for quiet) */
static int32_t enc(int role)
{
    int32_t v = plat_enc(role);
    if (v)
        ui.act_t = ui.turn_t = plat_ms();
    return v;
}

/* PERF (the performance screen and its test; see "performance" below) */
enum { PT_IDLE = -1, PT_DONE = 3 };
static struct {
    uint32_t t_ms, ticks, cyc, frames, prof[ENG_PROF_N], st[3];
    uint16_t mhz10;                    /* the CPU clock x 10; 0 = not known (no cycle counter) */
    uint8_t load, peak, stalls_on, have;
    uint16_t stage[ENG_PROF_N];        /* each stage's share of the CPU, x 10 (%) */
    uint16_t cps;                      /* the render's cycles per sample (cycle counter only) */
    uint8_t stall[3];                  /* fetch / read / write stall cycles, % */
    int8_t test, phase;                /* test: PT_IDLE, a scenario 0..2, PT_DONE (when phase is 4);
                                        * phase: 0 stop, 1 settle, 2 measure, 3 put back, 4 done */
    uint32_t test_t0;
    uint16_t res_load[3], res_cps[3];  /* per scenario: the render's share x 10, cycles per sample */
    uint8_t res_peak[3];
    pattern_t *pat;                    /* what the test changes, to put back */
    uint8_t ppat[NPARTS], song_on, ca, cb;
    uint32_t mute;
    float bpm;
} perf = {.test = PT_IDLE};

static pattern_t *pat_of(int part) { return &proj.pat[seq.ppat[part]]; }
static pattern_t *cur_pat(void) { return pat_of(ui.part); }    /* the pattern the part on screen plays */
static int part_view(void) { return ui.view == V_PART || ui.view == V_SEQ; }
static int steps_view(void) { return ui.view == V_PART || ui.view == V_SEQ; }   /* the keys are the part's steps */
static song_t *song(void) { return &proj.arr.song; }
static int is_drum(void) { return ui.part == PART_909; }
static void sp_step(int s, int accent);       /* a step put on / off, to a screen reader (accent: 1 total, 2 own, 3 fill) */

/* ============================================================ text helpers === */
static char *put_s(char *b, const char *s)
{
    while (*s)
        *b++ = *s++;
    *b = 0;
    return b;
}
static char *put_i(char *b, int v)
{
    char t[12];
    int n = 0, neg = v < 0;
    uint32_t u = (uint32_t)(neg ? -v : v);
    do {
        t[n++] = (char)('0' + u % 10u);
        u /= 10u;
    } while (u);
    if (neg)
        *b++ = '-';
    while (n)
        *b++ = t[--n];
    *b = 0;
    return b;
}
static const char *const NOTE_N[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static char *put_note(char *b, int n)
{
    b = put_s(b, NOTE_N[(n % 12 + 12) % 12]);
    return put_i(b, n / 12 - 1);
}
static char *put_hex(char *b, uint32_t v, int digits)
{
    int i;
    for (i = digits - 1; i >= 0; i--)
        *b++ = "0123456789ABCDEF"[(v >> (4 * i)) & 15u];
    *b = 0;
    return b;
}

static uint8_t pc_on, pc_step;                    /* wip433: the TRIG % shown big on the whole screen (full_kind 4): on, the step (1-based) */
static char pc_big[12];                           /* ... and its value ('75%', '1:2') */
static void say_as(const char *a, const char *b, uint32_t flags)
{
    char *q = put_s(ui.msg, a);
    pc_on = 0;                                    /* any other message ends it */
    if (b)
        put_s(q, b);
    ui.msg_warn = (flags & ANN_KEEP) ? 1u : 0u;
    ui.msg_until = plat_ms() + (ui.msg_warn ? 1800u : 1300u);   /* (a whole-screen warning stays a little longer: it has to be read) */
    ann_say(a, b, flags);                         /* and to a screen reader (announce.c) */
}
static void say(const char *a, const char *b) { say_as(a, b, ANN_NOW); }
/* something did not happen: a screen reader says it whole (what follows does not cut it off) */
static void warn(const char *a, const char *b) { say_as(a, b, ANN_NOW | ANN_KEEP); }

/* =============================================================== param refs === */
enum { R_NONE, R_ENG, R_SWING, R_DLEN, R_DRATE, R_BRKSET,
       R_BRKSLOT, R_TEMPO, R_ACCENT, R_CLKOUT, R_NOTEOUT, R_PALETTE, R_KEYLED, R_KEYSOUND, R_AUTOSAVE, R_ACT, R_SBAR, R_SPAT,
       R_SLEN, R_STALLS, R_HDR, R_TAPE, R_MIDICH, R_OUTLOUD, R_SREP, R_BATT, R_USBFULL, R_CORES,
       R_QUANT, R_FILLR, R_ZOOM, R_FIXLEN, R_SCROLL, R_FXALL, R_BEAT, R_ARPO, R_SHLO, R_SHHI, R_CLASSIC, R_NAMEMODE, R_SCINT, R_SCIN, R_REMCH, R_CCMODE, R_CCBASE, R_GACC, R_SWMODE };   /* R_SHLO / R_SHHI (wip289): the track's low / high shelving cut (a = the track) */   /* R_ARPO (wip251): the ARP page's keyboard octave */   /* wip74, GLOBAL PARAMETERS: R_QUANT = the pattern's step grid (slider position 1/8 .. 1/32), R_FILLR = FILL RATE (2x 4x 8x) */   /* R_TAPE: FX's one-knob TAPE (DIGI .. worn tape) */          /* R_HDR: a list's section header (a = its group), never selected */
typedef struct { uint8_t kind, a, b, c; } pref_t;
#define PR(k, a, b, c) ((pref_t){(k), (a), (b), (c)})
#define NONE PR(R_NONE, 0, 0, 0)

static const char *const RATE_N[] = {"1/16", "1/16T", "1/32", "1/8T", "1/8"};
static const char *const QUANT_N[] = {"1/8", "1/8T", "1/16", "1/16T", "1/32"};   /* the slider's order (musical) */
static const uint8_t QUANT_RATE[5] = {RATE_8, RATE_8T, RATE_16, RATE_16T, RATE_32};   /* ... and the rate of each position */
static const char *const FILL_N[] = {"2x", "4x", "8x"};
static const uint8_t FILL_CODE[3] = {1, 0, 2};                                       /* pattern_t.fill of each position */
static const x0x_param_t QUANT_P = {"QUANTIZATION", 4, 2, QUANT_N};                    /* default 1/16 */
static const x0x_param_t FILLR_P = {"FILL RATE", 2, 1, FILL_N};                        /* default 4x */
static const char *const DIR_N[] = {"FWD", "REV", "PING", "RND"};
static const char *const ONOFF_N[] = {"OFF", "ON"};
static const char *const FIXLEN_N[] = {"8", "16", "32", "64"};
static const x0x_param_t FIXLEN_P = {"FIXED LEN", 3, 1, FIXLEN_N};   /* SEQ K1: 8 / 16 / 32 / 64 steps, the pattern is copied into the new half(s) */
static const x0x_param_t FXALL_P = {"ALL FX", 1, 1, ONOFF_N};       /* PERFORM K4: right = all effects active, left = all off (dry signal). Not saved: ON at every switch-on */
static uint8_t fx_all_off;
static const char *const ARPO_N[] = {"-3", "-2", "-1", "0", "+1", "+2", "+3"};
static const x0x_param_t SHLO_P = {"LOW CUT", 127, 0, 0};       /* wip289: MIX 2 K3: 0 = 20 Hz (no OFF, wip318) .. 127 = 663 Hz */
static const x0x_param_t SHHI_P = {"HIGH CUT", 127, 127, 0};     /* wip289: MIX 2 K4: 127 = 20 kHz (no OFF, wip318) .. 0 = 1 kHz */
static const x0x_param_t SCINT_P = {"SIDECHAIN THRESHOLD", 127, 0, 0};    /* wip349, SENDS K3: the sidechain's depth, 0 = no ducking (shown 0-100 %) */
static const x0x_param_t SCIN_P = {"SIDECHAIN INPUT", 7, 0, 0};          /* wip349, SENDS K4: the track whose sound ducks the others (shown 1-8) */
static const x0x_param_t ARPO_P = {"OCTAVE", 6, 3, ARPO_N};         /* ARP K1: the keyboard's octave (value 3 = 0: the key C sounds the voice's own pitch) */
static const x0x_param_t SCROLL_P = {"AUTOSCRL", 1, 0, ONOFF_N};      /* SEQ K2: the step page follows the playhead */
static const char *const YESNO_N[] = {"NO", "YES"};
static const x0x_param_t BEAT_P = {"4/4 LED", 1, 0, ONOFF_N};     /* GLO: the faint 4 / 4 beat lights on white keys 1 5 9 13 (default OFF since wip234) */
static const char *const NAMEMODE_N[] = {"GENERATE", "MANUAL"};
static const x0x_param_t NAMEMODE_P = {"NAME PROJECTS", 1, 0, NAMEMODE_N};   /* wip324: GLO > PROJECT: GENERATE = the name generator (default), MANUAL = letter by letter (K4 cycles, OCT+ accepts) */
static const x0x_param_t ZOOM_P = {"ZOOM KNOB", 1, 1, YESNO_N};   /* GLO: a turned knob shows the zoom (default YES) */
static const char *const ROOT_N[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char *const THEME_N[] = {"GREEN", "AMBER", "CYAN", "RED", "MONO", "TONIC", "GRIGIOTONIC"};
enum { SQ_SWING, SQ_DLEN, SQ_DRATE, SQ_BLEN, SQ_BRATE, SQ_BDIR, SQ_BTRANS, SQ_TEMPO, SQ_ACCENT, SQ_CLK, SQ_NOTES,
       SQ_THEME, SQ_KEYLED, SQ_KEYSOUND, SQ_AUTOSAVE, NSQ };
static const char *const KEYSOUND_N[] = {"STOPPED", "ALWAYS"};
static const char *const LIGHTS_N[] = {"OFF", "ON", "KEYS"};   /* ON: the keys, and the unlit buttons glow */
static const x0x_param_t SEQ_P[NSQ] = {
    {"SWING", 100, 0, 0}, {"LENGTH", NSTEPS - 1, 15, 0}, {"RATE", 4, 0, RATE_N}, {"LENGTH", NSTEPS - 1, 15, 0},
    {"RATE", 4, 0, RATE_N}, {"DIR", 3, 0, DIR_N}, {"TRANSP", 48, 24, 0}, {"TEMPO", 255, 105, 0},
    {"ACCENT", 127, 88, 0}, {"CLOCK OUT", 1, 1, ONOFF_N}, {"NOTES OUT", 1, 0, ONOFF_N}, {"THEME", 6, 6, THEME_N},
    {"LIGHTS", 2, 1, LIGHTS_N}, {"KEY SOUND", 1, 0, KEYSOUND_N},
    {"AUTOSAVE", 1, 1, ONOFF_N},
};
static const x0x_param_t ACT_P[] = {   /* wip301: the order MUST be the ACT_ enum's (it had two stale entries left from the old engines, so the PERFORMANCE screen showed as DELETE BAR) */
    {"", 0, 0, 0}, {"SAVE PROJECT", 0, 0, 0}, {"CLEAR PATTERN", 0, 0, 0}, {"FACTORY RESET", 0, 0, 0},
    {"ABOUT FM1 TONIC", 0, 0, 0}, {"INSERT BAR", 0, 0, 0}, {"DELETE BAR", 0, 0, 0}, {"CLEAR SONG", 0, 0, 0},
    {"PERFORMANCE", 0, 0, 0}, {"RUN PERF TEST", 0, 0, 0},
};
static const x0x_param_t STALLS_P = {"STALLS", 1, 0, ONOFF_N};
static const x0x_param_t REMCH_P = {"REMOTE MIDI CH", 16, 0, 0};   /* wip385: 0 = OFF, 1-16 (proj.set.midi_ch[4]) */
static const char *const CCMODE_N[] = {"OFF", "SELECTED", "CHANNELS", "BOTH"};
static const x0x_param_t CCMODE_P = {"CC MODE", 3, 0, CCMODE_N};   /* wip386: OFF (default) / SELECTED TRACK (CC on the Tonic channel) / CHANNELS (channel = track) / BOTH (proj.set.cc_mode) */
static const char *const SWMODE_N[] = {"GLOBAL", "PER PATTERN"};
static const x0x_param_t SWMODE_P = {"SWING", 1, 0, SWMODE_N};   /* wip700: GLO > SWING: GLOBAL (default, also older projects) = one swing for every pattern, PER PATTERN = each pattern has its own */
static const x0x_param_t GACC_P = {"GLOBAL ACCENT", 9, 2, 0};   /* wip616: +1 .. +10 (index 0-9; +3 = 30 % default, +10 = full) of the total accent row's boost */
static const x0x_param_t CCBASE_P = {"CC CH BASE", 8, 0, 0};          /* wip386: the first channel of the 8-channel block, 1-9 (proj.set.cc_base 0-8) */
static const x0x_param_t MIDICH_P[NPARTS] = {   /* 0 = OFF (no notes in or out), 1-16 */
    {"TONIC MIDI CH", 16, 10, 0}, {"808 MIDI CH", 16, 11, 0}, {"BREAK MIDI CH", 16, 4, 0}};
static const char *const OUT_N[] = {"NORMAL", "LOUD"};
static const x0x_param_t OUT_P = {"OUTPUT", 1, 0, OUT_N};
static const char *const USBA_N[] = {"FOLLOWS KNOB", "FULL"};
static const x0x_param_t USBA_P = {"USB AUDIO", 1, 0, USBA_N};
static const char *const ENGMODE_N[] = {"ENHANCED", "CLASSIC"};
static const x0x_param_t ENGMODE_P = {"TONIC ENGINE", 1, 0, ENGMODE_N};   /* wip302: GLO > SYSTEM: ENHANCED (default) / CLASSIC (= what Microtonic can read) */
static const char *const CORES_N[] = {"ON", "OFF"};
static const x0x_param_t CORES_P = {"2ND CORE", 1, 0, CORES_N};   /* proj.set.one_core: at the next power-on */
static const x0x_param_t SREP_P = {"REPEAT", 16, 0, 0};   /* the bar plays 1-16 times; 16 = LOOP (wip608; 1.8f2: 16 repeats, was 8) */
static const char *const BATT_N[] = {"EMPTY", "1/3", "2/3", "FULL", "USB POWER", "..."};
static const x0x_param_t BATT_P = {"BATTERY", 5, 0, BATT_N};   /* read only (plat_battery) */
static const x0x_param_t SONG_P[] = {
    {"TONIC", NPAT - 1, 0, 0}, {"808", NPAT - 1, 0, 0}, {"303A", NPAT - 1, 0, 0}, {"303B", NPAT - 1, 0, 0},
    {"BREAK", NPAT - 1, 0, 0},
    {"LENGTH", NSONG, 0, 0},
};

/* the song's bar k exists from here on: a new bar repeats the one before (the first: what plays) */
static void song_fill(int k)
{
    song_t *sg = song();
    while (sg->len <= k && sg->len < NSONG) {
        song_bar_t *b = &sg->bar[sg->len];
        int p;
        if (sg->len)
            *b = sg->bar[sg->len - 1];
        else {
            for (p = 0; p < NPARTS; p++)
                b->pat[p] = seq.ppat[p];
            b->mute = 0;
        }
        if (SONG_LOOP(b))                                  /* wip608: a copy of a LOOP bar is a plain bar, played once */
            b->mute &= SONG_MUTE_MASK;
        sg->len++;
    }
}

/* wip608: OCT+ on the SONG screen: a new line right after the cursor, the same patterns as the line before it; on END it is added at the end */
static void song_add_cur(void)
{
    song_t *sg = song();
    int k = ui.song_sel, i;
    if (sg->len >= NSONG) {
        warn("SONG FULL", 0);
        return;
    }
    if (k >= sg->len || !sg->len) {
        song_fill(sg->len);
        ui.song_sel = (uint8_t)(sg->len - 1);
    } else {
        for (i = sg->len; i > k + 1; i--)
            sg->bar[i] = sg->bar[i - 1];
        sg->bar[k + 1] = sg->bar[k];
        if (SONG_LOOP(&sg->bar[k + 1]))
            sg->bar[k + 1].mute &= SONG_MUTE_MASK;
        sg->len++;
        ui.song_sel = (uint8_t)(k + 1);
    }
    mark_dirty();
    say("LINE ADDED", 0);
}

static void songexit_go(void);                        /* wip360: below */
/* wip356: FILL on the SONG screen: delete the bar on the cursor (the cursor past the end, on END: the last bar) */
static void song_del_cur(void)
{
    song_t *sg = song();
    int k = ui.song_sel, i;
    if (!sg->len)
        return;
    if (k >= sg->len)
        k = sg->len - 1;
    for (i = k; i + 1 < sg->len; i++)
        sg->bar[i] = sg->bar[i + 1];
    sg->len--;
    ui.song_sel = (uint8_t)k;
    mark_dirty();
    if (!sg->len) {                                   /* wip360: the last bar is gone = the song is deleted: SONG OFF, song mode left */
        ui.song_sel = 0;
        seq.chain_on = 0;
        songexit_go();
        say("SONG DELETED", 0);
        return;
    }
}

static const x0x_param_t *pref_desc(pref_t r)
{
    switch (r.kind) {
    case R_ENG: return engine_param(r.a, r.b, r.c);
    case R_BRKSET: return breaks_param(r.c);
    case R_SWING: return &SEQ_P[SQ_SWING];
    case R_QUANT: return &QUANT_P;
    case R_FILLR: return &FILLR_P;
    case R_DLEN: return &SEQ_P[SQ_DLEN];
    case R_DRATE: return &SEQ_P[SQ_DRATE];
    case R_BRKSLOT: {
        static x0x_param_t d = {"LOOP", 0, 0, 0};
        d.max = (uint8_t)(engine_brk_nslots() - 1);
        d.names = engine_brk_slot_names();
        return &d;
    }
    case R_TEMPO: return &SEQ_P[SQ_TEMPO];
    case R_ACCENT: return &SEQ_P[SQ_ACCENT];
    case R_CLKOUT: return &SEQ_P[SQ_CLK];
    case R_NOTEOUT: return &SEQ_P[SQ_NOTES];
    case R_PALETTE: return &SEQ_P[SQ_THEME];
    case R_KEYLED: return &SEQ_P[SQ_KEYLED];
    case R_KEYSOUND: return &SEQ_P[SQ_KEYSOUND];
    case R_AUTOSAVE: return &SEQ_P[SQ_AUTOSAVE];
    case R_NAMEMODE: return &NAMEMODE_P;
    case R_ACT: return &ACT_P[r.a];
    case R_SBAR: {
        static x0x_param_t d = {"BAR", 0, 0, 0};
        d.max = (uint8_t)(song()->len < NSONG ? song()->len : NSONG - 1);
        return &d;
    }
    case R_SPAT: return &SONG_P[r.a % NPARTS];
    case R_MIDICH: return &MIDICH_P[r.a % NPARTS];
    case R_REMCH: return &REMCH_P;
    case R_CCMODE: return &CCMODE_P;
    case R_CCBASE: return &CCBASE_P;
    case R_GACC: return &GACC_P;
    case R_SWMODE: return &SWMODE_P;
    case R_OUTLOUD: return &OUT_P;
    case R_USBFULL: return &USBA_P;
    case R_ZOOM: return &ZOOM_P;
    case R_BEAT: return &BEAT_P;
    case R_FIXLEN: return &FIXLEN_P;
    case R_SCROLL: return &SCROLL_P;
    case R_ARPO: return &ARPO_P;
    case R_SCINT: return &SCINT_P;
    case R_SCIN: return &SCIN_P;
    case R_SHLO: return &SHLO_P;
    case R_SHHI: return &SHHI_P;
    case R_FXALL: return &FXALL_P;
    case R_CORES: return &CORES_P;
    case R_CLASSIC: return &ENGMODE_P;
    case R_SREP: return &SREP_P;
    case R_BATT: return &BATT_P;
    case R_SLEN: return &SONG_P[5];
    case R_STALLS: return &STALLS_P;
    case R_TAPE: {
        static const x0x_param_t d = {"TAPE", 127, 72, 0};
        return &d;
    }
    default: return 0;
    }
}

static int pref_get(pref_t r)
{
    pattern_t *p = cur_pat();
    switch (r.kind) {
    case R_ENG: return proj.sound.v[r.a][r.b][r.c];
    case R_SWING: return pat_of(PART_909)->swing;
    case R_QUANT: {
        int i, rt = pat_of(PART_909)->drum[PART_909].rate;
        for (i = 0; i < 5; i++)
            if (QUANT_RATE[i] == rt)
                return i;
        return 2;                                      /* (anything else: 1/16) */
    }
    case R_FILLR: return pat_of(PART_909)->fill == 1 ? 0 : pat_of(PART_909)->fill == 2 ? 2 : 1;
    case R_DLEN: return pat_of(r.a)->drum[r.a].len - 1;
    case R_DRATE: return pat_of(r.a)->drum[r.a].rate;
    case R_BRKSET: return p->brk.set[r.c];
    case R_BRKSLOT: return r.a ? p->brk.slot_b : p->brk.slot_a;
    case R_TEMPO: return (int)(seq.bpm + 0.5f) - 20;
    case R_ACCENT: return seq.accent_q7;
    case R_CLKOUT: return seq.send_clock;
    case R_NOTEOUT: return seq.send_notes;
    case R_PALETTE: return proj.set.palette;
    case R_KEYLED: return proj.set.keyled;
    case R_KEYSOUND: return proj.set.keysound;
    case R_AUTOSAVE: return !proj.set.autosave_off;
    case R_NAMEMODE: return proj.set.name_manual;
    case R_SWMODE: return proj.set.swing_pp;
    case R_SBAR: return ui.song_sel;
    case R_SPAT: {
        const song_t *sg = song();
        int k = ui.song_sel < sg->len ? ui.song_sel : sg->len - 1;
        return k >= 0 ? sg->bar[k].pat[r.a] : seq.ppat[r.a];
    }
    case R_MIDICH: return seq.ch[r.a % NPARTS] == SEQ_CH_OFF ? 0 : seq.ch[r.a % NPARTS] + 1;
    case R_REMCH: return proj.set.midi_ch[4] >= 1 && proj.set.midi_ch[4] <= 16 ? proj.set.midi_ch[4] : 0;
    case R_CCMODE: return proj.set.cc_mode;
    case R_CCBASE: return proj.set.cc_base;
    case R_GACC: return (proj.set.gacc >= 1 && proj.set.gacc <= 10) ? proj.set.gacc - 1 : 2;   /* wip616: stored 1-10 = +1..+10, 0 (or an old wip612 value) = +3 */
    case R_OUTLOUD: return proj.set.out_loud;
    case R_USBFULL: return proj.set.usb_full;
    case R_ZOOM: return !proj.set.zoom_off;
    case R_BEAT: return !proj.set.beat_off;
    case R_SCROLL: return proj.set.autoscroll;
    case R_ARPO: return ui.arp_oct + 3;
    case R_SCINT: return 127 - proj.set.sc_int;                /* wip580: the knob runs like the compressor's THRESHOLD: all the way RIGHT = no ducking (inactive), turning left = more */
    case R_SCIN: return proj.set.sc_in;
    case R_SHLO: return proj.sound.v[T_909][SH_LO_ROW][r.a & 7];
    case R_SHHI: return 127 - proj.sound.v[T_909][SH_HI_ROW][r.a & 7];
    case R_FXALL: return !fx_all_off;
    case R_FIXLEN: { int n = pat_of(r.a)->drum[r.a].len; return n <= 8 ? 0 : n <= 16 ? 1 : n <= 32 ? 2 : 3; }   /* the block the length lives in */
    case R_CORES: return proj.set.one_core;
    case R_CLASSIC: return proj.set.classic;
    case R_SREP: return ui.song_sel < song()->len ? (SONG_LOOP(&song()->bar[ui.song_sel]) ? 16 : SONG_REP1(&song()->bar[ui.song_sel])) : 0;
    case R_BATT: return plat_battery() < 0 ? 5 : plat_battery();
    case R_SLEN: return song()->len;
    case R_STALLS: return perf.stalls_on;
    case R_TAPE: return proj.sound.v[T_FX][1][0];       /* (kept in an engine slot nothing reads) */
    default: return 0;
    }
}

/* SEQ K1: the pattern's block of steps 16 / 32 / 64. A bigger block copies what is there (the old block) into the new
 * steps (16 -> 32: 1-16 into 17-32; 32 -> 64: 1-32 into 33-64; 16 -> 64: 1-16 into every next block): the hits, accents, fills
 * and the p-locks / chances of the part. A smaller block only shortens the pattern (the steps stay in memory). */
static void fix_length(int part, int v)
{
    dpart_t *d = &pat_of(part)->drum[part];
    int old = d->len <= 8 ? 8 : d->len <= 16 ? 16 : d->len <= 32 ? 32 : 64, nw = 8 << v, dst, k, s, w;
    if (nw > old)
        for (dst = old; dst < nw; dst += old)
            for (s = 0; s < old; s++) {
                for (k = 0; k < NDRUM; k++) {
                    for (w = 0; w < 3; w++) {
                        uint32_t *rows[3] = {d->hit[k], d->vacc[k], d->vfill[k]};
                        if (sm_get(rows[w], s))
                            sm_set(rows[w], dst + s);
                        else
                            rows[w][(dst + s) >> 5] &= ~(1u << ((dst + s) & 31));
                    }
                }
                if (sm_get(d->accent, s))
                    sm_set(d->accent, dst + s);
                else
                    d->accent[(dst + s) >> 5] &= ~(1u << ((dst + s) & 31));
                if (part == PART_909 && seq.ppat[part] < NPAT_NOTE)   /* wip315: the ARP notes and the late hits go with the trigs */
                    for (k = 0; k < NTRK_NOTE; k++) {
                        int o = seq_late_get(&seq, seq.ppat[part], k, s);
                        proj.note[seq.ppat[part]][k][dst + s] = proj.note[seq.ppat[part]][k][s];
                        seq_late_set(&seq, seq.ppat[part], k, dst + s, o);
                    }
                for (k = 0; k < NLANE; k++) {
                    lane_t *l = &proj.arr.lane[k];
                    if (l->used && l->pat == seq.ppat[part] && l->part == part)
                        l->val[dst + s] = l->val[s];
                }
            }
    d->len = (uint8_t)nw;
}

/* wip302: CLASSIC engine: the knobs Microtonic does not have (the track's reverb send, delay send, low cut, high cut) are greyed and do not turn (wip354: the PAN is NOT greyed: it works in CLASSIC).
 * (Step 2: only these per-track extras; the master effects are still to be decided with him. The sound is not switched off yet: step 3.) */
static int extra_off(pref_t r)
{
    if (!proj.set.classic)
        return 0;
    if (r.kind == R_SHLO || r.kind == R_SHHI || r.kind == R_SCINT || r.kind == R_SCIN)   /* wip350: the sidechain is an extra too */
        return 1;
    return r.kind == R_ENG && r.a == T_909 && (r.c == TN_POT_RSND || r.c == TN_POT_DSND);
}

static void sc_send(void)                             /* wip350: the sidechain to the engine; CLASSIC engine = none (the values stay) */
{
    if (proj.set.classic)
        engine_tonic_sidechain(proj.set.sc_in, 0, 0);
    else
        engine_tonic_sidechain(proj.set.sc_in, proj.set.sc_int, proj.set.sc_mask);
}

static int sc_ducked(int v)                           /* wip351: track v (0-7) is ducked now (the red dot): intensity above 0, in the group, not the input, not CLASSIC */
{
    return !proj.set.classic && proj.set.sc_int > 0 && v != proj.set.sc_in && ((proj.set.sc_mask >> v) & 1u);
}

static void pref_set(pref_t r, int v)
{
    pattern_t *p = cur_pat();
    const x0x_param_t *d = pref_desc(r);
    if (!d || r.kind == R_ACT)
        return;
    if (extra_off(r)) {
        say("CLASSIC ENGINE", 0);
        return;
    }
    v = v < 0 ? 0 : v > d->max ? d->max : v;
    if (r.kind == R_FXALL) {                           /* a session switch: not part of the project, so no SAVE mark */
        fx_all_off = (uint8_t)!v;
        engine_fx_off(fx_all_off);
        return;
    }
    mark_dirty();
    switch (r.kind) {                                  /* wip321: the settings list (they live in proj.set): kept even when AUTOSAVE is off */
    case R_PALETTE: case R_KEYLED: case R_KEYSOUND: case R_AUTOSAVE: case R_MIDICH: case R_REMCH: case R_CCMODE: case R_CCBASE: case R_GACC: case R_SWMODE: case R_OUTLOUD: case R_USBFULL:
    case R_ZOOM: case R_BEAT: case R_CLASSIC: case R_NAMEMODE: case R_SCROLL: case R_CORES: case R_CLKOUT: case R_NOTEOUT:
        ui.set_dirty = 1;
        break;
    default:
        break;
    }
    switch (r.kind) {
    case R_ENG:
        proj.sound.v[r.a][r.b][r.c] = (uint8_t)v;
        engine_set(r.a, r.b, r.c, v);
        if (r.a == T_MST && r.c == MST_COMP1)          /* the macros: the details they set, for the list */
            master_comp1_pots(v, &proj.sound.v[T_MST][0][MST_THRESH], &proj.sound.v[T_MST][0][MST_RATIO],
                              &proj.sound.v[T_MST][0][MST_MAKEUP]);
        else if (r.a == T_MST && r.c == MST_FTYPE) {       /* a new filter type: the knob goes back to closed (no jump) */
            int closed = v == 2 ? 127 : 0;
            proj.sound.v[T_MST][0][MST_DJF] = (uint8_t)closed;
            engine_set(T_MST, 0, MST_DJF, closed);
            master_djf_pots(closed, v, &proj.sound.v[T_MST][0][MST_MODE], &proj.sound.v[T_MST][0][MST_CUTOFF]);
        } else if (r.a == T_MST && r.c == MST_DJF)
            master_djf_pots(v, proj.sound.v[T_MST][0][MST_FTYPE], &proj.sound.v[T_MST][0][MST_MODE],
                            &proj.sound.v[T_MST][0][MST_CUTOFF]);
        break;
    case R_SWING:
        pat_of(PART_909)->swing = (uint8_t)v;
        if (!proj.set.swing_pp) {                      /* wip700: SWING GLOBAL: every pattern gets it */
            int q;
            for (q = 0; q < NPAT; q++)
                proj.pat[q].swing = (uint8_t)v;
        }
        break;
    case R_SWMODE:
        proj.set.swing_pp = (uint8_t)v;
        if (!v) {                                      /* wip700: back to GLOBAL: the playing pattern's swing goes to all */
            int q;
            for (q = 0; q < NPAT; q++)
                proj.pat[q].swing = pat_of(PART_909)->swing;
        }
        break;
    case R_QUANT: pat_of(PART_909)->drum[PART_909].rate = QUANT_RATE[v]; break;
    case R_FILLR: pat_of(PART_909)->fill = FILL_CODE[v]; break;
    case R_DLEN: pat_of(r.a)->drum[r.a].len = (uint8_t)(v + 1); break;
    case R_DRATE: pat_of(r.a)->drum[r.a].rate = (uint8_t)v; break;
    case R_BRKSET: p->brk.set[r.c] = (uint8_t)v; break;
    case R_BRKSLOT:
        if (r.a)
            p->brk.slot_b = (uint8_t)v;
        else
            p->brk.slot_a = (uint8_t)v;
        engine_brk_loops();
        ui.outline_ok = 0;
        break;
    case R_TEMPO: seq.bpm = (float)(v + 20); break;
    case R_ACCENT: seq.accent_q7 = (uint8_t)v; break;
    case R_CLKOUT: seq.send_clock = (uint8_t)v; break;
    case R_NOTEOUT: seq.send_notes = (uint8_t)v; break;
    case R_PALETTE: proj.set.palette = (uint8_t)v; palette_set((uint32_t)v); break;
    case R_KEYLED: proj.set.keyled = (uint8_t)v; break;
    case R_KEYSOUND: proj.set.keysound = (uint8_t)v; break;
    case R_AUTOSAVE: proj.set.autosave_off = (uint8_t)!v; ui.last_autosave = (uint8_t)!v; break;
    case R_NAMEMODE: proj.set.name_manual = (uint8_t)v; break;
    case R_SBAR: ui.song_sel = (uint8_t)v; break;
    case R_SPAT:
        song_fill(ui.song_sel);
        song()->bar[ui.song_sel].pat[r.a] = (uint8_t)v;
        break;
    case R_MIDICH: seq.ch[r.a % NPARTS] = (uint8_t)(v ? v - 1 : SEQ_CH_OFF); break;
    case R_REMCH: proj.set.midi_ch[4] = (uint8_t)(v ? v : 17); engine_remote_ch(v ? v - 1 : -1); break;
    case R_CCMODE: proj.set.cc_mode = (uint8_t)v; engine_cc_cfg(proj.set.cc_mode, proj.set.cc_base); break;   /* wip386 */
    case R_CCBASE: proj.set.cc_base = (uint8_t)v; engine_cc_cfg(proj.set.cc_mode, proj.set.cc_base); break;
    case R_GACC: proj.set.gacc = (uint8_t)(v + 1); seq.gacc = (uint8_t)((v + 1) * 10); break;   /* wip616: +n = n x 10 % */
    case R_OUTLOUD: proj.set.out_loud = (uint8_t)v; engine_out_loud(v); break;
    case R_USBFULL: proj.set.usb_full = (uint8_t)v; engine_usb_full(v); break;
    case R_ZOOM: proj.set.zoom_off = (uint8_t)!v; break;
    case R_BEAT: proj.set.beat_off = (uint8_t)!v; break;
    case R_CLASSIC: proj.set.classic = (uint8_t)v; sc_send(); break;   /* wip350: classic = no sidechain in the sound */
    case R_SCROLL: proj.set.autoscroll = (uint8_t)v; break;
    case R_ARPO: ui.arp_oct = (int8_t)(v - 3); break;
    case R_SCINT: proj.set.sc_int = (uint8_t)(127 - v); sc_send(); break;   /* wip349; wip580: inverted (the stored intensity is unchanged: projects and presets stay compatible) */
    case R_SCIN: proj.set.sc_in = (uint8_t)v; sc_send(); break;
    case R_SHLO:                                       /* wip289: the shelving cuts: stored in the spare Tonic rows (the high one inverted), sent as the knob */
        proj.sound.v[T_909][SH_LO_ROW][r.a & 7] = (uint8_t)v;
        engine_set(T_909, r.a & 7, TN_POT_LOSH, v);
        break;
    case R_SHHI:
        proj.sound.v[T_909][SH_HI_ROW][r.a & 7] = (uint8_t)(127 - v);
        engine_set(T_909, r.a & 7, TN_POT_HISH, v);
        break;
    case R_FIXLEN: fix_length(r.a, v); break;
    case R_CORES:
        proj.set.one_core = (uint8_t)v;
        say("AT THE NEXT POWER-ON", 0);
        break;
    case R_SREP:
        if (ui.song_sel < NSONG) {
            song_bar_t *b;
            song_fill(ui.song_sel);
            b = &song()->bar[ui.song_sel];
            b->mute = (uint8_t)((b->mute & SONG_MUTE_MASK) | (v >= 16 ? 0x10u : (((unsigned)v & 7u) << 5) | (((unsigned)v >> 3 & 1u) << 3)));   /* wip608: 16 = LOOP (1.8f2: 0-15 = x1 .. x16) */
        }
        break;
    case R_TAPE: {                                    /* 0..15 the clean digital delay; above, tape, wearing */
        uint8_t ty = (uint8_t)(v >= 16), we = (uint8_t)(ty ? (v - 16) * 127 / 111 : proj.sound.v[T_FX][0][FX_DL_WEAR]);
        proj.sound.v[T_FX][1][0] = (uint8_t)v;
        proj.sound.v[T_FX][0][FX_DL_TYPE] = ty;
        proj.sound.v[T_FX][0][FX_DL_WEAR] = we;
        engine_set(T_FX, 0, FX_DL_TYPE, ty);
        engine_set(T_FX, 0, FX_DL_WEAR, we);
        break;
    }
    case R_STALLS:
        perf.stalls_on = (uint8_t)v;
        plat_stalls_enable(v);
        perf.have = 0;                                /* the next window starts the counts again */
        break;
    case R_SLEN:
        if (v > song()->len)
            song_fill(v - 1);
        else
            song()->len = (uint16_t)v;
        if (ui.song_sel > song()->len)
            ui.song_sel = (uint8_t)(song()->len < NSONG ? song()->len : NSONG - 1);
        break;
    default: break;
    }
}

/* a name in capitals (the engines' own are mixed case): two at a time */
static const char *caps_of(const char *n)
{
    static char t[2][24];
    static int w;
    char *q = t[w ^= 1];
    int i, j = 0;
    if (n[0] == 'N' && n[1] == 'F' && n[2] == 'M' && n[3] == 'o' && !n[6])
        return "FILTER MODE";                              /* (NOISE page names) */
    if (n[0] == 'N' && n[1] == 'F' && n[2] == 'Q' && !n[3])
        return "FILTER Q";
    if (n[0] == 'O' && n[1] == 's' && n[2] == 'c' && n[3] == 'A' && n[4] == 't' && n[5] == 'k' && !n[6])
        return "OSC ATTACK";                               /* (ENV page names, as the user writes them) */
    if (n[0] == 'O' && n[1] == 's' && n[2] == 'c' && n[3] == 'D' && n[4] == 'c' && n[5] == 'y' && !n[6])
        return "OSC DECAY";
    if (n[0] == 'V' && n[1] == 'e' && n[2] == 'l') {                       /* (VEL page names) */
        if (n[3] == 'O' && n[4] == 's' && n[5] == 'c' && !n[6])
            return "OSC";
        if (n[3] == 'N' && n[4] == 's' && !n[5])
            return "NOISE";
        if (n[3] == 'M' && n[4] == 'o' && n[5] == 'd' && !n[6])
            return "MOD";
    }
    if (n[0] == 'N' && n[1] == 'E' && n[2] == 'M' && n[3] == 'o' && !n[6])
        return "ENV MODE";                                 /* (NOISE ENVELOPE page names) */
    if (n[0] == 'N' && n[1] == 'E' && n[2] == 'A' && n[3] == 't' && n[4] == 'k' && !n[5])
        return "ENV ATTACK";
    if (n[0] == 'N' && n[1] == 'E' && n[2] == 'D' && n[3] == 'c' && n[4] == 'y' && !n[5])
        return "ENV DECAY";
    if (n[0] == 'M' && n[1] == 'o' && n[2] == 'd' && !n[6]) {              /* (SEL page names, as the user writes them) */
        if (n[3] == 'M' && n[4] == 'o' && n[5] == 'd')
            return "MOD MODE";
        if (n[3] == 'A' && n[4] == 'm' && n[5] == 't')
            return "MOD AMT";
    }
    if (n[0] == 'M' && n[1] == 'o' && n[2] == 'd' && n[3] == 'R' && n[4] == 'a' && n[5] == 't' && !n[6])
        return "MOD RATE";
    for (i = 0; n[i] && j < 22; i++) {
        if (i == 2 && n[0] == 'E' && n[1] == 'Q' && n[2] >= 'A' && n[2] <= 'Z')   /* "EQFreq" -> "EQ FREQ", "EQGain" -> "EQ GAIN" */
            q[j++] = ' ';
        q[j++] = n[i] >= 'a' && n[i] <= 'z' ? (char)(n[i] - 32) : n[i];
    }
    q[j] = 0;
    return q;
}

/* the value belongs to the pattern (it changes when the pattern does), not to the sound */
static int in_pattern(pref_t r)
{
    switch (r.kind) {
    case R_SWING: case R_DLEN: case R_DRATE: case R_QUANT: case R_FILLR: case R_FIXLEN:
    case R_BRKSET: case R_BRKSLOT:
        return 1;
    default:
        return 0;
    }
}

/* a knob's label: mixer refs name their part */
static const char *pref_name(pref_t r)
{
    static const char *const MIXN[NPARTS][MX_NPARAMS] = {{"TONIC", "TNC RV", "TNC DL", "TNC PAN"},
                                                         {"808", "808 RV", "808 DL", "808 PAN"},
                                                         {"BREAK", "BRK RV", "BRK DL", "BRK PAN"}};
    static const char *const PATN[6][2] = {{"TONIC LEN", "808 LEN"}, {"TONIC RATE", "808 RATE"}, {"A LEN", "B LEN"},
                                           {"A RATE", "B RATE"}, {"A DIR", "B DIR"}, {"A TRANSP", "B TRANSP"}};
    static const char *const CHN[MX_NPARAMS] = {"LEVEL", "REV", "DLY", "PAN"};
    const x0x_param_t *d = pref_desc(r);
    static const char *const SEQN[6] = {"FREE LEN", "RATE", "LENGTH", "RATE", "DIRECTION", "TRANSP"};
    if (r.kind == R_FIXLEN && ui.view == V_SEQ)
        return "FIXED LEN";
    if (r.kind >= R_DLEN && r.kind <= R_DRATE)       /* SEQ: the part's own; a list elsewhere: whose */
        return ui.view == V_SEQ ? SEQN[r.kind - R_DLEN] : PATN[r.kind - R_DLEN][r.a & 1];
    if (r.kind == R_ENG && r.a == T_MIX && ui.view == V_MIX && r.c < MX_NPARAMS)
        return CHN[r.c];                               /* MIX's PARTS: the picked part's */
    if (r.kind == R_ENG && r.a == T_909 && r.b < NDRUM && (r.c == TN_POT_RSND || r.c == TN_POT_DSND))
        return r.c == TN_POT_RSND ? "REVERB SEND" : "DELAY SEND";   /* wip242 */
    if (r.kind == R_ENG && r.a == T_909 && r.b == NDRUM && r.c == 1)
        return "ACCENT";                               /* the kits' depth of accents */
    if (ui.view == V_FX && r.kind == R_ENG && r.a == T_MIX && r.b == PART_909 && (r.c == MX_REV || r.c == MX_DLY))
        return r.c == MX_REV ? "REVERB D/W" : "DELAY D/W";   /* PERFORM: the Tonic's sends are the effects' dry / wet */
    if (ui.view == V_FX && r.kind == R_ENG && r.a == T_MST && r.c == MST_DJF)
        return "FILTER D/W";
    if (ui.view == V_FX && r.kind == R_ENG && r.a == T_MST && r.c == MST_FTYPE)
        return "FILTER TYPE";
    if (r.kind == R_ENG && r.a == T_MIX && r.b < NPARTS && r.c < MX_NPARAMS)
        return MIXN[r.b][r.c];
    if (r.kind == R_ENG && r.a == T_FX) {               /* the page says which effect: the knob, what */
        switch (r.c) {
        case FX_RV_DECAY: return "DECAY";
        case FX_RV_TONE: case FX_DL_TONE: return "TONE";
        case FX_DL_TIME: return "TIME";
        case FX_DL_FDBK: return "FEEDBACK";
        case FX_RV_HPF: return "REV LOW CUT";
        case FX_DL_HPF: return "DLY LOW CUT";
        case FX_RV_LEVEL: return "REV RETURN";
        case FX_DL_LEVEL: return "DLY RETURN";
        case FX_DL_PING: return "PING-PONG";
        case FX_COMP: return "GLUE";
        default: break;
        }
    }
    if (r.kind == R_BRKSET && r.c == BRK_BCHANCE)
        return "B CHANCE";
    if (r.kind == R_ENG && r.a == T_909 && r.b < NDRUM && ui.view == V_MIX && ui.overlay == O_LIST) {
        static char t[24];                             /* MIX's list: whose track ("909 BD LEVEL") */
        put_s(put_s(put_s(put_s(t, r.a == T_909 ? "TONIC " : "808 "), engine_voice_name(r.a, r.b)), " "), d ? d->name : "");
        return t;
    }
    if (r.kind == R_BRKSLOT)
        return r.a ? "LOOP B" : "LOOP A";
    return d ? d->name : "";
}

/* a pan: a part's (MIX) or a drum voice's ("Pan", its last pot): drawn L / C / R, its arc from the middle */
static int is_pan(pref_t r)
{
    const x0x_param_t *d;
    if (r.kind != R_ENG)
        return 0;
    if (r.a == T_MIX)
        return r.c == MX_PAN;
    d = pref_desc(r);
    return r.a == T_909 && d && d->name[0] == 'P' && d->name[1] == 'a' && d->name[2] == 'n' &&
           !d->name[3];
}

/* the Tonic's EQ FREQ: shown in Hz / kHz, 20 Hz - 20 kHz on the exponential scale of libpo32 (pot 0..127) */
static int is_oscfreq(pref_t r)                         /* the oscillator's Pitch, shown in Hz like the EQ Freq */
{
    const x0x_param_t *d = pref_desc(r);
    return r.kind == R_ENG && r.a == T_909 && d && d->name[0] == 'P' && d->name[1] == 'i' && d->name[2] == 't';
}
static int is_nffreq(pref_t r)                          /* the noise filter's freq, in Hz like the others */
{
    const x0x_param_t *d = pref_desc(r);
    return r.kind == R_ENG && r.a == T_909 && d && d->name[0] == 'N' && d->name[1] == 'F' && d->name[2] == 'F';
}
static int is_modamt(pref_t r)                          /* the pitch mod amount: -96.00 .. +96.00 semitones (libpo32: (x - 0.5) * 2 * 96) */
{
    const x0x_param_t *d = pref_desc(r);
    return r.kind == R_ENG && r.a == T_909 && d && d->name[0] == 'M' && d->name[1] == 'o' && d->name[2] == 'd' &&
           d->name[3] == 'A' && d->name[4] == 'm' && d->name[5] == 't';
}
static int is_eqfreq(pref_t r)
{
    const x0x_param_t *d = pref_desc(r);
    return r.kind == R_ENG && r.a == T_909 && d && d->name[0] == 'E' && d->name[1] == 'Q' && d->name[2] == 'F';
}
/* the value, as a number and a unit ("-6" "dB"); a switch gives its name and no unit */
static void pref_value_of(pref_t r, int v, char *num, char *unit)
{
    const x0x_param_t *d = pref_desc(r);
    *num = *unit = 0;
    if (!d || r.kind == R_ACT)
        return;
    if (d->names) {
        put_s(num, d->names[v]);
        return;
    }
    if (r.kind == R_TAPE) {
        if (v < 16)
            put_s(num, "DIGI");
        else {
            put_i(num, (v - 16) * 100 / 111);
            put_s(unit, "%");
        }
        return;
    }
    if (r.kind == R_ENG && r.a == T_MST && r.c == MST_DJF) {             /* PERFORM filter: how far it is closed in (OFF = open) */
        int a = proj.sound.v[T_MST][0][MST_FTYPE] == 2 ? 127 - v : v;
        a = a * 100 / 127;
        if (!a)
            put_s(num, "OFF");
        else
            put_i(num, a), put_s(unit, "%");
        return;
    }
    if (is_eqfreq(r) || is_oscfreq(r) || is_nffreq(r)) {                 /* 20 Hz .. 20 kHz */
        int hz = (int)(20.0f * fm_exp2f(9.9658f * (float)v / 127.0f) + 0.5f);
        if (hz < 1000)
            put_i(num, hz), put_s(unit, "Hz");
        else if (hz < 10000) {
            put_i(put_s(put_i(num, hz / 1000), "."), hz % 1000 / 100);
            put_s(unit, "kHz");
        } else
            put_i(num, (hz + 500) / 1000), put_s(unit, "kHz");
        return;
    }
    if (is_modamt(r)) {                                  /* +12.34 sm: (2v - 127) * 96 / 127, in hundredths; the middle pot (64) is exactly 0.00 */
        int h = v == 64 ? 0 : ((2 * v - 127) * 9600 + (v >= 64 ? 63 : -63)) / 127, ah = h < 0 ? -h : h;
        char *q = put_s(num, h < 0 ? "-" : h > 0 ? "+" : "");
        q = put_i(q, ah / 100);
        q = put_s(q, ".");
        put_i(put_i(q, ah % 100 / 10), ah % 10);
        put_s(unit, "sm");
        return;
    }
    if (is_pan(r)) {                                     /* L64 .. C .. R63 */
        if (v == 64)
            put_s(num, "C");
        else
            put_i(put_s(num, v < 64 ? "L" : "R"), v < 64 ? 64 - v : v - 64);
        return;
    }
    switch (r.kind) {
    case R_ENG:
        if (r.a == T_MST) {                              /* the master formats itself: "-12dB" */
            char t[16], *q = t;
            engine_master_format(r.c, t);
            while (*q && (*q == '-' || *q == '.' || (*q >= '0' && *q <= '9')))
                q++;
            put_s(unit, q);
            *q = 0;
            put_s(num, t);
            return;
        }
        if (r.a == T_BRK && r.c == 2) {                  /* the break's FILTER, as the master's: OFF, LP 40%, HP 20% */
            int a = v < 60 ? (60 - v) * 100 / 60 : v > 68 ? (v - 68) * 100 / 59 : 0;
            if (!a)
                put_s(num, "OFF");
            else {
                put_i(put_s(num, v < 60 ? "LP " : "HP "), a);
                put_s(unit, "%");
            }
            return;
        }
        if (r.a == T_MIX) {
            if (r.c == 0) {                              /* level: 100 = 0 dB, square law */
                if (!v)
                    put_s(num, "OFF");
                else {
                    float db = 12.0412f * fm_log2f((float)v / 100.0f);
                    int t10 = (int)(db * 10.0f + (db < 0 ? -0.5f : 0.5f));
                    char *q = num;
                    if (t10 > 0)
                        *q++ = '+';
                    else if (t10 < 0)
                        *q++ = '-';
                    t10 = t10 < 0 ? -t10 : t10;
                    q = put_i(q, t10 / 10);
                    if (t10 < 100 && t10 % 10) {
                        *q++ = '.';
                        put_i(q, t10 % 10);
                    }
                    put_s(unit, "dB");
                }
                return;
            }
            put_i(num, v * 100 / 127);
            put_s(unit, "%");
            return;
        }
        put_i(num, v);
        return;
    case R_DLEN: put_i(num, v + 1); put_s(unit, "st"); return;
    case R_SWING: put_i(num, v); put_s(unit, "%"); return;   /* wip74: 0 - 100 % (was 50 - 75) */
    case R_BRKSET: put_i(num, v); put_s(unit, "%"); return;
    case R_TEMPO: put_i(num, v + 20); put_s(unit, "bpm"); return;
    case R_SHLO: case R_SHHI: {                        /* wip289: the corner: 20 .. 663 Hz, 663 Hz .. 22 kHz (OFF at the open end) */
        int hz;                                        /* wip318: no OFF any more: LOW CUT 20 Hz .. 663 Hz, HIGH CUT 20 kHz .. 1 kHz */
        hz = tonic_shelf_hz(r.kind == R_SHHI, v);
        put_i(num, hz); put_s(unit, "Hz");                /* wip299: always the whole number of Hz (19900, 20000 ...) */
        return;
    }
    case R_SCINT: {                                    /* wip617; wip619: the real compressor THRESHOLD in dB: 0.0 all the way right (no ducking) to -59.0, then -INF all the way left (the knob 127 .. 0), as the Ableton compressor */
        int t10 = ((127 - v) * 590 + 63) / 126;       /* wip620: 0.0 .. -59.0 dB, the very last step (knob 0) = -INF */
        char *q = num;
        if (v == 0) {
            put_s(num, "-INF");
            unit[0] = 0;
            return;
        }
        if (t10 > 0)
            *q++ = '-';
        q = put_i(q, t10 / 10);
        *q++ = '.';
        put_i(q, t10 % 10);
        put_s(unit, "dB");
        return;
    }
    case R_SCIN: put_i(num, v + 1); return;                                      /* wip349: track 1-8 */
    case R_SBAR: put_i(num, v + 1); return;
    case R_SPAT: num[0] = 'P'; put_i(num + 1, v + 1); return;
    case R_MIDICH: case R_REMCH: if (v) put_i(num, v); else put_s(num, "OFF"); return;
    case R_GACC: num[0] = '+'; put_i(num + 1, v + 1); return;   /* wip616: +1 .. +10 */
    case R_CCBASE: {                                   /* wip386: the block, e.g. 1-8 */
        char *e = put_i(num, v + 1);
        *e++ = '-';
        put_i(e, v + 8);
        return;
    }
    case R_SREP:
        if (v >= 16)
            put_s(num, "LOOP");
        else {
            num[0] = 'x';
            put_i(num + 1, v + 1);
        }
        return;
    case R_SLEN: put_i(num, v); put_s(unit, "bar"); return;
    default: put_i(num, v); return;
    }
}
static void pref_value(pref_t r, char *num, char *unit) { pref_value_of(r, pref_get(r), num, unit); }

/* ===================================================================== pages === */
#define MAXPAGES 14
/* page groups: whose values a page holds (the SEL list's headers, the header's colour) */
enum { GR_NONE, GR_TRACK, GR_TMIX, GR_KIT, GR_PAT, GR_SOUND, GR_MORE,
       GR_MIDI, GR_AUDIO, GR_KEYS, GR_PROJECT, GR_SYSTEM };   /* (the last five: GLO's sections) */
#define MAXMORE 14
typedef struct {
    pref_t r[MAXPAGES][4];
    char title[MAXPAGES][12];
    uint8_t group[MAXPAGES];
    int n;
    pref_t more[MAXMORE];              /* the screen's other knobs: only in SEL's list, under MORE */
    int nmore;
} pages_t;
static pages_t pg;
static uint8_t pg_grp;                 /* the group pages added now belong to */
static void more(pref_t r)
{
    if (r.kind != R_NONE && pg.nmore < MAXMORE)
        pg.more[pg.nmore++] = r;
}

static void title(int i, const char *s)
{
    int k = 0;
    while (s[k] && k < 11) {
        pg.title[i][k] = s[k];
        k++;
    }
    pg.title[i][k] = 0;
}

static void add_page(const char *t, pref_t a, pref_t b, pref_t c, pref_t d)
{
    if (pg.n >= MAXPAGES)
        return;
    pg.r[pg.n][0] = a;
    pg.r[pg.n][1] = b;
    pg.r[pg.n][2] = c;
    pg.r[pg.n][3] = d;
    title(pg.n, t);
    pg.group[pg.n] = pg_grp;
    pg.n++;
}

static void add_eng_pages(int t, int v, int from, int to, const char *name)
{
    int n = engine_nparams(t, v), i;
    if (to > n)
        to = n;
    for (i = from; i < to && pg.n < MAXPAGES; i += 4) {
        int k;
        for (k = 0; k < 4; k++)
            pg.r[pg.n][k] = (i + k < to) ? PR(R_ENG, t, v, i + k) : NONE;
        title(pg.n, name);
        pg.group[pg.n] = pg_grp;
        pg.n++;
    }
}

static int name_is(const x0x_param_t *d, const char *n)
{
    const char *a = d ? d->name : "";
    while (*a && *a == *n) {
        a++;
        n++;
    }
    return !*a && !*n;
}
static pref_t find_ref(int t, int v, const char *n)
{
    int i;
    for (i = 0; i < engine_nparams(t, v); i++)
        if (name_is(engine_param(t, v, i), n))
            return PR(R_ENG, t, v, i);
    return NONE;
}

/* a drum track: its sound, four knobs a page (its DRIVE and DIST: SEL's list; its level, pan and sends:
 * MIX's 909 MIX / 808 MIX page) */
static void add_track_pages(int t, int v)
{
    const char *nm = engine_voice_name(t, v);
    pref_t snd[NPARAMS_MAX];
    int n = 0, i;
    for (i = 0; i < engine_nparams(t, v) && n < NPARAMS_MAX; i++) {
        const x0x_param_t *d = engine_param(t, v, i);
        if (name_is(d, "Drive") || name_is(d, "Dist"))
            more(PR(R_ENG, t, v, i));
        else if (!name_is(d, "Level") && !name_is(d, "Pan") && !name_is(d, "Rev") && !name_is(d, "Dly"))
            snd[n++] = PR(R_ENG, t, v, i);
    }
    pg_grp = GR_TRACK;
    for (i = 0; i < n; i += 4)
        add_page(nm, snd[i], i + 1 < n ? snd[i + 1] : NONE, i + 2 < n ? snd[i + 2] : NONE, i + 3 < n ? snd[i + 3] : NONE);
}

/* where a screen's page is kept: per part on the part's screens (and TB-3PO), else one per screen */
static uint8_t *page_of(int view)
{
    return &ui.page[view][view == V_PART || view == V_SEQ ? ui.part : 0];
}

static void build_pages(void)
{
    int p = ui.part;
    pg.n = pg.nmore = 0;
    pg_grp = GR_NONE;
    switch (ui.view) {
    case V_GLO:                                        /* wip129: GLO 2nd press = the master COMP (page 1 = MIX, 3 = the settings list) */
        add_page("COMP", PR(R_ENG, T_MST, 0, MST_ATTACK), PR(R_ENG, T_MST, 0, MST_RELEASE),
                 PR(R_ENG, T_MST, 0, MST_THRESH), PR(R_ENG, T_MST, 0, MST_MAKEUP));   /* the master compressor, Glue style (the ratio stays 4:1) */
        break;
    case V_HOME:                                       /* the pattern: how the parts run through it */
        /* wip368: K3 = the master LOW CUT (the PERFORM filter, MST_DJF, is kept in the code: to bring it back put PR(R_ENG, T_MST, 0, MST_DJF) here) */
        add_page("PERFORM", PR(R_TEMPO, 0, 0, 0), PR(R_SWING, 0, 0, 0), PR(R_ENG, T_MST, 0, MST_LOCUT),
                 PR(R_ENG, T_MST, 0, MST_PUMP));
        break;
    case V_SEQ:                                        /* the part's steps: how its pattern runs */
        pg_grp = GR_PAT;
        if (is_drum() && ui.arp)                       /* wip251: the ARP page: only the octave (the voice: SELECT) */
            add_page("ARP", PR(R_FIXLEN, p, 0, 0), PR(R_SCROLL, 0, 0, 0), NONE, PR(R_ARPO, 0, 0, 0));   /* wip287: K1 = FIXED LEN (as on SEQ); wip703: K2 = AUTOSCROLL (the same setting as on SEQ), K3 = the PAGE spot (drawn, no value), K4 = the octave (was K2) */
        else if (is_drum())
            add_page("SEQUENCE", PR(R_FIXLEN, p, 0, 0), PR(R_SCROLL, 0, 0, 0), PR(R_DLEN, p, 0, 0), PR(R_SWING, 0, 0, 0));   /* numbers only (draw_knobs); RATE is QUANTIZATION on GLOBAL */
        else {
            add_page("GROOVE", PR(R_BRKSET, 0, 0, BRK_COMPLEX), PR(R_BRKSET, 0, 0, BRK_ANCHOR),
                     PR(R_BRKSET, 0, 0, BRK_ROLL), PR(R_BRKSET, 0, 0, BRK_FILL));
            add_page("RETRIG", PR(R_BRKSET, 0, 0, BRK_R2), PR(R_BRKSET, 0, 0, BRK_R3), PR(R_BRKSET, 0, 0, BRK_R4),
                     PR(R_BRKSET, 0, 0, BRK_R8));
            more(PR(R_BRKSET, 0, 0, BRK_PHRASE));
            more(PR(R_BRKSET, 0, 0, BRK_ALEN));
            more(PR(R_BRKSET, 0, 0, BRK_BLEN));
        }
        break;
    case V_PART:                                       /* the part's sound */
        if (is_drum()) {
            add_track_pages(T_909, ui.sel[p]);
            pg_grp = GR_KIT;
            add_page("KIT", PR(R_ENG, T_909, NDRUM, 1), NONE, NONE, NONE);   /* (the kit's gain: the mixer's level does it) */
        } else {
            pg_grp = GR_PAT;                           /* (its groove and retrigs: SEQ) */
            add_page("LOOPS", PR(R_BRKSLOT, 0, 0, 0), PR(R_BRKSLOT, 1, 0, 0), PR(R_BRKSET, 0, 0, BRK_BCHANCE),
                     PR(R_ENG, T_BRK, 0, 1));
            add_page("SOUND", PR(R_ENG, T_BRK, 0, 2), PR(R_ENG, T_BRK, 0, 0), NONE, NONE);   /* FILTER (#14), LEVEL */
        }
        break;
    case V_FX:                                         /* (each part's sends: MIX's PARTS) */
        add_page("PERFORM", PR(R_ENG, T_MIX, PART_909, MX_REV), PR(R_ENG, T_MIX, PART_909, MX_DLY),
                 PR(R_ENG, T_MST, 0, MST_LOCUT), PR(R_FXALL, 0, 0, 0));   /* FX 2nd press: reverb D/W, delay D/W, master LOW CUT, ALL FX on/off (wip368: the filter, MST_DJF, is kept in the code: put PR(R_ENG, T_MST, 0, MST_DJF) back here to bring it back) */
        add_page("REVERB", PR(R_ENG, T_FX, 0, FX_RV_DECAY), PR(R_ENG, T_FX, 0, FX_RV_TONE), NONE, NONE);
        add_page("DELAY", PR(R_ENG, T_FX, 0, FX_DL_TIME), PR(R_ENG, T_FX, 0, FX_DL_FDBK),
                 PR(R_ENG, T_FX, 0, FX_DL_TONE), PR(R_TAPE, 0, 0, 0));
        add_page("REVERB 2", PR(R_ENG, T_FX, 0, FX_RV_HPF), PR(R_ENG, T_FX, 0, FX_RV_LEVEL), NONE, NONE);   /* wip270: ALGORITHM on REVERB / DELAY goes to these (page 3 / 4) and back */
        add_page("DELAY 2", PR(R_ENG, T_FX, 0, FX_DL_HPF), PR(R_ENG, T_FX, 0, FX_DL_LEVEL), PR(R_ENG, T_FX, 0, FX_DL_PING), NONE);
        more(PR(R_ENG, T_FX, 0, FX_RV_HPF));
        more(PR(R_ENG, T_FX, 0, FX_RV_LEVEL));
        more(PR(R_ENG, T_FX, 0, FX_DL_HPF));
        more(PR(R_ENG, T_FX, 0, FX_DL_LEVEL));
        more(PR(R_ENG, T_FX, 0, FX_DL_PING));
        break;
    case V_MIX:                                        /* wip277, GLO 1st press: SELECT cycles G1 LEVELS, G2 LEVELS, G1 PANS, G2 PANS (G1 = tracks 1-4, G2 = 5-8); K1-K4 = the four shown */
        add_page("TONIC MIX", find_ref(T_909, 0, "Level"), find_ref(T_909, 1, "Level"), find_ref(T_909, 2, "Level"), find_ref(T_909, 3, "Level"));
        add_page("TONIC MIX", find_ref(T_909, 4, "Level"), find_ref(T_909, 5, "Level"), find_ref(T_909, 6, "Level"), find_ref(T_909, 7, "Level"));
        add_page("TONIC PAN", find_ref(T_909, 0, "Pan"), find_ref(T_909, 1, "Pan"), find_ref(T_909, 2, "Pan"), find_ref(T_909, 3, "Pan"));
        add_page("TONIC PAN", find_ref(T_909, 4, "Pan"), find_ref(T_909, 5, "Pan"), find_ref(T_909, 6, "Pan"), find_ref(T_909, 7, "Pan"));
        break;
    case V_MIX2:                                       /* HOME, 1st press: the selected track's level and pan */
        {
            int v = ui.sel[0];
            add_page("MIX 2", find_ref(T_909, v, "Level"), find_ref(T_909, v, "Pan"), PR(R_SHLO, v, 0, 0), PR(R_SHHI, v, 0, 0));   /* wip289: K3 / K4 = the track's low / high shelving cut */
            add_page("SENDS", find_ref(T_909, v, "Rev"), find_ref(T_909, v, "Dly"), PR(R_SCINT, 0, 0, 0), PR(R_SCIN, 0, 0, 0));   /* wip598: K3 = THRESHOLD, K4 = INPUT (his order: the outlined sidechain section reads THRESHOLD, INPUT) */   /* wip349: K3 = sidechain intensity, K4 = sidechain input (global, not per track). wip288: HOME 2nd press: the track's own reverb / delay sends on K1 / K2 (they were K3 / K4 of MIX 2) */
        }
        break;
    case V_MIX1:                                       /* FX, 1st press: the selected track's Mix, EQ Freq, Distort, EQ Gain */
        {
            int v = ui.sel[0];
            add_page("MIX 1", find_ref(T_909, v, "Mix"), find_ref(T_909, v, "EQFreq"), find_ref(T_909, v, "Dist"),
                     find_ref(T_909, v, "EQGain"));
        }
        break;
    case V_GLOB:                                       /* HOME, 2nd press: the pattern's grid, swing and fill rate (no track number) */
        add_page("GLOBAL", PR(R_QUANT, 0, 0, 0), PR(R_SWING, 0, 0, 0), PR(R_FILLR, 0, 0, 0), PR(R_TEMPO, 0, 0, 0));   /* K4: the tempo, a slider at the bottom */
        break;
    case V_PMOD:                                       /* SEL: the selected track's pitch mod: shape, amount, rate */
        {
            int v = ui.sel[0];
            add_page("PMOD", find_ref(T_909, v, "ModMod"), find_ref(T_909, v, "ModAmt"), find_ref(T_909, v, "ModRat"), NONE);
        }
        break;
    case V_OSC:                                        /* ENV: the selected track's oscillator: wave, freq, attack, decay */
        {
            int v = ui.sel[0];
            add_page("OSC", find_ref(T_909, v, "Wave"), find_ref(T_909, v, "Pitch"), find_ref(T_909, v, "OscAtk"),
                     find_ref(T_909, v, "OscDcy"));
        }
        break;
    case V_NOISE:                                      /* LFO: the selected track's noise: filter mode, Q, freq */
        {
            int v = ui.sel[0];
            add_page("NOISE", find_ref(T_909, v, "NFMode"), find_ref(T_909, v, "NFFreq"), find_ref(T_909, v, "NFQ"), NONE);   /* wip558: K2 = filter freq (the slider), K3 = filter Q */
        }
        break;
    case V_NEV:                                        /* EDIT, 1st press: the selected track's noise envelope: mode (K1), CHOKE (K2), attack (K3), decay (K4) */
        {
            int v = ui.sel[0];
            add_page("NENV", find_ref(T_909, v, "NEMode"), NONE, find_ref(T_909, v, "NEAtk"), find_ref(T_909, v, "NEDcy"));   /* wip328: K2 = the CHOKE button (opens its screen), attack and decay on K3 / K4 */
        }
        break;
    case V_VEL:                                        /* wip290: GLO, 1st press (was EDIT 2nd): the selected track's velocity amounts: osc, noise, mod */
        {
            int v = ui.sel[0];
            add_page("VEL", find_ref(T_909, v, "VelOsc"), find_ref(T_909, v, "VelNs"), find_ref(T_909, v, "VelMod"), NONE);
        }
        break;
    case V_PERF:
        add_page("PERF", PR(R_STALLS, 0, 0, 0), NONE, NONE, NONE);
        break;
    case V_SONG:
        add_page("BAR", PR(R_SBAR, 0, 0, 0), PR(R_SPAT, PART_909, 0, 0), PR(R_SREP, 0, 0, 0),
                 NONE);                                /* wip356: ONE page (was two): K1 bar, K2 pattern, K3 repeat (SELECT has no 2nd page to turn to) */
        break;
    default: break;
    }
    if (pg.n == 0)
        add_page("", NONE, NONE, NONE, NONE);
    if (*page_of(ui.view) >= pg.n)
        *page_of(ui.view) = 0;
}

static int cur_page(void) { return *page_of(ui.view); }

/* the FX view's COMP page (wip121): the needle meter fills the main area, so no zoom dial over it */
static int fx_comp_page(void)
{
    const char *t = pg.title[cur_page()];
    return ui.view == V_GLO && ui.overlay == O_NONE && t[0] == 'C' && t[1] == 'O' && t[2] == 'M' && t[3] == 'P' && !t[4];
}

/* MIX's page of a drum machine's tracks: which machine (0 / 1), -1 = not one */
static int drum_mix_page(void)
{
    if (ui.view == V_MIX || ui.view == V_MIX2 || ui.view == V_MIX1 || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL)   /* HOME's MIX PAGE 2: the Tonic's tracks too */
        return 0;
    return -1;
}

/* a drum machine's page of the selected track's own sound (not the whole kit's: SENDS, PART, KIT) */
static int track_page(void)
{
    int g = pg.group[cur_page()];
    return ui.view == V_PART && is_drum() && (g == GR_TRACK || g == GR_TMIX);
}

/* ===================================================================== lists === */
/* A list is rows of refs. SEL on a page opens the list of every page's knobs (the deep view);
 * GLOBAL is a list of its own. SELECT moves, ALGORITHM changes, SEL runs an action. */
#define LIST_MAX 64
static pref_t list_rows[LIST_MAX];
static char list_title[16];

static void list_add(pref_t r)
{
    int i;
    if (r.kind == R_NONE || ui.list_n >= LIST_MAX)
        return;
    for (i = 0; i < ui.list_n; i++)                   /* a ref on two pages (LEVEL) lists once */
        if (list_rows[i].kind == r.kind && list_rows[i].a == r.a && list_rows[i].b == r.b && list_rows[i].c == r.c)
            return;
    list_rows[ui.list_n++] = r;
}

/* a list section's name: whose values follow */
static void hdr_name(int g, char *t)
{
    const char *who = is_drum() ? engine_voice_name(T_909, ui.sel[ui.part]) : PART_N[ui.part];
    switch (g) {
    case GR_TRACK: put_s(put_s(t, who), ": THIS TRACK"); break;
    case GR_TMIX: put_s(put_s(t, who), ": TRACK MIX"); break;
    case GR_KIT: put_s(put_s(t, PART_N[ui.part]), ": ALL TRACKS"); break;
    case GR_PAT: put_s(put_i(put_s(t, "P"), seq.ppat[ui.part] + 1), ": THE PATTERN"); break;
    case GR_MORE: put_s(t, "MORE"); break;
    case GR_MIDI: put_s(t, "MIDI"); break;
    case GR_AUDIO: put_s(t, "AUDIO"); break;
    case GR_KEYS: put_s(t, "KEYS AND SCREEN"); break;
    case GR_PROJECT: put_s(t, "PROJECT"); break;
    case GR_SYSTEM: put_s(t, "SYSTEM"); break;
    default: put_s(put_s(t, PART_N[ui.part]), ": THE SOUND"); break;
    }
}

static int list_skip(int i) { return i < ui.list_n && list_rows[i].kind == R_HDR; }

static void open_list_of_pages(void)
{
    int p, k, last = -1, groups = 0;
    ui.list_n = 0;
    for (p = 0; p < pg.n; p++)
        if (pg.group[p] != (p ? pg.group[p - 1] : GR_NONE))
            groups++;
    for (p = 0; p < pg.n; p++) {
        if (groups > 1 && pg.group[p] != last && ui.list_n < LIST_MAX)
            list_rows[ui.list_n++] = PR(R_HDR, pg.group[p], 0, 0);
        last = pg.group[p];
        for (k = 0; k < 4; k++)
            list_add(pg.r[p][k]);
    }
    if (pg.nmore && ui.list_n < LIST_MAX)              /* the knobs no page has */
        list_rows[ui.list_n++] = PR(R_HDR, GR_MORE, 0, 0);
    for (k = 0; k < pg.nmore; k++)
        list_add(pg.more[k]);
    if (ui.view == V_PERF)
        list_add(PR(R_ACT, ACT_PERF_TEST, 0, 0));
    if (ui.view == V_SONG) {
        list_add(PR(R_SLEN, 0, 0, 0));
        list_add(PR(R_ACT, ACT_SONG_INS, 0, 0));
        list_add(PR(R_ACT, ACT_SONG_DEL, 0, 0));
        list_add(PR(R_ACT, ACT_SONG_CLR, 0, 0));
    }
    if (ui.view == V_SEQ)
        put_s(put_s(list_title, PART_N[ui.part]), " SEQ");
    else
        put_s(list_title, ui.view == V_PART ? PART_N[ui.part] : VIEW_N[ui.view]);
    ui.list_sel = ui.list_top = 0;
    while (list_skip(ui.list_sel))
        ui.list_sel++;
    ui.overlay = ui.list_n ? O_LIST : O_NONE;
}

static void open_global(void)
{
    ui.list_n = 0;
    list_rows[ui.list_n++] = PR(R_HDR, GR_MIDI, 0, 0);
    list_add(PR(R_CLKOUT, 0, 0, 0));
    list_add(PR(R_NOTEOUT, 0, 0, 0));
    for (int p = 0; p < NPARTS; p++)
        if (p == PART_909)                               /* wip134: the Tonic's MIDI channel only (no 808, no BREAK) */
            list_add(PR(R_MIDICH, p, 0, 0));
    list_add(PR(R_REMCH, 0, 0, 0));                      /* wip385: REMOTE MIDI CH (OFF by default) */
    list_add(PR(R_CCMODE, 0, 0, 0));                     /* wip386: CC MODE (OFF by default) */
    list_add(PR(R_CCBASE, 0, 0, 0));                     /* wip386: CC CH BASE (channels 1-8 by default) */
    list_rows[ui.list_n++] = PR(R_HDR, GR_AUDIO, 0, 0);
    list_add(PR(R_OUTLOUD, 0, 0, 0));
    list_add(PR(R_USBFULL, 0, 0, 0));
    list_add(PR(R_GACC, 0, 0, 0));                       /* wip612: GLOBAL ACCENT +1 .. +10 */
    list_rows[ui.list_n++] = PR(R_HDR, GR_KEYS, 0, 0);
    list_add(PR(R_KEYLED, 0, 0, 0));
    list_add(PR(R_BEAT, 0, 0, 0));
    list_add(PR(R_PALETTE, 0, 0, 0));
    list_add(PR(R_ZOOM, 0, 0, 0));
    list_rows[ui.list_n++] = PR(R_HDR, GR_PROJECT, 0, 0);
    list_add(PR(R_ACT, ACT_SAVE, 0, 0));
    list_add(PR(R_AUTOSAVE, 0, 0, 0));
    list_add(PR(R_NAMEMODE, 0, 0, 0));
    list_add(PR(R_SWMODE, 0, 0, 0));                     /* wip700: SWING GLOBAL / PER PATTERN */
    list_add(PR(R_ACT, ACT_CLEAR_PAT, 0, 0));
    list_add(PR(R_ACT, ACT_RESET, 0, 0));
    list_rows[ui.list_n++] = PR(R_HDR, GR_SYSTEM, 0, 0);
    list_add(PR(R_CLASSIC, 0, 0, 0));
    list_add(PR(R_ACT, ACT_PERF, 0, 0));
    list_add(PR(R_CORES, 0, 0, 0));
    list_add(PR(R_ACT, ACT_ABOUT, 0, 0));
    list_add(PR(R_BATT, 0, 0, 0));
    put_s(list_title, "GLOBAL");
    ui.list_sel = ui.list_top = 0;
    while (list_skip(ui.list_sel))
        ui.list_sel++;
    ui.overlay = O_LIST;
}

/* ================================================================ questions === */
static void ask(int act, int arg, const char *q1, const char *q2)
{
    ui.ask_act = (uint8_t)act;
    ui.ask_arg = (uint16_t)arg;
    put_s(ui.ask_q[0], q1);
    put_s(ui.ask_q[1], q2 ? q2 : "");
    ui.overlay = O_ASK;
}

/* part `part` of a pattern has something to play */
static int part_used(const pattern_t *pt, int part)
{
    int v;
    if (part < NKIT) {
        for (v = 0; v < NDRUM; v++)
            if (sm_any(pt->drum[part].hit[v]))
                return 1;
        return 0;
    }
    return pt->brk.steps != 0;
}

static int pattern_used(const pattern_t *pt)
{
    int v, used = pt->brk.steps != 0;
    for (v = 0; v < NDRUM; v++)
        used |= sm_any(pt->drum[0].hit[v]) | sm_any(pt->drum[1].hit[v]);
    return used;
}

/* motion lanes of one part pattern: gone, and the knobs back to their own values */
static void clear_lanes(int pat, int part)
{
    int k;
    for (k = 0; k < NLANE; k++) {
        lane_t *l = &proj.arr.lane[k];
        if (l->used && l->pat == pat && l->part == part) {
            motion_clear(proj.arr.lane, k);
            if (l->t < NTARGETS)                      /* (T_PROB: no knob) */
                engine_set(l->t, l->v, l->i, proj.sound.v[l->t][l->v][l->i]);
        }
    }
}

/* part `part` of pattern src onto pattern dst, its lanes too; 0 = done, -1 = lanes did not fit */
static int copy_part(int dst, int src, int part)
{
    pattern_t *d = &proj.pat[dst];
    const pattern_t *a = &proj.pat[src];
    int k, rc = 0;
    if (part < NKIT) {
        d->drum[part] = a->drum[part];
        if (part == PART_909) {
            d->swing = a->swing, d->fill = a->fill;   /* the 909 is the bar: its groove and fill rate go with it */
            if (dst != src) {                         /* wip311: the late offsets go with the trigs */
                int v, s, o;
                seq_late_clear_pat(&seq, dst);
                for (v = 0; v < 8; v++)
                    for (s = 0; s < NSTEPS; s++)
                        if ((o = seq_late_get(&seq, src, v, s)) != 0)
                            seq_late_set(&seq, dst, v, s, o);
            }
            if (dst < NPAT_NOTE) {                    /* wip252: the notes go with the trigs (a source with none: empty) */
                int v, s;
                for (v = 0; v < NTRK_NOTE; v++)
                    for (s = 0; s < NSTEPS; s++)
                        proj.note[dst][v][s] = src < NPAT_NOTE ? proj.note[src][v][s] : 0;
            }
        }
    } else
        d->brk = a->brk;
    clear_lanes(dst, part);
    for (k = 0; k < NLANE; k++) {                     /* a copy lands on pattern dst: never copied again */
        const lane_t *l = &proj.arr.lane[k];
        int n, i;
        if (!l->used || l->pat != src || l->part != part)
            continue;
        if ((n = motion_alloc(proj.arr.lane, dst, part, l->t, l->v, l->i)) < 0) {
            rc = -1;
            break;
        }
        for (i = 0; i < NSTEPS; i++)
            proj.arr.lane[n].val[i] = l->val[i];
    }
    return rc;
}

static void clear_part(pattern_t *p, int part)
{
    int i;
    if (part < NKIT) {
        int w;
        for (w = 0; w < NSW; w++) {
            for (i = 0; i < NDRUM; i++) {
                p->drum[part].hit[i][w] = 0;
                p->drum[part].vacc[i][w] = 0;
                p->drum[part].vfill[i][w] = 0;
            }
            p->drum[part].accent[w] = 0;
        }
        if (part == PART_909 && p >= proj.pat && p < proj.pat + NPAT)   /* wip311: and the late offsets */
            seq_late_clear_pat(&seq, (int)(p - proj.pat));
        if (part == PART_909 && p >= proj.pat && p < proj.pat + NPAT_NOTE) {   /* wip252: the notes go with the trigs */
            int v, s, pi = (int)(p - proj.pat);
            for (v = 0; v < NTRK_NOTE; v++)
                for (s = 0; s < NSTEPS; s++)
                    proj.note[pi][v][s] = 0;
        }
    } else {
        p->brk.steps = 0;
        for (i = 0; i < 16; i++)
            p->brk.slice[i] = 0;
    }
}

/* ===================================================== Tonic: copy / paste / clear === */
/* wip206: FUNC + ACCENT (black key 9) copies the pattern that plays now (only its number is remembered: a paste takes the
 * pattern as it is then), FUNC + FILL (black key 10) pastes it here (when FILL is let go), FUNC + FILL held + a white key pastes it
 * into that slot (the key blinks fast), FUNC + PLAY asks to clear the pattern (a black key 1-8 held: only that track). The same two
 * keys copy / paste a track's sound (a black key 1-8 held) and a page (EDIT / LFO held). Undo (GLO + ACCENT) takes any of them back. */
#define TN_BLINK_MS 600u

static void tn_copy(void)
{
    char t[16];
    ui.clip = (int8_t)seq.ppat[PART_909];
    put_s(put_i(put_s(t, "P"), ui.clip + 1), " COPIED");
    say(t, 0);
}

static void tn_paste(int dst)
{
    int src = ui.clip, rc;
    char t[28], *q;
    if (src < 0) {
        warn("NOTHING COPIED YET", 0);
        return;
    }
    if (dst < 0 || dst >= NPAT)
        return;
    ui.blink_pat = (int8_t)(dst - ui.pbank * 16);
    ui.blink_until = plat_ms() + TN_BLINK_MS;
    if (src == dst) {
        say("THAT IS THE COPY", 0);
        return;
    }
    rc = copy_part(dst, src, PART_909);
    mark_dirty();
    q = put_i(put_s(t, "P"), src + 1);
    put_i(put_s(q, " PASTED TO P"), dst + 1);
    if (rc)
        warn("MOTION FULL: NOT ALL PASTED", 0);
    else
        say(t, 0);
}

/* wip416: FUNC + FILL on the pattern asks PASTE PATTERN? first (OCT+ YES / OCT- NO). Nothing copied yet, or the copy is this
 * pattern: no question, tn_paste says so as before. Tracks, pages and the white-key / COPY-PASTE MODE pastes are not asked. */
static void tn_paste_ask(int dst)
{
    if (ui.clip < 0 || ui.clip == dst || dst < 0 || dst >= NPAT) {
        tn_paste(dst);
        return;
    }
    ask(ACT_PASTE_TNPAT, dst, "PASTE PATTERN?", "[OCT+] YES [OCT-] NO");
}

/* wip250: after a clear, a pattern with no notes is no longer chained; when that was the last flag, the chain is not followed any more */
static void chain_after_clear(void)
{
    int before = project_chain_count();
    if (project_chain_tidy() == 0 && before > 0 && !ui.songmode)
        seq.chain_on = 0;
}

static void tn_clear_pat(void)
{
    clear_part(pat_of(PART_909), PART_909);
    clear_lanes(seq.ppat[PART_909], PART_909);
    chain_after_clear();
    mark_dirty();
    say("PATTERN CLEARED", 0);
}

static void tn_clear_all(void)                        /* wip632: every pattern of every bank: trigs, accents, fills, notes, late offsets, motion (sounds, presets, song, settings stay) */
{
    int p;
    for (p = 0; p < NPAT; p++) {
        clear_part(&proj.pat[p], PART_909);
        clear_lanes(p, PART_909);
    }
    chain_after_clear();
    mark_dirty();
    say("ALL PATTERNS CLEARED", 0);
}

static void tn_clear_track(int v)
{
    int p = seq.ppat[PART_909], w, k;
    dpart_t *d = &proj.pat[p].drum[PART_909];
    for (w = 0; w < NSW; w++)
        d->hit[v][w] = d->vacc[v][w] = d->vfill[v][w] = 0;
    if (p < NPAT_NOTE && v < NTRK_NOTE)                /* wip252: its notes too */
        for (w = 0; w < NSTEPS; w++)
            proj.note[p][v][w] = 0;
    if (v < 8)                                         /* wip311: and its late offsets */
        for (w = 0; w < NSTEPS; w++)
            seq_late_set(&seq, p, v, w, 0);
    for (k = 0; k < NLANE; k++) {                     /* its p-locks (motion) go too, the knobs back to their own values */
        lane_t *l = &proj.arr.lane[k];
        if (l->used && l->pat == p && l->part == PART_909 && l->t == T_909 && l->v == v) {
            motion_clear(proj.arr.lane, k);
            engine_set(l->t, l->v, l->i, proj.sound.v[l->t][l->v][l->i]);
        }
    }
    chain_after_clear();
    mark_dirty();
    say("CLEARED ", engine_voice_name(T_909, v));
}

/* wip367: the selected track's AUTOMATION (its recorded knob motion and p-locks) in the playing pattern: how many lanes, and gone.
 * Other patterns, other tracks and the step probabilities (T_PROB) are not touched; the knobs go back to their own values. */
static int tn_auto_n(int v)
{
    int p = seq.ppat[PART_909], k, n = 0;
    for (k = 0; k < NLANE; k++) {
        lane_t *l = &proj.arr.lane[k];
        if (l->used && l->pat == p && l->part == PART_909 && l->t == T_909 && l->v == v)
            n++;
    }
    return n;
}

static void tn_clear_auto(int v)
{
    int p = seq.ppat[PART_909], k;
    for (k = 0; k < NLANE; k++) {
        lane_t *l = &proj.arr.lane[k];
        if (l->used && l->pat == p && l->part == PART_909 && l->t == T_909 && l->v == v) {
            motion_clear(proj.arr.lane, k);
            engine_set(l->t, l->v, l->i, proj.sound.v[l->t][l->v][l->i]);
        }
    }
    mark_dirty();
    say("AUTOMATION DELETED: ", engine_voice_name(T_909, v));
}

/* A track's sound: FUNC + black key 1-8 + ACCENT copies it (all 22 knobs: sound, level, pan, and the mute; no trigs, accents,
 * fills or p-locks), FUNC + black key + FILL pastes it into the track whose black key is held. A page: FUNC + EDIT (or LFO) held + ACCENT / FILL / PLAY
 * copies / pastes / resets the page on the screen (its four knobs of the selected track: EDIT's pages, MIX's 909 MIX page), pasting
 * into the selected track, on the same page of the same screen. A black key 1-8 held: SEQ / PLAY copy / paste that track's sound. Undo takes a paste back. */
static int track_muted(int km, int v);
static int tonic_part(void);
static void tn_put(int v, int i, uint8_t pot, uint16_t val)
{
    proj.sound.v[T_909][v][i] = pot;
    if (i < TN_NPARAMS) {
        proj.sound.tn16[v][i] = val;
        engine_tonic_set16(v, i, val);
    } else
        engine_set(T_909, v, i, pot);
}

static uint16_t tn_exact(int v, int i)               /* the exact value of knob i: the 16-bit store, unless a knob moved the pot since */
{
    uint8_t pot = proj.sound.v[T_909][v][i];
    uint16_t w = proj.sound.tn16[v][i];
    return tonic_val_to_pot(i, w) == pot ? w : tonic_pot_to_val(i, pot);
}

static void tn_trk_copy(int v)
{
    char t[28];
    int i;
    for (i = 0; i < TN_NPOTS; i++)
        ui.tc_pot[i] = proj.sound.v[T_909][v][i];
    for (i = 0; i < TN_NPARAMS; i++)
        ui.tc16[i] = tn_exact(v, i);
    ui.tc_mute = (uint8_t)track_muted(PART_909, v);
    ui.tc_src = (int8_t)v;
    put_s(put_i(put_s(t, "T"), v + 1), " SOUND COPIED");
    say(t, 0);
}

static void tn_trk_paste(int v)
{
    char t[28], *q;
    int i;
    if (ui.tc_src < 0) {
        warn("NO TRACK COPIED YET", 0);
        return;
    }
    for (i = 0; i < TN_NPOTS; i++)
        tn_put(v, i, ui.tc_pot[i], i < TN_NPARAMS ? ui.tc16[i] : 0);
    if (ui.tc_mute)
        seq.vmute |= 1u << (PART_909 * NDRUM + v);
    else
        seq.vmute &= ~(1u << (PART_909 * NDRUM + v));
    mark_dirty();
    q = put_i(put_s(t, "T"), ui.tc_src + 1);
    put_i(put_s(q, " SOUND PASTED TO T"), v + 1);
    say(t, 0);
}

static int tn_page_ok(void)                          /* the screen shows a page of the selected track's own knobs? */
{
    return tonic_part() && ((ui.view == V_PART && track_page()) || ((ui.view == V_MIX || ui.view == V_MIX2 || ui.view == V_MIX1 || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL) && drum_mix_page() == 0));
}

static void tn_page_copy(void)
{
    int v = ui.sel[PART_909], k;
    if (!tn_page_ok() || ui.overlay != O_NONE) {
        warn("THIS PAGE IS NOT A TRACK'S", 0);
        return;
    }
    ui.pc_view = (int8_t)ui.view;
    ui.pc_page = (int8_t)cur_page();
    for (k = 0; k < 4; k++) {
        pref_t r = pg.r[cur_page()][k];
        if (r.kind == R_ENG && r.a == T_909 && r.c < TN_NPOTS) {
            ui.pc_i[k] = r.c;
            ui.pc_pot[k] = proj.sound.v[T_909][v][r.c];
            ui.pc16[k] = r.c < TN_NPARAMS ? tn_exact(v, r.c) : 0;
        } else
            ui.pc_i[k] = 255;
    }
    say("PAGE COPIED", 0);
}

static void tn_page_paste(void)
{
    int v = ui.sel[PART_909], k;
    char t[28];
    if (ui.pc_view < 0) {
        warn("NO PAGE COPIED YET", 0);
        return;
    }
    if (!tn_page_ok() || ui.overlay != O_NONE || ui.view != ui.pc_view || cur_page() != ui.pc_page) {
        warn("GO TO THE SAME PAGE", 0);
        return;
    }
    for (k = 0; k < 4; k++)
        if (ui.pc_i[k] != 255)
            tn_put(v, ui.pc_i[k], ui.pc_pot[k], ui.pc16[k]);
    mark_dirty();
    put_s(put_s(t, "PAGE PASTED: "), engine_voice_name(T_909, v));
    say(t, 0);
}

static void tn_page_clear(void)                      /* the page's four knobs of the selected track back to their power-on values */
{
    int v = ui.sel[PART_909], k;
    char t[28];
    if (!tn_page_ok() || ui.overlay != O_NONE) {
        warn("THIS PAGE IS NOT A TRACK'S", 0);
        return;
    }
    for (k = 0; k < 4; k++) {
        pref_t r = pg.r[cur_page()][k];
        const x0x_param_t *d = r.kind == R_ENG && r.a == T_909 ? engine_param(T_909, v, r.c) : 0;
        if (d)
            tn_put(v, r.c, (uint8_t)d->def, r.c < TN_NPARAMS ? tonic_pot_to_val(r.c, d->def) : 0);
    }
    mark_dirty();
    put_s(put_s(t, "PAGE RESET: "), engine_voice_name(T_909, v));
    say(t, 0);
}

static int page_held(void) { return (ui.btn & ((1u << B_EDIT) | (1u << B_LFO))) != 0; }

/* ============================================================ help cards === */
/* Hold one button for HELP_HOLD ms without using it with anything: a card says what it does here,
 * its combinations included. A key, a turn or another button puts the card away (and the
 * combination works as always); letting go of the button after its card does nothing else. */
#define HELP_HOLD 900u
static int perf_testing(void);
#define HELP_LINES 6
static int tonic_part(void);
static int octask(void)                                /* a question OCT+ confirms and OCT- / HOME go back from */
{
    return ui.ask_act == ACT_SONGMODE || ui.ask_act == ACT_SONGEXIT || ui.ask_act == ACT_SONGKILL || ui.ask_act == ACT_SONGBAR || ui.ask_act == ACT_AUTODEL || ui.ask_act == ACT_CLEAR_TNPAT || ui.ask_act == ACT_CLEAR_TNTRK || ui.ask_act == ACT_PASTE_TNPAT || ui.ask_act == ACT_CLEAR_ALL;
}
static const char *const BTN_LABEL[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                          "PLAY / STOP", "REC", "OCT-", "OCT+"};

static int help_lines(int b, const char *l[HELP_LINES])
{
    int n = 0;
#define L(s) (l[n++] = (s))
    switch (b) {
    case B_ARP: L("THE KEYS ARE A PIANO"); L("FOR THE SELECTED VOICE."); L("SELECT: THE VOICE"); L("K1: FIXED LENGTH"); L("K2: AUTOSCROLL. K4: OCTAVE"); L("FUNC + ARP: DELETE SONG"); break;
    case B_FX: L("MIX: MIX, EQ FREQ, DISTORT,"); L("EQ GAIN. AGAIN: SENDS."); L("HOLD: REVERB. HOLD HOME: DELAY"); L("FROM ANOTHER PAGE: THE LAST ONE"); break;   /* (wip223: FX / HOME have no hold-for-help: the card is not shown) */
    case B_SEL: L("PITCH MOD: SHAPE, AMOUNT,"); L("RATE. IN A QUESTION: YES");
        break;
    case B_ENV:
        L("OSCILLATOR: WAVE, FREQ,"); L("ATTACK, DECAY");
        if (is_drum()) { L("HOLD + WHITE KEY: ACCENT"); L("HOLD + BLACK KEY: LOUD HIT"); }
        else L("HOLD + KEY: AN ACCENT");
        L("HOLD + KNOB: ITS DEFAULT");
        break;
    case B_LFO:
        L("NOISE: FILTER MODE, Q,"); L("FILTER FREQ"); L("HOLD A STEP + KNOB: P-LOCK");
        break;
    case B_EDIT: L("NOISE ENVELOPE"); L("HOLD A STEP + KNOB: P-LOCK");
        L("HOLD A STEP + SELECT: ITS CHANCE"); L("FUNC + EDIT: DELETE AUTOMATION");   /* wip367 */
        break;
    case B_GLO: L("VEL: OSC NOISE MOD"); L("AGAIN: MIXER, COMP,"); L("THEN: MEMORY, SETTINGS, SAVE."); L("GLO + ACCENT: UNDO"); L("GLO + FILL: REDO"); break;   /* wip417: undo / redo moved here from HOME + REC / PLAY */
    case B_HOME:
        L("MIX 2: LEVEL PAN LOW HI CUT"); L("AGAIN: PERFORM, GLOBAL");
        L("LIST: BACK   QUESTION: NO");
        if (tonic_part()) L("FUNC + HOME: RENAME PROJECT");
        break;
    case B_SAVE:
        L("TAP: SAVE HERE? OR ANOTHER SLOT"); L("+ KNOB: FORGET ITS MOTION");
        if (tonic_part()) L("FUNC + SAVE: SAVE NOW, SAME SLOT");
        L(proj.set.autosave_off ? "AUTOSAVE IS OFF" : "STOPPED: SAVES BY ITSELF");
        break;
    case B_SEQ:
        L("THE PART'S STEPS: FIXED LENGTH 8 16 32 64"); L("AUTOSCROLL, FREE LENGTH, SWING"); L("HOLD A STEP + SELECT: ITS CHANCE");
        if (tonic_part()) L("FUNC + SEQ: SONG MODE ON / OFF");
        if (tonic_part()) L("FILL + SEQ: BACK TO THE SONG");
        break;
    case B_PLAY:
        L("START / STOP");
        if (tonic_part()) { L("FUNC + PLAY: CLEAR THE PATTERN"); L("FUNC + TRACK + PLAY: CLEAR TRACK"); L("FUNC + EDIT + PLAY: RESET PAGE"); L("FUNC + ACCENT / FILL: COPY / PASTE"); }
        break;
    case B_REC:
        L("RECORD ON / OFF:"); L("BLACK KEYS, KNOB MOVES");
        L("FUNC + REC: QUANTIZED / UNQUANTIZED");
        break;
    case B_OCTDN:
    case B_OCTUP:
        if (tonic_part() && ui.view == V_SEQ && ui.arp) { L(b == B_OCTUP ? "KEYBOARD OCTAVE UP" : "KEYBOARD OCTAVE DOWN"); L("THE VOICE PITCH STAYS"); }   /* wip693 */
        else if (tonic_part() && ui.view != V_SONG) { L(b == B_OCTUP ? "VOICE PITCH: OCTAVE UP" : "VOICE PITCH: OCTAVE DOWN"); L("THE SELECTED TRACK"); }
        else if (ui.view == V_SONG) L(b == B_OCTUP ? "ADD A LINE AFTER THE CURSOR" : "DELETE THE LINE");
        else if (ui.view == V_HOME) { L(b == B_OCTUP ? "VOICE PITCH: OCTAVE UP" : "VOICE PITCH: OCTAVE DOWN"); L("FUNC + SELECT: PATTERN BANK 1 / 2"); }   /* wip689: the bank is FUNC + SELECT now */
        else {
            L(b == B_OCTUP ? "THE NEXT 16 STEPS" : "THE 16 STEPS BEFORE"); L("PAGES: 1-16 17-32 33-48 49-64");
        }
        break;
    default: break;
    }
#undef L
    return n;
}

static void set_view(int v);
#define LONG_MS 500u                                     /* wip223: a long press of FX / HOME */
static void help_tick(uint32_t btn, uint32_t keys)
{
    uint32_t now = plat_ms();
    int i;
    if (ui.help) {
        uint32_t m = 1u << (ui.help - 1);
        if (btn != m || keys || (int32_t)(ui.turn_t - ui.down_t[ui.help - 1]) > 0 || ui.overlay == O_ASK)
            ui.help = 0;                                 /* used with something, or let go */
        return;
    }
    if (!btn || (btn & (btn - 1u)) || keys || ui.overlay == O_ASK || perf_testing())
        return;                                          /* exactly one button, nothing else */
    for (i = B_FX; i <= B_HOME; i += B_HOME - B_FX)   /* wip223: FX held alone = the REVERB page, HOME held alone = the DELAY page (no help card for these two) */
        if (btn == 1u << i && !(ui.btn_used & btn) && ui.overlay == O_NONE && now - ui.down_t[i] >= LONG_MS &&
            (int32_t)(ui.turn_t - ui.down_t[i]) <= 0) {
            *page_of(V_FX) = (uint8_t)(i == B_FX ? 1 : 2);
            set_view(V_FX);
            ui.btn_used |= btn;                          /* its release now does nothing */
            return;
        }
    for (i = 0; i < NB; i++)
        if (i != B_FX && i != B_HOME && btn == 1u << i && !(ui.btn_used & btn) && now - ui.down_t[i] >= HELP_HOLD &&
            (int32_t)(ui.turn_t - ui.down_t[i]) <= 0) {
            ui.help = (uint8_t)(i + 1);
            ui.btn_used |= btn;                          /* its release now does nothing */
        }
}

/* ================================================================ undo === */
/* wip417: GLO + ACCENT undoes, GLO + FILL redoes (undo.c; was HOME + REC / HOME + PLAY): everything after the settings is undoable, the
 * sound, the patterns, the song and the knob motion. A step is one gesture: what changed between
 * two quiet moments (UNDO_QUIET ms with nothing touched or held), or a whole recording pass. */
#define UNDO_QUIET 400u
#define UNDO_OFF __builtin_offsetof(project_t, sound)
#define UNDO_END __builtin_offsetof(project_t, note)   /* wip256: the ARP notes are NOT undoable (8 KB of pool RAM) */
static undo_t undo X0X_POOL;
static uint8_t undo_shadow[UNDO_END - UNDO_OFF] X0X_POOL;
static uint8_t undo_lanes;             /* the last undo / redo touched the knob motion */

static void undo_start(void) { undo_init(&undo, (uint8_t *)&proj + UNDO_OFF, undo_shadow, sizeof undo_shadow); }

static void undo_tick(void)
{
    if (perf_testing() || (ui.rec && seq.playing) || ui.btn || ui.keys || plat_ms() - ui.act_t < UNDO_QUIET ||
        (ui.frame & 3u))
        return;
    undo_commit(&undo);
}

/* a run of the project changed under the engine: the sound's pots go to the engine again */
static void undo_changed(uint32_t off, uint32_t len)
{
    const uint32_t s0 = __builtin_offsetof(project_t, sound), s1 = s0 + sizeof(sound_t);
    const uint32_t l0 = __builtin_offsetof(project_t, arr) + __builtin_offsetof(arrange_t, lane);
    uint32_t a = off + UNDO_OFF, k;
    for (k = a; k < a + len; k++) {
        if (k >= s0 && k < s1) {
            uint32_t x = k - s0, t = x / (NVOICES_MAX * NPARAMS_MAX), v = (x / NPARAMS_MAX) % NVOICES_MAX,
                     i = x % NPARAMS_MAX;
            if ((int)v < engine_nvoices((int)t) && (int)i < engine_nparams((int)t, (int)v))
                engine_set((int)t, (int)v, (int)i, proj.sound.v[t][v][i]);
        } else if (k >= l0 && k < __builtin_offsetof(project_t, note)) {   /* (wip252: the notes are not the knob motion) */
            undo_lanes = 1;
        }
    }
}

/* what an undo step touched, in words: "909 P3", "SOUND", "SONG", "MOTION" */
static void undo_name(uint32_t lo, uint32_t hi, char *t)
{
    const uint32_t p0 = __builtin_offsetof(project_t, pat), a0 = __builtin_offsetof(project_t, arr),
                   l0 = a0 + __builtin_offsetof(arrange_t, lane), n0 = __builtin_offsetof(project_t, note);
    uint32_t a = lo + UNDO_OFF, b = hi + UNDO_OFF;
    if (a >= n0) {                                    /* wip252: the ARP notes */
        put_s(t, "NOTES");
    } else if (b < p0) {
        put_s(t, "SOUND");
    } else if (a >= p0 && b < a0 && (a - p0) / sizeof(pattern_t) == (b - p0) / sizeof(pattern_t)) {
        uint32_t pi = (a - p0) / sizeof(pattern_t), x = (a - p0) % sizeof(pattern_t), y = (b - p0) % sizeof(pattern_t);
        int pa = (int)(x < __builtin_offsetof(pattern_t, brk) ? x / sizeof(dpart_t) : PART_BRK);
        int pb = (int)(y < __builtin_offsetof(pattern_t, brk) ? y / sizeof(dpart_t) : PART_BRK);
        char *q = put_s(t, pa == pb ? PART_N[pa] : "PATTERN");
        put_i(put_s(q, " P"), (int)pi + 1);
    } else if (a >= p0 && b < a0) {
        put_s(t, "PATTERNS");
    } else if (a >= a0 && b < l0) {
        put_s(t, "SONG");
    } else if (a >= l0 && b < n0) {
        put_s(t, "MOTION");
    } else {
        put_s(t, "CHANGES");
    }
}

static void do_undo(int redo)
{
    uint32_t lo, hi;
    char t[24];
    undo_commit(&undo);                              /* what was just done is a step too */
    undo_lanes = 0;
    if (!(redo ? undo_redo : undo_undo)(&undo, undo_changed, &lo, &hi)) {
        say(redo ? "NOTHING TO REDO" : "NOTHING TO UNDO", 0);
        return;
    }
    if (undo_lanes)
        engine_motion_reset();
    engine_brk_loops();
    ui.outline_ok = 0;
    if (ui.song_sel > song()->len)
        ui.song_sel = song()->len;
    build_pages();
    mark_dirty();
    undo_name(lo, hi, t);
    say(redo ? "REDO " : "UNDO ", t);
}

/* SAVE while playing waits for the stop: writing the flash stops the audio (the code and the samples
 * are read from it, and a 4 KB erase takes tens of ms) */
static uint8_t save_wait;
static uint8_t save_hold;              /* a backup over USB is reading the flash (backup.c): saves wait */
static void save_project(void)
{
    int rc;
    if (seq.playing || seq.req_stop || save_hold) {
        save_wait = 1;
        say(save_hold ? "SAVES AFTER THE BACKUP" : "SAVES WHEN STOPPED", 0);
        return;
    }
    save_wait = 0;
    rc = project_save();
    if (rc)
        warn(rc == -7 ? "MEMORY FULL: NOT SAVED" : "SAVE FAILED", 0);
    else
        say("SAVED", 0);
    if (!rc)
        ui.dirty = ui.set_dirty = 0;
}

/* ============================================================ performance === */
/* PERF: what the FM-1 measures of itself, a window a second: the clock (the core's cycle counter
 * against the 24 MHz timer), the audio load and its peak, dropouts, stall cycles (when switched on),
 * and each stage's share of the CPU (the engine times them: engine.h ENG_PROF_N). PERF TEST plays
 * three patterns of its own (the factory loop, all five parts, a dense worst case) on the factory
 * sound, 1 s to settle and 3 s measured each, and keeps the table; then it puts back the project's
 * patterns, sound, tempo, mutes, chain and song mode. The project itself is never written. */
static const char *const PT_N[3] = {"FACTORY LOOP", "ALL FIVE", "WORST CASE"};
static pattern_t perf_bench[3] X0X_POOL;
static sound_t perf_snd X0X_POOL;

static void perf_window(int force)
{
    uint32_t now = plat_ms(), t = plat_ticks24(), c = plat_cycles(), f = eng_prof_frames, st[3];
    uint32_t dt = t - perf.ticks, dc = c - perf.cyc, df = f - perf.frames, sum = 0;
    float ups;
    int k;
    if (!force && now - perf.t_ms < 1000u)
        return;
    perf.t_ms = now;
    if (perf.have && df) {
        if (plat_cycles_cpu() && dt >= 240u)
            perf.mhz10 = (uint16_t)(dc / (dt / 240u));          /* cycles per (dt / 24) us, x 10 */
        ups = plat_cycles_cpu() ? (float)perf.mhz10 * (100000.0f / 44100.0f) : (float)plat_cycles_hz() / 44100.0f;
        for (k = 0; k < ENG_PROF_N; k++) {
            uint32_t d = eng_prof[k] - perf.prof[k];
            sum += d;
            perf.stage[k] = (uint16_t)((float)d * 1000.0f / ((float)df * ups) + 0.5f);
        }
        perf.cps = plat_cycles_cpu() ? (uint16_t)(sum / df) : 0;
        perf.load = (uint8_t)plat_cpu_pct();
        perf.peak = (uint8_t)plat_cpu_peak_pct();
        if (plat_stalls(st) == 0 && dc)
            for (k = 0; k < 3; k++)
                perf.stall[k] = (uint8_t)((float)(st[k] - perf.st[k]) * 100.0f / (float)dc + 0.5f);
    } else {
        plat_cpu_peak_pct();
    }
    perf.ticks = t;
    perf.cyc = c;
    perf.frames = f;
    for (k = 0; k < ENG_PROF_N; k++)
        perf.prof[k] = eng_prof[k];
    if (plat_stalls(st) == 0)
        for (k = 0; k < 3; k++)
            perf.st[k] = st[k];
    perf.have = 1;
}

/* the test's patterns: 0 the factory loop (909 kick, clap, open hats; 303 A; the break), 1 all five,
 * 2 everything dense */
static void perf_build(void)
{
    /* the tracks in key order (pattern.h): BD SD LT MT HT RS CP CH OH CR RD (the Tonic uses the first eight) */
    enum { DR_BD, DR_SD, DR_LT, DR_MT, DR_HT, DR_RS, DR_CP, DR_CH, DR_OH, DR_CR, DR_RD };
    int i;
    for (i = 0; i < 3; i++) {
        pattern_t *p = &perf_bench[i];
        dpart_t *a = &p->drum[0];
        pattern_init(p);
        p->brk.steps = 0xFFFFu;
        a->hit[DR_BD][0] = 0x1111u;
        a->hit[DR_CP][0] = 0x1010u;
        a->hit[DR_OH][0] = 0x4444u;
        a->accent[0] = 0x0101u;
        if (i == 2) {
            a->hit[DR_SD][0] = 0x1010u;
            a->hit[DR_CH][0] = 0xFFFFu;
            a->hit[DR_CR][0] = 0x0001u;
            a->hit[DR_RD][0] = 0x1111u;
            a->hit[DR_LT][0] = 0x4000u;
            a->accent[0] = 0x1111u;
        }
    }
}

static void perf_restore(void)
{
    int p;
    seq.pat = perf.pat;
    for (p = 0; p < NPARTS; p++) {
        seq.ppat[p] = perf.ppat[p];
        seq.pcue[p] = 0xFF;
    }
    seq.mute = perf.mute;
    seq.song_on = perf.song_on;
    seq_chain(&seq, perf.ca, perf.cb);
    seq.bpm = perf.bpm;
    engine_apply_sound(&proj.sound);
    engine_motion_enable(1);
    engine_guard_enable(1);
    engine_brk_loops();
}

static void perf_test_start(void)
{
    int p;
    if (perf.test >= 0 && perf.phase != 4)
        return;
    perf.pat = seq.pat;
    for (p = 0; p < NPARTS; p++)
        perf.ppat[p] = seq.ppat[p];
    perf.mute = seq.mute;
    perf.song_on = seq.song_on;
    perf.ca = seq.chain_a;
    perf.cb = seq.chain_b;
    perf.bpm = seq.bpm;
    perf_build();
    if (seq.playing)
        seq_stop(&seq);
    perf.test = 0;
    perf.phase = 0;
    for (p = 0; p < 3; p++)
        perf.res_load[p] = perf.res_cps[p] = perf.res_peak[p] = 0;
}

static void perf_test_abort(void)
{
    seq_stop(&seq);
    perf.phase = 3;
    perf.test = 0;
    say("PERF TEST STOPPED", 0);
}

/* the test, one step a frame (the main loop): the transport changes take a block to land */
static void perf_test_tick(void)
{
    uint32_t now = plat_ms();
    int p;
    if (perf.test < 0 || perf.phase == 4)
        return;
    switch (perf.phase) {
    case 0:                                           /* stopped: swap in the test's patterns and sound */
        if (seq.playing || seq.req_stop)              /* (a pending stop would cancel the start below) */
            return;
        for (p = 0; p < NPARTS; p++) {
            seq.ppat[p] = 0;
            seq.pcue[p] = 0xFF;
        }
        seq.pat = perf_bench;
        seq.mute = 0;
        seq.song_on = 0;
        seq_chain(&seq, 0, 0);
        seq.bpm = 125.0f;
        engine_sound_defaults(&perf_snd);
        engine_apply_sound(&perf_snd);
        engine_motion_enable(0);
        engine_guard_enable(0);
        engine_brk_loops();
        seq_start(&seq);
        perf.phase = 1;
        perf.test_t0 = now;
        break;
    case 1:                                           /* settle on the scenario's pattern */
        if (!seq.playing && now - perf.test_t0 > 200u) {
            perf_test_abort();
            return;
        }
        if (now - perf.test_t0 >= 1000u && seq.ppat[PART_909] == perf.test) {
            perf_window(1);
            perf.phase = 2;
            perf.test_t0 = now;
        }
        break;
    case 2:                                           /* measure 3 s */
        if (!seq.playing) {
            perf_test_abort();
            return;
        }
        if (now - perf.test_t0 >= 3000u) {
            int k;
            uint32_t sum = 0;
            perf_window(1);
            for (k = 0; k < ENG_PROF_N; k++)
                sum += perf.stage[k];
            perf.res_load[perf.test] = (uint16_t)sum;
            perf.res_peak[perf.test] = perf.peak;
            perf.res_cps[perf.test] = perf.cps;
            if (++perf.test < 3) {
                seq_cue(&seq, perf.test);
                perf.phase = 1;
                perf.test_t0 = now;
            } else {
                seq_stop(&seq);
                perf.phase = 3;
            }
        }
        break;
    default:                                          /* put everything back once stopped */
        if (seq.playing || seq.req_stop)
            return;
        perf_restore();
        perf.phase = 4;
        if (perf.test == 3) {
            perf.test = PT_DONE;
            say("PERF TEST DONE", 0);
        } else {
            perf.test = PT_IDLE;
        }
        break;
    }
}

static void set_view(int v);

/* PRESETS: ids 0-7 are the factory bank, 8-39 the user slots (SAVE AS) that hold something. The PRESETS knob steps
 * through the ones that exist; a question asks first. */
#define NFACT 8                                        /* places 1-8 keep their override sectors (PLAT_NPRESET + id); the storage mapping is unchanged */
#define NDEMO 2                                        /* wip655: only places 1-2 are DEMO presets (editable, from the compiled bank); places 3-8 are ordinary empty places (were factory presets) */
static uint8_t pbuf[PLAT_STORE_MAX] X0X_POOL;                   /* the user preset being looked at */
static const uint8_t *preset_blob(int id, uint32_t *n)
{
    if (id < NFACT) {
        int r = plat_preset_load(PLAT_NPRESET + id, pbuf, sizeof pbuf);     /* wip255 (option B, B1): a preset written over this number wins */
        if (r >= (int)TNP_SIZE(1) && pbuf[0] == 'T') {
            *n = (uint32_t)r;
            return pbuf;
        }
        if (id >= NDEMO || id >= TONIC_BANK_N)         /* wip655: no demo behind places 3-8 */
            return 0;
        *n = TONIC_BANK[id].n;
        return TONIC_BANK[id].p;
    }
    {
        int r = plat_preset_load(id - NFACT, pbuf, sizeof pbuf);
        if (r < (int)TNP_SIZE(1) || pbuf[0] != 'T')
            return 0;
        *n = (uint32_t)r;
        return pbuf;
    }
}

/* An EMPTY user place (9-40) loads as: the voices, tempo, swing and rate of a factory preset (the base sound), NO steps at all.
 * wip358: the place's base is the factory preset (place - 9) mod 8 + 1: 9 = like 1 ... 16 = like 8, 17 = like 1 again ... 40 = like 8. */
static uint8_t pblank[TNP_SIZE(1)];
static const uint8_t *preset_blank(uint32_t *n, int place)   /* place = 0-based, 8-39 */
{
    uint32_t i, hdr = TNP_HDR + 8u * TNP_VOICE, src;
    if (TONIC_BANK_N < 1)
        return 0;
    src = (uint32_t)place % 8u % (uint32_t)TONIC_BANK_N;     /* wip655: = (place - 8) mod 8 for place >= 8 as before; places 3-8 (place 2-7) take the sound of factory 3-8, which they used to be */   /* (a bank of fewer than 8 presets, as in the host tests: round again) */
    if (TONIC_BANK[src].n < TNP_SIZE(1))
        return 0;
    for (i = 0; i < TNP_SIZE(1); i++)
        pblank[i] = TONIC_BANK[src].p[i];
    pblank[4] = (uint8_t)TNP_SIZE(1);                  /* total length u16 */
    pblank[5] = (uint8_t)(TNP_SIZE(1) >> 8);
    pblank[9] = 1;                                     /* one pattern */
    pblank[11] = 0;                                    /* nothing muted */
    for (i = 16; i < 16 + TNP_NAME_LEN; i++)           /* no name */
        pblank[i] = 0;
    pblank[hdr + 1] = 0;                               /* not chained */
    for (i = hdr + TNP_PAT_HDR; i < TNP_SIZE(1); i++)  /* no hits, no accents, no fills */
        pblank[i] = 0;
    *n = TNP_SIZE(1);
    return pblank;
}

#define NPRESETS (NFACT + PLAT_NUSER)                  /* wip654: the 30 places: 1-8 factory, 9-30 user slots (were 40) */
static int preset_next(int from, int dir)              /* the next place, from + dir, round and round (an empty one too) */
{
    return (from + dir + NPRESETS) % NPRESETS;
}

static void preset_label(int id, char *out)            /* "01 NAME" ... "40 NAME": one list, 1-8 factory, 9-40 user slots */
{
    uint32_t n;
    const uint8_t *b = preset_blob(id, &n);
    char *q = out;
    int k, num = id + 1;
    if (num < 10)
        *q++ = '0';
    q = put_i(q, num);
    *q++ = ' ';
    for (k = 0; b && k < (int)TNP_NAME_LEN && b[16 + k] && q - out < 27; k++)
        *q++ = (char)(b[16 + k] >= 'a' && b[16 + k] <= 'z' ? b[16 + k] - 32 : b[16 + k]);   /* the screen's names are capitals */
    if (!b)                                            /* nothing saved in that place */
        q = put_s(q, "EMPTY");                         /* wip308: as before (wip307 showed PROJECT n) */
    *q = 0;
}

/* wip229: the name of the loaded project (= preset) alone, capitals, for the GLO mixer's header chip; "MASTER" when none was loaded since power on */
static const char *preset_name_cur(char *out)
{
    uint32_t n;
    const uint8_t *b = ui.preset_cur ? preset_blob(ui.preset_cur - 1, &n) : 0;
    int k = 0;
    while (b && k < (int)TNP_NAME_LEN && b[16 + k] && k < 8) {
        out[k] = (char)(b[16 + k] >= 'a' && b[16 + k] <= 'z' ? b[16 + k] - 32 : b[16 + k]);
        k++;
    }
    out[k] = 0;
    return k ? out : "MASTER";
}

static void preset_ask(int id)
{
    char t[28];
    preset_label(id, t);
    ask(ACT_PRESET, id, "CHANGE PRESET?", t);
}

/* ---------------------------------------------------------------- SAVE AS --- */
/* SAVE: three screens. 1 "SAVE AS" with a made-up name (ALGORITHM: another; OCT+ goes on; OCT- cancels); 2 the name
 * alone, big (ALGORITHM: another; OCT+ SAVE goes on; OCT- back); 3 the slot, 9-40 (ALGORITHM turns; OCT+ saves; OCT- back).
 * The name is an elf-like word of 4 to 8 letters: a start, a middle and an end from short lists. */
static uint32_t sv_rng = 0x2F6E2B1u;
static uint32_t sv_rand(void)
{
    sv_rng ^= sv_rng << 13;
    sv_rng ^= sv_rng >> 17;
    sv_rng ^= sv_rng << 5;
    return sv_rng;
}

/* wip324: GLO > NAME PROJECTS = MANUAL: the name screen is a letter editor. The first letter (A) blinks; K4 turns through NM_SET
 * (the capitals, the digits, space, - . + # / % : ?); OCT+ accepts the letter and the next one appears; OCT- deletes the last accepted
 * letter (with none: back / cancel as before); from the first accepted letter PRESS SAVE blinks and the SAVE button saves
 * (8 letters at most: OCT+ saves after the eighth). Trailing spaces are dropped. */
static const char NM_SET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -.+#/%:?";   /* only what the big name font (FONT_M, tools/gen_font.py KEEP) has */
#define NM_N ((int)sizeof NM_SET - 1)
#define NM_MAX 8
static int name_manual_on(void)
{
    return proj.set.name_manual && ui.overlay == O_ASK && ui.ask_act == ACT_SAVEAS && ui.ask_arg == 1;
}
static void name_enter(void)                          /* the name screen just opened */
{
    if (!proj.set.name_manual)
        return;
    ui.nm_len = 0;
    ui.nm_ch = 0;
    ui.sv_name[0] = 0;
}
static int name_trim(void)                            /* drops trailing spaces; the length left (0 = nothing to save) */
{
    while (ui.nm_len && ui.sv_name[ui.nm_len - 1] == ' ')
        ui.sv_name[--ui.nm_len] = 0;
    return ui.nm_len;
}
static void name_turn(int d)
{
    if (ui.nm_len < NM_MAX)
        ui.nm_ch = (uint8_t)((ui.nm_ch + NM_N + (d > 0 ? 1 : -1)) % NM_N);
}
static int name_accept(void)                          /* OCT+: 1 = the letter was taken, 0 = the name is full (the caller saves) */
{
    if (ui.nm_len >= NM_MAX)
        return 0;
    ui.sv_name[ui.nm_len++] = NM_SET[ui.nm_ch];
    ui.sv_name[ui.nm_len] = 0;
    ui.nm_ch = 0;
    return 1;
}
static int name_delete(void)                          /* OCT-: 1 = a letter was taken back (it blinks again), 0 = none left */
{
    int i;
    if (!ui.nm_len)
        return 0;
    ui.nm_len--;
    for (i = 0; i < NM_N; i++)
        if (NM_SET[i] == ui.sv_name[ui.nm_len])
            ui.nm_ch = (uint8_t)i;
    ui.sv_name[ui.nm_len] = 0;
    return 1;
}

static char name_cand(void)                           /* the blinking letter as shown (a space shows as an underscore) */
{
    char c = NM_SET[ui.nm_ch];
    return c == ' ' ? '_' : c;
}
static void name_shown(char *t)                       /* the letters so far + the one that is turning (no blink: the small screen) */
{
    int i;
    for (i = 0; i < (int)ui.nm_len && i < NM_MAX; i++)
        t[i] = ui.sv_name[i];
    if (ui.nm_len < NM_MAX)
        t[i++] = name_cand();
    t[i] = 0;
}

static void elf_name(char *out)
{
    static const char *const ON[] = {"K", "L", "M", "N", "S", "T", "V", "TH", "EL", "AR", "CAL", "FA", "NI", "LO", "AE", "GA", "IR", "SY", "EN", "OR"};
    static const char *const MI[] = {"A", "E", "I", "O", "U", "AE", "IA", "AR", "EL", "AN", "IL", "OR", "EA", "IE"};
    static const char *const EN[] = {"MA", "RA", "N", "TH", "RIEL", "WEN", "ION", "DOR", "LAS", "LIN", "RE", "A", "EL", "IS", "YS", "DIL", "MIR", "LOR"};
    int tries;
    for (tries = 0; tries < 16; tries++) {
        char t[16], *q = t;
        const char *p;
        for (p = ON[sv_rand() % 20u]; *p; )
            *q++ = *p++;
        for (p = MI[sv_rand() % 14u]; *p; )
            *q++ = *p++;
        for (p = EN[sv_rand() % 18u]; *p; )
            *q++ = *p++;
        *q = 0;
        if (q - t >= 4 && q - t <= 8) {
            for (q = t; (*out++ = *q++) != 0; )
                ;
            return;
        }
    }
    put_s(out, "ELENAR");
}

static void sv_here_look(void)                         /* wip319: who is in the place SAVE HERE? points at (capitals; "" = empty) */
{
    uint32_t n;
    const uint8_t *b = preset_blob(ui.sv_here - 1, &n);
    int k;
    for (k = 0; b && k < (int)TNP_NAME_LEN && b[16 + k]; k++)
        ui.sv_occ[k] = (char)(b[16 + k] >= 'a' && b[16 + k] <= 'z' ? b[16 + k] - 32 : b[16 + k]);
    ui.sv_occ[b ? k : 0] = 0;
}

/* wip343: the slot screen of SAVE covers ALL 40 places (1-8 too, they are written to their override slot). It starts at the NEXT EMPTY place after the loaded
 * one (round: 40 then 1 ...); when every place is full, at the place after the loaded one. ALGORITHM turns 1 .. 40 and round. ui.sv_here holds the place. */
static int next_free_place(int from)                    /* from = the loaded place 1-40 (0 = none): the first empty place after it (round), else the next one */
{
    int k, p;
    for (k = 1; k <= NPRESETS; k++) {
        uint32_t n;
        p = (from - 1 + k + NPRESETS) % NPRESETS + 1;
        if (p > NDEMO && !preset_blob(p - 1, &n))      /* (1-8 hold the factory presets: never "empty") */
            return p;
    }
    return from % NPRESETS + 1;
}
static void sv_sync(void)                               /* ui.sv_slot (0-31, the host's probe) follows the place */
{
    ui.sv_slot = ui.sv_here > NFACT ? (uint8_t)(ui.sv_here - 1 - NFACT) : 0;
}

#define SV_HERE 3                                      /* wip317: the normal SAVE: SAVE HERE? (OCT+ YES keeps the name, OCT- NO) */
#define SV_CHOOSE 4                                    /* ... CHOOSE SLOT (OCT+ the slot screen, OCT- EXIT SAVE) */
static void saveas_open(void)
{
    int s, found = 0;
    sv_rng ^= plat_ms() * 2654435761u + 0x9E3779B9u;
    if (!sv_rng)
        sv_rng = 1;
    elf_name(ui.sv_name);
    ui.sv_fixed = 0;
    ui.sv_rename = 0;
    for (s = 0; s < PLAT_NUSER && !found; s++) {      /* the first empty slot (else 1) */
        uint32_t n;
        if (!preset_blob(NFACT + s, &n)) {
            ui.sv_slot = (uint8_t)s;
            found = 1;
        }
    }
    if (!found)
        ui.sv_slot = 0;
    ui.sv_occ[0] = 0;
    ui.sv_pn[0] = 0;
    ui.sv_here = ui.preset_cur;
    if (ui.preset_cur) {                                /* wip317: SAVE HERE? shows the loaded place; no place loaded yet: CHOOSE SLOT */
        uint32_t n;
        const uint8_t *b = preset_blob(ui.preset_cur - 1, &n);
        int k;
        for (k = 0; b && k < (int)TNP_NAME_LEN && b[16 + k]; k++)
            ui.sv_pn[k] = (char)(b[16 + k] >= 'a' && b[16 + k] <= 'z' ? b[16 + k] - 32 : b[16 + k]);
        ui.sv_pn[k] = 0;
        for (k = 0; ui.sv_pn[k]; k++)                   /* wip319: the loaded place is the one that would be replaced */
            ui.sv_occ[k] = ui.sv_pn[k];
        ui.sv_occ[k] = 0;
    }
    ui.sv_newname = 0;
    if (!ui.preset_cur) {                               /* wip708: no place loaded: SAVE HERE? opens at the first empty place with a made-up name */
        ui.sv_here = (uint8_t)next_free_place(0);
        sv_sync();
        sv_here_look();
        put_s(ui.sv_pn, ui.sv_name);
        ui.sv_newname = 1;
    }
    ask(ACT_SAVEAS, SV_HERE, "", "");   /* wip708: ONE screen, SAVE HERE?: SELECT = the spot, ALGORITHM = the name, OCT+ saves, OCT- cancels (CHOOSE SLOT and the slot screen are gone). ask_arg 1 = the letter editor (MANUAL names) */
}

/* wip652: SAVE AS / QUICK SAVE pack into pbuf (the preset-viewing buffer above, 3840 bytes, already in the pool): a save never runs while a
 * preset is being read (quick_write_to copies the name out of it BEFORE packing and does not touch it afterwards), so no second 3640-byte buffer.
 * (wip651 had a separate one: the pool headroom check failed, 5172 B < 8192 B) */
typedef char pbuf_big_enough[(sizeof pbuf >= TNP_SIZE(TNP_NPAT_SAVE) + TNP_FX3_LEN) ? 1 : -1];

/* wip657 (E2b of "a project = 2 spots"): a project is saved into its place as TWO spots: the main one (the TNP1 blob: sounds, patterns 1-16, FX) and the second
 * one (project_ext_pack: the song, the motion lanes and the ARP notes, lz-packed, plat_ext_*, same slot number). One buffer only (pbuf: the pool has no room for
 * a second): 1 the second spot is MEASURED first (a size, no buffer): too big = PLACE_FULL and NOTHING is written, the main spot too; 2 the main spot is packed
 * into pbuf and written; 3 the second spot is packed into the same pbuf and written. A flash code (-3 NO FLASH SPACE, others) comes back as it is. */
#define PLACE_FULL (-100)
static int place_save(int slot, const char *name)
{
    uint32_t n, mark;
    int rc;
    if (!project_ext_pack(0, PLAT_STORE_MAX, 0))
        return PLACE_FULL;
    n = project_preset_pack(pbuf, sizeof pbuf, name, (unsigned)(seq.bpm * 10.0f + 0.5f), (unsigned)((seq.vmute & 0xFFu) | ((unsigned)seq.choke << 8)));
    if (!n)
        return -1;
    mark = project_pair_mark(pbuf, n);                 /* wip659: the second spot carries the main spot's CRC: a half-finished save is recognised on load (project_ext_unpack) */
    if ((rc = plat_preset_save(slot, pbuf, n)) != 0)
        return rc;
    n = project_ext_pack(pbuf, PLAT_STORE_MAX, mark);
    return n ? plat_ext_save(slot, pbuf, n) : PLACE_FULL;
}

static void saveas_go(void)                            /* OCT+ on the third screen */
{
    int rc, slot = ui.sv_here - 1 < NFACT ? PLAT_NPRESET + ui.sv_here - 1 : ui.sv_here - 1 - NFACT, cur = ui.sv_here;   /* wip343: any of the 40 places (1-8: their override slot) */
    if (ui.sv_rename && ui.preset_cur) {               /* wip317: RENAME: the loaded place; 1-8 are written to their override slot (PLAT_NPRESET + n - 1), preset_cur stays */
        int id = ui.preset_cur - 1;
        slot = id < NFACT ? PLAT_NPRESET + id : id - NFACT;
        cur = ui.preset_cur;
    }
    /* wip317: no STOP FIRST any more: every SAVE works while playing (the flash write may make the sound hiccup) */
    rc = place_save(slot, ui.sv_name);                 /* wip657: both spots */
    ui.overlay = O_NONE;
    if (rc == PLACE_FULL)
        warn("MEMORY FULL: NOT SAVED", 0);
    else if (rc == -3)
        warn("NO FLASH SPACE YET", 0);
    else if (rc)
        warn("SAVE FAILED", 0);
    else {
        ui.preset_cur = (uint8_t)cur;
        proj.set.preset_cur = ui.preset_cur;           /* wip309: kept over power-off */
        say(ui.sv_rename ? "RENAMED " : "SAVED ", ui.sv_name);
        rc = project_save();                           /* wip227: the SAVE button also keeps the working project, and with it the compressor, */
        if (rc)                                        /* reverb and delay as they are now (the autosave never keeps those) */
            warn(rc == -7 ? "MEMORY FULL: NOT SAVED" : "SAVE FAILED", 0);
        else
            ui.dirty = ui.set_dirty = 0;
    }
}

/* wip263: FUNC + SAVE = QUICK SAVE: writes the project into the preset place that is loaded now (the same number, the same name), at once,
 * no question. Works while playing too (wip286; the flash write may make the sound hiccup); no place loaded since power on: it says so and saves nothing (SAVE asks for a place). An EMPTY place
 * has no name yet: it gets a made-up one, as SAVE AS does. Places 1-8 are written to their override slot (PLAT_NPRESET + n - 1, wip255). */
static int untitled_is(int slot, const char *nm)       /* wip308: does the storage slot (0-21 user places 9-30, 32-39 places 1-8) hold exactly this name (any case)? */
{
    uint32_t n;
    const uint8_t *b = preset_blob(NFACT + slot, &n);
    int k;
    if (!b)
        return 0;
    for (k = 0; nm[k]; k++) {
        char c = (char)b[16 + k];
        if (c >= 'a' && c <= 'z')
            c = (char)(c - 32);
        if (c != nm[k])
            return 0;
    }
    return k < (int)TNP_NAME_LEN ? b[16 + k] == 0 : 1;
}

static void untitled_name(char *out)                   /* wip308: UNTITLED 01, 02, 03 ...: the lowest number no place uses */
{
    int v, s;
    for (v = 1; v < 100; v++) {
        char *q = put_s(out, "UNTITLED ");
        int used = 0;
        if (v < 10)
            *q++ = '0';
        *put_i(q, v) = 0;
        for (s = 0; s < PLAT_NUSER + NFACT && !used; s++)   /* wip663: the user slots (places 9-30) AND the override slots (places 1-8: places 3-8 are ordinary user places since 655, a quick save into them is named UNTITLED nn too) */
            used = untitled_is(s < PLAT_NUSER ? s : PLAT_NPRESET + (s - PLAT_NUSER), out);
        if (!used)
            return;
    }
}

static void quick_write_to(int num, const char *forced);
static void quick_write(void)
{
    quick_write_to(ui.preset_cur, 0);
}

static void quick_write_to(int num, const char *forced)   /* wip319: num = the place (1-40); forced = the name to give it (0: the place's own name) */
{
    char name[TNP_NAME_LEN + 1];
    uint32_t n;
    int id, slot, k = 0, rc;
    const uint8_t *b;
    id = num - 1;                                      /* (wip286: no "STOP FIRST": the quick save also works while playing) */
    slot = id < NFACT ? PLAT_NPRESET + id : id - NFACT;
    b = forced ? 0 : preset_blob(id, &n);
    if (forced) {
        for (; forced[k] && k < (int)TNP_NAME_LEN; k++)
            name[k] = forced[k];
    } else {
        for (; b && k < (int)TNP_NAME_LEN && b[16 + k]; k++)
            name[k] = (char)b[16 + k];
    }
    name[k] = 0;
    if (!k)                                            /* an empty place: UNTITLED 01, 02, 03 ... (wip308) */
        untitled_name(name);
    rc = place_save(slot, name);                       /* wip657: both spots */
    if (rc == PLACE_FULL)
        warn("MEMORY FULL: NOT SAVED", 0);
    else if (rc == -3)
        warn("NO FLASH SPACE YET", 0);
    else if (rc)
        warn("SAVE FAILED", 0);
    else {
        char nm[TNP_NAME_LEN + 1];
        ui.preset_cur = (uint8_t)num;                  /* wip319: saved into another place: that one is the loaded one now */
        proj.set.preset_cur = ui.preset_cur;
        for (k = 0; name[k]; k++)                      /* the screen's names are capitals */
            nm[k] = name[k] >= 'a' && name[k] <= 'z' ? (char)(name[k] - 32) : name[k];
        nm[k] = 0;
        say("SAVED ", nm);
        rc = project_save();                           /* the compressor, reverb and delay too, as SAVE AS does */
        if (rc)
            warn(rc == -7 ? "MEMORY FULL: NOT SAVED" : "SAVE FAILED", 0);
        else
            ui.dirty = ui.set_dirty = 0;
    }
}

/* wip317: FUNC + HOME = RENAME PROJECT: the name screen alone (ALGORITHM = another name, OCT+ saves, OCT- / HOME cancel). The project is saved into the
 * place that is loaded now (1-8 too: saveas_go() maps them to their override slot) under the new name. No place loaded since power on: NO SLOT YET. */
static void rename_open(void)
{
    if (!ui.preset_cur) {
        warn("NO SLOT YET: USE SAVE", 0);
        return;
    }
    sv_rng ^= plat_ms() * 2654435761u + 0x9E3779B9u;
    if (!sv_rng)
        sv_rng = 1;
    elf_name(ui.sv_name);
    ui.sv_fixed = 1;
    ui.sv_rename = 1;
    ui.sv_slot = ui.preset_cur > NFACT ? (uint8_t)(ui.preset_cur - 1 - NFACT) : 0;   /* (only for the tests; saveas_go() uses preset_cur) */
    ask(ACT_SAVEAS, 1, "", "");
    name_enter();                                      /* wip324: MANUAL: the letter editor starts empty */
}

/* wip265: FUNC + SAVE decides: an EMPTY place -> SAVE AS from its name screen (a made-up name, ALGORITHM another, OCT+ OCT+ saves) with the slot
 * fixed = the loaded place (no slot screen); a used place (1-8 included) -> "OVERWRITE PROJECT?" OCT- NO / OCT+ YES, YES saves at once, the name kept. */
static void quick_save(void)
{
    uint32_t n;
    const uint8_t *b;
    int id, k;
    if (!ui.preset_cur) {                              /* (wip286: no "STOP FIRST": the quick save also works while playing) */
        warn("NO SLOT YET: USE SAVE", 0);
        return;
    }
    id = ui.preset_cur - 1;
    b = preset_blob(id, &n);
    if (!b && id >= NDEMO) {                           /* wip307/308: empty: saved at once (no name screens, no question), named UNTITLED nn */
        quick_write();
    } else {
        char t[TNP_NAME_LEN + 1];
        for (k = 0; b && k < (int)TNP_NAME_LEN && b[16 + k]; k++)
            t[k] = (char)(b[16 + k] >= 'a' && b[16 + k] <= 'z' ? b[16 + k] - 32 : b[16 + k]);
        t[k] = 0;
        ask(ACT_QSAVE, 0, "OVERWRITE SLOT?", t);
    }
}

/* SONG PATTERNS screen (ACT_CHAIN; wip211: FUNC + OCT+ no longer opens it, only SONG MODE with an empty song does). White keys 1-16 add a pattern to the chain (the same one as often as you
 * like), OCT- takes the last one off (empty: cancel), OCT+ makes it the SONG (one bar per pattern, every part) and
 * shows it on the song page; HOME cancels. */
static void chain_open(void)
{
    ui.ch_n = 0;
    ask(ACT_CHAIN, 0, "", "");
}

static void chain_go(void)
{
    song_t *sg = song();
    int i, p;
    if (seq.playing || seq.req_stop) {
        warn("STOP FIRST", 0);
        return;
    }
    if (!ui.ch_n) {
        warn("PRESS A PATTERN KEY FIRST", 0);
        return;
    }
    for (i = 0; i < ui.ch_n; i++) {
        for (p = 0; p < NPARTS; p++)
            sg->bar[i].pat[p] = ui.ch_pat[i];
        sg->bar[i].mute = 0;
    }
    sg->len = ui.ch_n;
    ui.song_sel = 0;
    ui.overlay = O_NONE;
    mark_dirty();
    set_view(V_SONG);
    ui.songmode = 1;                                    /* a chain is made: song mode, PLAY plays it */
    seq.chain_on = 1;
    say("SONG SET", 0);
}

/* SONG MODE: FUNC + SEQ asks "ENTER SONG MODE - PRESS [OCT+] CONFIRM". OCT+: a chain already made (the song has bars) goes
 * to the SONG page and plays in that chain from bar 1; no chain yet opens the CHAIN screen. OCT- / HOME cancel. */
static void songmode_go(void)
{
    song_t *sg = song();
    if (seq.playing || seq.req_stop) {
        warn("STOP FIRST", 0);
        return;
    }
    if (!sg->len) {
        chain_open();
        return;
    }
    ui.overlay = O_NONE;
    ui.song_sel = 0;
    ui.songmode = 1;
    seq.chain_on = 1;                                   /* the song advances (SONG ON); K1 of the pattern page can pause it */
    set_view(V_SONG);
    seq.song_on = 1;
    seq.song_start = 0;
    seq_start(&seq);
    say("SONG MODE", 0);
}

static void songexit_go(void)                         /* EXIT SONG MODE? confirmed: playing stops, back to the patterns */
{
    ui.overlay = O_NONE;
    ui.songmode = 0;
    seq.song_on = 0;                                    /* wip372: song mode is really off: the song no longer runs ... */
    seq.chain_on = 0;                                   /* ... and the pattern page says SONG OFF */
    if (seq.playing)
        seq_stop(&seq);
    ui.arp = 0;
    *page_of(V_SEQ) = 0;
    set_view(V_SEQ);                                    /* wip372: lands on the SEQ page (first press), not on the old HOME patterns page */
    say("SONG MODE OFF", 0);
}

static int load_preset_ex(const uint8_t *b, uint32_t n, int ext_slot);   /* wip659: app_load_preset + the place's second spot (ext_slot >= 0) */
static void run_action(int act, int arg)
{
    char t[28], *q;
    switch (act) {
    case ACT_PRESET: {
        uint32_t pn = 0;
        const uint8_t *pb = preset_blob(arg, &pn);
        int rc, es = -1;
        if (pb == pbuf)                                /* wip659: a place read from the storage (a user place, or 1-8 with an override) has a second spot: song, lanes, notes. A demo (1-2 never saved), an empty place: none, they load empty as before */
            es = arg < NFACT ? PLAT_NPRESET + arg : arg - NFACT;
        if (!pb && arg >= NDEMO)                       /* an empty user place: the sound of factory preset (place - 9) mod 8 + 1, no pattern (wip358) */
            pb = preset_blank(&pn, arg);
        rc = pb ? load_preset_ex(pb, pn, es) : -1;
        if (rc == 10)
            warn("STOP FIRST", 0);
        else if (rc)
            warn("PRESET FAILED", 0);
        else
        {
            ui.preset_cur = (uint8_t)(arg + 1);
            proj.set.preset_cur = ui.preset_cur;       /* wip309/310: kept over power-off: written to the flash AT ONCE with the project (stopped here), not left to the autosave */
            if (project_autosave() == 0)
                ui.dirty = ui.set_dirty = 0;
            else
                mark_dirty();
        }
        break;
    }
    case ACT_SAVE: save_project(); break;
    case ACT_CLEAR_TNPAT: tn_clear_pat(); break;
    case ACT_CLEAR_ALL: tn_clear_all(); break;
    case ACT_PASTE_TNPAT: tn_paste(arg); break;       /* wip416: PASTE PATTERN? yes (arg = the pattern pasted into) */
    case ACT_SONGBAR: song_del_cur(); break;           /* wip357: DELETE BAR? yes */
    case ACT_SONGKILL:                                /* wip314: DELETE SONG? yes: the song is gone and song mode is left */
        song()->len = 0;
        ui.song_sel = 0;
        seq.chain_on = 0;                             /* wip359: the song is gone: SONG OFF on the pattern page too (K1) */
        mark_dirty();
        songexit_go();
        say("SONG DELETED", 0);
        break;
    case ACT_CLEAR_TNTRK: tn_clear_track(arg); break;
    case ACT_AUTODEL: tn_clear_auto(arg); break;                  /* wip367 */
    case ACT_CLEAR_PAT: {
        int i;
        for (i = 0; i < NPARTS; i++) {
            clear_part(pat_of(i), i);
            clear_lanes(seq.ppat[i], i);
        }
        chain_after_clear();
        mark_dirty();
        say("PATTERN CLEARED", 0);
        break;
    }
    case ACT_SONG_INS:
    case ACT_SONG_DEL: {
        song_t *sg = song();
        int k = ui.song_sel, i;
        if (k >= sg->len)
            break;
        if (act == ACT_SONG_INS && sg->len < NSONG) {
            for (i = sg->len; i > k; i--)
                sg->bar[i] = sg->bar[i - 1];
            sg->len++;
        } else if (act == ACT_SONG_DEL) {
            for (i = k; i + 1 < sg->len; i++)
                sg->bar[i] = sg->bar[i + 1];
            sg->len--;
        }
        mark_dirty();
        say(act == ACT_SONG_INS ? "BAR INSERTED" : "BAR DELETED", 0);
        break;
    }
    case ACT_SONG_CLR:
        song()->len = 0;
        ui.song_sel = 0;
        mark_dirty();
        say("SONG CLEARED", 0);
        break;
    case ACT_RESET:
        project_defaults();
        engine_apply_sound(&proj.sound);
        engine_brk_loops();
        palette_set(proj.set.palette);
        mark_dirty();
        ui.outline_ok = 0;
        say("FACTORY SOUND + PATTERNS", 0);
        break;
    case ACT_PERF:
        set_view(V_PERF);
        break;
    case ACT_PERF_TEST:
        ui.overlay = O_NONE;                          /* the table is under the list */
        perf_test_start();
        say("PERF TEST: 15 SECONDS", 0);
        break;
    case ACT_ABOUT:
        q = put_s(t, "FM1 TONIC " X0X_VERSION "  MEM ");
        q = put_s(put_i(q, project_mem_pct()), "%  CPU ");
        put_s(put_i(q, (int)plat_cpu_pct()), "%");
        say(t, 0);
        break;
    default: break;
    }
}

/* ===================================================================== input === */
/* knob acceleration by speed: a slow turn is one step a detent (fine detail), a quick one up to 8,
 * so a fast half turn sweeps 0-127. Several detents in one read are a fast turn too. Small ranges
 * stay gentler; switches and lists never get here (turn_ref steps them one at a time). */
static uint8_t accel_off;              /* the host simulator's "spin": exact steps */
static int32_t accel(int role, int32_t s, int range)
{
    uint32_t now = plat_ms(), dt = now - ui.enc_t[role], a = (uint32_t)(s < 0 ? -s : s), m;
    ui.enc_t[role] = now;
    if (!a)
        return s;
    if (a > 1)
        dt /= a;                                      /* per detent */
    ui.enc_run[role] = (uint8_t)(dt < 60u ? (ui.enc_run[role] + a > 255u ? 255u : ui.enc_run[role] + a) : a);
    if (range <= 24 || accel_off || ui.enc_run[role] < 4u)
        return s;                                     /* a few clicks, however quick: one each */
    m = dt < 12u ? 8u : dt < 25u ? 5u : dt < 45u ? 3u : 1u;
    if (range < 100 && m > 3u)
        m = 3u;
    if (range > 150 && m > 1u)
        m *= 2u;                                      /* tempo and the other wide ones, when quick */
    return s * (int32_t)m;
}

/* wip696: the 2nd-page states of a view (MUTE / SOLO, DUCK, CHOKE, pattern page) are kept when you leave it, and put back by restore_sub() when a button brings you back */
static uint8_t sub_mem[NVIEWS];
static uint8_t home_mem;               /* wip696: the last page of the HOME group: 0 MIX 2, 1 PERFORM, 2 GLOBAL */
static uint8_t fx_pg;                  /* wip696: the effects page (V_FX page 1..4) last seen in the FX group (ui.fx_mem == 2) */

static uint8_t glo_mem;                /* wip697: the last page of the GLO group: 0 VEL, 1 MIXER, 2 COMP, 3 MEMORY, 4 SETTINGS list */

static void note_groups(void)          /* wip696: remember where each button group was left (called every frame and when HOME / FX is pressed) */
{
    if (ui.view == V_VEL && ui.overlay == O_NONE)
        glo_mem = 0;
    else if (ui.view == V_MIX && ui.overlay == O_NONE)
        glo_mem = 1;
    else if (ui.view == V_GLO)
        glo_mem = ui.overlay == O_NONE ? 2 : ui.overlay == O_MEM ? 3 : ui.overlay == O_LIST ? 4 : glo_mem;
    if (ui.overlay != O_NONE)
        return;
    switch (ui.view) {
    case V_MIX1: ui.fx_mem = 0; break;
    case V_MIX2:
        if (cur_page() == 0)
            home_mem = 0;
        else if (cur_page() == 1)
            ui.fx_mem = 1;
        break;
    case V_FX:
        if (cur_page() == 0)
            home_mem = 1;
        else {
            ui.fx_mem = 2;
            fx_pg = (uint8_t)cur_page();
        }
        break;
    case V_GLOB: home_mem = 2; break;
    default: break;
    }
}

static void restore_sub(int v)         /* wip696: after set_view(v) from a button: the 2nd-page state the view had when it was left */
{
    uint8_t m = sub_mem[v];
    if (v != ui.view)
        return;
    if (v == V_FX && cur_page() == 0)
        ui.msp = (uint8_t)(m & 1u);
    if (v == V_MIX2 && cur_page() == 1)
        ui.dck = (uint8_t)((m >> 1) & 1u);
    if (v == V_NEV)
        ui.chk = (uint8_t)((m >> 2) & 1u);
    if (v == V_SEQ && !ui.arp)
        ui.seqpat = (uint8_t)((m >> 3) & 1u);
}

static void set_view(int v)
{
    if (v != ui.view) {
        if (!(ui.view == V_SEQ && ui.arp))
            sub_mem[ui.view] = (uint8_t)((ui.msp & 1u) | ((ui.dck & 1u) << 1) | ((ui.chk & 1u) << 2) | ((ui.seqpat & 1u) << 3));
        ui.prev_view = ui.view;
    }
    ui.view = (uint8_t)v;
    ui.chk = 0;                                        /* wip328: and the CHOKE screen closes */
    ui.dck = 0;                                        /* wip350: and the DUCK screen */
    ui.msp = 0;                                        /* wip272: any page change goes back to the first PERFORM page (ALGO selects the MUTE / SOLO one) */
                                                       /* wip606: the MUTE / SOLO mode is NOT reset by a page change: back on PERFORM (or the GLO mixer) it is as it was left (FILL lit = SOLO, tracks can keep being soloed) */
    ui.seqpat = 0;
    ui.held_step = -1;
    ui.brk_held = -1;
    ui.hold_step = -1;
    ui.overlay = O_NONE;
    build_pages();
}

/* a sound knob turned: record its lane (REC, playing), or let the knob win over the lane for a pass */
static void motion_touch(pref_t r)
{
    int part, k;
    if (r.kind != R_ENG || !seq.playing)
        return;
    part = engine_motion_part(r.a, r.b);
    k = motion_find(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c);
    if (ui.rec) {
        if (k < 0 && (k = motion_alloc(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c)) < 0) {
            warn(k == -2 ? "MAX 3 PER TRACK" : "MOTION FULL", 0);
            return;
        }
        engine_motion_rec(k);
    } else if (k >= 0) {
        engine_motion_hold(k);
    }
}

/* SAVE held + a sound knob: forget its motion in this pattern */
static void motion_forget(pref_t r)
{
    int part, k;
    if (r.kind != R_ENG)
        return;
    part = engine_motion_part(r.a, r.b);
    k = motion_find(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c);
    if (k < 0) {
        warn("NO MOTION ON ", pref_name(r));
        return;
    }
    motion_clear(proj.arr.lane, k);
    engine_set(r.a, r.b, r.c, proj.sound.v[r.a][r.b][r.c]);
    mark_dirty();
    say("MOTION CLEARED: ", pref_name(r));
}

/* what a knob's lane plays now in this pattern, -1 none (the knob strip draws it) */
static int motion_now(pref_t r)
{
    int part;
    if (r.kind != R_ENG)
        return -1;
    part = engine_motion_part(r.a, r.b);
    return engine_motion_value(motion_find(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c));
}
static int has_motion(pref_t r)
{
    int part;
    if (r.kind != R_ENG)
        return 0;
    part = engine_motion_part(r.a, r.b);
    return motion_find(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c) >= 0;
}

/* p-locks: on EDIT, a step held (one already there: a tap still toggles it) and a sound knob turned
 * give that step its own value, one step of the knob's lane (motion.h: the other steps keep the knob's) */
/* the Tonic's voice pages (ENV, LFO, EDIT x2, pitch mod, FX / mix 1): a drum step held there + a knob = a p-lock
 * (wip195: they replaced V_PART for drum parts in 103, so V_PART alone never locked anything on the Tonic) */
static int voice_view(void)
{
    return ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL || ui.view == V_PMOD || ui.view == V_MIX1;
}
/* wip204: the mixer (V_MIX) and the FX pages after PERFORM (REVERB, DELAY ...; not COMP) also take a held step for its
 * PROBABILITY (SELECT while held). Not p-locks: lock_step() does not include them. The Tonic's drum machine only. */
static int step_hold_view(void)
{
    if (ui.overlay != O_NONE || !is_drum())
        return 0;
    return (ui.view == V_MIX && !tonic_part()) || (ui.view == V_FX && cur_page() != 0 && !fx_comp_page());   /* wip607: the Tonic's GLO mixer has the pattern keys now (white keys = next pattern) */
}
static int lock_step(void)
{
    if (ui.overlay != O_NONE)
        return -1;
    if (ui.view == V_PART)
        return is_drum() ? ui.hold_step : ui.brk_held;
    return is_drum() && voice_view() ? ui.hold_step : -1;
}
static int lockable(pref_t r) { return r.kind == R_ENG && engine_motion_part(r.a, r.b) == ui.part; }
static int lock_of(pref_t r, int s)                  /* step s's own value of r, -1 none */
{
    int k;
    if (s < 0 || !lockable(r))
        return -1;
    k = motion_find(proj.arr.lane, seq.ppat[ui.part], ui.part, r.a, r.b, r.c);
    return k >= 0 && proj.arr.lane[k].val[s] != MOT_NONE ? proj.arr.lane[k].val[s] : -1;
}
static void lock_turn(pref_t r, int e, int role)
{
    int s = lock_step(), part = ui.part, k, cur, nv;
    const x0x_param_t *d = pref_desc(r);
    lane_t *l;
    if (!d)
        return;
    if (!lockable(r)) {
        warn("NO P-LOCK: ", caps_of(pref_name(r)));
        return;
    }
    ui.hold_used = 1;
    k = motion_find(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c);
    if (ui.btn & (1u << B_SAVE)) {                    /* SAVE + the knob: the step's lock goes */
        int j, any = 0;
        ui.btn_used |= 1u << B_SAVE;
        if (k < 0 || proj.arr.lane[k].val[s] == MOT_NONE)
            return;
        l = &proj.arr.lane[k];
        l->val[s] = MOT_NONE;
        for (j = 0; j < NSTEPS; j++)
            any |= l->val[j] != MOT_NONE;
        if (!any)
            motion_clear(proj.arr.lane, k);
        if (!seq.playing)
            engine_set(r.a, r.b, r.c, proj.sound.v[r.a][r.b][r.c]);
        mark_dirty();
        say("P-LOCK CLEARED: ", caps_of(pref_name(r)));
        return;
    }
    if (k < 0 && (k = motion_alloc(proj.arr.lane, seq.ppat[part], part, r.a, r.b, r.c)) < 0) {
        warn(k == -2 ? "MAX 3 PER TRACK" : "MOTION FULL", 0);
        return;
    }
    l = &proj.arr.lane[k];
    cur = l->val[s] != MOT_NONE ? l->val[s] : proj.sound.v[r.a][r.b][r.c];
    e = d->names || d->max < 24 ? (e > 0 ? 1 : -1) : accel(role, e, d->max);
    nv = cur + e;
    nv = nv < 0 ? 0 : nv > d->max ? d->max : nv;
    l->val[s] = (uint8_t)nv;
    if (!seq.playing)                                 /* stopped: heard at once, until the step is let go */
        engine_set(r.a, r.b, r.c, nv);
    mark_dirty();
    ui.touched = (int8_t)(role - EN_K1);
    ui.touch_until = plat_ms() + 600u;
}
static void lock_release(void)                        /* the held step let go: stopped, the knobs' own back */
{
    int i;
    if (seq.playing)
        return;
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[cur_page()][i];
        if (lockable(r))
            engine_set(r.a, r.b, r.c, proj.sound.v[r.a][r.b][r.c]);
    }
}
/* probability: SEQ or EDIT, a step held, SELECT sets its chance (5..100 %, a T_PROB lane: motion.h's
 * MOT_NONE = always). Drums: the selected track's step; the 303s and the break: the step */
static int held_any_step(void)
{
    if (ui.overlay != O_NONE)
        return -1;
    if (steps_view())
        return is_drum() ? ui.hold_step : ui.brk_held;
    if (step_hold_view())
        return ui.hold_step;                             /* wip204: the mixer and the FX pages after PERFORM */
    return is_drum() && voice_view() ? ui.hold_step : -1;   /* wip197: the Tonic's EDIT (and ENV, LFO, SEL, FX) pages too */
}
static void prob_turn(int e)
{
    int s = held_any_step(), part = ui.part, v = part << 4 | (is_drum() ? ui.sel[part] : 0), k, nv, j, any = 0;
    char t[24], *q;
    lane_t *l;
    ui.hold_used = 1;
    k = motion_find(proj.arr.lane, seq.ppat[part], part, T_PROB, v, 0);
    nv = k >= 0 && proj.arr.lane[k].val[s] != MOT_NONE ? proj.arr.lane[k].val[s] : 100;
    {   /* one line, turned down from 100 %: 95 .. 5 %, then 1:2 2:2 1:3 .. 8:8 */
        int pos = nv > 100 ? 20 + nv - PROB_COND : (100 - nv) / 5;
        pos += e > 0 ? -1 : 1;
        pos = pos < 0 ? 0 : pos > 19 + PROB_NCOND ? 19 + PROB_NCOND : pos;
        nv = pos < 20 ? 100 - 5 * pos : PROB_COND + pos - 20;
    }
    if (nv != 100 && k < 0 && (k = motion_alloc(proj.arr.lane, seq.ppat[part], part, T_PROB, v, 0)) < 0) {
        warn("MOTION FULL", 0);
        return;
    }
    if (k >= 0) {
        l = &proj.arr.lane[k];
        l->val[s] = (uint8_t)(nv != 100 ? nv : MOT_NONE);
        for (j = 0; j < NSTEPS; j++)
            any |= l->val[j] != MOT_NONE;
        if (!any)
            motion_clear(proj.arr.lane, k);
    }
    mark_dirty();
    q = put_s(put_i(put_s(t, "STEP "), s + 1), ": ");
    if (nv > 100) {
        int a, b;
        prob_cond(nv - PROB_COND, &a, &b);
        put_i(put_s(put_i(q, a), ":"), b);
        put_i(put_s(put_i(pc_big, a), ":"), b);
    } else {
        put_s(put_i(q, nv), "%");
        put_s(put_i(pc_big, nv), "%");
    }
    say(t, 0);                                    /* (the screen reader says it; the header text is under the big screen) */
    pc_on = 1;                                    /* wip433: the chance big on the whole screen, as long as the message lasts */
    pc_step = (uint8_t)(s + 1);
}

/* the 16 steps on the keys that have values of their own (p-locks, recorded motion) for part's lanes
 * of target t (-1: any), voice v (-1: any) */
static uint32_t lock_marks(int t, int v, int base)
{
    uint32_t m = 0;
    int k, c;
    for (k = 0; k < NLANE; k++) {
        const lane_t *l = &proj.arr.lane[k];
        if (!l->used || l->part != ui.part || l->pat != seq.ppat[ui.part] || (t >= 0 && l->t != t) ||
            (v >= 0 && l->v != v))
            continue;
        for (c = 0; c < 16; c++)
            if (base + c < NSTEPS && l->val[base + c] != MOT_NONE)
                m |= 1u << c;
    }
    return m;
}

/* SLOW stepped sliders (wip74): quantization (5 positions) and fill rate (3) change one position only after
 * SLOW_DETENTS clicks the same way (a pause, or a turn back, starts the count again). The 3-value icon selectors
 * of the plan (waveform, mod shape, filter mode, envelope mode) join here later: add their refs to is_slow(). */
#define SLOW_DETENTS 4
static int is_slow(pref_t r)
{
    const x0x_param_t *d;
    if (r.kind == R_QUANT || r.kind == R_FILLR || r.kind == R_FIXLEN)
        return 1;
    d = pref_desc(r);                                    /* the pitch mod shape (3 values): very slow too */
    if (r.kind == R_ENG && r.a == T_909 && d && d->name && d->name[0] == 'W' && d->name[1] == 'a' && d->name[2] == 'v')
        return 1;                                        /* the oscillator's waveform (3 values) */
    if (r.kind == R_ENG && r.a == T_909 && d && d->name && d->name[0] == 'N' && d->name[1] == 'F' && d->name[2] == 'M')
        return 1;                                        /* the noise filter mode (3 values) */
    if (r.kind == R_ENG && r.a == T_909 && d && d->name && d->name[0] == 'N' && d->name[1] == 'E' && d->name[2] == 'M')
        return 1;                                        /* the noise envelope mode (3 values) */
    return d && d->name && d->name[0] == 'M' && d->name[3] == 'M' && d->name[4] == 'o' && d->name[5] == 'd';
}
static int slow_acc[NE];
static uint32_t slow_t[NE];
static int slow_turn(int knob, int e)
{
    uint32_t now = plat_ms();
    if (now - slow_t[knob] > 700u || (slow_acc[knob] < 0) != (e < 0))
        slow_acc[knob] = 0;                              /* a pause or the other way: count again */
    slow_t[knob] = now;
    slow_acc[knob] += e;
    if (slow_acc[knob] > -SLOW_DETENTS && slow_acc[knob] < SLOW_DETENTS)
        return 0;                                        /* not yet */
    e = slow_acc[knob] > 0 ? 1 : -1;
    slow_acc[knob] = 0;
    return e;
}

static void turn_ref(pref_t r, int e, int knob)
{
    const x0x_param_t *d = pref_desc(r);
    if (!d || r.kind == R_ACT)
        return;
    if (is_pan(r) && d->max == 127 && knob >= 0 && knob < NE) {   /* wip283: a pan is precise at the centre (64 = mono): it stops there, and leaving takes a few slow clicks */
        int cur = pref_get(r), nv;
        if (cur == 64) {
            e = slow_turn(knob, e);
            if (!e)
                return;
        } else {
            e = accel(knob, e, d->max);
            nv = cur + e;
            if ((cur < 64 && nv >= 64) || (cur > 64 && nv <= 64)) {
                e = 64 - cur;                             /* land on the centre, never jump over it */
                slow_acc[knob] = 0;
            }
        }
    } else if (is_slow(r) && knob >= 0 && knob < NE) {
        e = slow_turn(knob, e);
        if (!e)
            return;
    } else if (d->names || d->max < 24)
        e = e > 0 ? 1 : -1;                          /* a switch: one detent, one position */
    else
        e = accel(knob, e, d->max);
    pref_set(r, pref_get(r) + e);
    motion_touch(r);
}

static void rec_time(int k, int v, int s)         /* wip311: UNQUANTIZED REC keeps how late in the step the hit came (QUANTIZED: none) */
{
    int off;
    if (k != PART_909 || v >= 8 || s < 0 || s >= NSTEPS)
        return;
    off = ui.unq ? seq_rec_off(&seq, TRK_DRUM + k) : 0;
    seq_late_set(&seq, seq.ppat[k], v, s, off < 4 ? 0 : off);
}
static int rec_step(int track) { return seq_rec_step(&seq, track, !ui.unq); }   /* wip292: QUANTIZED REC = the nearest step; UNQUANTIZED REC = the step playing now (until the timing offsets exist) */

/* TONIC: black keys 9 and 10 are not tracks but modifiers, ACCENT and FILL, and key 11 is FUNC (reserved).
 * Keep a trig key (white) pressed and press ACCENT and / or FILL: each press adds the flag to that trig
 * (a step with the flag already on loses it). The other order works too: hold ACCENT / FILL, press white keys.
 * A trig with ACCENT or FILL put on it is never taken off by the release. */
static int tonic_part(void) { return ui.part == PART_909; }
static const char *view_title(void)                   /* the title strip: SEQ's 2nd press (the pattern number screen) says PAT */
{
    if (fx_comp_page())
        return "MASTER";                               /* wip121: the compressor is the master's, not a track's */
    if (ui.view == V_SEQ && ui.arp && tonic_part())
        return "ARP";                                  /* wip251 */
    return ui.view == V_SEQ && ui.seqpat && tonic_part() ? "PAT" : VIEW_N[ui.view];
}
#define NDRUM_TONIC 8                              /* the Tonic's tracks: black keys 1-8 */
/* wip287: the SEQ page with the voice selector (ALGO) on AC: the white keys are the global accent's trigs */
static int ac_on(void) { return ui.ac && ui.view == V_SEQ && !ui.arp && !ui.seqpat && tonic_part(); }

/* wip429: ACCENT / FILL held (Tonic): the trigs of the selected track that carry that flag blink on the white keys (accent = the voice's accent or the global accent, as the screen shows it) */
static int step_flag_blink(const dpart_t *d, int v, int s)
{
    if (!tonic_part() || ui.overlay == O_ASK || (ui.mod_held & 4u) || !(ui.mod_held & 3u) || v < 0 || v >= NDRUM)
        return 0;
    return ((ui.mod_held & 1u) && (sm_get(d->vacc[v], s) || sm_get(d->accent, s))) || ((ui.mod_held & 2u) && sm_get(d->vfill[v], s));
}

static void step_flag(int k, int v, int s, int fill)
{
    dpart_t *d = &cur_pat()->drum[k];
    uint32_t *row = fill ? d->vfill[v] : d->vacc[v];
    sm_flip(row, s);
    if (sm_get(row, s) && !sm_get(d->hit[v], s))
        sm_set(d->hit[v], s);                        /* a flag needs its hit */
    mark_dirty();
    sp_step(s, fill ? 3 : 2);
}

/* wip583: the step PAGE spot (1-16, 17-32 ...) on the 3rd knob's cell, in every skin: always on the ARP page (its K3 is free) and with AUTOSCROLL on (SEQ: the FREE LEN cell);
 * with AUTOSCROLL off the SEQ page keeps FREE LEN and shows the page for 1.2 s after a turn of ALGORITHM (wip699) */
static uint32_t spage_until;
static int spage_pages(void)                          /* the pages the track's pattern length has: 16 = 1, 32 = 2 ... */
{
    int n = (seq_part_len(&seq, TRK_DRUM + ui.part) + 15) / 16;
    return n < 1 ? 1 : n > NSTEPS / 16 ? NSTEPS / 16 : n;
}
static int spage_spot(void)
{
    if (ui.view != V_SEQ || !is_drum() || ui.seqpat)
        return 0;
    if (ui.arp || proj.set.autoscroll)
        return 1;
    return (int32_t)(spage_until - plat_ms()) > 0;
}

static void drum_mod(int bit, int down)
{
    if (!down) {
        ui.mod_held &= (uint8_t)~bit;
        return;
    }
    ui.mod_held |= (uint8_t)bit;
    if (ui.hold_step >= 0) {                         /* a trig is held: the flag goes on it */
        step_flag(ui.part, ui.sel[ui.part], ui.hold_step, bit == 2);
        ui.hold_mod = 1;
    }
}

/* wip326: RANDOM SOUND. GLO 1st press (the VEL page), K4 = a blue RND icon: a turn gives the selected track a new random sound.
 * Only the 21 libpo32 parameters (TNP_*) are drawn; the extras (pan, reverb / delay sends, low / high cut) are not touched.
 * wip344: LEVEL (the volume) is not drawn any more: it stays as it is (as the pan). The icon blinks for rnd_until. Not while a step is held. */
static uint32_t rnd_until, rnd_last, rnd_all_until;
/* wip610: one voice gets a new SAFE random sound (the body of RANDOM SOUND; RANDOM ALL calls it for every voice) */
static void tn_randomize_voice(int v)
{
    int i;
    for (i = 0; i < TN_NPARAMS; i++) {
        const x0x_param_t *d = tonic_param(v, i);
        int pot;
        if (i == TNP_LEVEL)
            continue;                                 /* wip344: RANDOM never touches the volume (and never the pan: not in this loop) */
        pot = d ? (int)(sv_rand() % (uint32_t)(d->max + 1)) : 0;
        if (d && i == TNP_DIST) {                     /* wip522: RANDOM never reaches the maximum distortion (speakers, ears): top = 3/4 of the range; by hand it still goes to the end */
            /* wip581: LEVEL and DISTORTION are inversely tied: the louder the track's level is above its default (101), the lower the
             * random distortion ceiling, 1 for 1 (level +20 = ceiling -20), so a high level and a high distortion never come together.
             * The level itself is still never drawn; a level at or under the default keeps the old ceiling (3/4). */
            int top = (d->max * 3) / 4;
            int over = (int)proj.sound.v[T_909][v][TNP_LEVEL] - 101;
            if (over > 0)
                top = over >= top ? 0 : top - over;
            pot = (int)(sv_rand() % (uint32_t)(top + 1));
        }
        tn_put(v, i, (uint8_t)pot, tonic_pot_to_val(i, pot));
    }
}
/* all = 0: RANDOM SOUND (the selected voice); all = 1: wip610 RANDOM ALL (SEL page = PITCH MOD, FUNC + K4): every voice, the same safe random */
static void tn_randomize(int all)
{
    int v = ui.sel[PART_909], n;
    uint32_t now = plat_ms();
    if (!tn_page_ok() || ui.overlay != O_NONE || lock_step() >= 0)
        return;
    if (now - rnd_last < 120u)                        /* a fast spin: one new sound per 120 ms at most */
        return;
    rnd_last = now;
    sv_rng ^= now * 2654435761u + 1u;                 /* the name generator's xorshift, stirred with the time */
    if (!sv_rng)
        sv_rng = 0x2F6E2B1u;
    if (all)
        for (n = 0; n < NDRUM_TONIC; n++)
            tn_randomize_voice(n);
    else
        tn_randomize_voice(v);
    mark_dirty();
    if (all)
        rnd_all_until = now + 900u;
    else
        rnd_until = now + 900u;
    if (!seq.playing)
        engine_drum(PART_909, v, 0.75f);              /* stopped: hear the new sound (playing: the pattern plays it) */
}

/* wip236 LIVE RECORDING on every page but PERFORM (and the pattern page, whose black keys are mutes): a black key 1-8 picks
 * its track, sounds it and, with REC on while playing, writes a trig at the playing step (as drum_key does on SEQ / PART) */
static void live_pad(int bl, int down)
{
    int km = PART_909;
    if (!down)
        return;
    ui.sel[km] = (uint8_t)bl;
    ui.ac = 0;                                       /* wip287 */
    engine_drum(km, bl, 0.75f);                      /* (audition: always, also while playing) */
    build_pages();
    if (ui.rec && seq.playing) {
        int s = rec_step(TRK_DRUM + km);
        sm_set(cur_pat()->drum[km].hit[bl], s);
        rec_time(km, bl, s);
        mark_dirty();
    }
}

static void drum_key(int v, int down)
{
    int k = ui.part;
    if (tonic_part() && v >= 8) {                    /* 9 = ACCENT, 10 = FILL, 11 = FUNC (reserved) */
        if (v < 10)
            drum_mod(v == 8 ? 1 : 2, down);
        return;
    }
    if (!down)
        return;
    ui.sel[k] = (uint8_t)v;
    ui.ac = 0;                                       /* wip287 */
    build_pages();
    /* AUDITION (wip108, the user's request): a black key always sounds its voice, also while the pattern plays
     * (it used to be silent then unless GLO > KEY SOUND = ALWAYS; the setting no longer changes this) */
    engine_drum(k, v, (ui.btn & (1u << B_ENV)) ? 1.0f : 0.75f);
    if (ui.btn & (1u << B_ENV))
        ui.btn_used |= 1u << B_ENV;
    if (ui.rec && seq.playing) {
        int s = rec_step(TRK_DRUM + k);
        sm_set(cur_pat()->drum[k].hit[v], s);
        rec_time(k, v, s);
        if (ui.btn & (1u << B_ENV))
            sm_set(cur_pat()->drum[k].accent, s);
        mark_dirty();
    }
}

#define STEP_TAP_MS 300u
/* a white key: a step. An empty one is on at once; one already on comes off on a tap (a quick
 * release), and stays when held (EDIT: its p-locks) */
static void drum_step(int white, int down)
{
    int k = ui.part, s = ui.spage * 16 + white;
    dpart_t *d = &cur_pat()->drum[k];
    uint32_t *bits = d->hit[ui.sel[k]];
    if (s >= NSTEPS)
        return;
    if (ac_on()) {                                   /* wip287: AC: a white key puts / takes the global accent on that step */
        if (down && s < d->len) {
            sm_flip(d->accent, s);
            mark_dirty();
            sp_step(s, 1);
        }
        return;
    }
    if (!down) {
        if (ui.hold_step == s) {
            if (!ui.hold_created && !ui.hold_used && !ui.hold_mod && plat_ms() - ui.hold_t0 < STEP_TAP_MS) {
                sm_flip(bits, s);
                if (!sm_get(bits, s)) {                /* a trig taken off takes its flags with it */
                    d->vacc[ui.sel[k]][s >> 5] &= ~(1u << (s & 31));
                    d->vfill[ui.sel[k]][s >> 5] &= ~(1u << (s & 31));
                    if (k == PART_909 && ui.sel[k] < NTRK_NOTE && seq.ppat[k] < NPAT_NOTE)   /* wip252: and its note */
                        proj.note[seq.ppat[k]][ui.sel[k]][s] = 0;
                    if (k == PART_909 && ui.sel[k] < 8)       /* wip311: and its late offset */
                        seq_late_set(&seq, seq.ppat[k], ui.sel[k], s, 0);
                }
                mark_dirty();
                sp_step(s, 0);
            }
            if (ui.hold_used)
                lock_release();
            ui.hold_step = -1;
        }
        return;
    }
    if (tonic_part() && ui.mod_held) {               /* ACCENT / FILL held first, then the trig key */
        if (ui.mod_held & 1u)
            step_flag(k, ui.sel[k], s, 0);
        if (ui.mod_held & 2u)
            step_flag(k, ui.sel[k], s, 1);
        ui.hold_step = (int8_t)s;
        ui.hold_used = 0;
        ui.hold_mod = 1;
        ui.hold_created = 0;
        ui.hold_t0 = plat_ms();
        return;
    }
    if (ui.btn & (1u << B_ENV)) {
        sm_flip(d->accent, s);
        ui.btn_used |= 1u << B_ENV;
        mark_dirty();
        sp_step(s, 1);
        return;
    }
    ui.hold_step = (int8_t)s;
    ui.hold_used = 0;
    ui.hold_mod = 0;
    ui.hold_t0 = plat_ms();
    ui.hold_created = !sm_get(bits, s);
    if (ui.hold_created) {
        sm_flip(bits, s);
        mark_dirty();
        sp_step(s, 0);
    }
}

/* an edit sounds as the drum keys do (KEY SOUND): while stopped or recording, or always */
static int key_sounds(void) { return proj.set.keysound || !seq.playing || ui.rec; }
static const uint32_t MUTE_MASK[NPARTS] = {(1u << NDRUM) - 1u, ((1u << NDRUM) - 1u) << NDRUM, 1u << MUTE_BRK};
static int part_muted(int p) { return (seq.mute & MUTE_MASK[p]) == MUTE_MASK[p]; }

static int perf_testing(void) { return perf.test >= 0 && perf.phase != 4; }

static int track_muted(int km, int v) { return (int)(seq.vmute >> (km * NDRUM + v) & 1u); }
static void mute_track(int km, int v)                /* one drum track on / off */
{
    seq.vmute ^= 1u << (km * NDRUM + v);
    say(track_muted(km, v) ? "MUTED " : "UNMUTED ", engine_voice_name(T_909, v));
}

static int held_track(void)                          /* the Tonic: a black key 1-8 held now (the lowest), -1 none */
{
    int v;
    for (v = 0; v < NDRUM_TONIC; v++)
        if (ui.keys & (1u << BLACK_KEY[v]))
            return v;
    return -1;
}

/* wip251: the ARP page's keyboard. The 27 keys run in semitones from F (white and black as on a piano: F F# G G# A A# B C ...), so
 * key k is semitone k; the key C (k = 7) plays the voice's own pitch, the others are k - 7 semitones from it, and OCT- / OCT+
 * move the whole keyboard by octaves (arp_oct, -3..+3). The hit is the voice's own sound, its OscFreq moved for this hit only
 * (engine_tonic_note): nothing stored changes. A key played here is not recorded yet (step 2). */
#define ARP_C_KEY 7
static void arp_key(int k, int down)
{
    int v = ui.sel[PART_909];
    if (!down || v < 0 || v >= TN_VOICES)
        return;
    int semis = k - ARP_C_KEY + 12 * ui.arp_oct;                /* -43 .. +55: inside the note's -64..63 */
    engine_tonic_note(v, semis, 0.75f);
    if (ui.rec && seq.playing && seq.ppat[PART_909] < NPAT_NOTE && v < NTRK_NOTE) {   /* wip252 (2b): REC + PLAY records the hit and its note */
        int s = rec_step(TRK_DRUM + PART_909);
        char t[24], *q;
        if (s >= 0 && s < NSTEPS) {
            sm_set(cur_pat()->drum[PART_909].hit[v], s);
            rec_time(PART_909, v, s);
            proj.note[seq.ppat[PART_909]][v][s] = (int8_t)semis;
            mark_dirty();
            q = put_s(t, semis > 0 ? "NOTE +" : "NOTE ");
            put_i(q, semis);
            say(t, 0);                                          /* (the note shows on the screen only, as asked) */
        }
    }
}

static int msp_on(void)                               /* wip272: the fixed MUTE / SOLO system is active: BOTH PERFORM pages (the big pattern number and the MUTE / SOLO boxes), the pattern page and the GLO mixer */
{
    return ((ui.view == V_FX && cur_page() == 0) || (ui.view == V_SEQ && ui.seqpat) || ui.view == V_MIX) && tonic_part() && ui.overlay == O_NONE;   /* wip285: the pattern page (SEQ, 2nd press) has the same system; wip607: the GLO mixer too (V_MIX: FUNC = MUTE mode, FILL = SOLO mode, black keys 1-8 = the tracks, white keys = the next pattern) */
}
static int msp_page(void)                             /* wip272: the second PERFORM page (the MUTE / SOLO boxes) is on screen (ALGO selects it) */
{
    return msp_on() && ui.view == V_FX && ui.msp;          /* (the boxes page exists on PERFORM only) */
}
static void msp_voice(int v)                          /* wip272: a black key 1-8 on either PERFORM page: MUTE mode = mute / unmute; SOLO mode = add / remove that voice (many at once) */
{
    uint32_t sb = 1u << (PART_909 * NDRUM + v);
    char tn[2] = {(char)('1' + v), 0};
    if (!ui.msmode && ui.view != V_MIX) {                /* wip630: the GLO mixer has no mute: its black keys only solo */
        mute_track(PART_909, v);
        return;
    }
    seq.vsolo ^= sb;                                   /* (the same mask as wip264; the sequencer is unchanged) */
    if (seq.vsolo & sb)
        say("SOLO TRACK ", tn);
    else
        say(seq.vsolo ? "SOLO OUT " : "SOLO OFF", seq.vsolo ? tn : 0);
}

static int chk_page(void)                             /* wip328: the CHOKE screen (EDIT, K2 turned right) is on screen */
{
    return ui.view == V_NEV && ui.chk && tonic_part() && ui.overlay == O_NONE && !ui.help;
}
static void chk_say(int v)                            /* wip333: the message for a track that joined / left the group */
{
    char tn[2] = {(char)('1' + v), 0};
    mark_dirty();                                      /* wip335: the autosave keeps the group */
    if (seq.choke & (1u << v))
        say("CHOKE IN: TRACK ", tn);
    else
        say("CHOKE OUT: TRACK ", tn);
}
static void chk_voice(int v)                          /* wip328: a black key 1-8 on the CHOKE screen: the track joins / leaves the group */
{
    seq.choke ^= (uint8_t)(1u << v);
    engine_tonic_choke(seq.choke);
    chk_say(v);
}

static int dck_page(void)                             /* wip350: the DUCK screen (SENDS, ALGORITHM right) is on screen */
{
    return ui.view == V_MIX2 && ui.dck && tonic_part() && ui.overlay == O_NONE && !ui.help;
}
static void dck_voice(int v)                          /* wip350: a black key 1-8 on the DUCK screen: the track is ducked / not ducked */
{
    char tn[2] = {(char)('1' + v), 0};
    if (proj.set.classic) {
        say("CLASSIC ENGINE", 0);
        return;
    }
    if (v == proj.set.sc_in) {                               /* the input can not be ducked */
        char m[48], *q = put_s(m, "TRACK ");
        q = put_s(q, tn);
        put_s(q, " IS INPUT: CHOOSE DIFFERENT TRACK");
        warn(m, 0);
        return;
    }
    proj.set.sc_mask ^= (uint8_t)(1u << v);
    mark_dirty();                                      /* wip353: the autosave keeps it */
    sc_send();
    say((proj.set.sc_mask >> v) & 1u ? "DUCK IN: TRACK " : "DUCK OUT: TRACK ", tn);
}

static void key_event(int k, int down)
{
    int w = KEY_WHITE[k], bl = KEY_BLACK[k], wp = w >= 0 ? ui.pbank * 16 + w : -1;   /* wp: a white key's pattern */
    if (!down && (bl == 8 || bl == 9 || bl == 10))          /* ACCENT / FILL / FUNC let go, whatever the screen shows now */
        ui.mod_held &= (uint8_t)~(1u << (bl - 8));
    {                                                       /* wip417: GLO held + ACCENT = UNDO, GLO held + FILL = REDO (any page, the ARP page too); GLO's own release then stays quiet */
        static uint8_t gu_keys;                             /* bit 0 ACCENT / bit 1 FILL: pressed as undo / redo, so their release is swallowed */
        if (bl == 8 || bl == 9) {
            uint8_t m = (uint8_t)(1u << (bl - 8));
            if (down && (ui.btn & (1u << B_GLO)) && tonic_part() && ui.overlay != O_ASK && !perf_testing() && !(ui.mod_held & 4u)) {
                gu_keys |= m;
                ui.mod_held &= (uint8_t)~m;                 /* (it is not an accent / fill modifier now) */
                ui.btn_used |= 1u << B_GLO;
                do_undo(bl == 9);
                return;
            }
            if (!down && (gu_keys & m)) {
                gu_keys &= (uint8_t)~m;
                return;
            }
        }
    }
    if (down && bl != 10)
        ui.cpf = 0;                                         /* wip377: another key used with FUNC: it is not a tap */
    if (!down && bl == 10) {
        if (ui.cpf && (ui.cpm & 1u))
            ui.cpm |= 4u;                                   /* wip377: FUNC tapped alone in the COPY-PASTE MODE: the mode stays on for many pastes */
        ui.cpf = 0;
    }
    if (down)
        ui.adel = 0;                                        /* wip367: a key pressed with EDIT held (ACCENT / FILL: page copy / paste): not a delete */
    if (!down && bl == 8)
        ui.cpm &= (uint8_t)~2u;                             /* wip362: ACCENT let go: a white key is a destination again */
    if (down && bl != 9)                                    /* wip356: any other key pressed while FILL is held: FILL is not a delete */
        ui.fdel = 0;
    if (!down && bl == 9 && ui.fdel) {                      /* wip356: FILL let go on the SONG screen, unused: delete the bar on the cursor */
        ui.fdel = 0;
        if (ui.view == V_SONG && ui.overlay == O_NONE && tonic_part() && !perf_testing()) {
            if (song()->len)                                 /* wip361: deletes at once, no question, no message (he said so) */
                song_del_cur();
        }
        return;
    }
    if (ui.view == V_SEQ && ui.arp && tonic_part() && ui.overlay == O_NONE && !perf_testing()) {   /* wip251: the ARP page: every key is a piano key */
        arp_key(k, down);                                   /* (so FUNC, ACCENT and FILL are notes here too) */
        return;
    }
    if (dck_page() && bl >= 0 && bl < NDRUM_TONIC) {        /* wip350: the DUCK screen: black keys 1-8 = the ducked tracks */
        if (down)
            dck_voice(bl);
        return;
    }
    if (chk_page() && bl >= 0 && bl < NDRUM_TONIC) {        /* wip328: the CHOKE screen: black keys 1-8 = the tracks of the group */
        if (down)
            chk_voice(bl);
        return;
    }
    if (ui.cpm && w >= 0 && tonic_part() && ui.overlay != O_ASK && (!(ui.mod_held & 4u) || (ui.cpm & 2u))) {   /* wip362: COPY-PASTE MODE: a white key never triggers or changes a pattern here (FUNC + white key, ACCENT let go: the old pattern change / FUNC + FILL paste work as before) */
        if (down) {
            if (ui.cpm & 2u) {                                         /* FUNC + ACCENT still held + white key: THAT pattern is the copy */
                char t[16];
                ui.clip = (int8_t)wp;
                ui.cpsrc = (int8_t)(wp - ui.pbank * 16);
                ui.cpdone = 0;
                put_s(put_i(put_s(t, "P"), wp + 1), " COPIED");
                say(t, 0);
            } else {                                                   /* a white key alone: paste the copy there, on the fly */
                int keep = (ui.cpm & 4u) != 0;                           /* wip377: FUNC tapped alone in the mode = stay, paste in many places */
                tn_paste(wp);
                if (keep) {
                    if (wp - ui.pbank * 16 >= 0 && wp - ui.pbank * 16 < 16)
                        ui.cpdone |= (uint16_t)(1u << (wp - ui.pbank * 16));   /* wip365: that key stops blinking */
                } else if (ui.clip >= 0) {                               /* wip377: one paste: that pattern becomes the current one and the mode ends */
                    if (ui.view != V_SONG && !ui.songmode) {             /* (the SONG page and song mode: the song decides, no pattern change) */
                        seq_chain(&seq, 0, 0);                           /* playing: at the end of the bar; stopped: at once (as FUNC + white key) */
                        seq_cue(&seq, wp);
                    }
                    ui.cpm = 0;
                    ui.cpdone = 0;
                    ui.cpsrc = -1;
                }
            }
        }
        return;
    }
    if (msp_on() && !perf_testing()) {                      /* wip272: both PERFORM pages: FUNC = MUTE mode, FILL = SOLO mode / clear every solo, black keys 1-8 = the voices */
        if (bl == 10) {                                     /* FUNC: mutes active = all unmuted (like FILL with solos); none = MUTE mode */
            if (down && ui.view == V_MIX)
                ui.mod_held |= 4u;                          /* wip607: on the GLO mixer FUNC is still the modifier of the combinations too (FUNC + HOME = RENAME, FUNC + REC, ...) */
            if (ui.view == V_MIX)
                return;                                     /* wip630: on the GLO mixer FUNC does not mute / unmute: it is only the modifier (FUNC + black key 1-8 = audition) */
            if (down) {
                uint32_t mm = seq.vmute & (((1u << NDRUM_TONIC) - 1u) << (PART_909 * NDRUM));   /* wip284: the Tonic voices that are muted now */
                ui.msmode = 0;
                if (mm) {
                    seq.vmute &= ~mm;                      /* (solos are not touched; the sequencer is unchanged) */
                    say("MUTE OFF", 0);
                } else
                    say("MUTE MODE", 0);
            }
            return;
        }
        if (bl == 9) {                                      /* FILL: solos active = all off (and back to MUTE mode); none = SOLO mode on / off */
            if (down) {
                if (ui.view == V_MIX) {                     /* wip630: GLO mixer: solo is the only mode: FILL clears every solo */
                    if (seq.vsolo) {
                        seq.vsolo = 0;
                        say("SOLO OFF", 0);
                    }
                    return;
                }
                if (seq.vsolo) {
                    seq.vsolo = 0;
                    ui.msmode = 0;
                    say("SOLO OFF", 0);
                } else {
                    ui.msmode = (uint8_t)!ui.msmode;
                    say(ui.msmode ? "SOLO MODE" : "MUTE MODE", 0);
                }
            }
            return;
        }
        if (bl >= 0 && bl < NDRUM_TONIC) {
            if (ui.view == V_MIX && (ui.mod_held & 4u)) {   /* wip630: GLO mixer, FUNC held + black key 1-8: AUDITION (the voice sounds; no solo, no selection, nothing recorded) */
                if (down)
                    engine_drum(PART_909, bl, 0.75f);
                return;
            }
            if (down)
                msp_voice(bl);
            return;
        }
    }
    if (down && bl == 10 && tonic_part() && ui.overlay != O_ASK) {
        ui.mod_held |= 4u;                                  /* FUNC held */
        ui.cpf = (uint8_t)((ui.cpm & 1u) != 0);             /* wip377: FUNC pressed inside the COPY-PASTE MODE: if it is let go with no other key used, the mode stays on */
    }
    if (!down && bl == 9 && ui.pst_sh) {                    /* FUNC + FILL let go: paste here (a white key used it: already pasted) */
        if (ui.pst_sh == 1) {
            if (ui.pst_pg)
                tn_page_paste();
            else if (ui.pst_trk)
                tn_trk_paste(ui.pst_trk - 1);
            else
                tn_paste_ask(seq.ppat[PART_909]);                  /* wip416: the pattern asks PASTE PATTERN? (tracks and pages: no question) */
        }
        ui.pst_sh = 0;
        ui.pst_trk = ui.pst_pg = 0;
        return;
    }
    if (!down && bl >= 0 && bl < NDRUM_TONIC && (ui.mute_pend >> bl & 1u)) {   /* FUNC + black key, let go unused: mute / unmute */
        ui.mute_pend &= (uint8_t)~(1u << bl);
        mute_track(PART_909, bl);
        return;
    }
    if (ui.overlay == O_ASK && ui.ask_act == ACT_CHAIN) {   /* CHAIN: white keys 1-16 = the next pattern of the chain */
        if (down && w >= 0) {
            if (ui.keys & (1u << BLACK_KEY[10])) {          /* FUNC + a white key: take the last pattern off (every press one more) */
                if (ui.ch_n)
                    ui.ch_n--;
            } else if (ui.ch_n < CH_MAX)
                ui.ch_pat[ui.ch_n++] = (uint8_t)w;
        }
        return;
    }
    if (ui.overlay == O_ASK || ui.view == V_PERF || perf_testing())
        return;
    if (down && bl >= 0 && bl < NDRUM_TONIC && tonic_part() && !(ui.mod_held & 4u)) {   /* wip264 SOLO: FILL + black key 1-8 = only that voice plays (its key blinks); */
        uint32_t sb = 1u << (PART_909 * NDRUM + bl);                                       /* again, or a press of that voice's key: the solo is gone */
        if ((ui.mod_held & 2u) && ui.hold_step < 0) {
            seq.vsolo = seq.vsolo == sb ? 0u : sb;
            ui.mute_pend = 0;
            {
                char tn[2] = {(char)('1' + bl), 0};
                say(seq.vsolo ? "SOLO TRACK " : "SOLO OFF", seq.vsolo ? tn : 0);
            }
            return;
        }
        if (seq.vsolo == sb) {
            seq.vsolo = 0;
            say("SOLO OFF", 0);
            if ((ui.view == V_FX && cur_page() == 0) || (ui.seqpat && ui.view == V_SEQ))   /* the mute pages: this press only ends the solo */
                return;
        }
    }
    if (((ui.seqpat && ui.view == V_SEQ) || (ui.view == V_FX && cur_page() == 0) || (ui.view == V_MIX && ui.overlay == O_NONE && tonic_part())) && w >= 0 && !(ui.mod_held & 4u)) {   /* (also the PERFORM page; wip607: the GLO mixer too: it used to hold a step) */   /* the pattern screen: a white key = the next pattern, */
        if (down && !ui.songmode) {                                         /* at the end of the current one (stopped: at once); wip625: not in SONG MODE (the song is written: it decides) */
            seq_chain(&seq, 0, 0);
            seq_cue(&seq, wp);
        }
        return;
    }
    if (((ui.view == V_FX && cur_page() == 0) || (ui.seqpat && ui.view == V_SEQ && tonic_part())) && bl >= 0 && bl < NDRUM_TONIC && !(ui.mod_held & 4u)) {   /* PERFORM and the pattern page (wip112): black keys 1-8 = mute buttons: */
        if (down)                                                                                      /* 1st press mutes that track, 2nd unmutes */
            mute_track(PART_909, bl);
        return;
    }
    if (down && bl == 8 && tonic_part() && (ui.mod_held & 4u)) {   /* FUNC + ACCENT: COPY the page (EDIT / LFO held), a track's sound (a black key held), else the pattern */
        int v = held_track();
        if (page_held())
            tn_page_copy();
        else if (v >= 0) {
            ui.mute_pend = 0;                         /* the track key was used: no mute when it is let go */
            tn_trk_copy(v);
        } else if (ui.cpm & 4u) {                     /* wip415: the mode is KEPT (FUNC was tapped): FUNC + ACCENT again leaves it, nothing is copied */
            ui.cpm = 0;
            ui.cpdone = 0;
            ui.cpsrc = -1;
            ui.cpf = 0;
            say("COPY MODE OFF", 0);
        } else {
            tn_copy();
            ui.cpdone = 0;
            ui.cpm = 3;                               /* wip362: the white keys now wait for a destination (bit 1: ACCENT is still held) */
            ui.cpsrc = (int8_t)(seq.ppat[PART_909] - ui.pbank * 16);   /* wip364: the playing pattern's key stops blinking (steady), the others wait */
        }
        return;
    }
    if (down && bl == 9 && tonic_part() && (ui.mod_held & 4u)) {   /* FUNC + FILL: PASTE (acts when FILL is let go; a white key pressed with it: into that slot) */
        int v = held_track();
        ui.pst_pg = (uint8_t)page_held();
        ui.pst_trk = (uint8_t)(!ui.pst_pg && v >= 0 ? v + 1 : 0);
        if (ui.pst_pg || v >= 0)
            ui.mute_pend = 0;
        ui.pst_sh = 1;
        return;
    }
    if (bl >= 0 && bl < NDRUM_TONIC && tonic_part() && (ui.mod_held & 4u)) {   /* FUNC + black key 1-8: mute / unmute that track when it is */
        if (down)                                                            /* let go, any page (not if PLAY / ACCENT / FILL used it) */
            ui.mute_pend |= (uint8_t)(1u << bl);
        return;
    }
    if (down && w >= 0 && (ui.pst_trk || ui.pst_pg) && ui.pst_sh)   /* FUNC + black key / page key + FILL: a white key does nothing */
        return;
    if (down && w >= 0 && tonic_part() && ui.pst_sh == 1) {   /* FUNC + FILL held + white key: */
        ui.pst_sh = 2;                                        /* paste the copy into that slot */
        tn_paste(wp);
        return;
    }
    if (w >= 0 && tonic_part() && (ui.mod_held & 4u)) {   /* FUNC + white key 1-16: change to that pattern, any page */
        if (down && ui.view != V_SONG && !ui.songmode) {   /* (the SONG page and song mode: the song decides, nothing) */
            seq_chain(&seq, 0, 0);                           /* playing: at the end of the bar; stopped: at once */
            seq_cue(&seq, wp);
        }
        return;
    }
    if ((ui.view == V_HOME || ui.view == V_SONG) && bl >= 0 && bl < TN_VOICES && tonic_part()) {   /* wip236: the pads play and record here too */
        live_pad(bl, down);
        return;
    }
    if (ui.view == V_SONG) {   /* SONG: white = the bar's pattern, FILL = delete the bar (black keys: nothing) */
        int k = ui.song_sel;
        if (bl == 9 && down && !(ui.mod_held & 4u) && ui.overlay == O_NONE) {   /* wip356: acts when FILL is let go, if nothing else was used with it */
            ui.fdel = 1;
            return;
        }
        if (!down || k >= NSONG || w < 0)
            return;
        song_fill(k);
        {
            int p;
            for (p = 0; p < NPARTS; p++)
                song()->bar[k].pat[p] = (uint8_t)wp;
            if (k + 1 < NSONG)
                ui.song_sel = (uint8_t)(k + 1);
        }
        mark_dirty();
        return;
    }
    if (ui.view == V_HOME || (w >= 0 && (ui.btn & (1u << B_HOME)))) {   /* HOME: the keys do nothing (FUNC + white key picks a */
        if (ui.btn & (1u << B_HOME))                         /* pattern, FUNC + black 1-8 mutes a track); HOME + white key: nothing */
            ui.btn_used |= 1u << B_HOME;
        return;
    }
    if (bl >= 0 && bl < TN_VOICES && drum_mix_page() >= 0) {   /* MIX, a drum machine's page: black keys 1-8 pick the track (ACCENT / FILL / FUNC do not: they used to set sel = 8..10) */
        if (down) {
            live_pad(bl, down);                          /* (wip236: it records too, REC + PLAY) */
            if (ui.view == V_MIX)                        /* the mixer: the group (1-4 / 5-8) of that track */
                *page_of(V_MIX) = (uint8_t)(bl / 4 + (cur_page() & 2));   /* the mixer: the group (1-4 / 5-8) of that track, in the SAME mode (levels / pans) (wip280) */
        }
        return;
    }
    if (w >= 0 && step_hold_view()) {                 /* wip204: the mixer, REVERB, DELAY ...: a white key holds a step (its chance) */
        drum_step(w, down);
        return;
    }
    if (ui.view == V_FX && bl >= 0 && bl < TN_VOICES && tonic_part()) {   /* wip236: REVERB, DELAY ...: the pads play and record (PERFORM: mutes, above) */
        live_pad(bl, down);
        return;
    }
    if (ui.view == V_MIX || ui.view == V_FX)          /* MASTER, FX: the other keys choose nothing (they used to */
        return;                                       /* play the part's steps or the break's slices) */
    if (is_drum()) {
        if (bl >= 0)
            drum_key(bl, down);
        else if (w >= 0)
            drum_step(w, down);
    } else {
        brkpart_t *bp = &cur_pat()->brk;
        if (bl >= 0 && bl < 8 && down && ui.brk_held >= 0) {     /* a step held + black 1-8: its own slice */
            int st = ui.brk_held;
            bp->slice[st] = (uint8_t)(bp->slice[st] == bl + 1 ? 0 : bl + 1);
            bp->steps |= 1u << st;
            ui.hold_used = 1;                          /* its release keeps the step */
            ui.brk_pinkeys |= (uint16_t)(1u << bl);
            mark_dirty();
            if (bp->slice[st]) {
                char t[24];
                put_i(put_s(put_i(put_s(t, "STEP "), st + 1), ": SLICE "), bl + 1);
                say(t, 0);
            } else
                say("STEP: THE GENERATOR'S SLICE", 0);
        } else if (bl >= 0 && !down && (ui.brk_pinkeys & (1u << bl))) {
            ui.brk_pinkeys &= (uint16_t)~(1u << bl);
        } else if (bl >= 0)
            engine_brk_live(bl, down);
        else if (w >= 0 && down) {                     /* as the drums: empty on at once, a tap takes it off */
            ui.brk_held = (int8_t)w;
            ui.hold_used = 0;
            ui.hold_t0 = plat_ms();
            ui.hold_created = !((bp->steps >> w) & 1u);
            if (ui.hold_created) {
                bp->steps |= 1u << w;
                mark_dirty();
            }
        } else if (w >= 0 && ui.brk_held == w) {
            if (!ui.hold_created && !ui.hold_used && plat_ms() - ui.hold_t0 < STEP_TAP_MS) {
                bp->steps &= ~(1u << w);
                mark_dirty();
            }
            if (ui.hold_used)
                lock_release();
            ui.brk_held = -1;
        }
    }
}

static void list_move(int d)
{
    int s = ui.list_sel, step = d < 0 ? -1 : 1, t;
    for (; d; d -= step) {                             /* a row at a time, over the section headers */
        for (t = s + step; t >= 0 && t < ui.list_n && list_skip(t); t += step)
            ;
        if (t < 0 || t >= ui.list_n)
            break;
        s = t;
    }
    ui.list_sel = (uint8_t)s;
    if (ui.list_sel < ui.list_top)
        ui.list_top = ui.list_sel;
    if (ui.list_sel >= ui.list_top + 6)
        ui.list_top = (uint8_t)(ui.list_sel - 5);
}

static void list_enter(void)
{
    pref_t r = list_rows[ui.list_sel];
    if (r.kind != R_ACT)
        return;
    if (r.a == ACT_CLEAR_PAT)
        ask(ACT_CLEAR_PAT, 0, "CLEAR THIS PATTERN?", "ALL FIVE PARTS");
    else if (r.a == ACT_RESET)
        ask(ACT_RESET, 0, "FACTORY RESET?", "SOUND, PATTERNS, SONG");
    else if (r.a == ACT_SONG_CLR)
        ask(ACT_SONG_CLR, 0, "CLEAR THE SONG?", 0);
    else
        run_action(r.a, 0);
}

static void close_overlay(void)
{
    if (ui.view == V_GLO) {
        ui.overlay = O_NONE;
        set_view(ui.prev_view == V_GLO ? V_PART : ui.prev_view);
        return;
    }
    ui.overlay = O_NONE;
}

/* OCT+ / OCT- on the Tonic: the selected voice's PITCH (TNP_OSCFREQ, 20 Hz - 20 kHz on an exponential scale: an octave is
 * ln 2 / ln 1000 = 6576 of 65535) one octave up or down, at full 16-bit precision, on every page */
#define TN_OCTAVE 6576
static void transpose_voice(int up)
{
    int v = ui.sel[PART_909], n;
    char t[24];
    if (v < 0 || v >= TN_VOICES)
        return;
    n = (int)proj.sound.tn16[v][TNP_OSCFREQ] + (up ? TN_OCTAVE : -TN_OCTAVE);
    n = n < 0 ? 0 : n > 65535 ? 65535 : n;
    proj.sound.tn16[v][TNP_OSCFREQ] = (uint16_t)n;
    proj.sound.v[T_909][v][TNP_OSCFREQ] = (uint8_t)tonic_val_to_pot(TNP_OSCFREQ, (uint16_t)n);
    engine_tonic_set16(v, TNP_OSCFREQ, (uint16_t)n);
    mark_dirty();
    put_s(put_s(t, engine_voice_name(T_909, v)), up ? " OCTAVE UP" : " OCTAVE DOWN");
    say(t, 0);
}

/* The Tonic with FUNC held (black key 11): SEQ asks to enter / leave SONG MODE, PLAY asks to clear the pattern (a black key
 * 1-8 held: that track only; EDIT / LFO held: reset the page); copy / paste are FUNC + ACCENT / FILL (key_event). REC cycles QUANTIZED / UNQUANTIZED
 * recording (wip292); HOME does nothing. Returns 0 = not a FUNC combination, 1 = done (the button's own action does not follow). */
static int shift_combo(int b)
{
    static const char *const OK = "[OCT+] CONFIRM [OCT-] GO BACK";
    int v;
    if (b == B_PLAY && ui.view == V_SEQ && ui.arp && tonic_part() && ui.overlay == O_NONE && !perf_testing() && (ui.keys & (1u << BLACK_KEY[10]))) {
        /* wip430: the ARP page: black key 11 (FUNC, but a piano key here) held + PLAY = CLEAR PATTERN? as everywhere else (PLAY does not start / stop then) */
        ask(ACT_CLEAR_TNPAT, 0, "CLEAR PATTERN?", OK);
        ui.ca_hold = 1;                           /* wip632: held 3 s: CLEAR ALL PATTERNS? */
        return 1;
    }
    if (!tonic_part() || !(ui.mod_held & 4u) || ui.overlay == O_ASK || perf_testing())
        return 0;
    switch (b) {
    case B_EDIT:                                  /* the page keys held with FUNC: ACCENT / FILL / PLAY then work on the page */
        ui.adel = 1;                              /* wip367: EDIT let go with nothing else used = DELETE AUTOMATION? (a page copy / paste / reset cancels it) */
        return 1;
    case B_LFO:
        return 1;
    case B_REC:                                   /* wip292: FUNC + REC cycles QUANTIZED REC / UNQUANTIZED REC */
        ui.unq ^= 1u;
        if (seq.playing && !ui.rec) {                 /* wip312: playing: FUNC + REC also switches recording on */
            ui.rec = 1;
            say(ui.unq ? "UNQUANTIZED REC ON" : "QUANTIZED REC ON", 0);
        } else
            say(ui.unq ? "UNQUANTIZED REC" : "QUANTIZED REC", 0);
        return 1;
    case B_PLAY:                                  /* CLEAR: the page (reset), a track's pattern (a black key held), else the pattern */
        v = held_track();
        if (page_held())
            tn_page_clear();
        else if (v >= 0) {
            ui.mute_pend = 0;
            ask(ACT_CLEAR_TNTRK, v, "CLEAR TRACK PATTERN?", OK);
        } else {
            ask(ACT_CLEAR_TNPAT, 0, "CLEAR PATTERN?", OK);
            ui.ca_hold = 1;                       /* wip632: still held after 3 s: CLEAR ALL PATTERNS? */
        }
        return 1;
    case B_SEQ:                                   /* (wip211) SONG MODE: on (asks), again: off (asks) */
        if (ui.songmode)
            ask(ACT_SONGEXIT, 0, "EXIT SONG MODE?", "PRESS [OCT+] CONFIRM");
        else
            ask(ACT_SONGMODE, 0, "ENTER SONG MODE", "PRESS [OCT+] CONFIRM");
        return 1;
    case B_ARP:                                   /* wip314: FUNC + ARP: delete the song and leave song mode (asks) */
        if (!song()->len && !ui.songmode) {
            warn("NO SONG YET", 0);
            return 1;
        }
        ask(ACT_SONGKILL, 0, "DELETE SONG?", "[OCT-] NO [OCT+] YES");
        return 1;
    case B_HOME:                                  /* wip317: RENAME PROJECT (wip211: it did nothing before, song mode is FUNC + SEQ) */
        rename_open();
        return 1;
    case B_SAVE:                                  /* wip263: QUICK SAVE into the loaded place, no question */
        quick_save();
        return 1;
    default: return 0;
    }
}

static void button_tap(int b)
{
    if (perf_testing()) {                             /* the test owns the transport: HOME or PLAY stop it */
        if ((b == B_HOME || b == B_PLAY) && perf.phase != 3)
            perf_test_abort();
        return;
    }
    if (ui.overlay == O_ASK) {                        /* a question takes SEL or HOME only (a preset: OCT+ / OCT-) */
        if (ui.ask_act == ACT_STOPFIRST) {              /* the save was refused while playing: PLAY stops it, OCT- goes back, OCT+ tries again */
            if (b == B_PLAY && seq.playing)
                seq_stop(&seq);
            else if (b == B_OCTDN)
                {
                    int fx = ui.sv_fixed;
                    ask(ACT_SAVEAS, fx ? 1 : SV_HERE, "", "");
                    if (fx)
                        name_enter();
                }
            else if (b == B_OCTUP)
                saveas_go();
            else if (b == B_HOME) {
                ui.overlay = O_NONE;
                say("CANCELLED", 0);
            }
            return;
        }
        if (ui.ask_act == ACT_SAVEAS) {                 /* wip317: SAVE HERE? (3) -OCT-> CHOOSE SLOT (4) -OCT+> slot (2) -OCT+> name (1) -OCT+> saves; OCT- goes back; HOME cancels */
            int st = (int)ui.ask_arg;
            if (b == B_OCTDN) {
                if (st == 1 && proj.set.name_manual && name_delete()) {
                    ;                                          /* wip324: a letter taken back */
                } else if (ui.sv_rename || st == SV_CHOOSE || st == SV_HERE) {      /* RENAME has only the name screen; wip708: SAVE HERE? OCT- = cancel */
                    ui.overlay = O_NONE;
                    say("CANCELLED", 0);
                } else if (st == 2)
                    ui.ask_arg = SV_HERE;
                else if (st == 1) {
                    ui.ask_arg = SV_HERE;                      /* wip708: the letter editor: OCT- with no letter goes back to SAVE HERE? */
                    sv_here_look();
                }
            } else if (b == B_OCTUP) {
                if (st == SV_HERE) {                       /* YES: the place shown (wip319: ALGORITHM may have changed it), the project's name, at once */
                    ui.overlay = O_NONE;
                    if (ui.sv_here == ui.preset_cur && !ui.sv_newname)
                        quick_write();
                    else
                        quick_write_to(ui.sv_here, ui.sv_pn);
                } else if (st == SV_CHOOSE) {
                    ui.ask_arg = 2;
                    ui.sv_here = (uint8_t)next_free_place(ui.preset_cur);   /* wip343: the next empty place (all full: the next one) */
                    sv_sync();
                    sv_here_look();
                } else if (st == 2) {
                    ui.ask_arg = 1;
                    name_enter();
                } else if (proj.set.name_manual) {          /* wip324: the letter editor: OCT+ accepts a letter (the name full: saves) */
                    if (!name_accept() && name_trim())
                        saveas_go();
                } else
                    saveas_go();
            } else if (b == B_SAVE && st == 1 && proj.set.name_manual) {   /* wip324: the SAVE button finishes a manual name */
                if (name_trim())
                    saveas_go();
                else
                    warn("TYPE A LETTER FIRST", 0);
            } else if (b == B_HOME) {
                ui.overlay = O_NONE;
                say("CANCELLED", 0);
            }
            return;
        }
        if (ui.ask_act == ACT_CHAIN) {                  /* OCT+ = OK, OCT- = take the last off (empty: cancel), HOME cancels */
            if (b == B_OCTUP)
                chain_go();
            else if (b == B_OCTDN) {
                if (ui.ch_n)
                    ui.ch_n--;
                else {
                    ui.overlay = O_NONE;
                    say("CANCELLED", 0);
                }
            } else if (b == B_HOME) {
                ui.overlay = O_NONE;
                say("CANCELLED", 0);
            } else if (b == B_PLAY && seq.playing)
                seq_stop(&seq);
            return;
        }
        if (octask()) {                                  /* OCT+ = confirm, OCT- / HOME go back */
            if (b == B_OCTUP) {
                if (ui.ask_act == ACT_SONGEXIT)
                    songexit_go();
                else if (ui.ask_act == ACT_SONGMODE)
                    songmode_go();
                else {
                    ui.overlay = O_NONE;
                    run_action(ui.ask_act, ui.ask_arg);
                }
            } else if (b == B_OCTDN || b == B_HOME) {
                ui.overlay = O_NONE;
                say("CANCELLED", 0);
            } else if (b == B_PLAY && seq.playing && ui.ask_act == ACT_SONGMODE)
                seq_stop(&seq);
            return;
        }
        if (ui.ask_act == ACT_PRESET && (b == B_OCTUP || b == B_OCTDN)) {   /* OCT+ = UPLOAD, OCT- = GO BACK */
            uint32_t pn = 0;
            ui.overlay = O_NONE;
            if (b == B_OCTDN)
                say("PRESET KEPT", 0);
            else if (!preset_blob(ui.ask_arg, &pn) && ui.ask_arg < NDEMO)
                warn("PRESET EMPTY", 0);
            else if (seq.playing || seq.req_stop) {      /* playing: ask again, the playing would stop */
                char t[28];
                preset_label(ui.ask_arg, t);
                ask(ACT_PRESETGO, ui.ask_arg, "CHANGE PRESETS?", t);
            } else
                run_action(ui.ask_act, ui.ask_arg);
            return;
        }
        if (ui.ask_act == ACT_QSAVE) {                   /* wip265: OCT+ = YES: save over it, the same name; OCT- / HOME = NO */
            ui.overlay = O_NONE;
            if (b == B_OCTUP)
                quick_write();
            else if (b == B_OCTDN || b == B_HOME)
                say("NOT SAVED", 0);
            else
                ui.overlay = O_ASK;
            return;
        }
        if (ui.ask_act == ACT_PRESETGO) {                /* OCT+ = YES: stop, then load; OCT- / HOME = NO */
            if (b == B_OCTUP) {
                ui.overlay = O_NONE;
                if (seq.playing)
                    seq_stop(&seq);
                ui.preset_pend = (uint8_t)(ui.ask_arg + 1);   /* ui_frame loads it as soon as the sequencer has stopped */
            } else if (b == B_OCTDN || b == B_HOME) {
                ui.overlay = O_NONE;
                say("PRESET KEPT", 0);
            }
            return;
        }
        if (b == B_SEL) {
            ui.overlay = O_NONE;
            run_action(ui.ask_act, ui.ask_arg);
        } else if (b == B_HOME) {
            ui.overlay = O_NONE;
            say("CANCELLED", 0);
        } else
            return;
        if (ui.view == V_GLO)
            ui.overlay = O_LIST;
        return;
    }
    switch (b) {
    case B_PLAY:
        if (ui.btn & (1u << B_HOME))                 /* wip417: HOME + PLAY is no longer redo (GLO + FILL is): plain start / stop, HOME's own release stays quiet */
            ui.btn_used |= 1u << B_HOME;
        if (seq.playing)
            seq_stop(&seq);
        else {
            seq.song_start = ui.song_sel < song()->len ? ui.song_sel : 0;
            seq_start(&seq);
        }
        break;
    case B_REC:
        if (ui.btn & (1u << B_HOME))                 /* wip417: HOME + REC is no longer undo (GLO + ACCENT is): plain record on / off, HOME's own release stays quiet */
            ui.btn_used |= 1u << B_HOME;
        ui.rec = !ui.rec;
        say(ui.rec ? "RECORD ON: KEYS + KNOBS" : "RECORD OFF", 0);
        break;
    case B_HOME:                                      /* MIX PAGE 2, PERFORM, GLOBAL, MIX PAGE 2 again; from another page: the HOME page you left (wip696) */
        note_groups();
        if (ui.overlay != O_NONE || ui.view == V_GLO)
            close_overlay();
        else if (ui.view == V_MIX2 && cur_page() == 0) {  /* wip695: MIX PAGE 2 (level, pan) -> PERFORM */
            *page_of(V_FX) = 0;
            set_view(V_FX);
        } else if (ui.view == V_FX && cur_page() == 0) {  /* PERFORM -> GLOBAL */
            set_view(V_GLOB);
        } else if (ui.view == V_GLOB) {                   /* GLOBAL -> MIX PAGE 2 */
            set_view(V_MIX2);
            *page_of(V_MIX2) = 0;
            build_pages();
        } else if (home_mem == 1) {                       /* wip696: from another page: PERFORM again (with its MUTE / SOLO page if that was open) */
            *page_of(V_FX) = 0;
            set_view(V_FX);
            restore_sub(V_FX);
        } else if (home_mem == 2) {
            set_view(V_GLOB);
        } else {
            set_view(V_MIX2);
            *page_of(V_MIX2) = 0;
            build_pages();
        }
        break;
    case B_SEL:
        if (ui.overlay == O_LIST)
            list_enter();
        else if (ui.view == V_PERF || ui.view == V_SONG)
            open_list_of_pages();                     /* only these two keep the list: it holds their actions (TEST, song insert / delete) */
        else
            set_view(V_PMOD);                         /* PITCH MOD (no more "this screen as a list") */
        break;
    case B_EDIT:
        if (!tonic_part() && ui.view == V_PART && ui.overlay == O_NONE)   /* the break keeps its old pages */
            *page_of(V_PART) = (uint8_t)((cur_page() + 1) % pg.n);
        else if (!tonic_part())
            set_view(V_PART);
        else {
            set_view(V_NEV);                          /* wip290: the Tonic: NOISE ENVELOPE (VEL moved to the GLO button, 1st press) */
            restore_sub(V_NEV);                       /* wip696: and its CHOKE screen if you left it there */
        }
        break;
    case B_ARP:                                       /* wip251: the ARP page: the SEQ view with a piano for keys */
        if (tonic_part() && !(ui.view == V_SEQ && ui.arp && ui.overlay == O_NONE)) {
            ui.arp = 1;
            ui.ac = 0;                                /* wip287: the ARP page has voices only */
            set_view(V_SEQ);
        }
        break;
    case B_ENV:                                       /* OSCILLATOR (hold + knob / key keep their old jobs; a 2nd press: later) */
        set_view(V_OSC);
        break;
    case B_FX:                                        /* MIX; again: SENDS, then MIX; from another page: the last FX page you left (wip696) */
        note_groups();
        if (ui.view == V_MIX1 && ui.overlay == O_NONE) {   /* wip695: MIX -> SENDS */
            set_view(V_MIX2);
            *page_of(V_MIX2) = 1;
            build_pages();
        }
        else if (ui.view == V_MIX2 && cur_page() == 1 && ui.overlay == O_NONE) {   /* wip695: SENDS (and its DUCK screen) -> MIX */
            set_view(V_MIX1);
        }
        else if (ui.view == V_FX && cur_page() != 0 && ui.overlay == O_NONE) {   /* REVERB / DELAY (long press pages) -> MIX */
            ui.msp = 0;
            set_view(V_MIX1);
        }
        else if (ui.fx_mem == 1) {                    /* wip696: from another page: SENDS again (its DUCK screen too if it was open) */
            set_view(V_MIX2);
            *page_of(V_MIX2) = 1;
            build_pages();
            restore_sub(V_MIX2);
        } else if (ui.fx_mem == 2 && fx_pg >= 1) {    /* wip696: the REVERB / DELAY page you left */
            *page_of(V_FX) = fx_pg;
            set_view(V_FX);
        } else
            set_view(V_MIX1);
        break;
    case B_LFO:                                       /* NOISE (the old mixer / master pages are no longer reached from here; 2nd press: later) */
        set_view(V_NOISE);
        break;
    case B_GLO:                                       /* wip290: 1st press = VEL, then the MIXER, COMP, MEMORY, the settings list, and back to VEL */
        if (ui.view == V_GLO && ui.overlay == O_NONE)
            ui.overlay = O_MEM;                       /* wip267: COMP -> MEMORY (the battery) */
        else if (ui.view == V_GLO && ui.overlay == O_MEM)
            open_global();                            /* MEMORY -> the settings list */
        else if (ui.view == V_GLO) {
            ui.overlay = O_NONE;                      /* the list -> back to the first page, VEL */
            set_view(V_VEL);
        }
        else if (ui.view == V_MIX)
            set_view(V_GLO);                          /* the MIXER -> COMP */
        else if (ui.view == V_VEL && ui.overlay == O_NONE)
            set_view(V_MIX);                          /* VEL -> the MIXER */
        else {
            ui.part = PART_909;
            if (glo_mem == 1)                         /* wip697: from another page: the GLO page you left (MIXER, COMP, MEMORY, SETTINGS) */
                set_view(V_MIX);
            else if (glo_mem >= 2) {
                set_view(V_GLO);
                if (glo_mem == 3)
                    ui.overlay = O_MEM;
                else if (glo_mem == 4)
                    open_global();
            } else
                set_view(V_VEL);
        }
        break;
    case B_SEQ:                                       /* the part's steps; again: the next page */
        if (ui.cpexit) {                              /* wip422: SEQ pressed in the COPY-PASTE MODE: out of the mode, on the SEQUENCER page (not the pattern page) */
            ui.cpexit = 0;
            ui.arp = 0;
            ui.seqpat = 0;
            set_view(V_SEQ);
        } else if (ui.arp) {                                 /* wip251: from the ARP page (or after it): the SEQ page itself */
            ui.arp = 0;
            set_view(V_SEQ);
        } else if (ui.view == V_SEQ && ui.overlay == O_NONE && tonic_part())
            ui.seqpat ^= 1u;                          /* the Tonic: 2nd press = the pattern number screen, 3rd = back */
        else if (ui.view == V_SEQ && ui.overlay == O_NONE)
            *page_of(V_SEQ) = (uint8_t)((cur_page() + 1) % pg.n);
        else {
            set_view(V_SEQ);
            restore_sub(V_SEQ);                       /* wip696: the pattern page again if you left it there */
        }
        break;
    case B_SAVE: saveas_open(); break;
    case B_OCTUP:
    case B_OCTDN: {
        int up = b == B_OCTUP;
        if (ui.view == V_SEQ && ui.arp && tonic_part() && ui.overlay == O_NONE) {   /* wip251 (back in wip693): the ARP page: OCT- / OCT+ = the keyboard's octave (the voice's pitch stays) */
            int o = ui.arp_oct + (up ? 1 : -1);
            char t[20];
            ui.arp_oct = (int8_t)(o < -3 ? -3 : o > 3 ? 3 : o);
            put_s(put_s(t, "OCTAVE "), ARPO_N[ui.arp_oct + 3]);
            say(t, 0);
        } else if (ui.view == V_SONG && tonic_part() && ui.overlay == O_NONE && !(ui.mod_held & 4u)) {   /* wip608: the SONG screen: OCT- deletes the line, OCT+ adds one (no octave change here) */
            if (up)
                song_add_cur();
            else if (song()->len) {
                song_del_cur();
                if (song()->len)
                    say("LINE DELETED", 0);
            }
        } else if (up && tonic_part() && (ui.mod_held & 4u)) {   /* (wip211: FUNC + OCT+ does nothing now) */
            ;
        } else if (tonic_part()) {                    /* the Tonic, any page: the selected voice's pitch an octave up / down */
            transpose_voice(up);
        } else if (ui.view == V_HOME || ui.view == V_SONG) {   /* the pattern bank: 1-16, 17-32 */
            ui.pbank = (uint8_t)up;
            say(up ? "PATTERNS 17-32" : "PATTERNS 1-16", 0);
        } else {
            char t[20];
            int pg_ = ui.spage + (up ? 1 : -1);
            ui.spage = (uint8_t)(pg_ < 0 ? 0 : pg_ > NSTEPS / 16 - 1 ? NSTEPS / 16 - 1 : pg_);
            put_i(put_s(put_i(put_s(t, "STEPS "), ui.spage * 16 + 1), "-"), ui.spage * 16 + 16);
            say(t, 0);
        }
        break;
    }
    default: break;
    }
}

/* wip356: FILL (black key 10) held + SEQ: back to the SONG screen from any page, in song mode (not in song mode: a message). Not on the ARP page (there FILL is a piano key). Returns 1 = done. */
static int fill_combo(int b)
{
    if (b != B_SEQ || !tonic_part() || !(ui.keys & (1u << BLACK_KEY[9])) || (ui.mod_held & 4u) || ui.overlay == O_ASK || perf_testing() ||
        (ui.view == V_SEQ && ui.arp))
        return 0;
    ui.fdel = 0;
    if (!ui.songmode)
        warn("NOT IN SONG MODE", 0);
    else if (ui.view != V_SONG || ui.overlay != O_NONE)
        set_view(V_SONG);
    return 1;
}

static uint32_t rkeys;                                 /* wip385: the REMOTE PANEL's keys held (bit k = panel key k), OR-ed into the real keys in input() */
static void input(void)
{
    uint32_t btn = plat_buttons(), keys = plat_keys() | rkeys, ch, i;   /* wip385: rkeys = the remote panel's keys */
    int e;
    if (ui.lowbatt) {                                  /* wip605: LOW BATTERY: only HOME is heard (it leaves); keys, buttons and knobs wait */
        for (i = 0; i < NE; i++)
            enc((int)i);
        if ((btn & ~ui.btn) & (1u << B_HOME)) {
            ui.lowbatt = 0;
            ui.btn_used |= 1u << B_HOME;               /* its release is not a HOME tap */
        }
        ui.btn = btn;
        ui.keys = keys;
        return;
    }
    ch = btn ^ ui.btn;
    for (i = 0; i < NB; i++) {
        uint32_t m = 1u << i;
        if (!(ch & m))
            continue;
        if (btn & m) {
            any_button = 1;
            ui.btn_used &= ~m;
            ui.down_t[i] = plat_ms();
            int sc;
            if (i != B_EDIT)
                ui.adel = 0;                           /* wip367: another button pressed: the FUNC + EDIT tap is cancelled */
            sc = shift_combo((int)i);
            ui.fdel = 0;                               /* wip356: a button pressed with FILL held: FILL is not a delete */
            ui.cpexit = (uint8_t)(ui.cpm && i == B_SEQ);   /* wip422: SEQ leaves the mode and opens the SEQUENCER page */
            ui.cpm = 0;                                /* wip362: any button leaves the COPY-PASTE MODE */
            ui.cpf = 0;
            if (sc == 0)
                sc = fill_combo((int)i);               /* wip356: FILL + SEQ = the SONG screen */
            if (sc == 1)
                ui.btn_used |= m;                      /* FUNC + the button: its own action does not follow */
            else if (sc == 0 && (i == B_REC || i == B_PLAY))   /* transport acts on the press: timing */
                button_tap((int)i);
        } else if (i == B_EDIT && ui.adel) {              /* wip367: FUNC + EDIT tapped alone: delete the selected track's automation (asks) */
            int v = ui.sel[PART_909];
            ui.adel = 0;
            if (tonic_part() && ui.overlay == O_NONE && !perf_testing() && v >= 0 && v < TN_VOICES) {
                if (tn_auto_n(v) == 0)
                    warn("NO AUTOMATION ON ", engine_voice_name(T_909, v));
                else
                    ask(ACT_AUTODEL, v, "DELETE AUTOMATION?", "[OCT+] CONFIRM [OCT-] GO BACK");
            }
        } else if (!(ui.btn_used & m) && i != B_REC && i != B_PLAY) {
            button_tap((int)i);
        }
    }
    if (btn || keys || btn != ui.btn || keys != ui.keys)
        ui.act_t = plat_ms();                         /* held or changed: being played */
    help_tick(btn, keys);
    ui.btn = btn;
    ch = keys ^ ui.keys;
    ui.keys = keys;
    if (ch) {                                          /* wip385: ACCENT / FILL / FUNC go down first and come up last, so a chord (a MIDI keyboard or the Digitakt) works in any note order */
        uint32_t mods = 0, pass[4];
        for (i = 0; i < NKEYS; i++)
            if (KEY_BLACK[i] >= 8)
                mods |= 1u << i;
        pass[0] = ch & keys & mods;                    /* modifiers down */
        pass[1] = ch & keys & ~mods;                   /* the other keys down */
        pass[2] = ch & ~keys & ~mods;                  /* the other keys up */
        pass[3] = ch & ~keys & mods;                   /* modifiers up */
        for (e = 0; e < 4; e++)
            for (i = 0; i < NKEYS; i++)
                if (pass[e] & (1u << i))
                    key_event((int)i, (keys >> i) & 1u);
    }
    build_pages();
    if (!seq.playing)                                  /* PLAY plays what is on screen: the song on SONG, */
        seq.song_on = tonic_part() ? ui.songmode : ui.view == V_SONG;   /* Tonic: only in SONG MODE; else the patterns */
    seq.song_hold = (uint8_t)(tonic_part() && ui.songmode && !seq.chain_on);   /* SONG OFF: loop this bar */
    seq.song_rec = (uint8_t)(seq.song_on && ui.rec);  /* the song + REC: the song is being written */
    if (ui.overlay == O_ASK && ui.ask_act == ACT_STOPFIRST && !seq.playing && !seq.req_stop)
        ask(ACT_SAVEAS, SV_HERE, "", "");              /* stopped: back to SAVE HERE?, OCT+ saves (wip708) */
    if (ui.overlay == O_ASK) {                         /* a question: the knobs wait (the PRESETS knob moves a preset's choice) */
        for (i = 0; i < NE; i++) {
            int d = enc((int)i);
            if (d && i == EN_PRESET && ui.ask_act == ACT_PRESET) {
                preset_ask(preset_next((int)ui.ask_arg, d > 0 ? 1 : -1));
            }
            if (d && i == EN_K4 && ui.ask_act == ACT_SAVEAS && name_manual_on())
                name_turn(d);                              /* wip324: K4 cycles the blinking letter */
            if (d && i == EN_SELECT && ui.ask_act == ACT_SAVEAS && ui.ask_arg == SV_HERE) {   /* wip708: SELECT = another spot (1-40, round); OCT+ saves there */
                ui.sv_here = (uint8_t)((ui.sv_here - 1 + NPRESETS + (d > 0 ? 1 : -1)) % NPRESETS + 1);
                sv_sync();
                sv_here_look();
            }
            if (d && i == EN_ALGO && ui.ask_act == ACT_SAVEAS) {
                if (ui.ask_arg == 1) {
                    if (!proj.set.name_manual)
                        elf_name(ui.sv_name);
                }
                else if (ui.ask_arg == SV_HERE) {          /* wip708: ALGORITHM = another name (MANUAL names: the letter editor) */
                    if (proj.set.name_manual) {
                        ui.ask_arg = 1;
                        name_enter();
                    } else {
                        elf_name(ui.sv_name);
                        put_s(ui.sv_pn, ui.sv_name);
                        ui.sv_newname = 1;
                    }
                } else if (ui.ask_arg == 2) {
                    ui.sv_here = (uint8_t)((ui.sv_here - 1 + NPRESETS + (d > 0 ? 1 : -1)) % NPRESETS + 1);   /* wip343: 1-40, round */
                    sv_sync();
                    sv_here_look();
                }
            }
        }
        return;
    }
    if (perf_testing()) {                              /* the test owns the knobs too */
        for (i = 0; i < NE; i++)
            enc((int)i);
        return;
    }
    if ((e = enc(EN_SELECT)) != 0) {             /* SELECT: move; with HOME held: tempo */
        if (((ui.mod_held & 4u) || (ui.view == V_SEQ && ui.arp && (ui.keys & (1u << BLACK_KEY[10])))) && tonic_part() && ui.overlay == O_NONE && !ui.help &&
            (ui.view == V_SEQ || ui.view == V_SONG || ui.view == V_HOME)) {   /* (on the ARP page every key is a piano key, so FUNC is read from the held keys, as FUNC + PLAY does) */
            ui.pbank = (uint8_t)(e > 0);              /* wip685: FUNC + SELECT: the pattern bank, right = bank 2 (patterns 17-32), left = bank 1 (1-16), on SEQ, ARP, the song screen and HOME */
            say(ui.pbank ? "PATTERN BANK 2" : "PATTERN BANK 1", 0);
        } else if (held_any_step() >= 0 && !(ui.btn & (1u << B_HOME))) {
            prob_turn(e);                             /* a step held: its chance */
        } else if (ui.btn & (1u << B_HOME)) {
            float bpm = seq.bpm + (float)accel(EN_SELECT, e, 200);
            seq.bpm = bpm < 20.0f ? 20.0f : bpm > 275.0f ? 275.0f : bpm;
            ui.btn_used |= 1u << B_HOME;
            ui.touched = 4;
            ui.touch_until = plat_ms() + 600u;
            mark_dirty();
        } else if (ui.overlay == O_LIST) {
            list_move(e > 0 ? 1 : -1);
        } else if (ui.view == V_SEQ && ui.overlay == O_NONE && tonic_part() && !ui.seqpat) {   /* wip699: SEQ and ARP pages: SELECT = the track (round); on SEQ after track 8 comes AC (the global accent) (was ALGORITHM, wip287) */
            if (!ui.help) {
                int n = ui.arp ? NDRUM_TONIC : NDRUM_TONIC + 1, v = ui.ac ? NDRUM_TONIC : (int)ui.sel[PART_909];
                v += e > 0 ? 1 : -1;
                v = v < 0 ? n - 1 : v >= n ? 0 : v;
                ui.ac = (uint8_t)(v == NDRUM_TONIC);
                if (!ui.ac)
                    ui.sel[PART_909] = (uint8_t)v;
                build_pages();
                say(ui.ac ? "ACCENT" : engine_voice_name(T_909, ui.sel[PART_909]), 0);
            }
        } else if (ui.view == V_MIX && ui.overlay == O_NONE && !ui.help && pg.n > 1) {
            int p = cur_page() + (e > 0 ? 1 : -1);        /* wip334: the GLO mixer: 1 2 3 4 then 1 again (right), 4 3 2 1 then 4 (left): round */
            *page_of(ui.view) = (uint8_t)(p < 0 ? pg.n - 1 : p >= pg.n ? 0 : p);
        } else {
            int p = cur_page() + (e > 0 ? 1 : -1);
            *page_of(ui.view) = (uint8_t)(p < 0 ? 0 : p >= pg.n ? pg.n - 1 : p);
        }
    }
    if ((e = enc(EN_ALGO)) != 0) {               /* ALGORITHM: the part; in a list, the value */
        if (ui.overlay == O_LIST) {
            turn_ref(list_rows[ui.list_sel], e, EN_ALGO);
        } else if (ui.view == V_SEQ && ui.overlay == O_NONE && tonic_part() && !ui.seqpat && !proj.set.autoscroll) {   /* wip699: SEQ and ARP pages, AUTOSCROLL off: ALGORITHM turns the step page 1-16 / 17-32 / 33-48 / 49-64 (round) (was SELECT, wip240) */
            char t[20];
            int n = (seq_part_len(&seq, TRK_DRUM + ui.part) + 15) / 16, pg_ = (int)ui.spage + (e > 0 ? 1 : -1);   /* wip241: only the pages the pattern length has (16 = one page, 32 = two ...) */
            if (n < 1)
                n = 1;
            if (n > NSTEPS / 16)
                n = NSTEPS / 16;
            if (ui.spage >= n)
                ui.spage = 0;
            if (n > 1) {
                ui.spage = (uint8_t)(pg_ < 0 ? n - 1 : pg_ >= n ? 0 : pg_);
                spage_until = plat_ms() + 1200u;      /* wip583: the 3rd knob shows the page (1-16, 17-32 ...) */
                put_i(put_s(put_i(put_s(t, "STEPS "), ui.spage * 16 + 1), "-"), ui.spage * 16 + 16);
                say(t, 0);
            }
        } else if (ui.view == V_SONG && ui.overlay == O_NONE && tonic_part() && !ui.help) {   /* wip624: the SONG screen: ALGORITHM moves the bar row like K1 (0 .. END, stops at the ends) */
            int mx = song()->len < NSONG ? (int)song()->len : NSONG - 1, nr = (int)ui.song_sel + (e > 0 ? 1 : -1);
            ui.song_sel = (uint8_t)(nr < 0 ? 0 : nr > mx ? mx : nr);
        } else if (ui.view == V_MIX2 && (ui.dck || cur_page() == 1) && ui.overlay == O_NONE && tonic_part() && !ui.help) {   /* wip350: SENDS: ALGORITHM right = the DUCK screen, left = back */
            ui.dck = (uint8_t)(e > 0);
        } else if (ui.view == V_NEV && ui.overlay == O_NONE && tonic_part() && !ui.help) {   /* wip333: EDIT: ALGORITHM right = show who is in the CHOKE group, left = back */
            ui.chk = (uint8_t)(e > 0);
        } else if (msp_on() && ui.view == V_FX && !ui.help) {        /* wip272: PERFORM <-> MUTE / SOLO page (both stay in PERFORM; the mode and the mutes / solos are shared) */
            ui.msp = (uint8_t)(e > 0);                 /* wip341: by DIRECTION (right = the MUTE / SOLO page, left = back), not a toggle: a second tick of one click no longer hides it at once */
        } else if (ui.view == V_FX && ui.overlay == O_NONE && cur_page() >= 1 && !ui.help && pg.n >= 5) {   /* wip270: REVERB <-> REVERB 2, DELAY <-> DELAY 2 (pages 1 <-> 3, 2 <-> 4) */
            int c = cur_page();                        /* wip341: by direction too: right = the 2nd page, left = back */
            if (e > 0 && c <= 2)
                *page_of(V_FX) = (uint8_t)(c + 2);
            else if (e < 0 && c >= 3)
                *page_of(V_FX) = (uint8_t)(c - 2);
        }                                         /* wip133: outside a list (and the FX pages) ALGO does nothing: the Tonic is the only part (no 808, no BREAK to switch to) */
    }
    if ((e = enc(EN_PRESET)) != 0 && ui.part == PART_909) {   /* PRESETS on the Tonic: a factory preset, asked first */
        preset_ask(preset_next(ui.preset_cur ? ui.preset_cur - 1 : (e > 0 ? NPRESETS - 1 : 0), e > 0 ? 1 : -1));
    } else if (e != 0) {                         /* PRESETS: a part's screen, that part; else all five */
        int part = part_view() ? ui.part : PART_909, c = seq_cue_of(&seq, part);
        int p = (c >= 0 ? c : seq.ppat[part]) + (e > 0 ? 1 : -1);
        p = p < 0 ? 0 : p >= NPAT ? NPAT - 1 : p;
        seq_chain(&seq, 0, 0);
        if (part_view())
            seq_cue_part(&seq, part, p);
        else
            seq_cue(&seq, p);
        ui.pbank = (uint8_t)(p / 16);                 /* HOME shows the bank it is in */
    }
    for (i = 0; i < 4; i++) {
        if ((e = enc(EN_K1 + (int)i)) == 0)
            continue;
        if (ui.seqpat && ui.view == V_SEQ) {              /* the pattern screen: only K1 = CHAIN ON / OFF (right = on, left = off); no hidden value changes */
            if (i == 0 && tonic_part() && seq.chain_on != (e > 0)) {
                seq.chain_on = e > 0 ? 1u : 0u;
                say(seq.chain_on ? "SONG ON" : "SONG OFF", 0);
            }
            continue;
        }
        if (ui.view == V_GLO && (ui.overlay == O_LIST || ui.overlay == O_MEM))     /* the settings list / the memory page: the knobs do nothing (the list has SELECT / ALGO) */
            continue;
        if (i == 1 && ui.view == V_NEV && ui.overlay == O_NONE && tonic_part()) {   /* wip328: EDIT, K2 = the CHOKE button: FUNC held + turn right = IN, left = OUT (without FUNC the big message HOLD FUNC + TURN K2: CHOKE; no zoom, no p-lock) */
            if (!(ui.btn & ((1u << B_SAVE) | (1u << B_ENV))) && lock_step() < 0) {   /* wip333: K2 right = the selected track joins the group, left = it leaves (the message says it); wip337: only with FUNC held */
                int v = (int)ui.sel[PART_909];
                if (!(ui.mod_held & 4u))
                    warn("HOLD FUNC + TURN KNOB2: CHOKE", "\nTURN ALGORITHM TO SEE CHOKE GROUP");   /* wip340 */
                else if (((seq.choke >> v) & 1u) != (e > 0)) {
                    seq.choke = (uint8_t)(e > 0 ? (seq.choke | (1u << v)) : (seq.choke & ~(1u << v)));
                    engine_tonic_choke(seq.choke);
                    chk_say(v);
                }
            }
            continue;
        }
        if (i == 3 && ui.view == V_VEL && ui.overlay == O_NONE) {   /* wip326: VEL page, K4 = RANDOM SOUND (wip327: FUNC held; no zoom, no p-lock, no ENV / SAVE use) */
            if (!(ui.btn & ((1u << B_SAVE) | (1u << B_ENV)))) {
                if (ui.mod_held & 4u)                  /* wip327: only with FUNC held */
                    tn_randomize(0);
                else
                    warn("HOLD FUNC + TURN KNOB4: RANDOM", 0);   /* wip340: KNOB4, not K4 */
            }
            continue;
        }
        if (i == 3 && ui.view == V_PMOD && ui.overlay == O_NONE) {  /* wip610: SEL page, K4 = RANDOM ALL (the purple button): FUNC held + turn */
            if (!(ui.btn & ((1u << B_SAVE) | (1u << B_ENV)))) {
                if (ui.mod_held & 4u)
                    tn_randomize(1);
                else
                    warn("HOLD FUNC + TURN KNOB4: RANDOM ALL", 0);
            }
            continue;
        }
        if (i == 2)
            spage_until = 0;                          /* wip583: K3 turned: the cell is FREE LEN again */
        if (ui.view == V_SEQ && ui.overlay == O_NONE && is_drum() && !ui.arp && !ui.seqpat && pg.r[cur_page()][i].kind == R_DLEN && proj.set.autoscroll) {   /* wip567: AUTOSCROLL on: the FREE LEN cell shows the page, the length is not changed (the cell says why) */
            ui.touched = (int8_t)i;
            ui.touch_until = plat_ms() + 1200u;
            warn("TURN AUTOSCROLL OFF", "TO CHANGE LENGTH");
            continue;
        }
        if (lock_step() >= 0) {                         /* EDIT, a step held: its p-lock */
            lock_turn(pg.r[cur_page()][i], e, EN_K1 + (int)i);
            continue;
        }
        if (ui.btn & (1u << B_SAVE)) {                  /* SAVE + knob: forget that knob's motion */
            ui.btn_used |= 1u << B_SAVE;
            motion_forget(pg.r[cur_page()][i]);
            continue;
        }
        if (ui.btn & (1u << B_ENV)) {                   /* ENV + knob: back to its power-on value (#19) */
            pref_t r = pg.r[cur_page()][i];
            const x0x_param_t *d = pref_desc(r);
            ui.btn_used |= 1u << B_ENV;
            if (d && r.kind != R_ACT && pref_get(r) != d->def) {
                pref_set(r, d->def);
                motion_touch(r);
            }
        } else
            turn_ref(pg.r[cur_page()][i], e, EN_K1 + (int)i);
        ui.touched = (int8_t)i;
        ui.touch_until = plat_ms() + 600u;
    }
}

/* ================================================================== drawing === */
#define HDR_H 20
#define MAIN_Y 20
#define BAND_H 69                     /* the main area: two bands of 69 (the knob row takes the 6 below) */
#define KNOB_Y 158
#define KNOB_H 82

static uint32_t blit_hash[6];
static uint32_t cv_hash(void)
{
    uint32_t i, h = 2166136261u, n = cv_w * cv_h;
    for (i = 0; i < n; i++)
        h = (h ^ cv_px[i]) * 16777619u;
    return h ^ (cv_w << 16) ^ cv_h;
}
static void cv_commit(int region, uint32_t x, uint32_t y)
{
    uint32_t h = cv_hash();
    if (h != blit_hash[region]) {
        blit_hash[region] = h;
        cv_blit(x, y);
    }
}

/* the main area sends only the rectangle that changed: the panel has no tearing-effect line the
 * FM-1 can see, so a transfer the panel's refresh overtakes shows half old, half new. The whole
 * area is 69 KB, 18 ms at 30 MHz, longer than a refresh, so every playhead step sheared; the two
 * columns a step changes are ~3 ms. Rows and 8-pixel column blocks are hashed against the last
 * frame sent; the changed rectangle is packed to the front of the canvas (it is rebuilt every
 * frame) and sent as one transfer. blit_hash[region] == 0 (a reset) sends it all. */
#define MAIN_ROWS (2 * BAND_H)
#define MAIN_COLB (240 / 8)
static uint32_t main_rowh[MAIN_ROWS], main_colh[MAIN_COLB];
static void cv_commit_rect(int region, uint32_t x, uint32_t y)
{
    uint32_t rowh[MAIN_ROWS], colh[MAIN_COLB], r, c, r0 = MAIN_ROWS, r1 = 0, c0 = MAIN_COLB, c1 = 0, w, h, k;
    for (c = 0; c < MAIN_COLB; c++)
        colh[c] = 2166136261u;
    for (r = 0; r < MAIN_ROWS; r++) {
        const uint16_t *q = cv_px + r * 240u;
        uint32_t hr = 2166136261u;
        for (c = 0; c < MAIN_COLB; c++) {
            uint32_t hc = colh[c];
            for (k = 0; k < 8u; k++) {
                hr = (hr ^ q[k]) * 16777619u;
                hc = (hc ^ q[k]) * 16777619u;
            }
            colh[c] = hc;
            q += 8;
        }
        rowh[r] = hr;
    }
    if (!blit_hash[region]) {                         /* after a reset: all of it */
        r0 = c0 = 0;
        r1 = MAIN_ROWS - 1;
        c1 = MAIN_COLB - 1;
    } else {
        for (r = 0; r < MAIN_ROWS; r++)
            if (rowh[r] != main_rowh[r]) {
                if (r0 == MAIN_ROWS)
                    r0 = r;
                r1 = r;
            }
        for (c = 0; c < MAIN_COLB; c++)
            if (colh[c] != main_colh[c]) {
                if (c0 == MAIN_COLB)
                    c0 = c;
                c1 = c;
            }
        if (r0 == MAIN_ROWS || c0 == MAIN_COLB)
            return;                                     /* nothing changed */
    }
    blit_hash[region] = 1;
    for (r = 0; r < MAIN_ROWS; r++)
        main_rowh[r] = rowh[r];
    for (c = 0; c < MAIN_COLB; c++)
        main_colh[c] = colh[c];
    w = (c1 - c0 + 1u) * 8u;
    h = r1 - r0 + 1u;
    for (r = 0; r < h; r++) {                         /* pack: the destination never passes the source */
        const uint16_t *src = cv_px + (r0 + r) * 240u + c0 * 8u;
        uint16_t *dst = cv_px + r * w;
        for (k = 0; k < w; k++)
            dst[k] = src[k];
    }
    lcd_blit(x + c0 * 8u, y + r0, w, h, cv_px);
}

static uint16_t blend(uint16_t a, uint16_t b, int t)  /* a..b, t of 16 */
{
    int r = ((a >> 11) * (16 - t) + (b >> 11) * t) >> 4, g = (((a >> 5) & 63) * (16 - t) + ((b >> 5) & 63) * t) >> 4,
        bl = ((a & 31) * (16 - t) + (b & 31) * t) >> 4;
    return (uint16_t)((r << 11) | (g << 5) | bl);
}
static uint16_t part_col(void) { return PART_COL[ui.part]; }
static uint16_t dim(uint16_t c, int t) { return blend(C_BG, c, t); }

static int32_t tw(const felucca_font_t *f, const char *s) { return text_w(f, s); }

/* a font's capital line: the first and last rows of 'H' (rounded half coverage), measured
 * once from the glyph bitmap, so text is centred on what the eye sees rather than on the
 * font's cell (which differs per face and size) */
typedef struct { const felucca_font_t *f; int8_t top, bot; } capm_t;
static capm_t capm[6];
static const capm_t *caps(const felucca_font_t *f)
{
    int i;
    for (i = 0; i < 6 && capm[i].f; i++)
        if (capm[i].f == f)
            return &capm[i];
    if (i == 6)
        i = 5;
    {
        uint32_t gi = ((uint32_t)'H' <= f->last ? 'H' : '0') - f->first, w = f->bw[gi], bpr = (w + 1u) / 2u, x, y;
        const uint8_t *gd = f->data + f->off[gi];
        int top = -1, bot = 0;
        for (y = 0; y < f->h; y++)
            for (x = 0; x < w; x++) {
                uint32_t a = gd[y * bpr + x / 2u];
                a = (x & 1u) ? (a & 15u) : (a >> 4);
                if (a >= 8u) {
                    if (top < 0)
                        top = (int)y;
                    bot = (int)y;
                }
            }
        capm[i].f = f;
        capm[i].top = (int8_t)(top < 0 ? 0 : top);
        capm[i].bot = (int8_t)bot;
    }
    return &capm[i];
}
/* parts of a line ("A 909 GR", "B 909 FL") with a real gap between them: a proportional face's
 * space is too narrow to separate them. A NULL ends the list. */
static int32_t segs(int32_t x, int32_t y, const felucca_font_t *f, uint16_t c, int32_t gap, const char *a,
                    const char *b, const char *d, const char *e)
{
    const char *p[4] = {a, b, d, e};
    int i;
    for (i = 0; i < 4 && p[i]; i++)
        x = cv_text(x, y, f, p[i], c) + gap;
    return x;
}

/* the y to draw f at so its capitals sit centred in [y0, y0 + h) */
static int32_t vc(const felucca_font_t *f, int32_t y0, int32_t h)
{
    const capm_t *m = caps(f);
    int32_t ch = m->bot - m->top + 1;
    return y0 + (h - ch + 1) / 2 - m->top;
}
/* the y to draw g at so its baseline matches f drawn at y */
static int32_t base_y(const felucca_font_t *f, int32_t y, const felucca_font_t *g)
{
    return y + caps(f)->bot - caps(g)->bot;
}
static void text_c(int32_t cx, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{
    cv_text(cx - tw(f, s) / 2, y, f, s, c);
}
static void text_r(int32_t rx, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{
    cv_text(rx - tw(f, s), y, f, s, c);
}

/* ---- anti-aliased shapes. Coverage is 0..16; a pixel is blended over what is there. ---- */
static void px_blend(int32_t x, int32_t y, uint16_t c, uint32_t a)
{
    uint16_t *q;
    uint32_t bg;
    y += cv_oy;
    if ((uint32_t)x >= cv_w || y < cv_y0 || y >= cv_y1 || !a)
        return;
    q = &cv_px[(uint32_t)y * cv_w + (uint32_t)x];
    if (a >= 16u) {
        *q = swap16(c);
        return;
    }
    bg = swap16(*q);
    *q = swap16((uint32_t)(((((c >> 11) * a + (bg >> 11) * (16u - a)) >> 4) << 11) |
                           (((((c >> 5) & 63u) * a + ((bg >> 5) & 63u) * (16u - a)) >> 4) << 5) |
                           (((c & 31u) * a + (bg & 31u) * (16u - a)) >> 4)));
}

/* corner coverage of a radius-r quarter circle, pixel (x, y) of the r x r corner square,
 * 4 x 4 supersampled; and of its 1-px ring (for outlines). Built once. */
#define RMAX 8
static uint8_t corner_fill[RMAX + 1][RMAX][RMAX], corner_ring[RMAX + 1][RMAX][RMAX];
static void corners_init(void)
{
    int r, x, y, sx, sy;
    for (r = 1; r <= RMAX; r++)
        for (y = 0; y < r; y++)
            for (x = 0; x < r; x++) {
                int in = 0, ring = 0;
                for (sy = 0; sy < 4; sy++)
                    for (sx = 0; sx < 4; sx++) {
                        float dx = (float)r - ((float)x + (sx + 0.5f) / 4.0f);
                        float dy = (float)r - ((float)y + (sy + 0.5f) / 4.0f);
                        float d2 = dx * dx + dy * dy;
                        if (d2 <= (float)(r * r)) {
                            in++;
                            if (d2 >= (float)((r - 1) * (r - 1)))
                                ring++;
                        }
                    }
                corner_fill[r][y][x] = (uint8_t)in;
                corner_ring[r][y][x] = (uint8_t)ring;
            }
}

/* a filled rounded rectangle; r is clamped to fit */
static void rbox(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t c)
{
    int32_t i, j;
    if (w <= 0 || h <= 0)
        return;
    if (r > w / 2)
        r = w / 2;
    if (r > h / 2)
        r = h / 2;
    if (r > RMAX)
        r = RMAX;
    if (r < 1) {
        cv_rect(x, y, w, h, c);
        return;
    }
    cv_rect(x + r, y, w - 2 * r, h, c);
    cv_rect(x, y + r, r, h - 2 * r, c);
    cv_rect(x + w - r, y + r, r, h - 2 * r, c);
    for (j = 0; j < r; j++)
        for (i = 0; i < r; i++) {
            uint32_t a = corner_fill[r][j][i];
            px_blend(x + i, y + j, c, a);
            px_blend(x + w - 1 - i, y + j, c, a);
            px_blend(x + i, y + h - 1 - j, c, a);
            px_blend(x + w - 1 - i, y + h - 1 - j, c, a);
        }
}

/* a 1-px rounded outline */
static void rframe(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t c)
{
    int32_t i, j;
    if (r > w / 2)
        r = w / 2;
    if (r > h / 2)
        r = h / 2;
    if (r > RMAX)
        r = RMAX;
    if (r < 1)
        r = 1;
    cv_rect(x + r, y, w - 2 * r, 1, c);
    cv_rect(x + r, y + h - 1, w - 2 * r, 1, c);
    cv_rect(x, y + r, 1, h - 2 * r, c);
    cv_rect(x + w - 1, y + r, 1, h - 2 * r, c);
    for (j = 0; j < r; j++)
        for (i = 0; i < r; i++) {
            uint32_t a = corner_ring[r][j][i];
            px_blend(x + i, y + j, c, a);
            px_blend(x + w - 1 - i, y + j, c, a);
            px_blend(x + i, y + h - 1 - j, c, a);
            px_blend(x + w - 1 - i, y + h - 1 - j, c, a);
        }
}

/* the radius that suits a box of height h: soft, never round */
static int32_t rad_of(int32_t h) { return h >= 30 ? 6 : h >= 18 ? 4 : h >= 9 ? 3 : 2; }
static void box(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c) { rbox(x, y, w, h, rad_of(h < w ? h : w), c); }
static void frame(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c) { rframe(x, y, w, h, rad_of(h < w ? h : w), c); }

/* an anti-aliased disc of radius r (float) at (cx, cy) */
static void disc(float cx, float cy, float r, uint16_t c)
{
    int32_t x0 = (int32_t)fm_floorf(cx - r - 1.0f), x1 = (int32_t)(cx + r + 1.0f);
    int32_t y0 = (int32_t)fm_floorf(cy - r - 1.0f), y1 = (int32_t)(cy + r + 1.0f), x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy, d = fm_sqrtf(dx * dx + dy * dy);
            float a = fm_clampf(r - d + 0.5f, 0.0f, 1.0f);
            px_blend(x, y, c, (uint32_t)(a * 16.0f + 0.5f));
        }
}
static void dot(int32_t x, int32_t y, int32_t r, uint16_t c) { disc((float)x + 0.5f, (float)y + 0.5f, (float)r + 0.3f, c); }

/* atan2 in turns, 0..1 from +x going clockwise on screen (y down) */
static float turns(float y, float x)
{
    float ax = fm_fabsf(x), ay = fm_fabsf(y), a = ax < ay ? ax / (ay + 1e-9f) : ay / (ax + 1e-9f), s = a * a, r;
    r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;   /* atan on [0, 1] */
    if (ay > ax)
        r = 1.57079637f - r;
    if (x < 0.0f)
        r = FM_PI - r;
    if (y < 0.0f)
        r = FM_TWO_PI - r;
    return r * (1.0f / FM_TWO_PI);
}

/* a knob: a 270-degree track from 7:30 to 4:30, thickness th, anti-aliased, round ends; the
 * value filled from the start (or from the top, for a bipolar value) and a round handle */
static void arc(int32_t cx, int32_t cy, int32_t r, float v, uint16_t track, uint16_t fill, int bipolar)
{
    const float th = 3.4f, a0 = 0.375f, sweep = 0.75f;  /* start at 135 degrees, in turns */
    float ro = (float)r, ri = ro - th, rm = ro - th * 0.5f, fx = (float)cx + 0.5f, fy = (float)cy + 0.5f;
    float lo = bipolar ? (v < 0.5f ? v : 0.5f) : 0.0f, hi = bipolar ? (v < 0.5f ? 0.5f : v) : v;
    int32_t x, y;
    v = fm_clampf(v, 0.0f, 1.0f);
    for (y = -r - 1; y <= r + 1; y++)
        for (x = -r - 1; x <= r + 1; x++) {
            float dx = (float)x + 0.5f - 0.5f, dy = (float)y + 0.5f - 0.5f, d = fm_sqrtf(dx * dx + dy * dy), t, a;
            a = fm_clampf(ro + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - ri + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns(dy, dx) - a0;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;                              /* the gap at the bottom; the caps close it */
            px_blend(cx + x, cy + y, (t >= lo && t <= hi) ? fill : track, (uint32_t)(a * 16.0f + 0.5f));
        }
    {   /* round ends: the track's, then the fill's, then the handle */
        float e0 = (a0) * FM_TWO_PI, e1 = (a0 + sweep) * FM_TWO_PI, ev = (a0 + sweep * v) * FM_TWO_PI;
        float el = (a0 + sweep * lo) * FM_TWO_PI;
        disc(fx + fm_cosf(e0) * rm - 0.5f, fy + fm_sinf(e0) * rm - 0.5f, th * 0.5f, lo <= 0.0f && hi > 0.0f ? fill : track);
        disc(fx + fm_cosf(e1) * rm - 0.5f, fy + fm_sinf(e1) * rm - 0.5f, th * 0.5f, hi >= 1.0f ? fill : track);
        if (hi > lo)
            disc(fx + fm_cosf(el) * rm - 0.5f, fy + fm_sinf(el) * rm - 0.5f, th * 0.5f, fill);
        disc(fx + fm_cosf(ev) * rm - 0.5f, fy + fm_sinf(ev) * rm - 0.5f, th * 0.5f + 1.3f, C_BG);
        disc(fx + fm_cosf(ev) * rm - 0.5f, fy + fm_sinf(ev) * rm - 0.5f, th * 0.5f + 0.6f, C_WHITE);
    }
}

/* a value's place on its knob, 0..1 (what the strip draws: a held step's own value, or recorded motion) */
static int pref_shown(pref_t r)                       /* the value to DRAW: a held step's lock, or recorded motion, else the knob's */
{
    int val = pref_get(r);
    if (lock_step() >= 0) {
        int lv = lock_of(r, lock_step());
        if (lv >= 0)
            val = lv;
    } else if (has_motion(r)) {
        int mv = motion_now(r);
        if (mv >= 0)
            val = mv;
    }
    return val;
}
static float pref_frac(pref_t r)
{
    const x0x_param_t *d = pref_desc(r);
    if (!d || !d->max)
        return 0.0f;
    return fm_clampf((float)pref_shown(r) / (float)d->max, 0.0f, 1.0f);
}
static int bipolar_of(pref_t r)
{
    return r.kind == R_SHHI ? 2 : (is_pan(r) || is_modamt(r));   /* wip296: 2 = the HIGH CUT: a ring that fills from the right end (off) as the knob turns left */
}

/* TONIC knob: a 270-degree ring of nseg bars (lit ones run from the theme's secondary blue to its
 * brightest), and, on the big one, a grey cap with a dark pointer. Anti-aliased, flat colours. */
#define C_SLBLUE RGB(63, 140, 240)           /* the lit colour of knobs AND sliders (wip73) */
#define C_SLTHUMB RGB(170, 210, 255)
/* the knob cap of the FM1 TONIC logo: white at the upper left to light grey, a grey (#7a7a7a) rim, one pass */
static void cap_logo(float cx, float cy, float rc)
{
    int32_t x0 = (int32_t)fm_floorf(cx - rc - 1.0f), x1 = (int32_t)(cx + rc + 1.0f);
    int32_t y0 = (int32_t)fm_floorf(cy - rc - 1.0f), y1 = (int32_t)(cy + rc + 1.0f), x, y;
    float hx = cx - 0.2f * rc, hy = cy - 0.3f * rc, rg = 1.6f * rc;      /* the highlight: as the logo's radial gradient */
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy, d = fm_sqrtf(dx * dx + dy * dy);
            float a = fm_clampf(rc + 1.0f - d, 0.0f, 1.0f), t, g;
            uint32_t gi;
            if (a <= 0.0f)
                continue;
            if (d > rc - 0.5f) {
                px_blend(x, y, RGB(122, 122, 122), (uint32_t)(a * 16.0f + 0.5f));      /* the rim */
                continue;
            }
            dx = (float)x + 0.5f - hx;
            dy = (float)y + 0.5f - hy;
            t = fm_clampf(fm_sqrtf(dx * dx + dy * dy) / rg, 0.0f, 1.0f);
            g = 255.0f - 75.0f * t;
            gi = (uint32_t)g;
            px_blend(x, y, RGB(gi, gi, gi), 16u);
        }
}
static int sk_red;                     /* wip283: 1 = the lit segments are red (a pan at the centre = mono) */
static void seg_knob(int32_t cx, int32_t cy, int32_t r, int nseg, float v, int bipolar, int big)
{
    const float a0 = 0.375f, sweep = 0.75f, gap = 0.34f;
    float ro = (float)r, ri = ro - (big ? (r >= 40 ? 10.0f : 6.0f) : 4.5f), rm = 0.5f * (ro + ri);
    float cell = sweep * FM_TWO_PI * rm / (float)nseg;          /* a bar's pitch in pixels */
    int rev = bipolar == 2;                              /* wip296: lit from the right end down to the knob (the HIGH CUT) */
    float lo, hi;
    int32_t x, y;
    if (rev)
        bipolar = 0;
    lo = bipolar ? (v < 0.5f ? v : 0.5f) : 0.0f;
    hi = bipolar ? (v < 0.5f ? 0.5f : v) : v;
    v = fm_clampf(v, 0.0f, 1.0f);
    for (y = -r - 1; y <= r + 1; y++)
        for (x = -r - 1; x <= r + 1; x++) {
            float dx = (float)x + 0.5f, dy = (float)y + 0.5f, d = fm_sqrtf(dx * dx + dy * dy), t, pos, fr, e, a;
            int k;
            a = fm_clampf(ro + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - ri + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns(dy, dx) - a0;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;
            pos = t * (float)nseg;
            k = (int)pos;
            if (k >= nseg)
                k = nseg - 1;
            fr = pos - (float)k;
            e = (fr - gap * 0.5f < 1.0f - gap * 0.5f - fr ? fr - gap * 0.5f : 1.0f - gap * 0.5f - fr) * cell;
            a *= fm_clampf(e + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            {
                float f = ((float)k + 0.5f) / (float)nseg;
                int on = rev ? (v < 1.0f && f >= v - 0.5f / (float)nseg) : bipolar ? (f >= lo && f <= hi && v != 0.5f) : (v > 0.0f && f <= v + 0.5f / (float)nseg);
                uint16_t col = on ? (sk_red ? RGB(255, 50, 50) : C_SLBLUE) : C_DIM;
                px_blend(cx + x, cy + y, col, (uint32_t)(a * 16.0f + 0.5f));
            }
        }
    if (big) {                                          /* the cap (the logo knob) and its blue pointer */
        float fx = (float)cx + 0.5f, fy = (float)cy + 0.5f, rc = ri - 2.5f, ev = (a0 + sweep * v) * FM_TWO_PI, q;
        float pr = rc * 0.085f;
        cap_logo((float)cx, (float)cy, rc);
        if (pr < 1.35f)
            pr = 1.35f;
        for (q = 0.0f; q <= rc * 0.8f; q += 1.0f)
            disc(fx + fm_cosf(ev) * q - 0.5f, fy + fm_sinf(ev) * q - 0.5f, pr, RGB(33, 70, 200));
    }
}

/* wip305: CLASSIC engine: a dark shade over a knob that is switched off (the lit ring and the cap stay visible but dull, so it is clear it cannot be used) */
static void shade_rect(int32_t x, int32_t y, int32_t w, int32_t h)
{
    int32_t i, j;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            px_blend(x + i, y + j, C_BG, 12u);
}

static void play_icon(int32_t x, int32_t y, uint16_t c)
{
    int32_t i, j;
    for (j = 0; j < 11; j++)                           /* a triangle, edges anti-aliased */
        for (i = 0; i < 9; i++) {
            float fy = (float)j + 0.5f - 5.5f, fx = (float)i + 0.5f;
            float edge = 8.0f - fx - fm_fabsf(fy) * 1.3f;
            float a = fm_clampf(edge * 0.7f + 0.5f, 0.0f, 1.0f) * fm_clampf(fx + 0.5f, 0.0f, 1.0f);
            px_blend(x + i, y + j, c, (uint32_t)(a * 16.0f + 0.5f));
        }
}

/* the length of the part on screen, when its keys are steps (0: they are not) */
static int steps_len(void)
{
    if (is_drum())
        return cur_pat()->drum[ui.part].len;
    return 0;
}

/* -------------------------------------------------------------- header --- */
/* wip550: GRIGIOTONIC header text in Poppins (gt_text is defined further down; the Tonic theme keeps the pixel fonts) */
static int grigio(void);
static int32_t htw(const felucca_font_t *f, const char *s);
static int32_t htext(int32_t x, const felucca_font_t *f, const char *s, uint16_t col);
static void htext_r(int32_t x, const felucca_font_t *f, const char *s, uint16_t col);
/* wip605: THE BATTERY, top right of the header: 3 stages (3 full, 2 half, 1 almost empty; plat_battery: 0 and 1 bars = stage 1, 4 = a USB host
 * powers it = 3 blue segments). The stage goes DOWN at once and UP only after 3 s steady (the reading hovers at a threshold). When it falls to
 * stage 1 the LOW BATTERY screen takes the whole screen until HOME is pressed (it comes back only after the battery has been up again). */
static int8_t bat_st;                    /* 0 = not read yet, 1..3, 4 = USB power */
static uint32_t bat_up;                  /* since when a higher stage has been read (0: none) */
static void batt_update(void)
{
    int b = plat_battery(), s, prev = bat_st;
    uint32_t now = plat_ms();
    if (b < 0)
        return;
    s = b >= 4 ? 4 : b < 1 ? 1 : b;
    if (!bat_st || s == 4 || s < bat_st) {
        bat_st = (int8_t)s;
        bat_up = 0;
    } else if (s > bat_st) {
        if (!bat_up)
            bat_up = now ? now : 1u;
        else if (now - bat_up >= 3000u) {
            bat_st = (int8_t)s;
            bat_up = 0;
        }
    } else
        bat_up = 0;
    if (bat_st == 1 && prev != 1)
        ui.lowbatt = 1;
    else if (bat_st != 1)
        ui.lowbatt = 0;
}
static void draw_batt(void)
{
    int st = bat_st, n = st > 3 ? 3 : st, i, blink_off = st == 1 && !(ui.frame & 16u);
    uint16_t on = st == 4 ? C_SLBLUE : st == 3 ? RGB(60, 190, 90) : st == 2 ? RGB(240, 170, 30) : RGB(222, 58, 58);
    if (st <= 0)
        return;
    cv_rect(212, 2, 24, 1, C_WHITE);                   /* wip707: BIGGER: the outline 24 x 15 (was 19 x 11), as tall as the header chips, the nub 3 x 7 */
    cv_rect(212, 16, 24, 1, C_WHITE);
    cv_rect(212, 2, 1, 15, C_WHITE);
    cv_rect(235, 2, 1, 15, C_WHITE);
    cv_rect(236, 6, 3, 7, C_WHITE);
    for (i = 0; i < 3; i++)
        cv_rect(214 + 7 * i, 4, 6, 11, i < n && !blink_off ? on : C_LINE);
}
static int rec_blink_on(void);                     /* wip690: defined below, the header's REC dot uses it too */
static void draw_header(void)
{
    char b[24];
    int32_t x = 4;
    int chips3 = 0;                                   /* wip687: SEQ and ARP: the header is three chips [P1] [PATTERN BANK n] [TRACK n] */
    cv_begin(240, HDR_H, C_BG);
    cv_rect(0, HDR_H - 1, 240, 1, C_LINE);
    if (ui.msg_until && (int32_t)(ui.msg_until - plat_ms()) > 0) {   /* what a button just did */
        box(0, 0, 240, HDR_H - 1, part_col());
        text_c(120, vc(&FONT_B, 0, HDR_H - 1), &FONT_B, ui.msg, C_BLACK);
        cv_commit(0, 0, 0);
        return;
    }
    ui.msg_until = 0;
    if (tonic_part()) {   /* the Tonic, EVERY page (wip705): the three chips [P1] [PATTERN BANK n] [TRACK n] */
        uint16_t chip_col = ui.pbank ? RGB(222, 58, 58) : part_col();   /* wip705: red in bank 2 on every page */   /* wip686: PATTERN BANK 2: the top chip is RED */
        const char *tn = engine_voice_name(T_909, ui.sel[PART_909]);   /* wip705: always the track (the old skin's GLO mixer showed the project name) */   /* wip229: the GLO mixer is the master mix: the project's name, not "TRACK n" */
        int32_t w = htw(&FONT_B, tn) + 12;
        if (1) {                                     /* wip705: every page (was SEQ and ARP only). wip687: [P1] [PATTERN BANK n] [TRACK n] (the track chip only on SEQ: ARP has the big one); all RED in bank 2 */
            int part = PART_909, cue = seq_cue_of(&seq, part);
            char pt[12], *q = pt;
            int32_t cx = 2, cw;
            chips3 = 1;
            if (seq.song_on) {
                *q++ = 'S';
                q = put_i(q, (seq.playing ? seq.song_pos : ui.song_sel) + 1);
            } else {
                *q++ = 'P';
                q = put_i(q, seq.ppat[part] + 1);
                if (cue >= 0 && cue != seq.ppat[part] && (ui.frame & 16u)) {   /* the cue blinks inside the chip: P1>2 */
                    *q++ = '>';
                    q = put_i(q, cue + 1);
                }
            }
            *q = 0;
            cw = htw(&FONT_B, pt) + 10;
            rbox(cx, 2, cw, 15, 3, chip_col);
            htext(cx + 5, &FONT_B, pt, C_BLACK);
            cx += cw + 3;
            cw = htw(&FONT_S, ui.pbank ? "PATTERN BANK 2" : "PATTERN BANK 1") + 10;
            rbox(cx, 2, cw, 15, 3, chip_col);
            htext(cx + 5, &FONT_S, ui.pbank ? "PATTERN BANK 2" : "PATTERN BANK 1", C_BLACK);
            cx += cw + 3;
            cw = htw(&FONT_B, tn) + 10;                /* wip692: the TRACK chip on ARP too (3 chips on both pages) */
            rbox(cx, 2, cw, 15, 3, chip_col);
            htext(cx + 5, &FONT_B, tn, C_BLACK);
            cx += cw + 3;
            x = cx;
        } else if (grigio()) {                       /* wip550: the mockup's chip: 56 wide at least, radius 3, text centred, the title 8 px after it */
            int32_t cw = w + 6 < 56 ? 56 : w + 6;
            rbox(2, 2, cw, 15, 3, chip_col);
            htext(2 + (cw - (w - 12)) / 2, &FONT_B, tn, C_BLACK);
            x = cw + 8;
        } else {
            box(0, 1, w, 17, chip_col);
            htext(6, &FONT_B, tn, C_BLACK);
            x = w + 6;
        }
    }
    if (part_view()) {                                /* the part chip, then where we are */
        if (!tonic_part()) {
            int32_t w = htw(&FONT_B, PART_N[ui.part]) + 12;
            box(0, 1, w, 17, part_col());
            htext(6, &FONT_B, PART_N[ui.part], C_BLACK);
            x = w + 6;
        }
        if (ui.view == V_SEQ && !chips3)
            x = htext(x, &FONT_S, view_title(), C_HI) + 6;   /* (wip687: SEQ and ARP: no page name, three chips) */
    } else if (!chips3) {                             /* (wip705: the three chips are on every page: no page name in the top bar) */
        {
            const felucca_font_t *vf = ui.view == V_MIX2 || ui.view == V_GLOB || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL ? &FONT_XS : &FONT_B;   /* (the long "MIX PAGE 2" must stop before the pattern tag) */
            const char *vt = grigio() && ui.view == V_OSC ? "OSC" : grigio() && ui.view == V_PMOD ? "PITCH" : grigio() && ui.view == V_NOISE ? "NOISE" : grigio() && ui.view == V_NEV ? "NOISE ENV" : grigio() && ui.view == V_MIX2 ? (cur_page() == 1 ? "SENDS" : "MIX 2") : grigio() && ui.view == V_GLOB ? "GLOBAL" : grigio() && ui.view == V_MIX ? "GLO MIX" : grigio() && fx_comp_page() ? "COMP" : view_title();   /* wip552: the OSC mockup's top bar says OSC (the page title says OSCILLATOR) */
            if (ui.view != V_MIX || x + htw(vf, vt) < 128)   /* wip229: a wide project name leaves no room for "MIX" before the pattern tag */
                x = htext(tonic_part() && (ui.view != V_GLOB || grigio()) && (!fx_comp_page() || grigio()) ? x : 4, vf, vt, C_HI) + 6;
        }
    }
    if (!chips3) {   /* the pattern of the part on screen (else the 909's), its cue; in SONG mode the bar (wip687: SEQ and ARP show it in the first chip) */
        int part = part_view() ? ui.part : PART_909, cue = seq_cue_of(&seq, part);
        if (seq.song_on) {
            b[0] = 'S';
            put_i(b + 1, (seq.playing ? seq.song_pos : ui.song_sel) + 1);
            x = htext(grigio() ? 118 : ui.view == V_NEV ? 160 : ui.view == V_NOISE ? 146 : 132, &FONT_B, b, seq.song_rec ? RGB(255, 90, 80) : C_AMB);
        } else {
            b[0] = 'P';
            put_i(b + 1, seq.ppat[part] + 1);
            x = htext(grigio() ? 118 : ui.view == V_NEV ? 160 : ui.view == V_NOISE ? 146 : 132, &FONT_B, b, C_WHITE);
        }
        if (cue >= 0 && cue != seq.ppat[part] && !seq.song_on) {
            b[0] = '>';
            put_i(b + 1, cue + 1);
            htext(x + 2, &FONT_S, b, (ui.frame & 16u) ? C_WHITE : C_GRAY);
        } else if (seq.chain_a != seq.chain_b && !seq.song_on) {
            htext(x + 3, &FONT_XS, "CHN", C_AMB);
        } else if (steps_view() && steps_len() > 16) {   /* the 16 steps the keys show */
            put_i(put_s(put_i(b, ui.spage * 16 + 1), "-"), ui.spage * 16 + 16);
            htext(x + 4, &FONT_XS, b, C_GRAY);
        }
    }
    {   /* following an external clock, the measured tempo wanders a little from clock to clock: the
         * display averages it over about a second, and moves only when that is half a BPM away */
        static float avg;
        static int shown;
        float t = seq_tempo(&seq);
        if (!seq.ext || avg < 1.0f)
            avg = t;
        avg += (t - avg) * 0.03f;
        if (!seq.ext || fm_fabsf(avg - (float)shown) > 0.6f)
            shown = (int)(avg + 0.5f);
        put_i(b, shown);
    }
    htext_r(183, &FONT_S, b, seq.ext ? C_AMB : C_HI);   /* wip605: 21 px to the left, the battery has the corner (wip707: 8 px more, the battery is bigger) */
    if (seq.playing)
        play_icon(188, 4, (eng_step[TRK_DRUM] & 3) == 0 ? C_WHITE : C_HI);
    if (ui.rec)
        dot(202, 9, 4, rec_blink_on() ? RGB(222, 30, 30) : RGB(150, 120, 125));   /* wip690: blinks exactly like the REC LED (same test as the ARP chip's dot) */
    else if (ui.dirty)
        dot(204, 9, 1, C_GRAY);
    draw_batt();
    cv_commit(0, 0, 0);
}

/* -------------------------------------------------------------- the views --- */
static int step_of(int col) { return ui.spage * 16 + col; }
#define GX 27
static int32_t col_x(int c) { return GX + c * 13 + (c >> 2); }   /* a pixel between beats */

#define DROW_T 15                                  /* wip220: the Tonic grid: 9 rows x 15 = 135 of the 138 px */
#define DROW 11                                    /* drum rows: 12 of them over both bands, a gap under */
/* the Tonic's grid: a trig white, with its own ACCENT red, with FILL blue, with both purple */
#define C_TACC C_SLBLUE                  /* a step with an accent: the knobs' blue */
#define C_TFIL RGB(125, 200, 255)        /* only a fill: light blue (hit with nothing on: white; both: purple) */
#define C_TBOTH RGB(166, 106, 224)
#define C_SQACC RGB(25, 70, 200)         /* wip219, the sequencer's steps only, the whole cell: accent = dark blue */
#define C_SQFIL RGB(250, 215, 40)        /* fill = yellow */
#define C_SQBOTH_G RGB(222, 58, 58)      /* wip575: GRIGIOTONIC: accent + fill = RED (the same red as the centred pan of the GLO mixer) */
#define C_SQBOTH RGB(60, 190, 90)        /* accent + fill = green, not too dark, not too light (the messages keep C_TBOTH) */
/* wip677: the ARP page's NEW LOOK, part 1 (docs/arp_look/arp_v8_final.png). The 8 rows are the 8 WHITE keys of ONE octave (C at the bottom,
 * C' on top); a black-key note sits half a row between its two white keys; the keyboard is on the left. Only the SELECTED voice shows: every
 * hit of it is a round ball at its note, in the sequencer's colours (white, accent dark blue, fill yellow, both green). FOLDED view: no note
 * ever leaves the grid; the octave you SEE (arp_oct: semitones 12 * arp_oct .. 12 * arp_oct + 12) is full colour, a note of another octave is
 * PALE with a ring and a small +1 / -1 (octaves away). A hit with no note (0) and no flag is pure white. Tails, lanes, the TRACK chip: next parts. */
static const uint8_t ARP_HALF[13] = {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14};   /* semitone 0..12 -> half rows above the C (0, .5, 1, 1.5, 2, 3, 3.5 ...) */
static float arp_yc(int half) { return 7.5f + (float)(14 - half) * 7.5f; }          /* its centre y in the main area (rows 15 px, the first at y 1) */
/* wip684: THE one REC blink test: the REC button's LED and the REC dot of the ARP chip both use it (so they always agree) */
static int rec_blink_on(void)
{
    if (!ui.rec)
        return 0;
    if (ui.unq) {                                      /* wip300: UNQUANTIZED REC: two fast blinks then one slow, over and over (stopped or playing) */
        uint32_t p = ui.frame & 63u;
        return p < 6u || (p >= 12u && p < 18u) || (p >= 24u && p < 40u);
    }
    if (!seq.playing)
        return 1;                                      /* QUANTIZED, armed, stopped: steady */
    return eng_step[TRK_DRUM] % 4 == 0;                /* QUANTIZED, recording: blinks with PLAY (on the beat) */
}
/* wip683: the soft glow TAIL: from the ball (x0) to x1, strongest at the ball and fading out to the right; the edges are soft too */
static void arp_tail(int x0, int x1, int yc, uint16_t col)
{
    int x, dy;
    for (x = x0; x < x1; x++) {
        int L = x1 - x0 > 0 ? x1 - x0 : 1, d = x - x0, fade = 15 - (15 * d * d) / (L * L);   /* 15 .. 0 of 16, stays strong for the first half */
        for (dy = -4; dy <= 4; dy++) {
            int ad = dy < 0 ? -dy : dy, ca = ad <= 1 ? 16 : ad == 2 ? 13 : ad == 3 ? 8 : 3;
            px_blend(x, yc + dy, col, (uint32_t)((fade * ca) >> 4));
        }
    }
}
/* the BIG track chip of the mockup (screen x 112..232, y 144..170): it starts in the main canvas and ends in the knob canvas, so it is drawn in both (main coords, oy = the offset of the canvas) */
/* wip703: the big track chip is on the ARP page AND on the SEQ page (not the pattern page); on SEQ it sits over the right part of the AC row (his choice: the AC row's trigs are thin lines) */
static int bigchip_page(void)
{
    return tonic_part() && ui.view == V_SEQ && (ui.arp || !ui.seqpat);
}
static void arp_chip(int oy)
{
    int v = ui.sel[PART_909];
    if (v < 0 || v >= NDRUM)
        return;
    rbox(112, 124 + oy, 120, 26, 7, RGB(125, 200, 255));
    if (ui.rec) {                                      /* wip684: a RED DOT (blinks like the REC LED) and the word REC on the left, the track name moves right */
        disc(120.5f, (float)(137 + oy), 4.5f, rec_blink_on() ? RGB(222, 30, 30) : RGB(150, 120, 125));
        cv_text(127, vc(&FONT_XS, 124 + oy, 26), &FONT_XS, "REC", RGB(15, 35, 70));
    }
    text_c(ui.rec ? 189 : 172, vc(&FONT_M, 124 + oy, 26), &FONT_M, engine_voice_name(T_909, v), RGB(15, 35, 70));
}
static void draw_arp_roll(int band)
{
    static const char *const WN[8] = {"C'", "B", "A", "G", "F", "E", "D", "C"};
    static const uint8_t BLK[5] = {1, 3, 6, 8, 10};
    int v = ui.sel[PART_909], pn = seq.ppat[PART_909], oy = -band * BAND_H, r, c, i;
    const dpart_t *d = &cur_pat()->drum[PART_909];
    int len = d->len, ph = seq.playing ? eng_step[TRK_DRUM + PART_909] - ui.spage * 16 : -1;
    for (r = 0; r < 8; r++) {                          /* the white keys, then the black ones on top */
        int y = 1 + r * 15 + oy;
        rbox(3, y, 20, 13, 2, RGB(252, 252, 252));
        rframe(3, y, 20, 13, 2, RGB(110, 110, 115));
        text_r(21, vc(&FONT_XS, y, 13), &FONT_XS, WN[r], RGB(120, 120, 125));
    }
    for (i = 0; i < 5; i++)
        rbox(3, (int)(arp_yc(ARP_HALF[BLK[i]]) - 4.5f) + oy, 12, 9, 2, RGB(34, 34, 44));
    for (i = 0; i < 5; i++)                            /* wip678: a faint dark LANE at every black-key position, like a piano roll */
        rbox(GX, (int)(arp_yc(ARP_HALF[BLK[i]]) - 3.5f) + oy, col_x(15) + 12 - GX, 7, 3, blend(C_BG, RGB(0, 0, 0), 2));
    if (ph >= 0 && ph < 16)                            /* the playing step: a pale band */
        box(col_x(ph), 1 + oy, 12, 119, blend(C_BG, RGB(255, 255, 255), 4));
    for (r = 0; r < 8; r++)                            /* the grid dots: a row per white key */
        for (c = 0; c < 16; c++)
            if (step_of(c) < len)
                disc((float)col_x(c) + 6.0f, 1.0f + (float)r * 15.0f + 6.5f + (float)oy, 1.3f, C_DIM);
    arp_chip(oy);                                      /* wip682 / 683: the big track chip (its lower part is drawn with the knob row) */
    if (v < 0 || v >= NDRUM || v >= NTRK_NOTE)
        return;
    for (c = 0; c < 16; c++) {                         /* the balls */
        int s = step_of(c), n, rel, sc, off = 0, acc, fil, black, muted;
        uint16_t col;
        float cx, cy, rad;
        if (s >= len || s >= NSTEPS || !sm_get(d->hit[v], s))
            continue;
        n = pn < NPAT_NOTE ? proj.note[pn][v][s] : 0;
        rel = n - 12 * ui.arp_oct;
        if (rel >= 0 && rel <= 12)
            sc = rel;                                  /* in the octave you see */
        else {
            sc = ((n % 12) + 12) % 12;                 /* folded: its pitch name, pale, with the octaves away */
            off = rel > 0 ? (rel - 1) / 12 : -((-rel + 11) / 12);
            off += rel > 0 ? 1 : 0;
        }
        acc = sm_get(d->vacc[v], s) || sm_get(d->accent, s);
        fil = sm_get(d->vfill[v], s);
        muted = ((seq.mute | seq.vmute) & (1u << (PART_909 * NDRUM + v))) != 0;
        col = muted ? C_DIM : acc && fil ? (grigio() ? C_SQBOTH_G : C_SQBOTH) : acc ? C_SQACC : fil ? C_SQFIL : n == 0 ? RGB(255, 255, 255) : C_WHITE;
        black = sc == 1 || sc == 3 || sc == 6 || sc == 8 || sc == 10;
        cx = (float)col_x(c) + 6.0f;
        cy = arp_yc(ARP_HALF[sc]) + (float)oy;
        rad = black ? 4.2f : 5.2f;
        {                                              /* wip681: the glow TAIL to the RIGHT: on to the next hit of this voice (4 steps at most; a note has no stored length) */
            int nc = c + 1, x1, xe = col_x(15) + 12;
            while (nc < 16 && nc - c <= 4 && !(step_of(nc) < len && sm_get(d->hit[v], step_of(nc))))
                nc++;
            x1 = (nc < 16 && nc - c <= 4) ? col_x(nc) : (int)cx + 4 * 13;
            if (x1 > xe)
                x1 = xe;
            if (step_of(15) >= len && x1 > col_x(len - 1 - ui.spage * 16) + 12)
                x1 = col_x(len - 1 - ui.spage * 16) + 12;   /* a short pattern: stop at its last step */
            if (x1 - (int)cx >= 4)
                arp_tail((int)cx, x1, (int)cy, col);
        }
        if (!off && n == 0 && !muted && !acc && !fil && C_WHITE == RGB(255, 255, 255)) {   /* wip680: a hit with NO note in a DARK skin (the normal ball is white too): a HOLLOW white ring (a dark ring would vanish on the black page) */
            disc(cx, cy, rad + 0.3f, col);
            disc(cx, cy, rad - 1.6f, C_BG);
        } else if (!off)
            disc(cx, cy, rad, col);
        else {                                         /* another octave: pale, a ring, +1 / -1 */
            char t[4] = {off > 0 ? '+' : '-', (char)('0' + (off > 0 ? off : -off)), 0, 0};
            disc(cx, cy, rad, col);
            disc(cx, cy, rad - 1.2f, dim(col, 6));
            text_c((int)cx, vc(&FONT_XS, (int)(cy - 4.5f), 9), &FONT_XS, t, col);
        }
    }
}

static void draw_drum(int band)
{
    int k = ui.part, r, v, c, len = cur_pat()->drum[k].len, tonic = k == PART_909;
    int nrows = tonic ? 9 : NDRUM + 1;                 /* the Tonic: its 8 tracks, then the total-accent row */
    const dpart_t *d = &cur_pat()->drum[k];
    int dr = tonic ? DROW_T : DROW;                    /* wip220: the Tonic has 9 rows only: taller rows (the cells nearly square) */
    int ph = seq.playing ? eng_step[TRK_DRUM + k] : -1;
    uint16_t col = part_col(), on_dim = dim(col, 8);
    uint32_t lmarks = lock_marks(T_909, ui.sel[k], ui.spage * 16) |
                      lock_marks(T_PROB, k << 4 | ui.sel[k], ui.spage * 16);
    if (tonic && ui.arp) {                             /* wip677: the ARP page has its own look (a keyboard and balls) */
        draw_arp_roll(band);
        return;
    }
    for (r = 0; r < nrows; r++) {
        int y = r * dr + 1 - band * BAND_H, sel;
        const char *nm;
        const uint32_t *bits;
        int muted;
        v = (tonic && r == 8) ? NDRUM : r;
        sel = ac_on() ? v == NDRUM : v < NDRUM && v == ui.sel[k];   /* wip287: AC selected: its row is the lit one */
        nm = v < NDRUM ? engine_voice_short(T_909, v) : "AC";
        bits = v < NDRUM ? d->hit[v] : d->accent;
        muted = v < NDRUM && ((seq.mute | seq.vmute) & (1u << (k * NDRUM + v)));
        if (y < -dr || y >= BAND_H)
            continue;
        if (sel)
            box(0, y - 1, 24, dr, col);
        cv_text(tonic ? 7 : 5, vc(tonic ? &FONT_S : &FONT_XS, y - 1, dr - 1), tonic ? &FONT_S : &FONT_XS, nm, sel ? C_BLACK : muted ? C_LINE : v < NDRUM ? C_GRAY : C_WHITE);
        for (c = 0; c < 16; c++) {
            int s = step_of(c), x = col_x(c), hit = s < NSTEPS && sm_get(bits, s);
            if (s >= len) {
                cv_rect(x + 5, y + 3, 2, 2, C_LINE);
                continue;
            }
            if (v == NDRUM) {                          /* the accent row: white marks */
                if (hit)
                    box(x + 2, y + 1, 8, 6, C_WHITE);
                else
                    rbox(x + 2, y + 3, 8, 2, 1, C_LINE);
                continue;
            }
            if (tonic) {
                int acc = hit && (sm_get(d->vacc[v], s) || sm_get(d->accent, s)), fil = hit && sm_get(d->vfill[v], s);
                uint16_t cc = !hit ? (sel || s == ph ? C_DIM : C_LINE) : muted ? C_DIM : acc && fil ? (grigio() ? C_SQBOTH_G : C_SQBOTH) : acc ? C_SQACC : fil ? C_SQFIL : C_WHITE;
                box(x, y, 12, dr - 2, cc);              /* wip219: white trig, dark blue accent, yellow fill, green both: the whole cell */
                if (hit && s == ph)
                    box(x + 3, y + 2, 6, dr - 6, C_BLACK);   /* the playhead: a dark core */
                if (hit && sel && ((lmarks >> c) & 1u))
                    cv_rect(x + 8, y + 1, 3, 3, C_BLACK);
                continue;
            }
            box(x, y, 12, dr - 2, hit ? (muted ? C_DIM : sel ? col : on_dim) : (s == ph ? C_DIM : C_LINE));
            if (hit && s == ph)
                box(x + 2, y + 2, 8, dr - 6, C_WHITE);
            if (hit && sel && ((lmarks >> c) & 1u))      /* the step's own values (p-locks): a notch */
                cv_rect(x + 8, y + 1, 3, 3, C_BLACK);
        }
    }
    if (bigchip_page())                                /* wip703: the big track chip on the SEQ page too (same place as ARP) */
        arp_chip(-band * BAND_H);
}

static void draw_break(int band)
{
    const brkpart_t *bp = &cur_pat()->brk;
    int c, ph = seq.playing ? eng_step[TRK_BRK] : -1, slice, bank, div, running;
    uint16_t col = part_col();
    uint32_t lmarks = lock_marks(-1, -1, 0);           /* (the break: its 16 steps) */
    engine_brk_state(&slice, &bank, &div, &running);
    if (!ui.outline_ok || ui.outline_slot != bp->slot_a) {
        ui.outline_ok = (uint8_t)engine_brk_outline(0, ui.outline, 232);
        ui.outline_slot = bp->slot_a;
    }
    if (band == 0) {                                   /* loop A, its 8 slices, the one sounding */
        int x;
        for (c = 0; c < 8; c++) {
            int x0 = 4 + c * 29;
            char t[2] = {(char)('1' + c), 0};
            if (running && c == slice && bank == 0)
                box(x0, 1, 28, 56, dim(col, 5));
            text_c(x0 + 14, 59, &FONT_XS, t, running && c == slice && bank == 0 ? C_WHITE : C_DIM);
        }
        for (x = 0; x < 232 && ui.outline_ok; x++) {
            int h = ui.outline[x] * 26 / 255, sl = x / 29;
            if (h)
                cv_rect(4 + x, 29 - h, 1, 2 * h, running && sl == slice && bank == 0 ? C_WHITE : col);
        }
        for (c = 1; c < 8; c++)
            cv_rect(4 + c * 29, 1, 1, 56, C_LINE);
    } else {
        const char *const *sn = engine_brk_slot_names();
        int ns = engine_brk_nslots();
        char t[40], *q = t;
        for (c = 0; c < 16; c++) {
            int x = col_x(c), on = (bp->steps >> c) & 1u;
            box(x, 2, 12, 10, on ? (c == ph ? C_WHITE : col) : (c == ph ? C_DIM : C_LINE));
            if (on && ((lmarks >> c) & 1u))            /* the step's own values (p-locks): a notch */
                cv_rect(x + 8, 3, 3, 3, C_BLACK);
            if (bp->slice[c]) {                        /* the step's own slice */
                char d[2] = {(char)('0' + bp->slice[c]), 0};
                text_c(x + 6, vc(&FONT_XS, 2, 10), &FONT_XS, d, on ? C_BLACK : C_GRAY);
            }
        }
        cv_text(3, 1, &FONT_XS, "ON", C_GRAY);
        {
            char ta[16], tb[16];
            put_s(put_s(ta, "A  "), sn[bp->slot_a % ns]);
            put_s(put_s(tb, "B  "), sn[bp->slot_b % ns]);
            segs(4, 18, &FONT_S, C_HI, 18, ta, tb, 0, 0);
            (void)q;
        }
        if (running) {
            q = put_s(t, bank ? "PLAYING B" : "PLAYING A");
            if (div > 1)
                put_i(put_s(q, "   RETRIG X"), div);
            cv_text(4, 36, &FONT_S, t, bank ? C_AMB : C_GRAY);
        }
    }
}

/* a part's lane on HOME: the 16 steps of the page its playhead is on (bits: those 16, from base) */
static void lane(int y, int p, uint32_t bits, int ph, int len, int base)
{
    int c, muted = part_muted(p);
    for (c = 0; c < 16 && base + c < len; c++) {
        uint16_t fc = (bits >> c & 1u) ? (muted ? C_DIM : PART_COL[p]) : C_LINE;
        box(col_x(c), y + 2, 12, 9, fc);
        if (seq.playing && (ph & 15) == c)
            rbox(col_x(c) + 2, y + 12, 8, 2, 1, C_WHITE);
    }
}

static void draw_home(int band)
{
    int p, c;
    if (band == 0) {                                   /* the 16 patterns; in each, a bar per part playing it */
        int cue = seq_cue_of(&seq, PART_909), c0 = ui.pbank * 16;
        for (c = c0; c < c0 + 16; c++) {
            int x = 4 + (c - c0) * 14 + (c - c0 >= 8 ? 4 : 0);
            int in_chain = seq.chain_a != seq.chain_b &&
                           c >= (seq.chain_a < seq.chain_b ? seq.chain_a : seq.chain_b) &&
                           c <= (seq.chain_a < seq.chain_b ? seq.chain_b : seq.chain_a);
            box(x, 3, 12, 18, pattern_used(&proj.pat[c]) ? C_DIM : C_LINE);
            for (p = 0; p < 1; p++) {                    /* the Tonic's line: bright where it plays now, faint where it is empty */
                int used = part_used(&proj.pat[c], p);   /* plays now; a part playing it empty, a faint one */
                if (used || seq.ppat[p] == c)
                    rbox(x + 2, 5 + p * 3, 8, 2, 1, seq.ppat[p] == c ? (used ? PART_COL[p] : dim(PART_COL[p], 4))
                                                                     : dim(PART_COL[p], 7));
            }
            if (c == cue && c != seq.ppat[PART_909] && (ui.frame & 16u))
                frame(x - 1, 2, 14, 20, C_WHITE);
            if (in_chain)
                cv_rect(x, 30, 12, 2, C_AMB);
            if ((c & 3) == 0) {                         /* every fourth: its number (and so the bank) */
                char t[4];
                put_i(t, c + 1);
                cv_text(x + 1, 21, &FONT_XS, t, ui.pbank ? C_AMB : C_GRAY);
            }
        }
        for (p = 0; p < 1; p++) {
            const dpart_t *d = &pat_of(p)->drum[p];
            uint32_t bits = 0;
            int v, base = seq.playing ? (eng_step[TRK_DRUM + p] & ~15) : 0;
            for (v = 0; v < NDRUM; v++)
                bits |= d->hit[v][base >> 5] >> (base & 31);
            lane(32 + p * 16, p, bits & 0xFFFFu, eng_step[TRK_DRUM + p], d->len, base);
        }
    }
}

/* SONG: a table of bars, six rows of 23 px across the two bands (the first is the heading, then five bars).
 * wip593: big characters (FONT_B, were FONT_XS); GRIGIOTONIC: all in black, the other skins keep their colours */
#define SONG_RP 23
static void draw_song(int band)
{
    const song_t *sg = song();
    const int gt = grigio();
    const uint16_t ink = C_WHITE;                      /* (GRIGIOTONIC: C_WHITE is the near-black ink) */
    int r, p, top = ui.song_sel > 4 ? ui.song_sel - 4 : 0;
    for (r = 0; r < 6; r++) {
        int y = r * SONG_RP + 1 - band * BAND_H, k = top + r - 1;
        char t[8];
        if (y < -SONG_RP || y >= BAND_H)
            continue;
        if (r == 0) {                                  /* the heading */
            cv_text(4, vc(&FONT_S, y, 21), &FONT_S, "BAR", gt ? ink : C_GRAY);
            for (p = 0; p <= PART_909; p++)             /* wip135: the Tonic's column only (no 808, no BREAK) */
                text_c(100 + p * 36, vc(&FONT_S, y, 21), &FONT_S, PART_S[p], gt ? ink : PART_COL[p]);
            continue;
        }
        if (k > sg->len || k >= NSONG)
            break;
        if (k == ui.song_sel)
            box(0, y, 236, SONG_RP - 2, C_LINE);
        put_i(t, k + 1);
        cv_text(4, vc(&FONT_B, y, 21), &FONT_B, t, gt ? ink : k == ui.song_sel ? C_WHITE : C_GRAY);
        if (seq.song_on && seq.playing && k == seq.song_pos)
            play_icon(46, y + 6, seq.song_rec ? RGB(255, 80, 70) : C_WHITE);
        if (k == sg->len) {                            /* one past the end: where the song grows */
            if (k)
                cv_text(70, vc(&FONT_B, y, 21), &FONT_B, "END", gt ? ink : C_DIM);
            else
                cv_text(40, vc(&FONT_S, y, 21), &FONT_S, "EMPTY: WHITE KEYS ADD", gt ? ink : C_DIM);
            continue;
        }
        for (p = 0; p <= PART_909; p++)             /* wip135: the Tonic's column only (no 808, no BREAK) */ {
            int muted = sg->bar[k].mute >> p & 1u;
            t[0] = 'P';
            put_i(t + 1, sg->bar[k].pat[p] + 1);
            text_c(100 + p * 36, vc(&FONT_B, y, 21), &FONT_B, muted ? "-" : t, gt ? ink : muted ? C_DIM : PART_COL[p]);
        }
        if (SONG_LOOP(&sg->bar[k]))                    /* wip608: LOOP */
            text_r(230, vc(&FONT_B, y, 21), &FONT_B, "LOOP", C_WHITE);
        else if (SONG_REP(&sg->bar[k]) > 1) {          /* a bar played more than once: x2 .. x16 */
            t[0] = 'x';
            put_i(t + 1, SONG_REP(&sg->bar[k]));
            text_r(230, vc(&FONT_B, y, 21), &FONT_B, t, C_WHITE);
        }
    }
}

static void draw_patinfo(int H);
static int pat_next(void)                          /* wip625: the pattern to come: the cued one; in SONG MODE the song's next bar */
{
    return ui.songmode ? seq_song_next(&seq) : seq_cue_of(&seq, PART_909);
}

/* the COMP page (palette: purple, blue, light blue, white, black; no red): an Ableton Glue-style needle meter of the gain reduction, 0 .. 20 dB left to right (the whole 2-band area is one
 * picture; the band only clips). The needle jumps to a new peak and falls back slowly, as the meter on the old MASTER page */
static void draw_comp_meter(void)
{
    static float needle;
    const int px = 120, py = 120;                      /* the needle's pivot */
    const float a0 = -0.96f, a1 = 0.96f;               /* the sweep, radians from straight up (about +-55 degrees) */
    float gr = engine_gr_db(), a;
    char t[24];
    int k;
    cv_oy = 0;
    needle = gr > needle ? gr : needle * 0.88f;
    if (needle < 0.05f)
        needle = 0.0f;
    rbox(8, 2, 224, 130, 8, RGB(14, 16, 20));
    for (k = 0; k <= 20; k++) {                        /* the scale: a tick per dB, a long one and a number every 5 */
        float an = a0 + (a1 - a0) * (float)k / 20.0f, sn = fm_sinf(an), cs = fm_cosf(an);
        int major = k % 5 == 0, r0 = major ? 90 : 98;     /* (wip144: longer and 2 px wide, like the lines of the rest of the GUI) */
        int tx0 = px + (int)(sn * (float)r0 + (sn < 0 ? -0.5f : 0.5f)), ty0 = py - (int)(cs * (float)r0 + 0.5f);
        int tx1 = px + (int)(sn * 106.0f + (sn < 0 ? -0.5f : 0.5f)), ty1 = py - (int)(cs * 106.0f + 0.5f);
        cv_line(tx0, ty0, tx1, ty1, major ? C_WHITE : C_GRAY);
        cv_line(tx0 + 1, ty0, tx1 + 1, ty1, major ? C_WHITE : C_GRAY);
        if (major) {                                   /* the long ticks 3 px wide */
            cv_line(tx0 - 1, ty0, tx1 - 1, ty1, C_WHITE);
        }
        if (major) {
            put_i(t, k);
            text_c(px + (int)(sn * 116.0f), vc(&FONT_XS, py - (int)(cs * 116.0f) - 5, 10), &FONT_XS, t, k >= 15 ? C_TBOTH : C_GRAY);
        }
    }
    a = a0 + (a1 - a0) * fm_clampf(needle / 20.0f, 0.0f, 1.0f);
    {
        float sn = fm_sinf(a), cs = fm_cosf(a);
        int x1 = px + (int)(sn * 98.0f + (sn < 0 ? -0.5f : 0.5f)), y1 = py - (int)(cs * 98.0f + 0.5f);
        cv_line(px - 1, py, x1 - 1, y1, C_TBOTH);      /* the needle 4 px wide (was 2) */
        cv_line(px, py, x1, y1, C_TBOTH);
        cv_line(px + 1, py, x1 + 1, y1, C_TBOTH);
        cv_line(px + 2, py, x1 + 2, y1, C_TBOTH);
    }
    dot(px, py, 7, C_GRAY);
    dot(px, py, 3, C_BLACK);
    cv_text(14, vc(&FONT_XS, 6, 10), &FONT_XS, "GAIN REDUCTION dB", C_GRAY);
    if ((int)(gr + 0.5f))
        put_s(put_i(put_s(t, "-"), (int)(gr + 0.5f)), " dB");
    else
        put_s(t, "0 dB");
    cv_text(14, vc(&FONT_B, 108, 18), &FONT_B, t, gr > 0.5f ? C_TBOTH : C_GRAY);
    text_r(226, vc(&FONT_B, 108, 18), &FONT_B, "4:1", C_DIM);
}

/* wip271: the MUTE / SOLO page (PERFORM, FX again): the mode in big letters, the 8 tracks as boxes (lit = plays, dark = muted or not soloed, purple frame = soloed) */
static void draw_msp(void)
{
    int v;
    cv_oy = 0;
    text_c(120, vc(&FONT_M, 2, 40), &FONT_M, ui.msmode ? "SOLO" : "MUTE", ui.msmode ? C_TBOTH : C_WHITE);
    cv_rect(60, 44, 120, 1, C_LINE);
    for (v = 0; v < NDRUM_TONIC; v++) {
        int x = 6 + (v % 4) * 58, y = 50 + (v / 4) * 44;
        uint32_t bit = 1u << (PART_909 * NDRUM + v);
        int mu = (seq.vmute & bit) != 0, so = (seq.vsolo & bit) != 0;
        int lit = ui.msmode ? so : (!mu && (!seq.vsolo || so));     /* MUTE mode: blue = plays; SOLO mode: purple = soloed */
        uint16_t col = ui.msmode ? C_TBOTH : C_SLBLUE;
        char t[4];
        if (so && !ui.msmode)                                       /* (a solo seen from MUTE mode: a purple frame) */
            rbox(x - 2, y - 2, 56, 40, 8, C_TBOTH);
        rbox(x, y, 52, 36, 6, lit ? col : dim(col, 4));
        put_i(t, v + 1);
        text_c(x + 26, vc(&FONT_L, y, 36), &FONT_L, t, lit ? C_WHITE : C_GRAY);   /* wip329: one big number per pad */
    }
}

/* wip328: the CHOKE screen (EDIT, K2): the same boxes as the MUTE / SOLO page, tall: blue = the track is in the group, grey = it is not. o = the row's offset in the area */
static void draw_chk(int o)
{
    int v;
    cv_oy = -o;
    text_c(120, vc(&FONT_M, 4, 40), &FONT_M, "CHOKE", C_WHITE);
    cv_rect(60, 48, 120, 1, C_LINE);
    for (v = 0; v < NDRUM_TONIC; v++) {
        int x = 6 + (v % 4) * 58, y = 58 + (v / 4) * 62, in = (seq.choke >> v) & 1;
        char t[4];
        rbox(x, y, 52, 54, 6, in ? C_SLBLUE : C_LINE);
        put_i(t, v + 1);
        text_c(x + 26, vc(&FONT_L, y, 54), &FONT_L, t, in ? C_WHITE : C_GRAY);   /* wip329: one big number per pad */
    }
    text_c(120, vc(&FONT_XS, 188, 12), &FONT_XS, "PRESS THE TRACKS OF THE GROUP", C_GRAY);
    cv_oy = 0;
}

/* wip350: the DUCK screen (SENDS, ALGORITHM right): blue = the track is ducked, grey = it is not, PURPLE = the sidechain's INPUT track (same colours as the SOLO page) */
static void draw_dck(int o)
{
    int v;
    cv_oy = -o;
    text_c(120, vc(&FONT_M, 4, 40), &FONT_M, "DUCK", C_WHITE);
    cv_rect(60, 48, 120, 1, C_LINE);
    for (v = 0; v < NDRUM_TONIC; v++) {
        int x = 6 + (v % 4) * 58, y = 58 + (v / 4) * 62, in = v == proj.set.sc_in, dk = (proj.set.sc_mask >> v) & 1;
        char t[4];
        rbox(x, y, 52, 54, 6, in ? C_TBOTH : dk ? C_SLBLUE : C_LINE);
        put_i(t, v + 1);
        text_c(x + 26, vc(&FONT_L, y, 54), &FONT_L, t, in || dk ? C_WHITE : C_GRAY);
    }
    text_c(120, vc(&FONT_XS, 188, 12), &FONT_XS, "PRESS THE TRACKS TO DUCK", C_GRAY);
    cv_oy = 0;
}

static void draw_fx(int band)
{
    if (fx_comp_page()) {
        draw_comp_meter();
    } else if (msp_page()) {
        draw_msp();
    } else if (cur_page() == 0) {                             /* PERFORM (FX 2nd press): the same information as the pattern screen */
        cv_oy = 0;                                     /* (the whole 2-band area is one picture; the band only clips) */
        draw_patinfo(2 * BAND_H);
    } else if (band == 0) {                                   /* wip269: the old send bars (TNC / 808 / BRK x reverb / delay) are gone: the page's name, big, over a thin line */
        const char *nm = pg.title[cur_page()];
        const felucca_font_t *f = tw(&FONT_M, nm) <= 220 ? &FONT_M : &FONT_B;   /* (FONT_L has numbers only) */
        text_c(120, vc(f, 2, 58), f, nm, C_WHITE);
        cv_rect(60, 62, 120, 1, C_LINE);
    } else {
        char a[16], u[8], t[40], *q;
        pref_value(PR(R_ENG, T_FX, 0, FX_DL_TIME), a, u);
        q = put_s(put_s(t, "DELAY "), a);
        pref_value(PR(R_ENG, T_FX, 0, FX_DL_TYPE), a, u);
        put_s(put_s(q, "  "), a);
        cv_text(4, 4, &FONT_B, t, C_HI);
        {
            char t2[20];
            put_s(put_i(put_s(t, "FEEDBACK "), proj.sound.v[T_FX][0][FX_DL_FDBK] * 100 / 127), "%");
            put_i(put_s(t2, "REVERB "), proj.sound.v[T_FX][0][FX_RV_DECAY] * 100 / 127);
            segs(4, 24, &FONT_S, C_GRAY, 18, t, t2, 0, 0);
            (void)q;
        }
    }
}

/* the GLO mixer (wip124): the Tonic's 8 tracks as vertical sliders in the look of the other sliders (blue track, blue
 * fill, a ring + light thumb). The picked track (K1 / black keys) has a lit box behind it, K2 moves its thumb. A hit
 * lights the fill up to the level (the volume animation); a muted track is grey. */
static void draw_drum_mix_g(int o, int k, int top, int fh, int bh, int ly)   /* wip224: o = the canvas' top row, top / fh = the slider, bh = the lit box, ly = the label row */
{
    int t = T_909, v, grp = ui.view == V_MIX ? cur_page() & 1 : cur_page(), tv = ui.touched >= 0 && ui.touched < 4 ? grp * 4 + ui.touched : -1;
    for (v = 0; v < NDRUM_TONIC; v++) {
        int x = 6 + v * 29, cx = x + 14, ing = v / 4 == grp, touch = v == tv;
        pref_t lv = find_ref(t, v, "Level");
        int l = lv.kind ? proj.sound.v[t][v][lv.c] : 0, f = l * fh / 127, ty = top + fh - f;
        int muted = (seq.mute | seq.vmute) & (1u << (k * NDRUM + v));
        uint16_t col = ing ? C_SLBLUE : dim(C_SLBLUE, 4);
        if (touch)
            box(x, 0 - o, 28, bh, dim(C_SLBLUE, 3));
        else if (ing)
            box(x, 0 - o, 28, bh, dim(C_SLBLUE, 1));
        box(cx - 3, top - o, 6, fh, dim(C_SLBLUE, 3));                        /* the track */
        if (f)
            box(cx - 3, ty - o, 6, f, muted ? C_DIM : col);                  /* the level */
        {
            int hf = f * eng_hit[k][v] / 255;                                 /* the hit: lit up to the level, falls back */
            if (hf > 1 && !muted)
                box(cx - 3, top + fh - hf - o, 6, hf, ing ? C_WHITE : C_GRAY);
        }
        dot(cx, ty - o, 7, muted ? C_LINE : col);                             /* the thumb: ring + light centre */
        dot(cx, ty - o, 5, muted ? C_DIM : ing ? C_SLTHUMB : C_GRAY);
        text_c(cx, vc(&FONT_XS, ly, 12) - o, &FONT_XS, engine_voice_short(t, v), touch ? C_WHITE : ing ? C_HI : muted ? C_LINE : C_GRAY);
    }
}
/* wip279: the GLO mixer = the wip271 look (all 8 vertical sliders over the whole screen, a lit box behind the group) + under
 * each slider a small PAN knob (bipolar arc, centre = 64) and the track number + name. SELECT cycles G1 LEVELS, G2 LEVELS,
 * G1 PANS, G2 PANS: the group of the page is lit; the mode of the page lights its part (sliders or knobs), the other is dim.
 * The touched knob's dial grows by 50% (r 10 -> 15) but stays in its place. */
/* wip352: the sidechain dots: a ducked track = RED, pulsing with the real duck (dark red at rest, bright red while the input sounds); the INPUT track = PURPLE */
static int mixer_tall(void);
static int sc_disp(void)                              /* wip356: the dot's depth: the engine's peak since the last frame, falling back slowly (so a short duck is visible at any tempo) */
{
    static uint32_t last = 0xFFFFFFFFu;
    static int disp;
    if (ui.frame != last) {
        int d = engine_tonic_duck();
        disp = d > disp ? d : disp * 3 / 4;
        last = ui.frame;
    }
    return disp;
}
static int sc_dot_shown(void)                         /* a dot is on this screen (SENDS, or the GLO mixer) */
{
    return !proj.set.classic && proj.set.sc_int > 0 && proj.set.sc_mask != 0 && ((ui.view == V_MIX2 && cur_page() == 1) || mixer_tall());
}
static void sc_dot(int x, int y, int r, int v)
{
    int d, on = !proj.set.classic && proj.set.sc_int > 0 && proj.set.sc_mask != 0;
    if (!on)
        return;
    if (v == proj.set.sc_in) {
        dot(x, y, r, C_TBOTH);
        return;
    }
    if (!sc_ducked(v))
        return;
    d = sc_disp();
    dot(x, y, r, RGB(70 + d * 185 / 255, 15 + d * 35 / 255, 15 + d * 35 / 255));
}
static void draw_mix_tall(int o, int k)
{
    int t = T_909, v, pg_i = cur_page(), grp = pg_i & 1, panmode = pg_i >> 1, tv = ui.touched >= 0 && ui.touched < 4 ? grp * 4 + ui.touched : -1;
    int top = 10, fh = 112, py = 134, ph = 36, ky = 152, ny = 174, nh = 20, pass;
    for (pass = 0; pass < 2; pass++)                                   /* pass 1 = the touched knob last (it grows over its neighbours) */
        for (v = 0; v < NDRUM_TONIC; v++) {
            int x = 6 + v * 29, cx = x + 14, ing = v / 4 == grp, touch = v == tv;
            pref_t lv = find_ref(t, v, "Level"), pn = find_ref(t, v, "Pan");
            int l = lv.kind ? proj.sound.v[t][v][lv.c] : 0, pv = pn.kind ? proj.sound.v[t][v][pn.c] : 64;
            int f = l * fh / 127, ty = top + fh - f, muted = (int)(((seq.mute | seq.vmute) >> (k * NDRUM + v)) & 1u);
            int sl_on = ing && !panmode;
            if (seq.vsolo && !((seq.vsolo >> (k * NDRUM + v)) & 1u))
                muted = 1;                                             /* wip607: a track the solo silences is grey, as a muted one */                               /* the sliders' lit box: level pages only */
            uint16_t col = C_SLBLUE;                                   /* wip281: the sliders keep the normal slider colours in every mode */
            if (pass && !touch)
                continue;
            if (!pass) {
                if (sl_on)
                    box(x, 0 - o, 28, py - 4, touch ? dim(C_SLBLUE, 3) : dim(C_SLBLUE, 2));       /* the sliders' lit box */
                if (ing && panmode)                                                              /* wip334: the lit box moves to the pan knobs on the pan pages (the track numbers have none) */
                    box(x, py - o, 28, ny - 2 - py, touch ? dim(C_SLBLUE, 3) : dim(C_SLBLUE, 2));
                box(cx - 3, top - o, 6, fh, dim(C_SLBLUE, 3));
                if (f)
                    box(cx - 3, ty - o, 6, f, muted ? C_DIM : col);
                {
                    int hf = f * eng_hit[k][v] / 255;
                    if (hf > 1 && !muted)
                        box(cx - 3, top + fh - hf - o, 6, hf, C_WHITE);   /* wip284: the volume animation is WHITE (was purple in wip281) */
                }
                sc_dot(cx, 128 - o, 3, v);                             /* wip352: under the slider: red pulsing = ducked, purple = the input */
                dot(cx, ty - o, 7, muted ? C_LINE : col);
                dot(cx, ty - o, 5, muted ? C_DIM : C_SLTHUMB);
                text_c(cx, vc(&FONT_XS, ny + nh / 2, 12) - o, &FONT_XS, engine_voice_short(t, v), touch ? C_WHITE : ing ? C_HI : muted ? C_LINE : C_GRAY);
            }
            if (pass == touch)       /* (the touched one in pass 1, the others in pass 0) */
            {
                sk_red = pv == 64;                                     /* wip283: the centre = mono = red */
                seg_knob(cx, ky - o, touch ? 17 : 10, touch ? 15 : 9, (float)pv / 127.0f, 1, 0);
                sk_red = 0;
            }   /* touched: +15% over the wip279 15, in place */
        }
}
static void draw_drum_mix(int band, int k) { draw_drum_mix_g(band * BAND_H, k, 10, 92, 2 * BAND_H - 4, 114); }
/* wip224: the GLO mixer alone on the screen: no knob strip, the sliders 164 px long (were 92) over the whole area (2 x Q_H + the footer rows) */
static int mixer_tall(void) { return ui.view == V_MIX && !ui.help && ui.overlay == O_NONE; }

static void draw_mix(int band)
{
    int p;
    if (drum_mix_page() >= 0) {
        draw_drum_mix(band, drum_mix_page());
        return;
    }
    if (band == 0 && cur_page() == pg.n - 1) {         /* MASTER: the level over the last moments, not */
        static uint8_t hist[98];                       /* the parts (it shows the pump and the comp) */
        static uint8_t hw;
        int x, pk = 0;
        for (p = 0; p < NPARTS; p++)
            pk = eng_peak[p] > pk ? eng_peak[p] : pk;
        hist[hw++ % 98u] = (uint8_t)(pk * 62 / 32768);
        for (x = 0; x < 98; x++) {
            int h = hist[(hw + (unsigned)x) % 98u];
            cv_rect(4 + x * 2, 66 - h, 2, h + 1, part_col());
        }
    }
    if (band == 0 && cur_page() != pg.n - 1) {         /* channel strips */
        for (p = 0; p < NPARTS; p++) {
            int x, lvl, pk;
            if (p == PART_808)
                continue;
            x = 6 + (p > PART_808 ? p - 1 : p) * 38;
            lvl = proj.sound.v[T_MIX][p][0];
            pk = eng_peak[p] * 64 / 32768;
            if (cur_page() == 0 && p == ui.mixsel)       /* PARTS: the part the knobs set */
                box(x - 2, 0, 30, BAND_H, dim(PART_COL[p], 3));
            rbox(x + 10, 4, 6, 64, 3, C_LINE);
            if (pk > 1)
                rbox(x + 10, 68 - pk, 6, pk, 3, part_muted(p) ? C_DIM : PART_COL[p]);
            rbox(x + 4, 67 - lvl * 64 / 127, 18, 3, 1, C_WHITE);
        }
    }
    if (band == 0) {                                   /* the master's gain reduction */
        {
            int gr = (int)(engine_gr_db() * 64.0f / 24.0f);
            gr = gr > 64 ? 64 : gr;
            rbox(206, 4, 8, 64, 4, C_LINE);
            if (gr > 1)
                rbox(206, 4, 8, gr, 4, RGB(255, 80, 60));
            cv_text(218, 2, &FONT_XS, "GR", C_GRAY);
        }
    } else {
        char t[24], a[12], u[8], *q;
        for (p = 0; p < NPARTS && cur_page() != pg.n - 1; p++)   /* the strips' names (not on MASTER) */
            if (p != PART_808)
                text_c(6 + (p > PART_808 ? p - 1 : p) * 38 + 13, 0, &FONT_XS, PART_N[p], part_muted(p) ? C_LINE : PART_COL[p]);
        if ((int)(engine_gr_db() + 0.5f))
            put_s(put_i(put_s(t, "COMP -"), (int)(engine_gr_db() + 0.5f)), "dB");
        else
            put_s(t, "COMP 0dB");
        cv_text(4, 20, &FONT_B, t, engine_gr_db() > 0.5f ? RGB(255, 110, 90) : C_GRAY);
        pref_value(PR(R_ENG, T_MST, 0, MST_PUMP), a, u);
        q = put_s(put_s(t, "PUMP "), a);
        put_s(q, u);
        cv_text(124, 20, &FONT_B, t, proj.sound.v[T_MST][0][MST_PUMP] ? C_HI : C_GRAY);
        put_s(put_i(a, (int)plat_cpu_pct()), "%");
        cv_text(4, 50, &FONT_XS, "AUDIO LOAD", C_DIM);
        cv_text(70, 50, &FONT_XS, a, plat_cpu_pct() > 85u ? RGB(255, 60, 60) : C_GRAY);
        if (engine_guard_active())                    /* the overload guard is saving the CPU */
            cv_text(100, 50, &FONT_XS, "GUARD", C_AMB);
    }
}

/* PERF: the live window, or the test's progress and table */
static char *put_pct10(char *q, int v)                /* 123 -> "12.3%" */
{
    q = put_i(q, v / 10);
    *q++ = '.';
    q = put_i(q, v % 10);
    return put_s(q, "%");
}
static void draw_perf(int band)
{
    static const char *const SN[ENG_PROF_N] = {"TONIC", "808", "BREAK", "FX", "MASTER", "SEQ"};
    char t[40], *q;
    int k;
    if (band == 0) {
        q = put_s(t, "CLOCK ");
        if (perf.mhz10) {
            q = put_i(q, perf.mhz10 / 10);
            put_s(q, " MHz");
        } else
            put_s(q, plat_cycles_cpu() ? "..." : "? (NO CYCLE COUNTER)");
        cv_text(4, 2, &FONT_B, t, C_WHITE);
        q = put_i(t, plat_sysvdd_mv() / 1000);        /* SYSVDD / VDC14 as X0X set them (fm1_sys.h), under the cores */
        *q++ = '.';
        q = put_i(q, plat_sysvdd_mv() / 10 % 100);
        q = put_i(put_s(q, " / "), plat_vdc14_mv() / 1000);
        *q++ = '.';
        q = put_i(q, plat_vdc14_mv() / 10 % 100);
        put_s(q, "V");
        text_r(236, 14, &FONT_XS, t, C_DIM);
        if (plat_cpu2_on())                           /* the cores: one after the second gave up (fm1-x0x#10) */
            text_r(236, 4, &FONT_XS, "2 CORES", C_GRAY);
        else {
            q = put_s(t, "1 CORE");
            if (plat_cpu2_gave_up() == 1)
                put_i(put_s(q, ": BAD READS "), (int)plat_cpu2_bad());
            else if (plat_cpu2_gave_up() == 2)
                put_s(q, ": 2ND TIMED OUT");
            else if (plat_cpu2_gave_up() == 3)
                put_s(q, " TILL POWER-OFF");
            else if (proj.set.one_core)
                put_s(q, " (GLO)");
            text_r(236, 4, &FONT_XS, t, plat_cpu2_gave_up() ? RGB(255, 80, 60) : C_GRAY);
        }
        if (perf.test >= 0) {
            int done = perf.phase == 4;
            q = put_s(t, done ? "PERF TEST DONE" : "PERF TEST ");
            if (!done) {
                q = put_i(q, perf.test + 1);
                q = put_s(q, "/3  ");
                put_s(q, PT_N[perf.test < 3 ? perf.test : 2]);
            }
            cv_text(4, 24, &FONT_S, t, done ? C_HI : C_AMB);
            if (!done)
                cv_text(4, 46, &FONT_XS, "HOME OR PLAY: STOP", C_DIM);
            else
                cv_text(4, 46, &FONT_XS, "SHARE OF THE CPU (PEAK)  CYCLES / SAMPLE", C_DIM);
            return;
        }
        q = put_s(t, "LOAD ");
        q = put_i(q, perf.load);
        q = put_s(q, "%  PEAK ");
        q = put_i(q, perf.peak);
        q = put_s(q, "%  DROPOUTS ");
        put_i(q, (int)plat_xruns());
        cv_text(4, 24, &FONT_S, t, C_HI);
        if (perf.stalls_on) {
            q = put_s(t, "STALLS  FETCH ");
            q = put_i(q, perf.stall[0]);
            q = put_s(q, "%  READ ");
            q = put_i(q, perf.stall[1]);
            q = put_s(q, "%  WRITE ");
            put_s(put_i(q, perf.stall[2]), "%");
        } else
            put_s(t, "STALLS OFF (KNOB 1)");
        cv_text(4, 46, &FONT_XS, t, perf.stalls_on ? C_GRAY : C_DIM);
        if (!perf.stalls_on && plat_irq_stack_size()) {   /* the interrupts' 8 KiB: the deepest so far */
            q = put_s(t, "IRQ STACK ");
            q = put_i(q, (int)plat_irq_stack_used());
            q = put_s(q, " / ");
            put_s(put_i(q, (int)plat_irq_stack_size()), " B");
            text_r(236, 46, &FONT_XS, t, plat_irq_stack_used() * 4u > plat_irq_stack_size() * 3u ? RGB(255, 80, 60) : C_GRAY);
        }
        return;
    }
    if (perf.test >= 0) {                             /* the table, so far */
        for (k = 0; k < 3; k++) {
            int y = k * 14 + 2;
            cv_text(4, y, &FONT_S, PT_N[k], perf.test == k && perf.phase != 4 ? C_AMB : C_GRAY);
            if (!perf.res_load[k])
                continue;
            q = put_pct10(t, perf.res_load[k]);
            q = put_s(q, " (");
            q = put_i(q, perf.res_peak[k]);
            put_s(q, "%)");
            cv_text(112, y, &FONT_S, t, C_WHITE);
            if (perf.res_cps[k]) {
                put_i(t, perf.res_cps[k]);
                text_r(236, y, &FONT_S, t, C_HI);
            }
        }
        if (perf.phase == 4)
            cv_text(4, 50, &FONT_XS, "SEL: RUN IT AGAIN", C_DIM);
        if (plat_irq_stack_size()) {                 /* the interrupts' stack, the deepest so far (after the test too) */
            q = put_s(t, "IRQ STACK ");
            q = put_i(q, (int)plat_irq_stack_used());
            q = put_s(q, " / ");
            put_s(put_i(q, (int)plat_irq_stack_size()), " B");
            text_r(236, 50, &FONT_XS, t, plat_irq_stack_used() * 4u > plat_irq_stack_size() * 3u ? RGB(255, 80, 60) : C_GRAY);
        }
        return;
    }
    for (k = 0; k < ENG_PROF_N; k++) {                /* each stage's share of the CPU */
        int x = (k & 1) * 118 + 4, y = (k >> 1) * 12;
        cv_text(x, y, &FONT_XS, SN[k], C_GRAY);
        put_pct10(t, perf.stage[k]);
        text_r(x + 108, y, &FONT_XS, t, C_WHITE);
    }
    {
        uint32_t c[5];
        if (plat_last_crash(c)) {                     /* a crash since power-on: its red screen's numbers, to photograph */
            q = put_hex(put_s(t, "CRASH  PC "), c[1], 8);
            put_hex(put_s(q, "  RA "), c[4], 8);
            cv_text(4, 46, &FONT_XS, t, RGB(255, 80, 60));
            q = put_hex(put_s(t, "EMU "), c[2], 8);
            put_hex(put_s(q, "  DBG "), c[3], 8);
            cv_text(4, 56, &FONT_XS, t, RGB(255, 80, 60));
        } else {
            if (perf.cps) {
                q = put_s(t, "RENDER ");
                q = put_i(q, perf.cps);
                put_s(q, " CYCLES / SAMPLE");
                cv_text(4, 44, &FONT_XS, t, C_GRAY);
            }
            cv_text(4, 55, &FONT_XS, "SEL: RUN PERF TEST", C_DIM);
        }
    }
    q = put_s(t, engine_guard_active() ? "GUARD ON  " : "GUARD ");   /* the overload guard: engaged, times */
    put_s(put_i(q, (int)eng_guard_count), "x");
    text_r(236, 55, &FONT_XS, t, engine_guard_active() ? C_AMB : C_DIM);
}

/* the readout: the knob being turned, large, across the bottom band */
/* the readout's knob (ui.touched) and its name: TEMPO, a track's own "BD  TUNE", the pattern's "P3  LENGTH" */
static const char *readout_name(pref_t *rp)
{
    static char t[24];
    const char *nm;
    pref_t r;
    if (ui.touched == 4) {
        *rp = PR(R_TEMPO, 0, 0, 0);
        return "TEMPO";
    }
    r = *rp = pg.r[cur_page()][ui.touched];
    nm = caps_of(pref_name(r));
    if (track_page()) {
        put_s(put_s(put_s(t, engine_voice_name(T_909, ui.sel[ui.part])), "  "), nm);
        nm = t;
    } else if (in_pattern(r)) {                          /* (it changes with the pattern) */
        int pp = r.kind == R_DLEN || r.kind == R_DRATE ? r.a 
                 : r.kind == R_SWING ? PART_909 : part_view() ? ui.part : PART_909;
        put_s(put_s(put_i(put_s(t, "P"), seq.ppat[pp] + 1), "  "), nm);
        nm = t;
    }
    return nm;
}

static void draw_mod_icon(int cx, int cy, int r, int shape, int lit);
static void draw_wave_icon(int cx, int cy, int r, int shape);
static void draw_filter_icon(int cx, int cy, int r, int mode);
static void draw_env_icon(int cx, int cy, int r, int mode);
static void draw_readout_h(int32_t H)
{
    char num[16], unit[8];
    pref_t r;
    int32_t w, x, ly, dy = (H - 2 * BAND_H) / 2;
    const char *nm = readout_name(&r);
    const felucca_font_t *vf = &FONT_L, *nf = &FONT_B;     /* the big face has numbers only: a name in the next size */
    int32_t uw, tx = 142, right = 228;
    const char *q;
    if (r.kind == R_NONE)                                    /* an empty cell was turned: nothing to show */
        return;
    pref_value_of(r, pref_shown(r), num, unit);
    uw = unit[0] ? tw(&FONT_S, unit) + 3 : 0;
    for (q = num; *q; q++)
        if ((uint8_t)*q > '9')
            vf = &FONT_M;
    if (tw(nf, nm) > right - tx)
        nf = &FONT_S;
    cv_rect(0, 0, 240, H, C_BG);               /* the whole main area: a big knob, its name and value */
    frame(2, 2, 236, H - 4, C_AMB);
    if (ui.view == V_PMOD && ui.touched == 0)    /* SEL K1: the zoom is the circle (the shape's icon), not a dial */
        draw_mod_icon(72, H / 2, 58, pref_shown(r), 1);
    else if (ui.view == V_OSC && ui.touched == 0)    /* ENV K1: the circle zoom (the waveform icon), never a dial */
        draw_wave_icon(72, H / 2, 58, pref_shown(r));
    else if (ui.view == V_NEV && ui.touched == 0)    /* EDIT K1: the circle zoom (the envelope icon) */
        draw_env_icon(72, H / 2, 58, pref_shown(r));
    else if (ui.view == V_NOISE && ui.touched == 0)  /* LFO K1: the circle zoom (the filter icon) */
        draw_filter_icon(72, H / 2, 58, pref_shown(r));
    else
        seg_knob(72, H / 2, 58, 36, pref_frac(r), bipolar_of(r), 1);
    if (extra_off(r))                                    /* wip305: CLASSIC: the zoom dial shaded too */
        shade_rect(12, H / 2 - 60, 120, 120);
    cv_text(tx, vc(nf, 14 + dy, 24), nf, nm, extra_off(r) ? C_DIM : C_GRAY);
    ly = vc(vf, 56 + dy, 44);
    w = tw(vf, num);
    x = right - uw - w;
    cv_text(x, ly, vf, num, extra_off(r) ? C_DIM : C_WHITE);
    if (unit[0])
        cv_text(right - uw + 3, base_y(vf, ly, &FONT_S), &FONT_S, unit, extra_off(r) ? C_DIM : C_GRAY);
}
static void draw_readout(void) { draw_readout_h(2 * BAND_H); }

/* a list over the main area: six rows of 24 px */
/* wip267: GLO 3rd press: the project memory as an upright battery with blue liquid (the main area, both bands as one 240 x 138 picture) */
static void draw_mem(int oy)                 /* wip275: a rectangular BUCKET (a thick rounded outline, the top open, no cap) with 5 rounded BARS that grow with the memory; oy = the row of the canvas in the area */
{
    const int cx = 120, cy = 106, bw = 76, bh = 124, wall = 6, nb = 5, sh = 17, gap = 5, padb = 5, padx = 5;
    int pct = project_mem_pct(), full, hot, top = cy - bh / 2, bot = top + bh, bx = cx - bw / 2, k, lx, rx;
    uint16_t col, wire = C_GRAY;
    char t[8];
    if (pct > 100)
        pct = 100;
    full = pct >= 100;
    hot = pct >= 90;
    col = hot ? C_TBOTH : RGB(36, 128, 246);                                /* wip267: the purple of the messages, not red */
    rbox(bx, top - oy, bw, bh, 9, wire);                                    /* the thick outline (a filled rounded box, the inside black again), the top left open */
    cv_rect(bx + wall, top - oy, bw - 2 * wall, bh - wall, C_BG);
    for (k = 0; k < nb; k++) {                                              /* the bars, from the bottom: each one grows in its place, a fifth of the memory each */
        int lo = k * 100 / nb, hi = (k + 1) * 100 / nb, h, yb = bot - wall - padb - k * (sh + gap);
        if (pct <= lo)
            break;
        h = pct >= hi ? sh : (sh * (pct - lo) + (hi - lo) / 2) / (hi - lo);
        if (h < 3)
            h = 3;
        rbox(bx + wall + padx, yb - h - oy, bw - 2 * wall - 2 * padx, h, h < 7 ? h / 2 : 3, col);
    }
    lx = bx - 12;                                                           /* the label's gutter: 6 .. lx */
    rx = bx + bw + 12;                                                      /* the % gutter: rx .. 234 */
    put_i(t, pct);
    {
        int32_t nw = tw(&FONT_L, t), pw = tw(&FONT_B, "%"), gw = nw + pw + 2, gx = rx + (234 - rx - gw) / 2;
        if (gx < rx - 6)
            gx = rx - 6;
        cv_text(gx, vc(&FONT_L, cy - 32 - oy, 40), &FONT_L, t, hot ? col : C_WHITE);
        cv_text(gx + nw + 2, vc(&FONT_B, cy - 16 - oy, 24), &FONT_B, "%", C_GRAY);
    }
    text_c((6 + lx) / 2, vc(&FONT_S, cy - 28 - oy, 20), &FONT_S, "PROJECT", C_GRAY);
    text_c((6 + lx) / 2, vc(&FONT_S, cy - 8 - oy, 20), &FONT_S, "MEMORY", C_GRAY);
    if (full)
        text_c((rx + 234) / 2, vc(&FONT_B, cy + 20 - oy, 22), &FONT_B, "FULL", (ui.frame & 8u) ? col : C_WHITE);
    else if (hot) {
        text_c((rx + 234) / 2, vc(&FONT_S, cy + 18 - oy, 22), &FONT_S, "NEARLY", (ui.frame & 8u) ? col : C_WHITE);
        text_c((rx + 234) / 2, vc(&FONT_S, cy + 34 - oy, 22), &FONT_S, "FULL", (ui.frame & 8u) ? col : C_WHITE);
    }
}

static void draw_list(int band)
{
    int i;
    for (i = 0; i < 3; i++) {
        int row = ui.list_top + band * 3 + i, y = i * 23, sel = row == ui.list_sel;
        pref_t r;
        char num[16], unit[8];
        const x0x_param_t *d;
        if (row >= ui.list_n)
            break;
        r = list_rows[row];
        if (r.kind == R_HDR) {                         /* a section: whose values follow */
            char t[28];
            hdr_name(r.a, t);
            cv_text(8, vc(&FONT_XS, y + 4, 16), &FONT_XS, t, C_AMB);
            cv_rect(8, y + 19, 220, 1, C_LINE);
            continue;
        }
        d = pref_desc(r);
        if (sel)
            box(0, y + 1, 232, 21, dim(part_col(), 6));
        cv_text(8, vc(sel ? &FONT_B : &FONT_S, y + 1, 18), sel ? &FONT_B : &FONT_S, caps_of(pref_name(r)), sel ? C_WHITE : C_GRAY);
        if (r.kind == R_ACT) {                         /* an action: the return arrow */
            uint16_t ac = sel ? C_WHITE : C_DIM;
            cv_rect(212, y + 12, 12, 2, ac);
            cv_rect(222, y + 6, 2, 7, ac);
            cv_line(212, y + 13, 216, y + 9, ac);
            cv_line(212, y + 13, 216, y + 17, ac);
            continue;
        }
        pref_value(r, num, unit);
        {
            const felucca_font_t *nf = sel ? &FONT_B : &FONT_S;
            int32_t ny = vc(nf, y + 1, 18), ux = 226 - tw(&FONT_XS, unit);
            cv_text(ux, base_y(nf, ny, &FONT_XS), &FONT_XS, unit, C_GRAY);
            text_r(unit[0] ? ux - 2 : 226, ny, nf, num, sel ? C_WHITE : C_HI);
        }
        if (d && !d->names && d->max >= 24) {          /* a continuous value: its bar */
            int v = pref_get(r);
            rbox(108, y + 18, 70, 3, 1, C_LINE);
            if (r.kind == R_SHHI) {                    /* wip296: the HIGH CUT's bar grows from the right (off) to the left */
                if (v < d->max) { int w = 3 + (d->max - v) * 67 / d->max; rbox(178 - w, y + 18, w, 3, 1, sel ? part_col() : C_DIM); }
            } else if (v)
                rbox(108, y + 18, 3 + v * 67 / d->max, 3, 1, sel ? part_col() : C_DIM);
        }
    }
    if (ui.list_n > 6) {                               /* the scrollbar */
        int h = 2 * BAND_H * 6 / ui.list_n, y = 2 * BAND_H * ui.list_top / ui.list_n - band * BAND_H;
        rbox(235, 0, 3, BAND_H, 1, C_LINE);
        rbox(235, y, 3, h, 1, C_HI);
    }
}

static const felucca_font_t *btnfont(const char *s, int w)   /* wip317: the biggest font whose text fits the button */
{
    return tw(&FONT_B, s) <= w ? &FONT_B : tw(&FONT_S, s) <= w ? &FONT_S : &FONT_XS;
}
/* the three SAVE AS screens (see saveas_open): the top band says what, the lower one what the buttons do */
static void draw_saveas(int band)
{
    uint16_t purple = C_TBOTH;       /* the messages: the purple of a step with accent + fill */
    int st = (int)ui.ask_arg;
    const char *ok = st == 1 && proj.set.name_manual ? "OCT+  OK" : st == SV_HERE ? "OCT+  YES" : st == SV_CHOOSE ? "OCT+  CHOOSE SLOT" : st == 2 ? "OCT+  CONFIRM" : st == 1 ? "OCT+  SAVE" : "OCT+  OK";   /* OCT- left, OCT+ right: as on the device */
    const char *back = st == 1 && proj.set.name_manual && ui.nm_len ? "OCT-  DELETE" : st == SV_HERE ? "OCT-  CANCEL" : st == SV_CHOOSE ? "OCT-  NO, EXIT" : st == 0 || ui.sv_rename ? "OCT-  CANCEL" : "OCT-  BACK";
    if (band == 0) {
        char t[40], *q = t;
        frame(6, 6, 228, 63, purple);
        if (st == SV_HERE) {                       /* wip319: SAVE HERE? big, the number and the name small under it */
            q = t;
            if (ui.sv_here < 10)                       /* the number as in the list: 09 */
                *q++ = '0';
            q = put_i(q, ui.sv_here);
            q = put_s(q, "  ");
            put_s(q, ui.sv_pn[0] ? ui.sv_pn : "UNTITLED");
            text_c(120, 8, &FONT_B, "SAVE HERE?", C_WHITE);
            text_c(120, 36, &FONT_S, t, C_HI);
            if (ui.sv_here != ui.preset_cur) {         /* wip708: another spot: who goes (amber), or an empty place */
                char u[40], *w = put_s(u, ui.sv_occ[0] ? "REPLACES " : "EMPTY PLACE");
                if (ui.sv_occ[0])
                    put_s(w, ui.sv_occ);
                text_c(120, 54, &FONT_XS, u, ui.sv_occ[0] ? C_AMB : C_GRAY);
            }
        } else if (st == SV_CHOOSE) {
            text_c(120, 24, &FONT_B, "CHOOSE SLOT", C_WHITE);
        } else if (st == 0) {
            text_c(120, 11, &FONT_B, "SAVE AS", C_WHITE);
            put_s(q, ui.sv_name);
            text_c(120, 34, &FONT_M, t, C_HI);
        } else if (st == 1) {
            text_c(120, 11, &FONT_S, ui.sv_rename ? "RENAME" : "NAME", C_GRAY);
            if (proj.set.name_manual) {                /* wip324 */
                name_shown(t);
                text_c(120, 32, &FONT_M, t, C_WHITE);
            } else
                text_c(120, 32, &FONT_M, ui.sv_name, C_WHITE);
        } else {
            text_c(120, 9, &FONT_S, "SLOT", C_GRAY);
            put_i(t, ui.sv_here);
            text_c(120, 30, &FONT_L, t, C_WHITE);
        }
    } else {
        char t[40], *q;
        cv_rect(6, 0, 1, 50, purple);
        cv_rect(233, 0, 1, 50, purple);
        cv_rect(7, 50, 226, 1, purple);
        if (st == 2) {
            q = put_s(t, "TURN ALGORITHM  ");
            put_s(q, ui.sv_occ[0] ? ui.sv_occ : "EMPTY");
        } else if (st == SV_HERE) {                    /* wip708: the hint is always there: ALGORITHM = name, SELECT = spot; who is in another spot above it */
            put_s(t, "ALGORITHM: NAME  SELECT: SPOT");
        } else if (st == SV_CHOOSE)
            put_s(t, "ANOTHER SLOT, OR EXIT");
        else
            put_s(t, "ALGORITHM: NEW NAME");
        if (st == 2)
            text_c(120, 3, &FONT_XS, t, ui.sv_occ[0] ? C_AMB : C_GRAY);
        else                                    /* wip248: the name hint a size up and white */
            text_c(120, vc(&FONT_S, 0, 22), tw(&FONT_S, t) <= 226 ? &FONT_S : &FONT_XS, t, C_WHITE);
        frame(14, 22, 104, 24, C_GRAY);
        text_c(66, vc(&FONT_B, 22, 24), btnfont(back, 96), back, C_GRAY);
        box(122, 22, 104, 24, purple);
        text_c(174, vc(&FONT_B, 22, 24), btnfont(ok, 96), ok, C_BLACK);
    }
}

/* the CHAIN screen: the title and the chain as little numbered squares (the last 10 when it is longer), then the buttons */
static void draw_chain(int band)
{
    uint16_t purple = C_TBOTH;       /* the messages: the purple of a step with accent + fill */
    if (band == 0) {
        char t[24];
        int i, first = ui.ch_n > 10 ? ui.ch_n - 10 : 0;
        frame(6, 6, 228, 63, purple);
        put_s(t, "PATTERNS");
        text_c(120, 9, &FONT_S, t, C_WHITE);
        if (!ui.ch_n) {
            text_c(120, 38, &FONT_S, "PRESS 1-16", C_GRAY);
        } else {
            for (i = first; i < ui.ch_n; i++) {
                int x = 10 + (i - first) * 22;
                char n[4];
                put_i(n, ui.ch_pat[i] + 1);
                box(x, 28, 20, 24, i == ui.ch_n - 1 ? purple : C_GRAY);
                text_c(x + 10, vc(&FONT_S, 28, 24), &FONT_S, n, C_BLACK);
            }
        }
    } else {
        char t[24];
        cv_rect(6, 0, 1, 50, purple);
        cv_rect(233, 0, 1, 50, purple);
        cv_rect(7, 50, 226, 1, purple);
        put_i(put_s(t, "STEPS: "), ui.ch_n);
        text_c(120, 3, &FONT_XS, t, C_GRAY);
        frame(14, 22, 104, 24, C_GRAY);
        text_c(66, vc(&FONT_B, 22, 24), &FONT_B, ui.ch_n ? "OCT-  DEL" : "OCT-  CANCEL", C_GRAY);
        box(122, 22, 104, 24, purple);
        text_c(174, vc(&FONT_B, 22, 24), &FONT_B, "OCT+  OK", C_BLACK);
    }
}


static void draw_ask(int band)
{
    uint16_t purple = C_TBOTH;       /* the messages: the purple of a step with accent + fill */
    if (ui.ask_act == ACT_SAVEAS) {
        draw_saveas(band);
        return;
    }
    if (ui.ask_act == ACT_CHAIN) {
        draw_chain(band);
        return;
    }
    if (ui.ask_act == ACT_STOPFIRST) {               /* the whole screen: a thick purple border, the message big, it stays */
        cv_rect(0, 0, 4, BAND_H, purple);
        cv_rect(236, 0, 4, BAND_H, purple);
        if (band == 0) {
            cv_rect(0, 0, 240, 4, purple);
            text_c(120, 14, &FONT_M, "STOP FIRST", C_WHITE);
            text_c(120, 46, &FONT_B, "BEFORE SAVING", C_GRAY);
        } else {
            cv_rect(0, BAND_H - 4, 240, 4, purple);
            text_c(120, 2, &FONT_XS, "PLAY STOPS IT, THEN OCT+ SAVES", C_GRAY);
            frame(14, 20, 104, 24, C_GRAY);
            text_c(66, vc(&FONT_B, 20, 24), &FONT_B, "OCT-  BACK", C_GRAY);
            box(122, 20, 104, 24, purple);
            text_c(174, vc(&FONT_B, 20, 24), &FONT_B, "PLAY  STOP", C_BLACK);
        }
        return;
    }
    if (band == 0) {
        frame(6, 10, 228, 62, purple);
        cv_rect(7, 71, 226, 1, C_BG);
        if (ui.ask_act == ACT_PRESET || ui.ask_act == ACT_PRESETGO) {   /* wip198: the preset's number and name big and purple, the question small */
            const felucca_font_t *pf = tw(&FONT_M, ui.ask_q[1]) <= 216 ? &FONT_M : tw(&FONT_B, ui.ask_q[1]) <= 216 ? &FONT_B : &FONT_S;
            text_c(120, 16, &FONT_S, ui.ask_q[0], C_GRAY);
            text_c(120, vc(pf, 32, 36), pf, ui.ask_q[1], purple);
        } else if (ui.ask_act == ACT_QSAVE) {            /* wip308: the slot name big (purple), the question above it */
            const felucca_font_t *pf = tw(&FONT_M, ui.ask_q[1]) <= 216 ? &FONT_M : tw(&FONT_B, ui.ask_q[1]) <= 216 ? &FONT_B : &FONT_S;
            text_c(120, 14, &FONT_B, ui.ask_q[0], C_WHITE);
            text_c(120, vc(pf, 36, 30), pf, ui.ask_q[1], purple);
        } else {
            text_c(120, 24, &FONT_B, ui.ask_q[0], C_WHITE);
            text_c(120, 46, &FONT_S, ui.ask_q[1], C_GRAY);
        }
    } else {
        cv_rect(6, 0, 1, 44, purple);
        cv_rect(233, 0, 1, 44, purple);
        cv_rect(7, 44, 226, 1, purple);
        frame(22, 10, 92, 24, C_GRAY);
        text_c(68, vc(&FONT_B, 10, 24), &FONT_B, ui.ask_act == ACT_QSAVE ? "OCT-  EXIT" : ui.ask_act == ACT_PRESETGO || ui.ask_act == ACT_PASTE_TNPAT ? "OCT-  NO" : ui.ask_act == ACT_PRESET || octask() ? "OCT-  GO BACK" : "HOME  NO", C_GRAY);
        box(126, 10, 92, 24, purple);
        text_c(172, vc(&FONT_B, 10, 24), &FONT_B, (ui.ask_act == ACT_PRESETGO || ui.ask_act == ACT_QSAVE || ui.ask_act == ACT_PASTE_TNPAT) ? "OCT+  YES" : ui.ask_act == ACT_PRESET ? "OCT+  UPLOAD" : octask() ? "OCT+  CONFIRM" : "SEL  YES", C_BLACK);
    }
}

static void draw_help(int band)
{
    const char *l[HELP_LINES];
    int n = help_lines(ui.help - 1, l), k;
    uint16_t c = part_col();
    if (band == 0) {
        box(6, 4, 228, 24, c);
        text_c(120, vc(&FONT_B, 4, 24), &FONT_B, BTN_LABEL[ui.help - 1], C_BLACK);
    }
    for (k = 0; k < n; k++) {                            /* evenly spaced; a line across the bands in both */
        int32_t y = 34 + k * 18 - band * BAND_H;
        if (y > -18 && y < BAND_H)
            text_c(120, y, &FONT_XS, l[k], C_WHITE);
    }
}

#ifdef X0X_HOST
/* every card, on every kind of screen, fits the screen: 1 = yes (prints the first that does not) */
static int help_fits(void)
{
    static const uint8_t views[] = {V_PART, V_SEQ, V_HOME, V_SONG, V_FX, V_MIX, V_MIX2, V_MIX1, V_GLOB, V_PMOD, V_OSC, V_NOISE, V_NEV, V_VEL};
    const char *l[HELP_LINES];
    ui_t keep = ui;
    uint8_t keep_any = any_button;
    int b, p, v, kb, n, k, ok = 1;
    for (v = 0; v < (int)sizeof views; v++)
        for (p = 0; p < NPARTS; p++)
            for (kb = 0; kb < 2; kb++)
                for (b = 0; b < NB; b++) {
                    ui.view = views[v];
                    ui.part = (uint8_t)p;
                    ui.rec = (uint8_t)kb;
                    {
                        const char *fa, *fb, *fc;
                        int32_t w;
                        ui.overlay = (uint8_t)(b % 3);           /* none, a list, a question */
                        ui.rec = (uint8_t)(b & 2);
                        any_button = (uint8_t)(b != 0);
                        footer_text(&fa, &fb, &fc);
                        w = 4 + (fa ? tw(&FONT_T, fa) : 0) + (fb ? FOOT_GAP + tw(&FONT_T, fb) : 0) + (fc ? FOOT_GAP + tw(&FONT_T, fc) : 0);
                        if (w > 236) {
                            if (ok)
                                printf("footer too wide: \"%s / %s / %s\" (%d px)\n", fa ? fa : "", fb ? fb : "", fc ? fc : "", (int)w);
                            ok = 0;
                        }
                    }
                    n = help_lines(b, l);
                    if (n < 1 || n > HELP_LINES) {
                        if (ok)
                            printf("help card has %d lines: %s (view %d, part %d)\n", n, BTN_LABEL[b], views[v], p);
                        ok = 0;
                    }
                    for (k = 0; k < n; k++)
                        if (tw(&FONT_XS, l[k]) > 224) {
                            if (ok)
                                printf("help card too wide: %s: \"%s\" (%d px)\n", BTN_LABEL[b], l[k], (int)tw(&FONT_XS, l[k]));
                            ok = 0;
                        }
                }
    ui = keep;
    any_button = keep_any;
    return ok;
}
#endif

/* the footer: one line at the bottom of the main area, what the keys do on this screen (until the
 * first button, how to find out what any of them does) */
static const char ALGO_WORD[] = "TURN ALGORITHM";           /* wip594: TURN in front, all skins */
static const char CK_WORD[] = "CHOKE TURN ALGORITHM";             /* wip374: the EDIT page's whole footer */
static const char SC_WORD[] = "SIDECHAIN TURN ALGORITHM";           /* wip373: the SENDS page's whole footer */
static uint16_t footer_text(const char **a, const char **b, const char **c)
{
    *a = *b = *c = 0;
    if (ui.cpm && tonic_part() && ui.overlay != O_ASK) {          /* wip364: COPY-PASTE MODE: the blue hint */
        *a = "PRESS PATTERN TO PASTE";                            /* wip422: the same two lines whether the mode is kept (FUNC tapped) or not */
        *b = "PRESS SEQ TO EXIT";
        return C_SLBLUE;
    }
    if (!any_button) {
        *a = "HOLD ANY BUTTON: WHAT IT DOES";
        return C_SLBLUE;                                          /* wip368: every footer is in our blue */
    }
    if (ui.overlay == O_ASK && ui.ask_act == ACT_SAVEAS) {
        *a = ui.ask_arg == 2 ? "ALGO: SLOT" : ui.ask_arg == 1 ? (proj.set.name_manual ? "KNOB 4: LETTER" : "ALGO: NEW NAME") : ui.ask_arg == SV_HERE ? "ALGO: PLACE" : "OCT+: CHOOSE SLOT";
        *b = ui.ask_arg == 1 && proj.set.name_manual && ui.nm_len ? "OCT-: DELETE" : ui.sv_rename ? "OCT-: CANCEL" : "OCT-: BACK";
        *c = "OCT+: OK";                                         /* wip431: the letter editor says OK too */
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_STOPFIRST) {
        *a = "OCT-: BACK";
        *b = "PLAY: STOP";
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_CHAIN) {
        *a = "KEYS 1-16: ADD";
        *b = "OCT-: DELETE";
        *c = "OCT+: OK";
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_PASTE_TNPAT) {
        *a = "OCT-: NO";
        *b = "OCT+: YES";
    } else if (ui.overlay == O_ASK && octask()) {
        *a = "OCT-: GO BACK";
        *b = "OCT+: CONFIRM";
    } else if (ui.overlay == O_ASK && (ui.ask_act == ACT_PRESETGO || ui.ask_act == ACT_QSAVE)) {
        *a = ui.ask_act == ACT_QSAVE ? "OCT-: EXIT" : "OCT-: NO";
        *b = "OCT+: YES";
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_PRESET) {
        *a = "OCT-: GO BACK";
        *b = "OCT+: UPLOAD";
        *c = "PRESETS: OTHER";
    } else if (ui.overlay == O_ASK) {
        *a = "HOME: NO";
        *b = "SEL: YES";
    } else if (ui.overlay == O_MEM) {
        *a = "GLO: SETTINGS";
        *b = "HOME: BACK";
    } else if (ui.overlay == O_LIST) {
        *a = "SELECT: MOVE";
        *b = "ALGO: CHANGE";
        *c = "HOME: BACK";
    } else if (mixer_tall()) {                                    /* wip371: the GLO mixer: what SELECT does and what the red pan means */
        *a = "SELECT: CYCLE LVL/PAN";
        *b = "PAN CENTER = RED";
    } else if (ui.view == V_SEQ && ui.arp && tonic_part()) {      /* wip251 */
        *a = "KEYS: PIANO";
        *b = "SELECT: VOICE";
        *c = "K4/OCT: OCTAVE";
    } else if (ui.view == V_HOME) {
        *a = "FUNC+WHITE: PATTERN";
        *b = "FUNC+BLK: MUTE";
    } else if (ui.view == V_SONG) {                               /* wip356: in our blue, only on the SONG screen */
        *a = "WHITE: ADD ROW";                           /* wip362 / wip364 (he typed POLY by mistake: FILL) */
        *b = "FILL: DELETE";
        *c = "OCT+ ADD OCT- DEL";
        return C_SLBLUE;
    } else if (ui.view == V_MIX2 && cur_page() == 1 && !ui.dck && tonic_part()) {   /* wip373: SENDS: only SIDECHAIN ALGORITHM, at the right, next to the triangle */
        *c = SC_WORD;
    } else if (dck_page()) {                                      /* wip350: DUCK */
        *a = "BLACK: DUCK/NOT";
        *b = "PURPLE: INPUT";
        *c = "ALGO: BACK";
    } else if (chk_page()) {                                      /* wip328: CHOKE */
        *a = "BLACK: IN/OUT";
        *b = "GREY: NOT IN";
        *c = "ALGO LEFT: BACK";
    } else if (ui.view == V_NEV && !ui.chk && tonic_part()) {     /* wip374: EDIT: only CHOKE ALGORITHM, at the right, next to the triangle */
        *c = CK_WORD;
    } else if (drum_mix_page() >= 0) {                            /* the sound pages (wip119): the white keys are the steps (and light up) */
        *a = "BLACK: TRACK";
        *b = "WHITE: STEP";
        *c = "FUNC+BLK: MUTE";
    } else if (ui.seqpat && ui.view == V_SEQ && tonic_part()) {   /* the pattern page (wip112) */
        *a = "BLACK: MUTE/SOLO";
        *b = "WHITE: NEXT PAT";
        *c = "K1: SONG ON/OFF";
    } else if (msp_page()) {                                      /* wip272: MUTE / SOLO */
        *a = "FUNC: MUTE/CLEAR";
        *b = "FILL: SOLO/CLEAR";
        *c = "ALGO: BACK";                                          /* wip341: shorter, the blue triangle takes the corner */
    } else if (ui.view == V_FX && cur_page() == 0) {              /* PERFORM (wip118): black = mute / solo, white = next pattern */
        *a = "BLACK: MUTE/SOLO";
        *b = "WHITE: NEXT PAT";
        *c = ALGO_WORD;                                           /* wip370: just the word, right next to the blue triangle (draw_footer_y) */
    } else if (ui.view == V_FX && cur_page() >= 1) {             /* wip270: REVERB / DELAY and their second pages */
        *a = "ALGO: OTHER PAGE";
        *b = "SEL: PITCH MOD";
    } else if (ui.view == V_FX || ui.view == V_MIX || ui.view == V_GLO) {   /* (wip122: "SEL: ALL AS A LIST" was wrong: SEL opens PITCH MOD) */
        *a = "AGAIN: NEXT PAGE";
        *b = "SEL: PITCH MOD";
    } else if (is_drum()) {
        *a = "BLACK: TRACK";
        *b = "WHITE: STEP";
        *c = "FUNC+BLK: MUTE";
    } else {
        *a = "STEP + 1-8: PIN A SLICE";                /* (1-8 play the slices numbered above) */
        *b = "9 REV 10 HALF 11 STUT";
    }
    return C_SLBLUE;                                              /* wip368: every footer is in our blue (was C_DIM) */
}
#define FOOT_Y 71                     /* in the knob row: its last line, in the tiny face */
/* wip341: a page that has a 2nd page behind ALGORITHM: +1 = there is one to the right (turn ALGORITHM right), -1 = you are on it (turn left to go back), 0 = none */
static int algo_page_dir(void)
{
    int c;
    if (ui.help || ui.overlay != O_NONE || !tonic_part())
        return 0;
    if (ui.view == V_MIX2 && cur_page() == 1 && tonic_part())
        return ui.dck ? -1 : 1;                       /* wip350: SENDS <-> DUCK */
    if (ui.view == V_NEV)
        return ui.chk ? -1 : 1;                       /* EDIT <-> CHOKE */
    if (msp_on() && ui.view == V_FX)
        return ui.msp ? -1 : 1;                       /* PERFORM <-> MUTE / SOLO */
    c = cur_page();
    if (ui.view == V_FX && c >= 1 && pg.n >= 5)
        return c >= 3 ? -1 : 1;                       /* REVERB / DELAY <-> REVERB 2 / DELAY 2 */
    return 0;
}
static void algo_tri(int32_t x, int32_t y, int dir, uint16_t c)   /* 7 x 9, a little triangle pointing right (dir 1) or left (-1), edges anti-aliased */
{
    int32_t i, j;
    for (j = 0; j < 9; j++)
        for (i = 0; i < 7; i++) {
            float fy = (float)j + 0.5f - 4.5f, fx = (float)(dir > 0 ? i : 6 - i) + 0.5f;
            float edge = 6.5f - fx - fm_fabsf(fy) * 1.45f;
            float a = fm_clampf(edge * 0.7f + 0.5f, 0.0f, 1.0f) * fm_clampf(fx + 0.5f, 0.0f, 1.0f);
            px_blend(x + i, y + j, c, (uint32_t)(a * 16.0f + 0.5f));
        }
}
static int algo_level(void)                       /* wip359: the triangle fades slowly in and out (3-16, about 2 s round), no hard flash */
{
    int f = (int)(ui.frame & 63u), t = f < 32 ? f : 63 - f;
    return 3 + t * 13 / 31;
}
static void gt_footer(int y, int dir, const char *a, const char *b, const char *c, uint16_t col);
static int grigio(void);
/* wip594: the BIG way-back triangle of the DUCK / CHOKE screens in the skin: 16 x 24, black, bottom left of the area (o = the canvas's first row), same slow blink + pulse */
static void gt_back_tri(int32_t o)
{
    const int32_t w = 16, h = 24, x0 = 3, y0 = 180;
    int32_t i, j;
    uint32_t lv;
    if (!grigio() || (ui.frame & 8u) != 0)
        return;
    lv = (uint32_t)(9 + (algo_level() - 3) * 7 / 13);
    for (j = 0; j < h; j++) {
        int32_t py = y0 + j - o;
        if (py < 0 || py >= 104)
            continue;
        for (i = 0; i < w; i++) {
            float fy = (float)j + 0.5f - (float)h * 0.5f, fx = (float)(w - 1 - i) + 0.5f;
            float edge = ((float)w - 0.5f) - fx - fm_fabsf(fy) * (((float)w - 0.5f) / ((float)h * 0.5f));
            float a = fm_clampf(edge * 0.7f + 0.5f, 0.0f, 1.0f) * fm_clampf(fx + 0.5f, 0.0f, 1.0f);
            px_blend(x0 + i, py, RGB(0, 0, 0), (uint32_t)(a * (float)lv + 0.5f));
        }
    }
}
static int gt_big_page(void);
static int gt_fx_knobs(void);
static int gt_zoom_on(void);
static void gt_big_hint(void);
static void draw_footer_y(int y)
{
    const char *a, *b, *c;
    uint16_t col = footer_text(&a, &b, &c);
    int dir = algo_page_dir();
    if (grigio() && gt_zoom_on())                     /* wip576: the knob zoom owns the whole area: no hint */
        return;
    if (grigio() && gt_big_page()) {                  /* wip570: the skin's pages draw the hint BIG over the free strip under the knobs (gt_big_hint); this last canvas holds its lowest rows */
        cv_oy = -2 * 104;
        gt_big_hint();
        cv_oy = 0;
        return;
    }
    if (grigio()) {                                   /* wip550: Poppins, one blue line centred (the mockup), the triangles as before */
        gt_footer(y, dir, a, b, c, col);
        return;
    }
    if (dir > 0 && (c == ALGO_WORD || c == SC_WORD || c == CK_WORD)) {                  /* wip370: PERFORM: the word ALGORITHM sits right at the triangle (nothing says what it does) */
        int gap = FOOT_GAP;                           /* wip594: TURN ALGORITHM is longer: if the tips and the word do not fit, the tips close up */
        if ((a ? tw(&FONT_T, a) : 0) + (b ? tw(&FONT_T, b) : 0) + FOOT_GAP * 2 + tw(&FONT_T, c) > 221)
            gap = 5;
        segs(3, vc(&FONT_T, y, 9), &FONT_T, col, gap, a, b, 0, 0);
        segs(226 - tw(&FONT_T, c), vc(&FONT_T, y, 9), &FONT_T, col, FOOT_GAP, c, 0, 0, 0);
    } else
        segs(dir < 0 ? 15 : 4, vc(&FONT_T, y, 9), &FONT_T, col, FOOT_GAP, a, b, c, 0);   /* (on a 2nd page the text starts after the triangle) */
    if (dir)                                          /* wip341: our blue, bottom right = there is a page to the right; bottom LEFT = there is one to the left (the way back); wip359: it fades slowly in and out, the footer stays */
        algo_tri(dir > 0 ? 229 : 4, vc(&FONT_T, y, 9), dir, dim(C_SLBLUE, algo_level()));
}
static void draw_footer(void) { draw_footer_y(FOOT_Y); }

static int grigio(void);
/* wip568: GRIGIOTONIC: the PERFORM page (FX 2nd press, the first page of V_FX; not the MUTE / SOLO page) is drawn in his skin over the whole area */
static int gt_perf(void) { return grigio() && ui.view == V_FX && cur_page() == 0 && !msp_page() && !ui.help && ui.overlay == O_NONE; }
/* wip570: the pages of the skin that own the whole area: the hint line is drawn BIG under their knobs */
static int gt_big_page(void)
{
    return grigio() && (ui.view == V_MIX1 || ui.view == V_MIX2 || ui.view == V_GLOB || ui.view == V_VEL || ui.view == V_OSC || ui.view == V_PMOD || ui.view == V_NOISE || ui.view == V_NEV || fx_comp_page() || gt_perf()) && ui.overlay == O_NONE && !ui.help && !chk_page() && !dck_page();
}
/* QUARTER LAYOUT (HANDOFF_65 B): the screen under the title is 4 equal quarters, one knob each (MIX pages first) */
static int quarter_view(void)
{
    if (gt_perf())
        return 1;                                     /* wip568: PERFORM in the skin: the whole area */
    if (mixer_tall())
        return 1;                                     /* wip224: the mixer takes the whole area, no knob strip */
    if (ui.seqpat && ui.view == V_SEQ && tonic_part() && !ui.help && ui.overlay == O_NONE)
        return 1;                                     /* the pattern number screen takes the whole area */
    if (ui.view == V_GLO && ui.overlay == O_MEM && !ui.help)
        return 1;                                     /* wip274: the MEMORY page (the bucket) takes the whole area, so it can sit in the middle of the screen */
    if (grigio() && fx_comp_page() && !ui.help)
        return 1;                                     /* wip566: GRIGIOTONIC: the COMPRESSOR page, the whole area */
    return (ui.view == V_MIX1 || ui.view == V_MIX2 || ui.view == V_GLOB || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL) && !ui.help && ui.overlay == O_NONE;
}
static void draw_quarters(void);

static void draw_main(void)
{
    int band, readout = ui.touched >= 0 && (int32_t)(ui.touch_until - plat_ms()) > 0 && ui.overlay == O_NONE;
    if (!readout)
        ui.touched = -1;
    if (quarter_view()) {
        draw_quarters();
        return;
    }
    cv_begin(240, 2 * BAND_H, C_BG);           /* both bands, then one transfer: no tear between them */
    for (band = 0; band < 2; band++) {
        cv_band(band * BAND_H, BAND_H);
        if (ui.help)
            draw_help(band);
        else if (ui.overlay == O_ASK)
            draw_ask(band);
        else if (ui.overlay == O_LIST)
            draw_list(band);
        else if (ui.view == V_HOME)
            draw_home(band);
        else if (ui.view == V_FX || ui.view == V_GLO)
            draw_fx(band);
        else if (ui.view == V_MIX || ui.view == V_MIX2 || ui.view == V_MIX1 || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL)
            draw_mix(band);
        else if (ui.view == V_SONG)
            draw_song(band);
        else if (ui.view == V_PERF)
            draw_perf(band);
        else if (is_drum())
            draw_drum(band);
        else
            draw_break(band);
    }
    cv_band(0, 2 * BAND_H);
    if (readout && !proj.set.zoom_off && !(ui.view == V_SEQ && is_drum()) && !(ui.view == V_FX && cur_page() == 0) && !fx_comp_page() && !gt_fx_knobs() && ui.view != V_MIX && ui.view != V_SONG)   /* (the SEQ page: numbers only, no dial) */
        draw_readout();
    cv_commit_rect(1, 0, MAIN_Y);
}

/* -------------------------------------------------------------- knob strip --- */
static void cell_value(int cx, int y, const char *num, const char *unit, uint16_t c)
{
    int32_t w = tw(&FONT_B, num) + (unit[0] ? tw(&FONT_XS, unit) + 1 : 0), x = cx - w / 2;
    x = cv_text(x, y, &FONT_B, num, c);
    if (unit[0])
        cv_text(x + 1, base_y(&FONT_B, y, &FONT_XS), &FONT_XS, unit, C_GRAY);
}

static void cell_value_big(int cx, int y, const felucca_font_t *f, const char *num, const char *unit, uint16_t c)
{
    int32_t w = tw(f, num) + (unit[0] ? tw(&FONT_S, unit) + 1 : 0), x = cx - w / 2;
    x = cv_text(x, y, f, num, c);
    if (unit[0])
        cv_text(x + 1, base_y(f, y, &FONT_S), &FONT_S, unit, C_GRAY);
}

/* what the knob strip shows, hashed: it is only redrawn when this changes (the arcs cost a
 * square root and an arctangent per pixel; most frames nothing in the strip moves) */
static uint32_t knobs_sig(void)
{
    uint32_t h = 2166136261u, i;
    int pgi = cur_page();
#define MIXIN(v) (h = (h ^ (uint32_t)(v)) * 16777619u)
    MIXIN(ui.view);
    MIXIN(ui.part);
    MIXIN(ui.arp);
    MIXIN(bigchip_page() ? (uint32_t)ui.rec | (uint32_t)rec_blink_on() << 1 : 0u);   /* wip684 / wip703: the lower half of the big chip (ARP and SEQ) carries the REC dot */
    MIXIN(pgi);
    MIXIN(ui.touched);
    MIXIN(ui.spage | proj.set.autoscroll << 8 | spage_spot() << 9);   /* wip567: the SEQ page spot follows the playhead (wip583: and it shows after ALGORITHM) */
    MIXIN(ui.held_step);
    MIXIN(lock_step() + 1);
    MIXIN(proj.set.palette);
    MIXIN(ui.sel[0] | ui.sel[1] << 8);
    MIXIN(seq.ppat[ui.part]);
    MIXIN(ui.song_sel);
    if (algo_page_dir())
        MIXIN(algo_level());                            /* wip359: the triangle fades */
    {
        const char *fa, *fb, *fc;
        MIXIN(footer_text(&fa, &fb, &fc));
        MIXIN((uintptr_t)fa ^ ((uintptr_t)fb << 3) ^ ((uintptr_t)fc << 7));
        MIXIN(pg.n);
    }
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        MIXIN(r.kind | r.a << 8 | r.b << 16 | (uint32_t)r.c << 24);
        MIXIN(pref_get(r));
        MIXIN(has_motion(r) | (motion_now(r) + 1) << 1);
        MIXIN(lock_of(r, lock_step()) + 1);
    }
#undef MIXIN
    return h;
}

static int strcmp_s(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a != *b;
}

/* the knob row's tag: whose knobs these are (BD, 909 KIT, P3 PATTERN, 909 MIX: BD, 303A STEP 5) */
static void knob_tag(char *t)
{
    int g = pg.group[cur_page()], dm = drum_mix_page();
    const char *ti = pg.title[cur_page()];
    if (lock_step() >= 0)
        put_i(put_s(put_s(t, is_drum() ? engine_voice_name(T_909, ui.sel[ui.part])
                                        : PART_N[ui.part]), " P-LOCK STEP "), lock_step() + 1);
    else if (dm >= 0)
        put_s(put_s(put_s(t, ti), ": "), engine_voice_name(T_909, ui.sel[dm]));
    else if (g == GR_PAT)
        put_s(put_s(put_i(put_s(t, "P"), seq.ppat[ui.part] + 1), " "), ti);
    else if (g == GR_KIT || g == GR_SOUND)
        put_s(put_s(put_s(t, PART_N[ui.part]), " "), ti);
    else
        put_s(t, ti);
    {   /* a section over several pages (same name): which of them, "BD (1/2)" */
        int p = cur_page(), a = p, b = p;
        while (a > 0 && !strcmp_s(pg.title[a - 1], ti))
            a--;
        while (b + 1 < pg.n && !strcmp_s(pg.title[b + 1], ti))
            b++;
        if (b > a) {
            char *q = t;
            while (*q)
                q++;
            put_s(put_i(put_s(put_i(put_s(q, " ("), p - a + 1), "/"), b - a + 1), ")");
        }
    }
}

static void gt_knob_cell(int i, int32_t cy, int32_t ny, int32_t vy);
/* wip590: the REVERB / DELAY pages (long press FX / HOME, and their 2nd pages) in the skin: the knob strip uses the compressor's small knobs, no big zoom */
static int gt_fx_knobs(void) { return grigio() && ui.view == V_FX && cur_page() != 0 && !msp_page() && ui.overlay == O_NONE && !ui.help; }
static void draw_knobs(void)
{
    static uint32_t last_sig;
    int i, pgi = cur_page();
    uint32_t sig = knobs_sig();
    if (quarter_view())
        return;
    if (ui.view == V_GLO && (ui.overlay == O_LIST || ui.overlay == O_MEM)) {   /* wip224: the settings list (wip267: and MEMORY): no compressor knobs below it */
        cv_begin(240, KNOB_H, C_BG);
        cv_commit(3, 0, KNOB_Y);
        last_sig = 0;
        return;
    }
    if (sig == last_sig && blit_hash[3])
        return;
    last_sig = sig;
    cv_begin(240, KNOB_H, C_BG);
    {   /* the rule; at its right end, where you are: the tag, then the page dots */
        char t[28];
        int dots = pg.n > 1 && ui.view != V_GLO;
        int32_t xd = 237 - (dots ? pg.n * 5 : 0), xt;
        knob_tag(t);
        if (ui.view == V_SONG)                         /* wip357: the SONG screen has no tag (BAR) */
            t[0] = 0;
        if (bigchip_page())   /* wip683 / wip703: the ARP and SEQ pages: no tag, no dots, the rule stops before the big chip */
            t[0] = 0, dots = 0, xd = 237;
        xt = xd - (dots ? 5 : 0) - tw(&FONT_XS, t);
        cv_rect(0, 5, (bigchip_page() ? 100 : xt - 5), 1, C_LINE);
        cv_text(xt, vc(&FONT_XS, 0, 11), &FONT_XS, t, C_HI);
        if (dots) {
            int k;
            for (k = 0; k < pg.n; k++)
                cv_rect(xd + k * 5, 4, 3, 3, k == cur_page() ? C_HI : C_LINE);
        }
    }
    if (bigchip_page()) {  /* wip683 / wip703: the lower part of the big track chip */
        cv_oy = -(KNOB_Y - MAIN_Y);
        arp_chip(0);
        cv_oy = 0;
    }
    draw_footer();
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        char num[16], unit[8];
        int cx = i * 60 + 30, val, touched = ui.touched == i;
        uint16_t col = drum_mix_page() >= 0 ? PART_COL[drum_mix_page()] : part_col();   /* MIX: 909 in the 909's */
        if (gt_fx_knobs()) {
            cv_oy = -(KNOB_Y - MAIN_Y);
            gt_knob_cell(i, 162, 185, 196);
            cv_oy = 0;
            continue;
        }
        if (i)
            cv_rect(i * 60, 14, 1, 50, C_LINE);
        if (i == 2 && spage_spot() && (ui.arp || r.kind == R_DLEN)) {   /* wip583: also the ARP page (its K3 is free) and, with AUTOSCROLL off, for 1.2 s after ALGORITHM   /* wip567: AUTOSCROLL on: the FREE LEN cell is the big PAGE spot (where the playhead is); turned = why the length stays */
            char pb[16];
            const felucca_font_t *pf = &FONT_L;
            box(i * 60 + 2, 11, 56, 56, dim(C_AMB, touched ? 3 : 2));
            if (touched && !ui.arp && proj.set.autoscroll) {
                text_c(cx, vc(&FONT_XS, 20, 9), &FONT_XS, "TURN AUTOSCROLL", C_WHITE);
                text_c(cx, vc(&FONT_XS, 32, 9), &FONT_XS, "OFF TO CHANGE", C_WHITE);
                text_c(cx, vc(&FONT_XS, 44, 9), &FONT_XS, "LENGTH", C_WHITE);
                continue;
            }
            put_i(put_s(put_i(pb, ui.spage * 16 + 1), "-"), ui.spage * 16 + 16);
            if (tw(pf, pb) > 54)
                pf = &FONT_M;
            if (tw(pf, pb) > 54)
                pf = &FONT_B;
            cell_value_big(cx, vc(pf, 16, 30), pf, pb, "", C_AMB);
            text_c(cx, vc(&FONT_XS, 52, 9), &FONT_XS, "PAGE", C_WHITE);
            continue;
        }
        if (!d)
            continue;
        if (touched)
            box(i * 60 + 2, 11, 56, 56, dim(col, 4));
        val = pref_get(r);
        if (lock_step() >= 0) {                        /* a held step: its own value (amber mark: locked) */
            int lv = lock_of(r, lock_step());
            if (lv >= 0)
                val = lv;
            if (lockable(r))
                dot(i * 60 + 52, 17, 2, lv >= 0 ? C_AMB : C_LINE);
        } else if (has_motion(r)) {                    /* recorded motion: a mark, and what it plays now */
            int mv = motion_now(r);
            dot(i * 60 + 52, 17, 2, mv >= 0 ? C_WHITE : C_DIM);
            if (mv >= 0)
                val = mv;
        }
        if (ui.view == V_SEQ && is_drum()) {            /* the SEQ page: only BIG numbers (a small screen), no knob, no pips */
            const felucca_font_t *vf = &FONT_L;
            int k;
            pref_value_of(r, val, num, unit);
            for (k = 0; num[k]; k++)
                if ((uint8_t)num[k] > '9')
                    vf = &FONT_M;
            if (tw(vf, num) > 54)
                vf = &FONT_B;
            if (r.kind == R_DLEN)
                unit[0] = 0;
            cell_value_big(cx, vc(vf, 16, 30), vf, num, unit, touched ? C_WHITE : C_HI);
            text_c(cx, vc(&FONT_XS, 52, 9), &FONT_XS, caps_of(pref_name(r)), touched ? C_WHITE : C_GRAY);
            continue;
        }
        if (d->names && d->max < 12) {                 /* a switch: its positions as pips (more: a ring) */
            int n = d->max + 1, k, pw = n > 6 ? 3 : 6, gap = 2, w0 = n * (pw + gap) - gap;
            for (k = 0; k < n; k++)
                box(cx - w0 / 2 + k * (pw + gap), 21, pw, 9, k == val ? col : C_LINE);
        } else {
            seg_knob(cx, 26, 13, 13, d->max ? (float)val / (float)d->max : 0.0f, bipolar_of(r), 0);
            if (extra_off(r))                          /* wip305: CLASSIC: a shade over the knob */
                shade_rect(cx - 16, 11, 32, 32);
        }
        {
            const char *nm = caps_of(pref_name(r));
            if (ui.view == V_FX && (nm[0] == 'R' || nm[0] == 'D') && nm[1] == (nm[0] == 'R' ? 'E' : 'L') && (nm[2] == 'V' || nm[2] == 'Y') && nm[3] == ' ')
                nm += 4;                                  /* wip270: "REV LOW CUT" / "DLY RETURN" on the REVERB 2 / DELAY 2 pages: the page's title says which */
            text_c(cx, vc(&FONT_XS, 42, 9), &FONT_XS, nm, touched ? C_WHITE : C_GRAY);   /* the name, under the knob */
        }
        pref_value_of(r, val, num, unit);
        cell_value(cx, vc(&FONT_B, 53, 13), num, unit, touched ? C_WHITE : C_HI);
    }
    cv_commit(3, 0, KNOB_Y);
}


#define Q_H 104                              /* a quarter: 120 x 104; the footer keeps the last 12 rows */
/* TONIC slider (wip72): a dark track, a blue trace from the MIDDLE to the thumb (nothing in the middle), a round thumb */
/* trace: 0 = from the middle (bipolar), 1 = from the left end, 2 = none (a thumb only) */
static void seg_slider(int32_t x, int32_t y, int32_t w, float v, int trace)
{
    int32_t mid = trace == 1 ? x : x + w / 2, tx, ty = y + 7, a, b;
    v = fm_clampf(v, 0.0f, 1.0f);
    tx = x + 7 + (int32_t)(v * (float)(w - 14) + 0.5f);
    box(x, ty - 2, w, 5, dim(C_SLBLUE, 3));                   /* the track */
    a = tx < mid ? tx : mid;
    b = tx < mid ? mid : tx;
    if (trace != 2 && b - a > 1)
        box(a, ty - 2, b - a, 5, C_SLBLUE);                   /* the trace: middle (or left end) .. thumb */
    dot(tx, ty, 7, C_SLBLUE);                                 /* the thumb: ring + light centre */
    dot(tx, ty, 5, C_SLTHUMB);
}
/* a slider cell: no dial. is_mixsl = the Mix, is_eqfreq = the EQ Freq. Add new sliders here. */
static int is_mixsl(pref_t r)
{
    const x0x_param_t *d = pref_desc(r);
    return r.kind == R_ENG && r.a == T_909 && d && d->name[0] == 'M' && d->name[1] == 'i' && d->name[2] == 'x' && !d->name[3];
}
static int slider_cell(int i) { return (ui.view == V_OSC && i == 1) || (ui.view == V_NOISE && i == 1) || (ui.view == V_MIX1 && i >= 0 && i < 2) || (ui.view == V_GLOB && i >= 0 && i < 4); }

/* the 3 words over a slider: left end, name (middle), right end */
static void slider_words(pref_t r, const char **l, const char **c, const char **rt)
{
    if (is_mixsl(r))
        *l = "OSC", *c = "MIX", *rt = "NOISE";
    else if (is_eqfreq(r))
        *l = "20 Hz", *c = "EQ FREQ", *rt = "20 kHz";
    else if (is_oscfreq(r))
        *l = "20 Hz", *c = "OSC FREQ", *rt = "20 kHz";
    else if (is_nffreq(r))
        *l = "20 Hz", *c = "FILTER FREQ", *rt = "20 kHz";
    else if (r.kind == R_QUANT)
        *l = "1/8", *c = "QUANTIZATION", *rt = "1/32";
    else if (r.kind == R_SWING)
        *l = "0%", *c = "SWING", *rt = "100%";
    else if (r.kind == R_FILLR)
        *l = "2x", *c = "FILL RATE", *rt = "8x";
    else if (r.kind == R_TEMPO)
        *l = "20", *c = "TEMPO", *rt = "275";
    else if (r.kind == R_SCINT)
        *l = "-INF", *c = "SIDECHAIN THRESHOLD", *rt = "0dB";   /* wip617; wip619; wip620 */
    else if (r.kind == R_SCIN)
        *l = "1", *c = "SIDECHAIN INPUT", *rt = "8";
    else if (is_pan(r))
        *l = "L", *c = "PAN", *rt = "R";                   /* wip556 */
    else if (r.kind == R_ENG && r.a == T_909 && pref_desc(r) && name_is(pref_desc(r), "Rev"))
        *l = "0", *c = "REVERB SEND", *rt = "100%";        /* wip559 */
    else if (r.kind == R_ENG && r.a == T_909 && pref_desc(r) && name_is(pref_desc(r), "Dly"))
        *l = "0", *c = "DELAY SEND", *rt = "100%";
    else if (r.kind == R_ENG && r.a == T_909 && pref_desc(r) && name_is(pref_desc(r), "VelOsc"))
        *l = "0", *c = "OSC", *rt = "100%";                /* wip561: the VELOCITY page */
    else if (r.kind == R_ENG && r.a == T_909 && pref_desc(r) && name_is(pref_desc(r), "VelNs"))
        *l = "0", *c = "NOISE", *rt = "100%";
    else if (r.kind == R_ENG && r.a == T_909 && pref_desc(r) && name_is(pref_desc(r), "VelMod"))
        *l = "0", *c = "PITCH MOD", *rt = "100%";
    else if (r.kind == R_ENG && r.a == T_909 && name_is(pref_desc(r), "Level") && pref_desc(r)) {   /* the track's level: the real value at both ends */
        static char lb[2][16];
        char num[16], unit[8];
        pref_value_of(r, 0, num, unit);
        put_s(put_s(lb[0], num), unit);
        pref_value_of(r, pref_desc(r)->max, num, unit);
        put_s(put_s(lb[1], num), unit);
        *l = lb[0], *c = "LEVEL", *rt = lb[1];
    } else
        *l = "", *c = caps_of(pref_name(r)), *rt = "";
}
/* hundredths "05.00" .. "100.00" (at least 2 integer digits) */
static char *put_h(char *b, int h)
{
    if (h < 1000)
        *b++ = '0';
    b = put_i(b, h / 100);
    *b++ = '.';
    *b++ = (char)('0' + h / 10 % 10);
    *b++ = (char)('0' + h % 10);
    *b = 0;
    return b;
}
/* the big rectangle's two lines for a slider: title, value */
static void slider_readout_text(pref_t r, int v, char *t1, char *t2)
{
    char num[16], unit[8];
    if (is_mixsl(r)) {
        int m = v * 10000 / 127;                              /* 0..100.00 */
        char *e;
        put_s(t1, "OSC/NOISE MIX");
        e = put_h(t2, 10000 - m);
        *e++ = '/';
        put_h(e, m);
        return;
    }
    put_s(t1, is_oscfreq(r) ? "OSC FREQ" : is_nffreq(r) ? "FILTER FREQ" : caps_of(pref_name(r)));
    pref_value_of(r, v, num, unit);
    put_s(put_s(put_s(t2, num), unit[0] ? " " : ""), unit);
}
/* the big rectangle (same frame as the knob zoom) over the knobs' place, with the slider's values */
static void draw_slider_box(pref_t r)
{
    char t1[24], t2[32];
    const felucca_font_t *vf = &FONT_M, *nf = &FONT_M;   /* wip74: the name in the biggest letter font (was FONT_B) */
    slider_readout_text(r, pref_shown(r), t1, t2);          /* wip249: a held step shows its own (locked) value */
    cv_rect(0, 0, 240, Q_H, C_BG);
    frame(2, 2, 236, Q_H - 4, C_AMB);
    if (tw(nf, t1) > 224)
        nf = &FONT_B;                                      /* (a very long name only) */
    text_c(120, vc(nf, 12, 32), nf, t1, C_GRAY);
    if (tw(vf, t2) > 224)
        vf = &FONT_B;
    text_c(120, vc(vf, 56, 40), vf, t2, C_WHITE);
}
/* little marks under a stepped slider: one per position (the thumb sits on one of them) */
static void slider_ticks(int32_t x, int32_t y, int32_t w, int n)
{
    int k;
    for (k = 0; k < n; k++)
        box(x + 7 + (k * (w - 14) + (n - 1) / 2) / (n - 1) - 1, y, 2, 4, C_GRAY);
}

/* one slider (52 rows high): the 3 words, the slider, and for a stepped one the marks of its positions */
static void draw_slider_cell(int sl, int32_t y0)
{
    int pgi = cur_page(), touched = ui.touched == sl, val;
    pref_t r = pg.r[pgi][sl];
    const x0x_param_t *d = pref_desc(r);
    const char *wl, *wc, *wr;
    if (!d)
        return;
    val = pref_shown(r);                              /* wip249: the slider follows a held step's lock (or the recorded motion) */
    if (touched)
        box(4, y0 + 2, 232, Q_H / 2 - 4, dim(PART_COL[0], 4));
    slider_words(r, &wl, &wc, &wr);
    cv_text(14, vc(&FONT_S, y0 + 12, 11), &FONT_S, wl, touched ? C_WHITE : C_GRAY);
    text_c(120, vc(&FONT_S, y0 + 12, 11), &FONT_S, wc, touched ? C_WHITE : C_HI);
    text_r(226, vc(&FONT_S, y0 + 12, 11), &FONT_S, wr, touched ? C_WHITE : C_GRAY);
    seg_slider(14, y0 + 24, 212, d->max ? (float)val / (float)d->max : 0.0f,
               ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL ? 1 : ui.view != V_GLOB ? 0 : d->names ? 2 : 1);   /* GLOBAL: swing fills from the left, the stepped two only a thumb */
    if (d->names && d->max >= 1 && d->max < 12)
        slider_ticks(14, y0 + 40, 212, d->max + 1);
}

/* the 3-value icon of the pitch mod shape (SEL, K1): a blue disc with the black curve of DEC / SIN / RND (Microtonic look) */
static void draw_mod_icon(int cx, int cy, int r, int shape, int lit)
{
    static const int8_t rnd[8] = {5, -8, 2, 9, -4, -9, 7, -2};   /* RND: a stepped random line (fixed, in 1/10 of the half height) */
    int hw = r - 8, x, ph = r - 12;
    disc((float)cx + 0.5f, (float)cy + 0.5f, (float)r, lit ? C_SLBLUE : dim(C_SLBLUE, 3));
    for (x = -hw; x <= hw; x++) {
        float t = (float)(x + hw) / (float)(2 * hw), y;
        if (shape == 0)                                   /* DEC: starts high, falls away */
            y = 1.0f - 2.0f * (1.0f - fm_expf(-4.0f * t)) / (1.0f - fm_expf(-4.0f));
        else if (shape == 1)                              /* SIN: one and a half waves */
            y = -fm_sinf(t * 3.0f * 3.14159265f);
        else {                                            /* RND: 8 steps */
            int k = (int)(t * 7.999f);
            y = (float)rnd[k] / 10.0f;
        }
        disc((float)(cx + x) + 0.5f, (float)cy + 0.5f - y * (float)ph * 0.7f, 2.0f, C_BLACK);
        if (shape == 2 && x > -hw) {                      /* join the steps with a vertical line */
            int k0 = (int)((float)(x - 1 + hw) / (float)(2 * hw) * 7.999f), k1 = (int)(t * 7.999f), j;
            if (k0 != k1)
                for (j = 0; j <= 8; j++)
                    disc((float)(cx + x) + 0.5f, (float)cy + 0.5f - ((float)rnd[k0] + (float)(rnd[k1] - rnd[k0]) * (float)j / 8.0f) / 10.0f * (float)ph * 0.7f, 2.0f, C_BLACK);
        }
    }
}

/* the oscillator's waveform icon (ENV, K1): blue disc, black SIN / TRI / SAW */
static void draw_wave_icon(int cx, int cy, int r, int shape)
{
    int hw = r - 7, x, j;
    float amp = (float)(r - 10) * 0.7f;
    disc((float)cx + 0.5f, (float)cy + 0.5f, (float)r, C_SLBLUE);
    for (x = -hw; x <= hw; x++) {
        float t = (float)(x + hw) / (float)(2 * hw) * 2.0f, f = t - (float)(int)t, y;   /* two cycles */
        if (shape == 0)
            y = fm_sinf(f * 6.2831853f);
        else if (shape == 1)
            y = f < 0.25f ? f * 4.0f : f < 0.75f ? 2.0f - f * 4.0f : f * 4.0f - 4.0f;
        else
            y = f < 0.5f ? f * 2.0f : f * 2.0f - 2.0f;                     /* SAW: ramp up, then the drop */
        disc((float)(cx + x) + 0.5f, (float)cy + 0.5f - y * amp, 2.0f, C_BLACK);
        if (shape == 2 && f < 0.5f && f + 1.0f / (float)hw >= 0.5f)       /* the drop of the SAW: a vertical line */
            for (j = -8; j <= 8; j++)
                disc((float)(cx + x) + 0.5f, (float)cy + 0.5f - (float)j * amp / 8.0f, 2.0f, C_BLACK);
    }
}

/* the noise filter icon (LFO, K1): blue disc, black response curve of LP / BP / HP */
static void draw_filter_icon(int cx, int cy, int r, int mode)
{
    int hw = r - 8, x;
    float amp = (float)(r - 12) * 0.9f;
    disc((float)cx + 0.5f, (float)cy + 0.5f, (float)r, C_SLBLUE);
    for (x = -hw; x <= hw; x++) {
        float t = (float)x / (float)hw, y;                       /* -1 .. 1 */
        if (mode == 0)                                          /* LP: flat, then falls */
            y = t < 0.1f ? 0.45f : 0.45f - (t - 0.1f) * 1.7f;
        else if (mode == 2)                                     /* HP: rises, then flat */
            y = t > -0.1f ? 0.45f : 0.45f + (t + 0.1f) * 1.7f;
        else {                                                  /* BP: a hump */
            float a = t < 0.0f ? -t : t;
            y = 0.5f - a * 1.5f;
        }
        if (y < -0.5f)
            y = -0.5f;
        disc((float)(cx + x) + 0.5f, (float)cy + 0.5f - y * amp * 0.8f, 2.0f, C_BLACK);
    }
}

/* the noise envelope icon (EDIT, K1): blue disc, black EXP / LIN / CLAP curve */
static void draw_env_icon(int cx, int cy, int r, int mode)
{
    int hw = r - 8, x, prev = -1;
    float ph = (float)(r - 12) * 0.7f, py = 0.0f;
    disc((float)cx + 0.5f, (float)cy + 0.5f, (float)r, C_SLBLUE);
    for (x = -hw; x <= hw; x++) {
        float t = (float)(x + hw) / (float)(2 * hw), a, y;
        int j;
        if (mode == 0)                                    /* EXP: starts high, falls away */
            a = fm_expf(-4.0f * t);
        else if (mode == 1)                               /* LIN: a straight fall */
            a = 1.0f - t;
        else if (t < 0.48f) {                             /* CLAP: three short bursts, */
            int k = (int)(t / 0.16f);
            a = fm_expf(-2.5f * (t - (float)k * 0.16f) / 0.16f);
        } else                                            /* then a longer tail */
            a = 0.9f * fm_expf(-3.0f * (t - 0.48f) / 0.52f);
        y = 1.0f - 2.0f * a;
        if (prev >= 0 && (y - py > 0.12f || py - y > 0.12f))   /* a steep part or the jump of a burst: join with a line */
            for (j = 0; j <= 12; j++)
                disc((float)(cx + x) + 0.5f, (float)cy + 0.5f + (py + (y - py) * (float)j / 12.0f) * ph, 2.0f, C_BLACK);
        disc((float)(cx + x) + 0.5f, (float)cy + 0.5f + y * ph, 2.0f, C_BLACK);
        prev = x;
        py = y;
    }
}

/* text enlarged k times (nearest neighbour on the anti-aliased glyph, coverage blended as cv_text does); returns the end x.
 * Used by the pattern screen (SEQ 2nd press): the number must fill the screen */
static int32_t big_text(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c, int32_t k)
{
    int32_t w = 0;
    const char *q;
    for (q = s; *q; q++)
        w += f->adv[(uint8_t)*q - f->first] * k;
    {
        uint32_t cr = c >> 11, cg = (c >> 5) & 63u, cb = c & 31u;
        for (; *s; s++) {
            uint32_t gi = (uint8_t)*s - f->first, gx, gy, gw = f->bw[gi], bpr = (gw + 1u) / 2u;
            const uint8_t *gd = f->data + f->off[gi];
            for (gy = 0; gy < f->h; gy++)
                for (gx = 0; gx < gw; gx++) {
                    uint32_t a = gd[gy * bpr + gx / 2u], bg, i, j;
                    a = (gx & 1u) ? (a & 15u) : (a >> 4);
                    if (!a)
                        continue;
                    for (j = 0; j < (uint32_t)k; j++)
                        for (i = 0; i < (uint32_t)k; i++) {
                            int32_t px = x + ((int32_t)gx - f->pad) * k + (int32_t)i, py = y + (int32_t)gy * k + (int32_t)j + cv_oy;
                            if ((uint32_t)px >= cv_w || py < cv_y0 || py >= cv_y1)
                                continue;
                            if (a == 15u) {
                                cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16(c);
                            } else {
                                bg = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
                                cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16((((cr * a + (bg >> 11) * (15u - a)) / 15u) << 11) |
                                    ((((cg * a + ((bg >> 5) & 63u) * (15u - a)) / 15u)) << 5) | ((cb * a + (bg & 31u) * (15u - a)) / 15u));
                            }
                        }
                }
            x += f->adv[(uint8_t)*s - f->first] * k;
        }
    }
    (void)w;
    return x;
}
static int32_t big_w(const felucca_font_t *f, const char *s, int32_t k)
{
    int32_t w = 0;
    for (; *s; s++)
        w += f->adv[(uint8_t)*s - f->first] * k;
    return w;
}

/* the pattern information, drawn in an area 240 wide and H high at the canvas origin (the caller sets cv_oy): the NEXT pattern as a
 * blinking purple box "PATTERN n" (top third, only while one is cued), the current pattern number as big as fits below.
 * Used by the pattern screen (SEQ 2nd press, H = the whole area) and by the PERFORM page (FX 2nd press, the area above the knobs) */
static void draw_patinfo(int H)
{
    char t[8], n[20], *q;
    int cur = seq.ppat[PART_909], cue = pat_next();
    int ch, pend = cue >= 0 && cue != cur, k, top = pend ? H * 35 / 100 : 0, avail = H - top, nw;
    put_i(t, cur + 1);
    nw = big_w(&FONT_L, t, 1);
    ch = caps(&FONT_L)->bot - caps(&FONT_L)->top + 1;
    for (k = 12; k > 1 && (ch * k > avail - 8 || nw * k > 232); k--)
        ;
    k = k / 2 > 1 ? k / 2 : 1;                   /* wip110: the current number at half the size that would fill the area */
    q = put_s(n, "PATTERN ");
    put_i(q, cue + 1);
    if (pend && (plat_ms() % 500u) < 250u) {     /* the NEXT pattern: a purple box about a third of the area, blinking (wip625: by the clock, even) */
        rbox(4, 4, 232, top - 8, 10, grigio() ? RGB(166, 106, 224) : C_TBOTH);
        {                                        /* wip110: the box text twice as big (FONT_B x2), centred in the box */
            int bch = caps(&FONT_B)->bot - caps(&FONT_B)->top + 1;
            big_text(120 - big_w(&FONT_B, n, 2) / 2, 4 + (top - 8 - bch * 2) / 2 - caps(&FONT_B)->top * 2, &FONT_B, n, grigio() ? RGB(255, 255, 255) : C_WHITE, 2);
        }
    }
    big_text(120 - big_w(&FONT_L, t, k) / 2, top + (avail - ch * k) / 2 - caps(&FONT_L)->top * k, &FONT_L, t, C_WHITE, k);
}

/* ---- GRIGIOTONIC drawing tools (wip545; docs/grigiotonic_*_mockup.png, tools/grigiotonic_*_mockup.py are the reference, units = pixels of the 240 screen).
 * Cheap on purpose: solid shapes in a few layers (no per-pixel gradients except the knob's arc). Used only when THEME = GRIGIOTONIC. ---- */
#define GT_INK RGB(62, 64, 66)
#define GT_B1 RGB(28, 140, 214)
#define GT_B2 RGB(70, 200, 240)
#define GT_LIGHT RGB(233, 234, 233)
#define GT_DARK RGB(146, 148, 147)
#define GT_FILL RGB(201, 202, 201)
#define GT_GREEN RGB(48, 184, 104)
#define GT_GREEN_L RGB(150, 232, 176)
#define GT_RED RGB(222, 58, 58)                    /* wip572: the centred pan of the GLO mixer */
#define GT_RED_L RGB(246, 150, 150)
#define GT_PURPLE_L RGB(200, 160, 244)
#define GT_PURPLE RGB(166, 106, 224)               /* the firmware purple (accent + fill): the RNDM button */
#define GT_SH RGB(104, 106, 105)                 /* neutral shadow grey (thumb shadow, drawn at 3/16) */
static int grigio(void) { return proj.set.palette == GRIGIO_PAL; }
/* wip547: the art is PRE-RENDERED (tools/gen_gt_art.py -> gt_art.h: groove, thumb, knob base as RGB565 + alpha, and Poppins glyphs), same
 * geometry and soft shadows as the mockup; the code draws only what moves (thumb place, blue arc, white thumb, texts). */
#include "gt_art.h"
static void gt_px(int32_t x, int32_t y, uint16_t c, uint32_t a)             /* a = 0..255 */
{
    uint16_t *q;
    uint32_t bg, r, g, b, v;
    y += cv_oy;
    if ((uint32_t)x >= cv_w || y < cv_y0 || y >= cv_y1 || !a)
        return;
    q = &cv_px[(uint32_t)y * cv_w + (uint32_t)x];
    if (a >= 255u) {
        *q = swap16(c);
        return;
    }
    bg = swap16(*q);
    v = (uint32_t)(c >> 11) * a + (bg >> 11) * (255u - a);
    r = (v + (v >> 8) + 1u) >> 8;
    v = (uint32_t)((c >> 5) & 63u) * a + ((bg >> 5) & 63u) * (255u - a);
    g = (v + (v >> 8) + 1u) >> 8;
    v = (uint32_t)(c & 31u) * a + (bg & 31u) * (255u - a);
    b = (v + (v >> 8) + 1u) >> 8;
    *q = swap16((uint32_t)((r << 11) | (g << 5) | b));
}
static void gt_blit(int32_t x0, int32_t y0, int32_t w, int32_t h, const uint16_t *c, const uint8_t *a)
{
    int32_t i, j;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            gt_px(x0 + i, y0 + j, c[j * w + i], a[j * w + i]);
}
typedef struct { const gt_glyph_t *g; int32_t mid8; } gt_face_t;
static const gt_face_t GTF_TITLE = {gtf_title, GTF_TITLE_MID}, GTF_MID = {gtf_mid, GTF_MID_MID}, GTF_SIDE = {gtf_side, GTF_SIDE_MID},
                       GTF_KNOB3 = {gtf_knob3, GTF_KNOB3_MID}, GTF_KNOB4 = {gtf_knob4, GTF_KNOB4_MID}, GTF_KNOBS = {gtf_knobs, GTF_KNOBS_MID},
                       GTF_DIAL = {gtf_dial, GTF_DIAL_MID}, GTF_DIALS = {gtf_dials, GTF_DIALS_MID}, GTF_DIALV = {gtf_dialv, GTF_DIALV_MID}, GTF_DIALR = {gtf_dialr, GTF_DIALR_MID},
                       GTF_FOOT = {gtf_foot, GTF_FOOT_MID}, GTF_BAR = {gtf_bar, GTF_BAR_MID}, GTF_PNUM = {gtf_pnum, GTF_PNUM_MID}, GTF_ZVAL = {gtf_zval, GTF_ZVAL_MID};
static int32_t gt_tw8(const gt_face_t *f, const char *s)                      /* width in 1/8 px */
{
    int32_t w = 0;
    for (; *s; s++)
        if ((uint8_t)*s >= 32u && (uint8_t)*s < 128u && gtf_map[(uint8_t)*s - 32] != 255u)
            w += f->g[gtf_map[(uint8_t)*s - 32]].adv8;
    return w;
}
/* al: 0 = x is the left edge, 1 = the centre, 2 = the right edge; ymid = the middle line of the text (like the mockup's anchor m) */
static void gt_text(const gt_face_t *f, float x, float ymid, const char *s, uint16_t col, int al)
{
    int32_t pen = (int32_t)(x * 8.0f + 0.5f), base = (int32_t)(ymid + (float)f->mid8 / 8.0f + 0.5f);
    if (al == 1)
        pen -= gt_tw8(f, s) / 2;
    else if (al == 2)
        pen -= gt_tw8(f, s);
    for (; *s; s++) {
        const gt_glyph_t *g;
        int32_t i, j, gx, gy;
        if ((uint8_t)*s < 32u || (uint8_t)*s >= 128u || gtf_map[(uint8_t)*s - 32] == 255u)
            continue;
        g = &f->g[gtf_map[(uint8_t)*s - 32]];
        gx = ((pen + 4) >> 3) + g->ox;
        gy = base + g->oy;
        for (j = 0; j < g->h; j++)
            for (i = 0; i < g->w; i++)
                gt_px(gx + i, gy + j, col, gtf_atlas[g->off + j * g->w + i]);
        pen += g->adv8;
    }
}
static int32_t htw(const felucca_font_t *f, const char *s) { return grigio() ? (gt_tw8(&GTF_BAR, s) + 4) / 8 : tw(f, s); }
static int32_t htext(int32_t x, const felucca_font_t *f, const char *s, uint16_t col)
{
    if (!grigio())
        return cv_text(x, vc(f, 1, 17), f, s, col);
    gt_text(&GTF_BAR, (float)x, 9.6f, s, col, 0);
    return x + htw(f, s);
}
static void htext_r(int32_t x, const felucca_font_t *f, const char *s, uint16_t col)
{
    if (!grigio())
        text_r(x, vc(f, 1, 17), f, s, col);
    else
        gt_text(&GTF_BAR, (float)x, 9.6f, s, col, 2);
}
/* the footer hint: the (up to) three words joined by three spaces, centred, Poppins Medium; ALGORITHM-type words stay at the right */
static void gt_footer(int y, int dir, const char *a, const char *b, const char *c, uint16_t col)
{
    char ln[96], *q = ln;
    const char *p[3];
    const gt_face_t *f = &GTF_FOOT;
    int i, n = 0;
    float ym = (float)y + 4.5f;
    p[0] = a; p[1] = b; p[2] = c;
    col = RGB(0, 0, 0);                                                         /* wip595 (1.8b): the whole footer hint is BLACK in Grigiotonic, no blue words (was: tips blue, only the TURN ALGORITHM word black) */
    ln[0] = 0;
    if (dir > 0 && (c == ALGO_WORD || c == SC_WORD || c == CK_WORD)) {
        q = put_s(q, a ? a : "");
        if (b && *b) {
            q = put_s(q, "   ");
            q = put_s(q, b);
        }
        gt_text(f, 4.0f, ym, ln, col, 0);
        gt_text(f, 220.0f, ym, c, RGB(0, 0, 0), 2);
    } else {
        for (i = 0; i < 3; i++)
            if (p[i] && *p[i]) {
                if (n++)
                    q = put_s(q, "   ");
                q = put_s(q, p[i]);
            }
        if (gt_tw8(f, ln) > 8 * (dir ? 210 : 224))
            f = &GTF_BAR;                                                       /* too long: the smaller face (wip594: next to the big arrow it has 210 px) */
        gt_text(f, dir < 0 ? 132.0f : dir > 0 ? 116.0f : 120.0f, ym, ln, col, 1);
    }
    if (dir && !(dir < 0 && (dck_page() || chk_page())) && (ui.frame & 8u) == 0) {   /* wip569 (wip570: it BLINKS); wip594: DUCK / CHOKE have their own big one (gt_back_tri): the 2nd-page arrow in his skin, BLACK and big, a slow soft pulse */
        const int32_t w = 13, h = 20, x0 = dir > 0 ? 226 : 2, y0 = y - 9;   /* wip594: BIG (was 9 x 11), both ways */
        int32_t i, j;
        uint32_t lv = (uint32_t)(9 + (algo_level() - 3) * 7 / 13);
        for (j = 0; j < h; j++)
            for (i = 0; i < w; i++) {
                float fy = (float)j + 0.5f - (float)h * 0.5f, fx = (float)(dir > 0 ? i : w - 1 - i) + 0.5f;
                float edge = ((float)w - 0.5f) - fx - fm_fabsf(fy) * (((float)w - 0.5f) / ((float)h * 0.5f));
                float a = fm_clampf(edge * 0.7f + 0.5f, 0.0f, 1.0f) * fm_clampf(fx + 0.5f, 0.0f, 1.0f);
                px_blend(x0 + i, y0 + j, RGB(0, 0, 0), (uint32_t)(a * (float)lv + 0.5f));
            }
    }
}
static void gt_disc(float cx, float cy, float r, uint16_t c, uint32_t a16)
{
    int32_t x0 = (int32_t)fm_floorf(cx - r - 1.0f), x1 = (int32_t)(cx + r + 1.0f);
    int32_t y0 = (int32_t)fm_floorf(cy - r - 1.0f), y1 = (int32_t)(cy + r + 1.0f), x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy, d = fm_sqrtf(dx * dx + dy * dy);
            float a = fm_clampf(r - d + 0.5f, 0.0f, 1.0f);
            px_blend(x, y, c, (uint32_t)(a * (float)a16 + 0.5f));
        }
}
/* the sunken groove of a slider (224 x 26, at x, y) */
static void gt_groove(int32_t x, int32_t y)
{
    gt_blit(x - 4, y - 4, GTS_GROOVE_W, GTS_GROOVE_H, gts_groove_c, gts_groove_a);
}
/* the raised blue thumb with three ridges, 43 x 21, centred on cx, cy (its shadow is in the sprite) */
static void gt_thumb(int32_t cx, int32_t cy)
{
    gt_blit(cx - 29, cy - 18, GTS_THUMB_W, GTS_THUMB_H, gts_thumb_c, gts_thumb_a);
}
/* a standard horizontal slider: groove (y .. y+26), thin line, thumb at v (0..1); the three words under it */
static void gt_slider(int32_t y, float v, const char *l, const char *m, const char *r, int hot)
{
    int32_t cy = y + 13, tx = 29 + (int32_t)(fm_clampf(v, 0.0f, 1.0f) * 183.0f + 0.5f);
    float ly = (float)y + 34.5f;
    gt_groove(8, y);
    for (tx = 29; tx < 212; tx++) {                                             /* the thin line: 0.9 px wide on the line between two pixel rows */
        gt_px(tx, cy - 1, GT_INK, 118);
        gt_px(tx, cy, GT_INK, 118);
    }
    tx = 29 + (int32_t)(fm_clampf(v, 0.0f, 1.0f) * 183.0f + 0.5f);
    gt_thumb(tx, cy);
    gt_text(&GTF_SIDE, 9.0f, ly, l, GT_INK, 0);
    gt_text(&GTF_SIDE, 231.0f, ly, r, GT_INK, 2);
    gt_text(&GTF_MID, 120.0f, ly - 0.5f, m, hot ? GT_B1 : GT_INK, 1);
}
/* wip557: the GLOBAL page's slider: groove 20 high (y .. y+20), thumb 15 high, the three words 27 px under its top (mockup rows 35 / 73 / 111) */
static void gt_slider20(int32_t y, float v, const char *l, const char *m, const char *r, int hot)
{
    int32_t cy = y + 10, tx;
    float ly = (float)y + 27.0f;
    gt_blit(4, y - 4, GTS_GROOVE20_W, GTS_GROOVE20_H, gts_groove20_c, gts_groove20_a);
    for (tx = 29; tx < 212; tx++) {                                             /* the thin line, as on the other sliders */
        gt_px(tx, cy - 1, GT_INK, 118);
        gt_px(tx, cy, GT_INK, 118);
    }
    tx = 29 + (int32_t)(fm_clampf(v, 0.0f, 1.0f) * 183.0f + 0.5f);
    gt_blit(tx - 29, cy - 15, GTS_THUMB15_W, GTS_THUMB15_H, gts_thumb15_c, gts_thumb15_a);
    gt_text(&GTF_SIDE, 9.0f, ly, l, GT_INK, 0);
    gt_text(&GTF_SIDE, 231.0f, ly, r, GT_INK, 2);
    gt_text(&GTF_MID, 120.0f, ly - 0.5f, m, hot ? GT_B1 : GT_INK, 1);
}
/* a big knob (diameter 56): raised body, groove + blue arc from 6 o'clock clockwise (330 degrees, dark blue to light cyan), white thumb, inner button with the label */
static void gt_knob_b(int32_t cx, int32_t cy, const char *label, float v, int hot, int bip);
static void gt_knob(int32_t cx, int32_t cy, const char *label, float v, int hot) { gt_knob_b(cx, cy, label, v, hot, 0); }
/* bip = 1: a centred knob (the PAN): the blue runs from the middle (12 o'clock) to the value, left or right */
static void gt_knob_b(int32_t cx, int32_t cy, const char *label, float v, int hot, int bip)
{
    const float R = 28.0f, ra = 23.8f, aw = 4.0f, sweep = 330.0f / 360.0f, fcx = (float)cx + 0.5f, fcy = (float)cy + 0.5f;
    const float st = bip ? 105.0f / 360.0f : 0.25f;                              /* wip565: the pan knob's groove is SYMMETRIC about 12 o'clock (105 to 435 degrees) */
    int32_t x, y;
    float ang, tx, ty;
    float lo = 0.0f, hi;
    int centred = 0;
    v = fm_clampf(v, 0.0f, 1.0f);
    if (bip) {                                                                   /* 64 of 0..127 = the centre = 0.5: each half is stretched to half the sweep */
        const float vc = 64.0f / 127.0f;
        v = v <= vc ? v / vc * 0.5f : 0.5f + (v - vc) / (1.0f - vc) * 0.5f;
        centred = fm_fabsf(v - 0.5f) < 0.012f;
        if (centred)
            v = 0.5f;
        lo = v < 0.5f ? v : 0.5f;
        hi = v < 0.5f ? 0.5f : v;
    } else
        hi = v;
    (void)R;
    gt_blit(cx - 36, cy - 36, GTS_KNOB_W, GTS_KNOB_H, bip ? gts_knobp_c : gts_knob_c, bip ? gts_knobp_a : gts_knob_a);   /* body, shadows, the groove and the inner button */
    for (y = -28; y <= 28; y++)
        for (x = -28; x <= 28; x++) {
            float d = fm_sqrtf((float)(x * x + y * y)), a, t;
            a = fm_clampf(ra + aw * 0.5f + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - (ra - aw * 0.5f) + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns((float)y, (float)x) - st;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;
            if (centred) {                                                       /* wip565: centred = the WHOLE ring purple, both sides (lighter toward the centre) */
                px_blend(cx + x, cy + y, blend(GT_PURPLE, GT_PURPLE_L, (int)(16.0f * (1.0f - fm_fabsf(t - 0.5f) * 2.0f))), (uint32_t)(a * 16.0f + 0.5f));
                continue;
            }
            if (t > hi || t < lo)
                continue;
            px_blend(cx + x, cy + y, blend(GT_B1, GT_B2, (int)(16.0f * (hi > lo ? (t - lo) / (hi - lo) : 0.0f))), (uint32_t)(a * 16.0f + 0.5f));
        }
    if (centred) {
        float a0 = st * FM_TWO_PI, a1 = (st + sweep) * FM_TWO_PI;
        disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, GT_PURPLE);
        disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, GT_PURPLE);
    } else if (hi > lo) {
        float a0 = (st + sweep * lo) * FM_TWO_PI, a1 = (st + sweep * hi) * FM_TWO_PI;
        disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, GT_B1);   /* the round start */
        disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, GT_B2);   /* the round tip */
    }
    ang = (st + sweep * v) * FM_TWO_PI;
    tx = fcx + ra * fm_cosf(ang);
    ty = fcy + ra * fm_sinf(ang);
    for (x = 3; x >= 1; x--)                                                    /* the white thumb with its soft shadow */
        gt_disc(tx + 0.6f, ty + 1.0f, 3.6f + (float)x * 0.5f, GT_SH, 3);
    disc(tx, ty, 3.6f, RGB(250, 250, 250));
    gt_text((label[0] && label[1] && label[2] && label[3]) ? &GTF_KNOB4 : &GTF_KNOB3, fcx, fcy + 0.3f - 0.5f, label, hot ? GT_B1 : GT_INK, 1);
}
static float gt_rsdf(float px, float py, float x0, float y0, float x1, float y1, float r);
/* wip597 / wip598: the SIDECHAIN section of the SENDS page: a thin blue rounded outline that CLOSES round K3 (THRESHOLD) + K4 (INPUT), SIDECHAIN written over K3 and INPUT over K4, as big as the column allows */
static void gt_sc_section(void)
{
    const int32_t x0 = 118, y0 = 116, x1 = 238, y1 = 190;
    const float r = 9.0f;
    const gt_face_t *fs[3] = {&GTF_KNOB4, &GTF_FOOT, &GTF_BAR};
    const gt_face_t *f = fs[2];
    int32_t x, y, ya = y0 - 3, yb = y1 + 3, k;
    if (ya < -cv_oy)
        ya = -cv_oy;
    if (yb > -cv_oy + Q_H)
        yb = -cv_oy + Q_H;
    for (y = ya; y < yb; y++)
        for (x = x0 - 3; x < x1 + 3; x++) {
            float sd = gt_rsdf((float)x + 0.5f, (float)y + 0.5f, (float)x0, (float)y0, (float)x1, (float)y1, r), a;
            a = fm_clampf(1.4f - fm_fabsf(sd + 0.7f) + 0.5f, 0.0f, 1.0f);       /* a 2 px ring, anti-aliased, closed */
            if (a > 0.0f)
                px_blend(x, y, GT_B1, (uint32_t)(a * 16.0f + 0.5f));
        }
    for (k = 0; k < 3; k++)                                                     /* the biggest face in which SIDECHAIN fits its column (58 px) */
        if (gt_tw8(fs[k], "SIDECHAIN") <= 8 * 58) {
            f = fs[k];
            break;
        }
    gt_text(f, 150.0f, 125.0f, "SIDECHAIN", ui.touched == 2 ? GT_B1 : GT_INK, 1);
    gt_text(f, 209.0f, 125.0f, "INPUT", ui.touched == 3 ? GT_B1 : GT_INK, 1);
}
static void gt_line(float x0, float y0, float x1, float y1, float w, uint16_t col, uint32_t a16);
/* wip599: SENDS page symbol 1 = the REVERB CUBE (blue), FACING THE USER: a square front face with horizontal / vertical edges, the top and the right side going back (up and right).
 * A LIQUID inside with gravity: the blue rises from the BASE as a horizontal level (the front and the side fill from the bottom, the level shows as a light surface); empty at 0, full at 100 % */
static void gt_send_cube(float cx, float cy, float v)
{
    const float a = 38.0f, d = 14.0f;                                            /* the front square and the depth offset */
    const float x0 = cx - (a + d) * 0.5f, yt = cy - (a + d) * 0.5f, yf = yt + d, yb = yf + a;   /* front: x0 .. x0 + a, yf .. yb */
    const float H = fm_clampf(v, 0.0f, 1.0f) * a;                                /* the liquid height up from the base */
    int32_t x, y, ya = (int32_t)fm_floorf(yt - 2.0f), yz = (int32_t)(yb + 3.0f);
    if (ya < -cv_oy)
        ya = -cv_oy;
    if (yz > -cv_oy + Q_H)
        yz = -cv_oy + Q_H;
    for (y = ya; y < yz; y++)
        for (x = (int32_t)fm_floorf(x0 - 2.0f); x < (int32_t)(x0 + a + d + 3.0f); x++) {
            int nin = 0, nf = 0, nr = 0, ns = 0, sx, sy;
            for (sy = 0; sy < 3; sy++)                                            /* 3 x 3 samples per pixel */
                for (sx = 0; sx < 3; sx++) {
                    float qx = (float)x + ((float)sx + 0.5f) / 3.0f, qy = (float)y + ((float)sy + 0.5f) / 3.0f;
                    float u = qx - (x0 + a), t = (yf - qy) / d;                  /* u: into the side face; t: into the top face (0 front .. 1 back) */
                    int inF = qx >= x0 && qx <= x0 + a && qy >= yf && qy <= yb;
                    int inR = u > 0.0f && u <= d && qy >= yf - u && qy <= yb - u;
                    int inT = t > 0.0f && t <= 1.0f && qx >= x0 + t * d && qx <= x0 + a + t * d;
                    float ts;
                    if (!inF && !inR && !inT)
                        continue;
                    nin++;
                    if (H < 0.4f)
                        continue;
                    ts = ((yb - H) - qy) / d;                                     /* the liquid's surface: a parallelogram at the level */
                    if (ts >= 0.0f && ts <= 1.0f && qx >= x0 + ts * d && qx <= x0 + a + ts * d) {
                        ns++;
                        continue;
                    }
                    if (inT)
                        continue;                                                 /* nothing of the liquid above its level */
                    if (inF && qy >= yb - H)
                        nf++;
                    else if (inR && qy >= yb - u - H)
                        nr++;
                }
            if (nin)
                px_blend(x, y, GT_B1, (uint32_t)((nin * 2 + 4) / 9));            /* the empty cube: a faint blue body */
            if (nf)
                px_blend(x, y, GT_B1, (uint32_t)((nf * 16 + 4) / 9));
            if (nr)
                px_blend(x, y, RGB(16, 100, 168), (uint32_t)((nr * 16 + 4) / 9));
            if (ns)
                px_blend(x, y, GT_B2, (uint32_t)((ns * 16 + 4) / 9));
        }
    gt_line(x0, yf, x0 + a, yf, 2.0f, GT_B1, 16);                                /* the front square */
    gt_line(x0 + a, yf, x0 + a, yb, 2.0f, GT_B1, 16);
    gt_line(x0 + a, yb, x0, yb, 2.0f, GT_B1, 16);
    gt_line(x0, yb, x0, yf, 2.0f, GT_B1, 16);
    gt_line(x0, yf, x0 + d, yt, 2.0f, GT_B1, 16);                                /* the top and the side going back */
    gt_line(x0 + d, yt, x0 + a + d, yt, 2.0f, GT_B1, 16);
    gt_line(x0 + a + d, yt, x0 + a + d, yb - d, 2.0f, GT_B1, 16);
    gt_line(x0 + a + d, yb - d, x0 + a, yb, 2.0f, GT_B1, 16);
    gt_line(x0 + a, yf, x0 + a + d, yt, 2.0f, GT_B1, 16);
}
/* wip599: SENDS page symbol 2 = the DELAY CIRCLE (blue): the outer circle is always there; at 0 nothing inside, then rings appear one by one from the small one in the middle outwards (4 rings) */
static void gt_send_rings(float cx, float cy, float v)
{
    const float R = 29.0f;
    float ak[4];
    int32_t x, y, ya = (int32_t)fm_floorf(cy - R - 2.0f), yb = (int32_t)(cy + R + 3.0f);
    int k;
    for (k = 0; k < 4; k++)
        ak[k] = fm_clampf((fm_clampf(v, 0.0f, 1.0f) * 4.0f - (float)k) * 2.0f, 0.0f, 1.0f);   /* each ring fades in over half a step */
    if (ya < -cv_oy)
        ya = -cv_oy;
    if (yb > -cv_oy + Q_H)
        yb = -cv_oy + Q_H;
    for (y = ya; y < yb; y++)
        for (x = (int32_t)fm_floorf(cx - R - 2.0f); x < (int32_t)(cx + R + 3.0f); x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy, d = fm_sqrtf(dx * dx + dy * dy);
            float a = fm_clampf(1.5f - fm_fabsf(d - R), 0.0f, 1.0f);              /* the outer ring, 2 px */
            for (k = 0; k < 4; k++)
                if (ak[k] > 0.0f) {
                    float b = fm_clampf(1.5f - fm_fabsf(d - (5.0f + 6.0f * (float)k)), 0.0f, 1.0f) * ak[k];
                    if (b > a)
                        a = b;
                }
            if (a > 0.0f)
                px_blend(x, y, GT_B1, (uint32_t)(a * 16.0f + 0.5f));
        }
}
/* the MIX page (FX 1st press): title, the two sliders (they follow knobs 1 and 2: reference only), the four big knobs */
static void gt_page_mix(const char *title, const char *const *LAB)
{
    int i, pgi = cur_page();
    gt_text(&GTF_TITLE, 120.0f, 8.0f, title, GT_INK, 1);
    for (i = 0; i < 2; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        const char *wl, *wc, *wr;
        char b1[24], b2[24], b3[24];                  /* caps_of() answers in ONE shared buffer: copy each word */
        if (!d)
            continue;
        if (ui.view == V_MIX2 && pgi == 1) {          /* wip599: SENDS: no fader, the blue symbols: the cube (reverb) and the circle (delay), the word under each */
            float v = d->max ? (float)pref_shown(r) / (float)d->max : 0.0f;
            if (i == 0)
                gt_send_cube(60.0f, 56.0f, v);
            else
                gt_send_rings(180.0f, 56.0f, v);
            gt_text(&GTF_MID, i ? 180.0f : 60.0f, 98.0f, i ? "DELAY" : "REVERB", ui.touched == i ? GT_B1 : GT_INK, 1);
            if (extra_off(r)) {                       /* CLASSIC: a shade over the symbol */
                int32_t xx, yy;
                for (yy = 18; yy < 108; yy++)
                    for (xx = i ? 122 : 4; xx < (i ? 236 : 118); xx++)
                        px_blend(xx, yy, C_BG, 10u);
            }
            continue;
        }
        slider_words(r, &wl, &wc, &wr);
        put_s(b1, caps_of(wl));
        put_s(b2, caps_of(wc));
        put_s(b3, caps_of(wr));
        gt_slider(i ? 72 : 18, d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, b1, b2, b3, ui.touched == i);
        if (extra_off(r)) {                           /* wip559: CLASSIC: a shade over the slider (reverb / delay send) */
            int32_t xx, yy;
            for (yy = 0; yy < 34; yy++)
                for (xx = 4; xx < 236; xx++)
                    px_blend(xx, (i ? 72 : 18) - 4 + yy, C_BG, 10u);
        }
    }
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        char nb[16], un[8];
        const char *lab = LAB[i];
        if (!d)
            continue;
        if (r.kind == R_SCIN) {                       /* wip559: SENDS K3 = the sidechain input track, its NUMBER is the label */
            pref_value_of(r, pref_shown(r), nb, un);
            lab = nb;
        }
        gt_knob_b(31 + i * 59 + (i + 1) / 3, 159, lab, d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i, is_pan(r));
        if (extra_off(r))                             /* wip559: CLASSIC: the knob greyed with a shade */
            gt_disc((float)(31 + i * 59 + (i + 1) / 3) + 0.5f, 159.5f, 31.0f, C_BG, 10);
    }
    if (ui.view == V_MIX2 && pgi == 1)                /* wip559: the sidechain dot (red pulsing = the selected track is ducked, purple = it is the input) between K3 and K4 */
        sc_dot(230, 123, 4, ui.sel[0]);
    if (ui.view == V_MIX2 && pgi == 1 && tonic_part())  /* wip597 */
        gt_sc_section();
}

/* wip557: the GLOBAL page (HOME 3rd press): title, three sliders (QUANTIZATION, SWING, FILL RATE: they follow knobs 1 to 3, reference only),
 * four big knobs: QNT, SWG, then the fill rate (2X 4X 8X) and the tempo as numbers in the knob */
static void gt_page_glob(void)
{
    static const char *const LAB[2] = {"QNT", "SWG"};
    int i, pgi = cur_page();
    char lb[2][8];
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "GLOBAL", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        const char *wl, *wc, *wr;
        char b1[24], b2[24], b3[24];
        if (!d)
            continue;
        slider_words(r, &wl, &wc, &wr);
        put_s(b1, caps_of(wl));
        put_s(b2, caps_of(wc));
        put_s(b3, caps_of(wr));
        gt_slider20(15 + i * 38, d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, b1, b2, b3, ui.touched == i);
    }
    {   /* the two numbers on the knobs: the fill rate (4X) and the tempo (125) */
        pref_t r2 = pg.r[pgi][2], r3 = pg.r[pgi][3];
        char num[16], unit[8];
        pref_value_of(r2, pref_shown(r2), num, unit);
        put_s(lb[0], caps_of(num));
        put_i(lb[1], pref_shown(r3) + 20);
    }
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        if (d)
            gt_knob(31 + i * 59 + (i + 1) / 3, 159, i < 2 ? LAB[i] : lb[i - 2], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
}

/* wip561: the RNDM button (the purple ring + purple inner button, white RNDM): the knob body sprite, then the ring and the inner disc by code; lit = 1 */
static void gt_rndm(int32_t cx, int32_t cy, int lit, int purple)   /* wip562: purple = 0 BLUE (voice random, GLO velocity), 1 PURPLE (RANDOM ALL, SEL: future) */
{
    const float ra = 23.8f, aw = 4.0f, rin = 19.1f, fcx = (float)cx + 0.5f, fcy = (float)cy + 0.5f;
    int32_t x, y;
    gt_blit(cx - 36, cy - 36, GTS_KNOB_W, GTS_KNOB_H, gts_knob_c, gts_knob_a);
    for (y = -28; y <= 28; y++)
        for (x = -28; x <= 28; x++) {
            float d = fm_sqrtf((float)(x * x + y * y)), a, g;
            a = fm_clampf(ra + aw * 0.5f + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - (ra - aw * 0.5f) + 0.5f, 0.0f, 1.0f);
            if (a > 0.0f)
                px_blend(cx + x, cy + y, purple ? GT_PURPLE : GT_B1, (uint32_t)(a * 16.0f + 0.5f));
            a = fm_clampf(rin - d + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            g = (float)(y + 19) / 38.0f;                                          /* light purple at the top, darker below */
            g = fm_clampf(g, 0.0f, 1.0f);
            px_blend(cx + x, cy + y, purple ? RGB((int)(190.0f - 48.0f * g), (int)(136.0f - 52.0f * g), (int)(238.0f - 32.0f * g))
                                        : RGB((int)(120.0f - 70.0f * g), (int)(205.0f - 60.0f * g), (int)(248.0f - 28.0f * g)), (uint32_t)(a * (lit ? 16.0f : 9.0f) + 0.5f));
        }
    gt_text(&GTF_KNOB4, fcx, fcy + 0.3f - 0.5f, "RNDM", RGB(255, 255, 255), 1);
}
/* wip561: the VELOCITY page (GLO 1st press): title, three sliders (OSC, NOISE, PITCH MOD: they follow knobs 1 to 3), knobs OSC NSE MOD from the left,
 * the 4th place = the RNDM button (FUNC + K4 randomizes the sound, as before) */
static void gt_page_vel(void)
{
    static const char *const LAB[3] = {"OSC", "NSE", "MOD"};
    int i, pgi = cur_page();
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "VELOCITY", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        const char *wl, *wc, *wr;
        char b1[24], b2[24], b3[24];
        if (!d)
            continue;
        slider_words(r, &wl, &wc, &wr);
        put_s(b1, caps_of(wl));
        put_s(b2, caps_of(wc));
        put_s(b3, caps_of(wr));
        gt_slider20(15 + i * 38, d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, b1, b2, b3, ui.touched == i);
        gt_knob(31 + i * 59 + (i + 1) / 3, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
    gt_rndm(31 + 3 * 59 + 1, 159, (int32_t)(rnd_until - plat_ms()) <= 0 || (ui.frame & 4u), 0);
}

/* wip552: an anti-aliased polyline (width w): the coverage of each pixel = the best of its segments, ONE blend per pixel (small icons only: 36 x 30 px at most) */
static void gt_poly(const float *p, int n, float w, uint16_t col)
{
    uint8_t cov[36 * 30];
    float hw = w * 0.5f, minx = p[0], maxx = p[0], miny = p[1], maxy = p[1];
    int32_t i, x, y, ox, oy, bw, bh;
    for (i = 1; i < n; i++) {
        minx = p[2 * i] < minx ? p[2 * i] : minx;
        maxx = p[2 * i] > maxx ? p[2 * i] : maxx;
        miny = p[2 * i + 1] < miny ? p[2 * i + 1] : miny;
        maxy = p[2 * i + 1] > maxy ? p[2 * i + 1] : maxy;
    }
    ox = (int32_t)fm_floorf(minx - hw - 1.0f);
    oy = (int32_t)fm_floorf(miny - hw - 1.0f);
    bw = (int32_t)(maxx + hw + 1.0f) - ox + 1;
    bh = (int32_t)(maxy + hw + 1.0f) - oy + 1;
    if (bw > 36 || bh > 30)
        return;
    for (i = 0; i < bw * bh; i++)
        cov[i] = 0;
    for (i = 0; i + 1 < n; i++) {
        float ax = p[2 * i], ay = p[2 * i + 1], bx = p[2 * i + 2], by = p[2 * i + 3], dx = bx - ax, dy = by - ay, l2 = dx * dx + dy * dy;
        int32_t x0 = (int32_t)fm_floorf((ax < bx ? ax : bx) - hw - 1.0f), x1 = (int32_t)((ax > bx ? ax : bx) + hw + 1.0f);
        int32_t y0 = (int32_t)fm_floorf((ay < by ? ay : by) - hw - 1.0f), y1 = (int32_t)((ay > by ? ay : by) + hw + 1.0f);
        for (y = y0; y <= y1; y++)
            for (x = x0; x <= x1; x++) {
                float qx = (float)x + 0.5f - ax, qy = (float)y + 0.5f - ay, t = l2 > 0.0f ? (qx * dx + qy * dy) / l2 : 0.0f, ex, ey, a;
                int32_t k = (y - oy) * bw + (x - ox);
                uint8_t v;
                t = fm_clampf(t, 0.0f, 1.0f);
                ex = qx - t * dx;
                ey = qy - t * dy;
                a = fm_clampf(hw - fm_sqrtf(ex * ex + ey * ey) + 0.5f, 0.0f, 1.0f);
                v = (uint8_t)(a * 255.0f + 0.5f);
                if (k >= 0 && k < bw * bh && v > cov[k])
                    cov[k] = v;
            }
    }
    for (y = 0; y < bh; y++)
        for (x = 0; x < bw; x++)
            gt_px(ox + x, oy + y, col, cov[y * bw + x]);
}
/* a round mode button (radius 25, pre-rendered: raised light disc, or the lit one with the blue glow), centred on the pixel cx, cy */
static void gt_btn(int32_t cx, int32_t cy, int on)
{
    if (on)
        gt_blit(cx - 32, cy - 32, GTS_BTNON_W, GTS_BTNON_H, gts_btnon_c, gts_btnon_a);
    else
        gt_blit(cx - 32, cy - 32, GTS_BTNOFF_W, GTS_BTNOFF_H, gts_btnoff_c, gts_btnoff_a);
}
/* the three waveform pictures of the OSC buttons (sine, triangle, saw), the mockup's numbers, centre cx, cy */
static void gt_wave_icon(float cx, float cy, int kind)
{
    float p[2 * 25];
    int i, n = 4;
    const float w = 12.6f, h = 10.7f;
    if (kind == 0) {
        n = 25;
        for (i = 0; i < n; i++) {
            p[2 * i] = cx + w * ((float)i / 24.0f * 2.0f - 1.0f);
            p[2 * i + 1] = cy - h * fm_sinf((float)i / 24.0f * FM_TWO_PI);
        }
    } else if (kind == 1) {
        p[0] = cx - w;        p[1] = cy;
        p[2] = cx - w * 0.5f; p[3] = cy - h;
        p[4] = cx + w * 0.5f; p[5] = cy + h;
        p[6] = cx + w;        p[7] = cy;
    } else {
        p[0] = cx - w * 0.9f; p[1] = cy - 0.3f * h;
        p[2] = cx - w * 0.9f; p[3] = cy - h;
        p[4] = cx + w * 0.9f; p[5] = cy + h;
        p[6] = cx + w * 0.9f; p[7] = cy + 0.05f * h;
    }
    gt_poly(p, n, 1.5f, RGB(36, 38, 40));
}
/* the OSC page (ENV button): title, the three wave buttons (the lit one = the wave), WAVEFORM, the OSC FREQ slider (follows knob 2), the four big knobs */
static void gt_page_osc(void)
{
    static const char *const LAB[4] = {"WAV", "OSC", "ATK", "DCY"};
    int i, pgi = cur_page(), wv = 0;
    pref_t wr = pg.r[pgi][0], sr = pg.r[pgi][1];
    const x0x_param_t *wd = pref_desc(wr), *sd = pref_desc(sr);
    if (wd)
        wv = pref_shown(wr);
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "OSCILLATOR", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        gt_btn(48 + i * 72, 40, i == wv);
        gt_wave_icon(48.5f + (float)(i * 72), 40.5f, i);
    }
    gt_text(&GTF_MID, 120.0f, 73.5f, "WAVEFORM", ui.touched == 0 ? GT_B1 : GT_INK, 1);
    if (sd) {
        const char *wl, *wc, *wrr;
        char b1[24], b2[24], b3[24];                  /* caps_of() answers in ONE shared buffer: copy each word */
        slider_words(sr, &wl, &wc, &wrr);
        put_s(b1, caps_of(wl));
        put_s(b2, caps_of(wc));
        put_s(b3, caps_of(wrr));
        gt_slider(82, sd->max ? (float)pref_shown(sr) / (float)sd->max : 0.0f, b1, b2, b3, ui.touched == 1);
    }
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        if (d)
            gt_knob(31 + i * 59 + (i + 1) / 3, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
}

/* the three pictures of the PITCH MOD buttons (exponential decay, sine, random), the mockup's numbers, centre cx, cy */
static void gt_mod_icon(float cx, float cy, int kind)
{
    static const float RND[17] = {0.1f, -0.9f, 0.7f, -0.4f, 0.95f, -0.8f, 0.3f, -1.0f, 0.8f, -0.2f, 0.6f, -0.9f, 0.4f, -0.5f, 0.9f, -0.3f, 0.2f};
    float p[2 * 49];
    int i, n;
    if (kind == 0) {
        n = 31;
        for (i = 0; i < n; i++) {
            float t = (float)i / 30.0f;
            p[2 * i] = cx + 12.5f * (t * 2.0f - 1.0f);
            p[2 * i + 1] = cy - 10.5f + 21.0f * (1.0f - fm_expf(-5.0f * t)) / (1.0f - fm_expf(-5.0f));
        }
    } else if (kind == 1) {
        n = 49;
        for (i = 0; i < n; i++) {
            float t = (float)i / 48.0f;
            p[2 * i] = cx + 11.5f * (t * 2.0f - 1.0f);
            p[2 * i + 1] = cy - 11.5f * fm_sinf(t * 2.0f * FM_TWO_PI);
        }
    } else {
        n = 17;
        for (i = 0; i < n; i++) {
            p[2 * i] = cx + 12.5f * ((float)i / 16.0f * 2.0f - 1.0f);
            p[2 * i + 1] = cy - 10.5f * RND[i];
        }
    }
    gt_poly(p, n, 1.5f, RGB(36, 38, 40));
}
/* the PITCH MOD page (SEL button): title, three mode buttons (the lit one = the mod mode), then WAV AMT RATE from the left (no 4th knob yet) */
static void gt_page_pmod(void)
{
    static const char *const LAB[3] = {"WAV", "AMT", "RATE"};
    int i, pgi = cur_page(), wv = 0;
    pref_t wr = pg.r[pgi][0];
    if (pref_desc(wr))
        wv = pref_shown(wr);
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "PITCH MOD", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        gt_btn(48 + i * 72, 60, i == wv);
        gt_mod_icon(48.5f + (float)(i * 72), 60.5f, i);
    }
    for (i = 0; i < 3; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        if (d)
            gt_knob(31 + i * 59 + (i + 1) / 3, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
    gt_rndm(31 + 3 * 59 + 1, 159, (int32_t)(rnd_all_until - plat_ms()) <= 0 || (ui.frame & 4u), 1);   /* wip610: RANDOM ALL = the purple button */
}

/* the three pictures of the NOISE FILTER buttons (low pass, band pass, high pass), the mockup's numbers, centre cx, cy */
static void gt_filt_icon(float cx, float cy, int kind)
{
    float p[2 * 41];
    int i;
    const float w = 12.6f, h = 10.4f;
    for (i = 0; i < 41; i++) {
        float t = (float)i / 40.0f, y, u;
        if (kind == 0) {
            u = fm_clampf((t - 0.3f) / 0.45f, 0.0f, 1.0f);
            y = cy - h * (1.0f - 2.0f * u * u * (3.0f - 2.0f * u));
        } else if (kind == 1) {
            u = (t - 0.5f) / 0.2f;
            y = cy + h - 2.0f * h * fm_expf(-u * u);
        } else {
            u = fm_clampf((t - 0.25f) / 0.45f, 0.0f, 1.0f);
            y = cy + h * (1.0f - 2.0f * u * u * (3.0f - 2.0f * u));
        }
        p[2 * i] = cx + w * (t * 2.0f - 1.0f);
        p[2 * i + 1] = y;
    }
    gt_poly(p, 41, 1.5f, RGB(36, 38, 40));
}
/* the NOISE FILTER page (LFO button): title, three filter buttons (lit = the mode), FILTER MODE, the FILTER FREQ slider (follows K2), knobs MODE FREQ FLTQ from the left */
static void gt_page_noise(void)
{
    static const char *const LAB[3] = {"MODE", "FREQ", "FLTQ"};
    int i, pgi = cur_page(), wv = 0;
    pref_t wr = pg.r[pgi][0], sr = pg.r[pgi][1];
    const x0x_param_t *sd = pref_desc(sr);
    if (pref_desc(wr))
        wv = pref_shown(wr);
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "NOISE FILTER", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        gt_btn(48 + i * 72, 40, i == wv);
        gt_filt_icon(48.5f + (float)(i * 72), 40.5f, i);
    }
    gt_text(&GTF_MID, 120.0f, 73.5f, "FILTER MODE", ui.touched == 0 ? GT_B1 : GT_INK, 1);
    if (sd) {
        const char *wl, *wc, *wrr;
        char b1[24], b2[24], b3[24];                  /* caps_of() answers in ONE shared buffer: copy each word */
        slider_words(sr, &wl, &wc, &wrr);
        put_s(b1, caps_of(wl));
        put_s(b2, caps_of(wc));
        put_s(b3, caps_of(wrr));
        gt_slider(82, sd->max ? (float)pref_shown(sr) / (float)sd->max : 0.0f, b1, b2, b3, ui.touched == 1);
    }
    for (i = 0; i < 3; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        if (d)
            gt_knob(31 + i * 59 + (i + 1) / 3, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
}

/* the three pictures of the NOISE ENVELOPE buttons (EXP spike + curve, LIN triangle, CLAP burst + curve), the mockup's numbers, centre cx, cy */
static void gt_env_icon(float cx, float cy, int kind)
{
    float p[2 * 60];
    int i, n = 0;
    const float w = 11.2f, h = 11.0f, L = cx - w, T = cy - h, B = cy + h, wd = 2.0f * w;
    if (kind == 1) {
        p[0] = L;                    p[1] = B;
        p[2] = L + 0.34f * wd;       p[3] = T;
        p[4] = cx + w;               p[5] = B;
        n = 3;
    } else {
        float x = L, x0, ww, k;
        if (kind == 0) {
            p[0] = L;                p[1] = B;
            x0 = L + 0.20f * wd;
            p[2] = x0;               p[3] = T;
            n = 2;
            ww = wd * 0.80f;
            k = 4.2f;
        } else {
            int j;
            p[0] = x;                p[1] = B;
            n = 1;
            for (j = 0; j < 4; j++) {
                p[2 * n] = x + 1.6f;  p[2 * n + 1] = T + 1.2f;  n++;
                p[2 * n] = x + 3.2f;  p[2 * n + 1] = B - 0.5f * h;  n++;
                x += 3.2f;
            }
            p[2 * n] = x + 1.6f;     p[2 * n + 1] = T;  n++;
            x0 = x + 1.6f;
            ww = wd - (x0 - L);
            k = 4.0f;
        }
        for (i = 1; i <= 24; i++) {
            float t = (float)i / 24.0f;
            p[2 * n] = x0 + ww * t;
            p[2 * n + 1] = B - (B - T) * fm_expf(-k * t);
            n++;
        }
    }
    gt_poly(p, n, 1.5f, RGB(36, 38, 40));
}
/* his CHOKE icon (two walls, a slanted stroke, a dot): k = size, centre cx, cy */
static void gt_choke_icon(float cx, float cy, float k)
{
    float p[4];
    const uint16_t ink = RGB(36, 38, 40);
    p[0] = cx - 9.6f * k; p[1] = cy - 8.4f * k; p[2] = cx - 9.6f * k; p[3] = cy + 8.4f * k;
    gt_poly(p, 2, 1.3f * k, ink);
    p[0] = cx + 9.6f * k; p[2] = cx + 9.6f * k;
    gt_poly(p, 2, 1.3f * k, ink);
    p[0] = cx - 6.6f * k; p[1] = cy - 5.6f * k; p[2] = cx + 6.6f * k; p[3] = cy + 5.6f * k;
    gt_poly(p, 2, 1.5f * k, ink);
    gt_disc(cx, cy, 2.6f * k, ink, 255);
}
/* the NOISE ENVELOPE page (EDIT button): title, three envelope buttons (lit = the mode), ENVELOPE MODE, the CHOKE button (blue = this track is in the group), knobs MODE (CHOKE icon) ATK DCY */
static void gt_page_nenv(void)
{
    static const char *const LAB[4] = {"MODE", "", "ATK", "DCY"};
    int i, pgi = cur_page(), wv = 0, in = (seq.choke >> ui.sel[0]) & 1;
    pref_t wr = pg.r[pgi][0];
    if (pref_desc(wr))
        wv = pref_shown(wr);
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "NOISE ENVELOPE", GT_INK, 1);
    for (i = 0; i < 3; i++) {
        gt_btn(48 + i * 72, 40, i == wv);
        gt_env_icon(48.5f + (float)(i * 72), 40.5f, i);
    }
    gt_text(&GTF_MID, 120.0f, 73.5f, "ENVELOPE MODE", ui.touched == 0 ? GT_B1 : GT_INK, 1);
    if (in)
        gt_blit(120 - 24, 98 - 24, GTS_CHKON_W, GTS_CHKON_H, gts_chkon_c, gts_chkon_a);
    else
        gt_blit(120 - 24, 98 - 24, GTS_CHKOFF_W, GTS_CHKOFF_H, gts_chkoff_c, gts_chkoff_a);
    gt_choke_icon(120.5f, 98.5f, 1.0f);
    gt_text(&GTF_SIDE, 120.0f, 122.0f, "CHOKE", ui.touched == 1 ? GT_B1 : GT_INK, 1);
    for (i = 0; i < 4; i++) {
        pref_t r = pg.r[pgi][i];
        const x0x_param_t *d = pref_desc(r);
        int32_t kx = 31 + i * 59 + (i + 1) / 3;
        if (i == 1) {                                   /* K2 = the CHOKE switch: the icon in the knob, the arc full when in */
            gt_knob(kx, 159, "", in ? 1.0f : 0.0f, 0);
            gt_choke_icon((float)kx + 0.5f, 159.7f, 1.12f);
        } else if (d)
            gt_knob(kx, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
}


/* wip566: the COMPRESSOR (GLO 3rd press) in his drawing: the old gain-reduction dial (0 to 20 dB, 15 and 20 purple, purple needle, 4:1) on an inset panel, the four
 * knobs ATK REL THRS GAIN. The panel and the dial are drawn by code (no new sprite). */
static void gt_line(float x0, float y0, float x1, float y1, float w, uint16_t col, uint32_t a16)
{
    float hw = w * 0.5f, dx = x1 - x0, dy = y1 - y0, l2 = dx * dx + dy * dy;
    int32_t xa = (int32_t)fm_floorf((x0 < x1 ? x0 : x1) - hw - 1.0f), xb = (int32_t)((x0 > x1 ? x0 : x1) + hw + 1.0f);
    int32_t ya = (int32_t)fm_floorf((y0 < y1 ? y0 : y1) - hw - 1.0f), yb = (int32_t)((y0 > y1 ? y0 : y1) + hw + 1.0f), x, y;
    for (y = ya; y <= yb; y++)
        for (x = xa; x <= xb; x++) {
            float qx = (float)x + 0.5f - x0, qy = (float)y + 0.5f - y0, t = l2 > 0.0f ? (qx * dx + qy * dy) / l2 : 0.0f, ex, ey, a;
            t = fm_clampf(t, 0.0f, 1.0f);
            ex = qx - t * dx;
            ey = qy - t * dy;
            a = fm_clampf(hw - fm_sqrtf(ex * ex + ey * ey) + 0.5f, 0.0f, 1.0f);
            if (a > 0.0f)
                px_blend(x, y, col, (uint32_t)(a * (float)a16 + 0.5f));
        }
}
/* an inset panel (the slider groove's look: flat grey, a dark edge top left, a light edge bottom right), x0..x1 / y0..y1, corner radius r */
static float gt_rsdf(float px, float py, float x0, float y0, float x1, float y1, float r)
{
    float hx = (x1 - x0) * 0.5f - r, hy = (y1 - y0) * 0.5f - r, qx = fm_fabsf(px - (x0 + x1) * 0.5f) - hx, qy = fm_fabsf(py - (y0 + y1) * 0.5f) - hy;
    float mx = qx > 0.0f ? qx : 0.0f, my = qy > 0.0f ? qy : 0.0f, m = qx > qy ? qx : qy;
    return fm_sqrtf(mx * mx + my * my) + (m < 0.0f ? m : 0.0f) - r;
}
static void gt_inset_panel(int32_t x0, int32_t y0, int32_t x1, int32_t y1, float r)
{
    const float dd = 1.7f, bl = 1.9f;
    int32_t x, y;
    cv_rect(x0 + 8, y0 + 8, x1 - x0 - 16, y1 - y0 - 16, RGB(189, 191, 190));
    for (y = y0; y < y1; y++)
        for (x = x0; x < x1; x++) {
            float fx = (float)x + 0.5f, fy = (float)y + 0.5f, sd, dk, lt, rr, gg, bb;
            if (x >= x0 + 8 && x < x1 - 8 && y >= y0 + 8 && y < y1 - 8)
                continue;
            sd = gt_rsdf(fx, fy, (float)x0, (float)y0, (float)x1, (float)y1, r);
            if (sd > 0.5f)
                continue;
            dk = fm_clampf(0.5f + gt_rsdf(fx - dd, fy - dd, (float)x0, (float)y0, (float)x1, (float)y1, r) / (2.0f * bl * 0.8f), 0.0f, 1.0f) * 0.75f;
            lt = fm_clampf(0.5f + gt_rsdf(fx + dd, fy + dd, (float)x0, (float)y0, (float)x1, (float)y1, r) / (2.0f * bl * 0.8f), 0.0f, 1.0f) * 0.9f;
            rr = 189.0f; gg = 191.0f; bb = 190.0f;
            rr += (146.0f - rr) * dk; gg += (148.0f - gg) * dk; bb += (147.0f - bb) * dk;
            rr += (233.0f - rr) * lt; gg += (234.0f - gg) * lt; bb += (233.0f - bb) * lt;
            px_blend(x, y, RGB((int)rr, (int)gg, (int)bb), (uint32_t)(fm_clampf(0.5f - sd, 0.0f, 1.0f) * 16.0f + 0.5f));
        }
}
/* wip589: the touched knob grows 25 % (as the GLO mixer's); wip588: no big zoom on the compressor page (the value is always under the knob). wip587: the small knob of the compressor page (the old skin's size): grey body drawn by code, the blue trace from the start of the groove to the value, the white thumb */
static void gt_knob_s(int32_t cx, int32_t cy, float v, int grow)
{
    const float gs = grow ? 1.25f : 1.0f, R = 15.0f * gs, ra = 12.4f * gs, aw = 3.0f * gs, sweep = 330.0f / 360.0f, st = 0.25f, fcx = (float)cx + 0.5f, fcy = (float)cy + 0.5f;
    int32_t x, y;
    float ang, tx, ty;
    v = fm_clampf(v, 0.0f, 1.0f);
    gt_disc(fcx + 0.5f, fcy + 1.0f, R + 1.0f, GT_SH, 3);
    disc(fcx, fcy, R, GT_DARK);
    disc(fcx, fcy, R - 0.8f, GT_FILL);
    disc(fcx, fcy, ra - aw * 0.5f - 1.2f, RGB(214, 215, 214));
    for (y = -21; y <= 21; y++)
        for (x = -21; x <= 21; x++) {
            float d = fm_sqrtf((float)(x * x + y * y)), a, t;
            a = fm_clampf(ra + aw * 0.5f + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - (ra - aw * 0.5f) + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns((float)y, (float)x) - st;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;
            if (t > v)
                continue;
            px_blend(cx + x, cy + y, blend(GT_B1, GT_B2, (int)(16.0f * (v > 0.0f ? t / v : 0.0f))), (uint32_t)(a * 16.0f + 0.5f));
        }
    if (v > 0.0f) {
        float a0 = (st) * FM_TWO_PI, a1 = (st + sweep * v) * FM_TWO_PI;
        disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, GT_B1);
        disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, GT_B2);
    }
    ang = (st + sweep * v) * FM_TWO_PI;
    tx = fcx + ra * fm_cosf(ang);
    ty = fcy + ra * fm_sinf(ang);
    for (x = 2; x >= 1; x--)
        gt_disc(tx + 0.5f, ty + 0.8f, (2.4f + (float)x * 0.4f) * gs, GT_SH, 3);
    disc(tx, ty, 2.4f * gs, RGB(250, 250, 250));
}
/* wip587 / wip590: one knob in the old skin's way, in the Grigiotonic look: the small grey knob (it grows 25 % while turned), the NAME under it, the VALUE and its unit under the name.
 * cy / ny / vy = the page rows of the knob centre, the name and the value (the compressor page and the REVERB / DELAY pages use it) */
static void gt_knob_cell(int i, int32_t cy, int32_t ny, int32_t vy)
{
    int pgi = cur_page(), touched = ui.touched == i;
    pref_t r = pg.r[pgi][i];
    const x0x_param_t *d = pref_desc(r);
    char nb[16], un[8], nm[16];
    const char *np;
    const felucca_font_t *vf = touched ? &FONT_M : &FONT_B;
    int32_t cx = 30 + i * 60, w, x, vt;
    if (!d)
        return;
    gt_knob_s(cx, touched ? cy - 2 : cy, d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, touched);   /* the grown knob moves up 2 px: it keeps clear of the name */
    put_s(nm, caps_of(pref_name(r)));
    np = nm;
    if (ui.view == V_FX && (np[0] == 'R' || np[0] == 'D') && np[1] == (np[0] == 'R' ? 'E' : 'L') && (np[2] == 'V' || np[2] == 'Y') && np[3] == ' ')
        np += 4;                                                                /* "REV LOW CUT" / "DLY RETURN" on REVERB 2 / DELAY 2: the title says which */
    text_c(cx, vc(&FONT_XS, ny - 4, 9), &FONT_XS, np, C_BLACK);                 /* wip591: the old skin's fonts, in black (the skin's greys hide small text); the turned knob's value is the bigger font */
    pref_value_of(r, pref_shown(r), nb, un);
    w = tw(vf, nb) + (un[0] ? tw(&FONT_XS, un) + 1 : 0);
    if (w > 58) {
        vf = &FONT_B;
        w = tw(vf, nb) + (un[0] ? tw(&FONT_XS, un) + 1 : 0);
    }
    x = cx - w / 2;
    vt = vc(vf, vy - 7, 14);
    x = cv_text(x, vt, vf, nb, C_BLACK);
    if (un[0])
        cv_text(x + 1, base_y(vf, vt, &FONT_XS), &FONT_XS, un, C_BLACK);
}
static void gt_page_comp(void)
{
    static float needle;
    static uint32_t last = 0xFFFFFFFFu;
    const float CX = 120.0f, CY = 121.0f, RR = 82.0f, A0 = 148.0f, A1 = 32.0f;
    int i, k;
    float gr = engine_gr_db(), ang;
    char t[24];
    if (ui.frame != last) {                                                     /* the needle rises at once and falls slowly (as the old dial) */
        needle = gr > needle ? gr : needle * 0.88f;
        if (needle < 0.05f)
            needle = 0.0f;
        last = ui.frame;
    }
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "COMPRESSOR", GT_INK, 1);
    gt_inset_panel(8, 15, 232, 128, 9.0f);
    for (k = 0; k <= 20; k++) {
        int major = k % 5 == 0;
        float a = (A0 + (A1 - A0) * (float)k / 20.0f) * FM_PI / 180.0f, c = fm_cosf(a), sn = fm_sinf(a), r0 = major ? RR - 9.0f : RR - 5.0f;
        uint16_t col = k >= 15 ? GT_PURPLE : RGB(52, 54, 56);
        gt_line(CX + r0 * c, CY - r0 * sn, CX + RR * c, CY - RR * sn, major ? 1.5f : 0.8f, col, 16);
        if (major) {
            put_i(t, k);
            gt_text(&GTF_DIAL, CX + (RR + 8.5f) * c, CY - (RR + 8.5f) * sn, t, k >= 15 ? GT_PURPLE : GT_INK, 1);
        }
    }
    ang = (A0 + (A1 - A0) * fm_clampf(needle / 20.0f, 0.0f, 1.0f)) * FM_PI / 180.0f;
    gt_line(CX + 0.6f, CY + 1.0f, CX + (RR - 6.0f) * fm_cosf(ang) + 0.6f, CY - (RR - 6.0f) * fm_sinf(ang) + 1.0f, 2.2f, GT_SH, 7);   /* its soft shadow */
    gt_line(CX, CY, CX + (RR - 6.0f) * fm_cosf(ang), CY - (RR - 6.0f) * fm_sinf(ang), 1.7f, GT_PURPLE, 16);
    gt_disc(CX + 0.6f, CY + 1.0f, 7.0f, GT_SH, 4);                              /* the hub */
    gt_disc(CX, CY, 6.0f, RGB(206, 207, 206), 16);
    gt_disc(CX, CY, 2.6f, RGB(110, 112, 114), 16);
    gt_text(&GTF_DIALS, 16.0f, 23.0f, "GAIN REDUCTION DB", RGB(110, 112, 114), 0);
    put_s(put_i(t, (int)(needle + 0.5f)), " DB");
    gt_text(&GTF_DIALV, 16.0f, 118.0f, t, GT_INK, 0);
    gt_text(&GTF_DIALR, 224.0f, 118.0f, "4:1", RGB(130, 132, 134), 2);
    for (i = 0; i < 4; i++)
        gt_knob_cell(i, 158, 180, 191);
}

/* wip564: the GLO MIXER (GLO 2nd press) in his drawing: 8 tall faders (BLUE = the live volume of the track, WHITE dot = its level), under each a small PAN knob
 * (the track number inside; pan at the centre = purple on both sides of the thumb, at a side = a blue trace from the centre), a blue line = the group the four
 * hardware knobs act on, the sidechain dots under it. The arc starts at 6 o'clock, so the pan centre is drawn at 12 o'clock (as the HOME pan knob, wip562). */
static void gt_fader(int32_t x0, int32_t y0, float level, float meter, int muted)
{
    const int32_t yt = y0 + 8, yb = y0 + 114 - 8;
    const float cx = (float)x0 + 11.0f, gw = 3.5f;
    int32_t x, y, top;
    float ty;
    level = fm_clampf(level, 0.0f, 1.0f);
    meter = fm_clampf(meter, 0.0f, 1.0f);
    gt_blit(x0 - 4, y0 - 4, GTS_FADER_W, GTS_FADER_H, gts_fader_c, gts_fader_a);
    top = (int32_t)((float)yb - (float)(yb - yt) * meter);
    if (!muted && meter > 0.004f)
        for (y = top - 4; y <= yb + 4; y++) {                                   /* the blue capsule: dark at the bottom, light cyan at the tip */
            float yy = (float)y + 0.5f, dy = yy < (float)top ? (float)top - yy : yy > (float)yb ? yy - (float)yb : 0.0f, t = ((float)yb + gw - yy) / ((float)(yb - top) + 2.0f * gw);
            for (x = x0 + 6; x < x0 + 16; x++) {
                float dx = fm_fabsf((float)x + 0.5f - cx), d = fm_sqrtf(dx * dx + dy * dy), a = fm_clampf(gw - d + 0.5f, 0.0f, 1.0f);
                if (a <= 0.0f)
                    continue;
                t = fm_clampf(t, 0.0f, 1.0f);
                px_blend(x, y, blend(GT_B1, GT_B2, (int)(16.0f * fm_sqrtf(t) * 0.9f + 0.5f)), (uint32_t)(a * 16.0f + 0.5f));
            }
        }
    ty = (float)yb - (float)(yb - yt) * level;
    {
        int k;
        for (k = 3; k >= 1; k--)
            gt_disc(cx + 0.7f, ty + 1.2f, 4.4f + (float)k * 0.4f, GT_SH, 3);
        disc(cx, ty, 4.4f, muted ? RGB(205, 205, 205) : RGB(250, 250, 250));
    }
}
/* the small pan knob: v = the pan 0..1 (64 / 127 = the centre); wip565: symmetric groove (105 to 435 degrees), centred = the whole ring purple */
static void gt_pan_small(int32_t cx, int32_t cy, const char *label, float v, int hot, int grow)
{
    const float gs = grow ? 1.25f : 1.0f, k = 27.0f / 56.0f * gs, R = 13.5f * gs, ra = R - 4.2f * k, aw = 4.0f * k, sweep = 330.0f / 360.0f, st = 105.0f / 360.0f, fcx = (float)cx, fcy = (float)cy;
    const float vc = 64.0f / 127.0f;
    int32_t x, y;
    int centred;
    float lo, hi, ang, tx, ty;
    v = fm_clampf(v, 0.0f, 1.0f);
    v = v <= vc ? v / vc * 0.5f : 0.5f + (v - vc) / (1.0f - vc) * 0.5f;
    centred = fm_fabsf(v - 0.5f) < 0.012f;
    if (centred)
        v = 0.5f;
    lo = v < 0.5f ? v : 0.5f;
    hi = v < 0.5f ? 0.5f : v;
    if (grow) {                                                                 /* wip567: the touched knob grows 25 % (as the old skin), body by code */
        gt_disc(fcx + 0.4f, fcy + 0.8f, R + 1.0f, GT_SH, 3);
        disc(fcx, fcy, R, GT_DARK);
        disc(fcx, fcy, R - 0.8f, GT_FILL);
    } else
        gt_blit(cx - GTS_SKNOBP_W / 2, cy - GTS_SKNOBP_H / 2, GTS_SKNOBP_W, GTS_SKNOBP_H, gts_sknobp_c, gts_sknobp_a);
    if (centred)                                                                /* wip567: centred = the WHOLE knob RED (wip572) (body and ring) */
        gt_disc(fcx, fcy, R - 0.5f, GT_RED, 9);
    {
        int32_t m = grow ? 21 : 15;
    for (y = cy - m; y < cy + m; y++)
        for (x = cx - m; x < cx + m; x++) {
            float dx = (float)x + 0.5f - fcx, dy = (float)y + 0.5f - fcy, d = fm_sqrtf(dx * dx + dy * dy), a, t;
            a = fm_clampf(ra + aw * 0.5f + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - (ra - aw * 0.5f) + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns(dy, dx) - st;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;
            if (centred) {                                                      /* the WHOLE ring purple, both sides (lighter toward the centre) */
                px_blend(x, y, blend(GT_RED, GT_RED_L, (int)(16.0f * (1.0f - fm_fabsf(t - 0.5f) * 2.0f))), (uint32_t)(a * 16.0f + 0.5f));
                continue;
            }
            if (t > hi || t < lo)
                continue;
            px_blend(x, y, blend(GT_B1, GT_B2, (int)(16.0f * (hi > lo ? (v > 0.5f ? (t - lo) : (hi - t)) / (hi - lo) : 0.0f))), (uint32_t)(a * 16.0f + 0.5f));
        }
    }
    if (centred) {
        float a0 = st * FM_TWO_PI, a1 = (st + sweep) * FM_TWO_PI;
        disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, GT_RED);
        disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, GT_RED);
    } else if (hi > lo) {
        float a0 = (st + sweep * lo) * FM_TWO_PI, a1 = (st + sweep * hi) * FM_TWO_PI;
        disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, v > 0.5f ? GT_B1 : GT_B2);
        disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, v > 0.5f ? GT_B2 : GT_B1);
    }
    ang = (st + sweep * v) * FM_TWO_PI;
    tx = fcx + ra * fm_cosf(ang);
    ty = fcy + ra * fm_sinf(ang);
    gt_disc(tx + 0.3f, ty + 0.5f, 1.74f + 0.5f, GT_SH, 3);
    disc(tx, ty, 1.74f, RGB(250, 250, 250));
    gt_text(&GTF_KNOBS, fcx, fcy + 0.15f - 0.5f, label, hot ? GT_B1 : GT_INK, 1);
}
/* wip568: the PERFORM page in his skin: the pattern number (two digits fit) in a raised circle like a knob, the knobs REV DLY HPF and the ALL FX ON / OFF selector.
 * The circle: R 48, a blue ring (purple and blinking while the next pattern is queued: its number sits in a small purple disc), the number in Poppins Medium 48 (PNUM: a 2 digit number is 62 x 37 px, its corner is 36 px from the middle, the ring's inside is 40) */
static void gt_page_perf(void)
{
    static const char *const LAB[3] = {"REV", "DLY", "HPF"};
    int i, on;
    int32_t x0 = 179, x1 = 237, y0 = 145, y1 = 173, tx;
    draw_patinfo(134);                                                          /* wip625: the same pattern information as the normal screen (the purple PATTERN n box blinking when one is to come, the number big under it); only the knobs and the colours are the skin's */
    for (i = 0; i < 3; i++) {
        pref_t r = pg.r[0][i];
        const x0x_param_t *d = pref_desc(r);
        if (d)
            gt_knob(31 + i * 59 + (i + 1) / 3, 159, LAB[i], d->max ? (float)pref_shown(r) / (float)d->max : 0.0f, ui.touched == i);
    }
    on = pref_get(pg.r[0][3]) != 0;                                             /* ALL FX: a selector, the thumb right = ON (blue), left = OFF */
    gt_inset_panel(x0, y0, x1, y1, 14.0f);
    tx = on ? x1 - 14 : x0 + 14;
    if (ui.touched == 3)
        gt_disc((float)tx, 159.0f, 13.5f, GT_B1, 6);
    gt_disc((float)tx + 0.4f, 160.0f, 12.0f, GT_SH, 3);
    gt_disc((float)tx, 159.0f, 11.0f, on ? GT_B1 : RGB(250, 250, 250), 16);
    if (on)
        gt_disc((float)tx, 158.2f, 8.0f, GT_B2, 9);
    gt_text(&GTF_KNOB4, on ? (float)(x0 + 8 + tx - 11) * 0.5f : (float)(tx + 11 + x1 - 8) * 0.5f, 159.0f - 0.2f, on ? "ON" : "OFF", on ? GT_B1 : RGB(104, 106, 105), 1);
}
/* wip570: the hint line of the skin's pages, BIG (Poppins 14.6, blue) in the strip under the knobs (page rows 190 to 218), and the 2nd-page arrow: BLACK, 22 x 30, BLINKING, at the right edge
 * (a page to the right: the word ALGORITHM is in the hint) or the left edge (the way back). Long hints: the smaller face, then two lines. Drawn with cv_oy set by the caller (the strip crosses two canvases). */
static void gt_big_hint(void)
{
    const char *a, *b, *c, *p[3];
    char l1[96], l2[96], *q1 = l1, *q2 = l2;
    const float xl = 6.0f, xr = 234.0f;
    uint16_t col = footer_text(&a, &b, &c);
    int dir = algo_page_dir(), i, n = 0;
    float x0 = dir < 0 ? 30.0f : xl, x1 = dir > 0 ? 210.0f : xr, cx = (x0 + x1) * 0.5f;
    int32_t avail8 = (int32_t)((x1 - x0) * 8.0f);
    const gt_face_t *f = &GTF_KNOB3;
    p[0] = a; p[1] = b; p[2] = c;
    col = RGB(0, 0, 0);                                                         /* wip595 (1.8b): the whole hint is BLACK in Grigiotonic, no blue words */
    l1[0] = 0;
    l2[0] = 0;
    if (dir > 0 && (c == ALGO_WORD || c == SC_WORD || c == CK_WORD)) {          /* wip594: the tips in our blue (first line), the TURN ALGORITHM word in the arrow's black (last line) */
        const gt_face_t *fw = &GTF_KNOB3;
        for (i = 0; i < 2; i++)
            if (p[i] && *p[i]) {
                if (n++)
                    q1 = put_s(q1, "   ");
                q1 = put_s(q1, p[i]);
            }
        if (gt_tw8(fw, c) > avail8)
            fw = &GTF_KNOB4;
        if (l1[0]) {
            const gt_face_t *ft = &GTF_KNOB4;
            if (gt_tw8(ft, l1) > avail8)
                ft = &GTF_BAR;
            gt_text(ft, cx, 197.5f, l1, col, 1);
            gt_text(fw, cx, 211.5f, c, RGB(0, 0, 0), 1);
        } else
            gt_text(fw, cx, 204.0f, c, RGB(0, 0, 0), 1);
    } else {
    n = 0;
    for (i = 0; i < 3; i++)
        if (p[i] && *p[i]) {
            if (n++)
                q1 = put_s(q1, "   ");
            q1 = put_s(q1, p[i]);
        }
    if (gt_tw8(f, l1) > avail8)
        f = &GTF_KNOB4;
    if (gt_tw8(f, l1) <= avail8) {
        gt_text(f, cx, 204.0f, l1, col, 1);
    } else {                                                                    /* two lines: the first part, then the rest */
        n = 0;
        for (i = 1; i < 3; i++)
            if (p[i] && *p[i]) {
                if (n++)
                    q2 = put_s(q2, "   ");
                q2 = put_s(q2, p[i]);
            }
        (void)q2;
        put_s(l1, p[0] ? p[0] : "");
        if (gt_tw8(&GTF_KNOB4, l2) > avail8 || gt_tw8(&GTF_KNOB4, l1) > avail8)
            f = &GTF_BAR;
        else
            f = &GTF_KNOB4;
        gt_text(f, cx, 197.5f, l1, col, 1);
        gt_text(f, cx, 211.5f, l2, col, 1);
    }
    }
    if (dir && (ui.frame & 8u) == 0) {                                          /* wip573: twice as fast */
        const int32_t w = 22, h = 30, tx0 = dir > 0 ? 216 : 2, ty0 = 189;
        int32_t ii, jj;
        for (jj = 0; jj < h; jj++)
            for (ii = 0; ii < w; ii++) {
                float fy = (float)jj + 0.5f - (float)h * 0.5f, fx = (float)(dir > 0 ? ii : w - 1 - ii) + 0.5f;
                float edge = ((float)w - 0.5f) - fx - fm_fabsf(fy) * (((float)w - 0.5f) / ((float)h * 0.5f));
                float al = fm_clampf(edge * 0.7f + 0.5f, 0.0f, 1.0f) * fm_clampf(fx + 0.5f, 0.0f, 1.0f);
                px_blend(tx0 + ii, ty0 + jj, RGB(0, 0, 0), (uint32_t)(al * 16.0f + 0.5f));
            }
    }
}
/* wip578 / wip579: the group the four hardware knobs act on = a BLUE OUTLINE (4 px, rounded) round its four faders (level pages) or its four pan knobs (pan pages) */
static void gt_group_glow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const float r = 10.0f;
    int32_t x, y, ya = y0 - 3, yb = y1 + 3;
    if (ya < -cv_oy)
        ya = -cv_oy;                                                            /* only the rows of the canvas being drawn */
    if (yb > -cv_oy + Q_H)
        yb = -cv_oy + Q_H;
    for (y = ya; y < yb; y++)
        for (x = x0 - 3; x < x1 + 3; x++) {
            float sd = gt_rsdf((float)x + 0.5f, (float)y + 0.5f, (float)x0, (float)y0, (float)x1, (float)y1, r), a;
            a = fm_clampf(2.0f - fm_fabsf(sd + 2.0f) + 0.5f, 0.0f, 1.0f);       /* wip579: a ring 4 px wide, anti-aliased */
            if (a > 0.0f)
                px_blend(x, y, GT_B1, (uint32_t)(a * 16.0f + 0.5f));
        }
}
static void gt_page_gmix(int k)
{
    int v, pgi = cur_page(), grp = pgi & 1, panmode = pgi >> 1, tv = ui.touched >= 0 && ui.touched < 4 ? grp * 4 + ui.touched : -1;
    int32_t xs[8];
    char tnb[2] = {0, 0};
    int tpv = 64;
    gt_text(&GTF_TITLE, 120.0f, 8.0f, "GLOBAL MIXER", GT_INK, 1);
    gt_group_glow(7 + (int32_t)((float)(grp * 4) * 29.2f + 0.5f) + 11 - 17, panmode ? 134 : 13, 7 + (int32_t)((float)(grp * 4 + 3) * 29.2f + 0.5f) + 11 + 17, panmode ? 175 : 134);
    for (v = 0; v < NDRUM_TONIC; v++) {
        pref_t lv = find_ref(T_909, v, "Level"), pn = find_ref(T_909, v, "Pan");
        int l = lv.kind ? proj.sound.v[T_909][v][lv.c] : 0, pv = pn.kind ? proj.sound.v[T_909][v][pn.c] : 64;
        int muted = (int)(((seq.mute | seq.vmute) >> (k * NDRUM + v)) & 1u);
        float lvl = (float)l / 127.0f, mt = lvl * (float)eng_hit[k][v] / 255.0f;
        char nb[4];
        if (seq.vsolo && !((seq.vsolo >> (k * NDRUM + v)) & 1u))
            muted = 1;                                                          /* wip607: a track the solo silences is grey, as a muted one */
        xs[v] = 7 + (int32_t)(v * 29.2f + 0.5f);                                /* the pill's left edge (his drawing: centres 17.8 + 29.2 v) */
        gt_fader(xs[v], 16, lvl, mt, muted);
        nb[0] = (char)('1' + v);
        nb[1] = 0;
        if (!(v == tv && panmode))
            gt_pan_small(xs[v] + 11, 152, nb, (float)pv / 127.0f, 0, 0);
        else {
            tnb[0] = nb[0];
            tnb[1] = 0;
            tpv = pv;
        }
        sc_dot(xs[v] + 11, 180, 4, v);
    }
    if (tnb[0])                                                                 /* wip567: the touched pan knob last (it grows over its neighbours) */
        gt_pan_small(xs[tv] + 11, 152, tnb, (float)tpv / 127.0f, 1, 1);
}

/* wip576: the KNOB ZOOM in his skin (GLO settings ZOOM KNOB = YES, as the old skin): a turned knob on the skin's pages shows a BIG knob with its name and value over the whole
 * area (the same moments as the old zoom: not for the sliders (a later piece), not on PERFORM / SONG, not for the K1 icon cells (a later piece)). */
/* wip577: the knobs of the skin's pages that have a SLIDER moving together with them (the old skin's slider cells, plus the VELOCITY sliders): their big readout is a RECTANGLE
 * with the value over the knobs' place, never a zoom. (GLOBAL K4 = the tempo has no slider: it zooms.) */
static int gt_slider_cell(int i)
{
    if (!grigio() || i < 0 || i > 3)
        return 0;
    return (ui.view == V_MIX1 && i < 2) || (ui.view == V_OSC && i == 1) || (ui.view == V_NOISE && i == 1) || (ui.view == V_GLOB && i < 3) || (ui.view == V_VEL && i < 3);
}
static int gt_slbox_on(void)
{
    pref_t r;
    int t = ui.touched;
    if (!gt_big_page() || ui.overlay != O_NONE || ui.help || gt_perf() || !gt_slider_cell(t))
        return 0;
    r = pg.r[cur_page()][t];
    return r.kind != R_NONE && pref_desc(r) != 0;
}
static int gt_zoom_on(void)
{
    pref_t r;
    int t = ui.touched;
    if (!grigio() || !gt_big_page() || proj.set.zoom_off || gt_perf() || ui.view == V_SONG || fx_comp_page() || ui.overlay != O_NONE || ui.help)
        return 0;
    if (t < 0 || t > 3 || gt_slider_cell(t))
        return 0;                                                               /* wip577: a knob with a slider moving together = the slider's rectangle (gt_slider_box), not a zoom */
    if (ui.view == V_MIX2 && cur_page() == 1 && t < 2)
        return 0;                                                               /* wip599: SENDS: REV and DLY have their symbols (cube, circle): no zoom */
    if (t == 0 && (ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NEV || ui.view == V_NOISE))
        return 0;                                                               /* K1 = an icon cell */
    if (ui.view == V_NEV && t == 1)
        return 0;                                                               /* the CHOKE switch */
    if ((ui.view == V_VEL || ui.view == V_PMOD) && t == 3)
        return 0;                                                               /* RANDOM SOUND / wip610 RANDOM ALL */
    r = pg.r[cur_page()][t];
    return r.kind != R_NONE && r.kind != R_ACT && pref_desc(r) != 0;
}
/* a big knob (R 56): the sprite look by code: soft shadow, light body, the groove with the blue value arc, the inner button, the white thumb. bip = a centred knob (pan). */
static void gt_knob_big(int32_t cx, int32_t cy, float v, int bip)
{
    const float R = 56.0f, ra = 47.6f, aw = 8.0f, sweep = 330.0f / 360.0f, st = bip ? 105.0f / 360.0f : 0.25f, fcx = (float)cx + 0.5f, fcy = (float)cy + 0.5f;
    float lo = 0.0f, hi, ang, tx, ty;
    int32_t x, y;
    int centred = 0, k;
    v = fm_clampf(v, 0.0f, 1.0f);
    if (bip) {
        const float vc = 64.0f / 127.0f;
        v = v <= vc ? v / vc * 0.5f : 0.5f + (v - vc) / (1.0f - vc) * 0.5f;
        centred = fm_fabsf(v - 0.5f) < 0.012f;
        if (centred)
            v = 0.5f;
        lo = v < 0.5f ? v : 0.5f;
        hi = v < 0.5f ? 0.5f : v;
    } else
        hi = v;
    for (k = 4; k >= 1; k--)
        gt_disc(fcx + 1.0f, fcy + 2.4f, R + (float)k * 1.3f, GT_SH, 3);          /* the soft shadow */
    gt_disc(fcx, fcy, R + 0.6f, GT_DARK, 16);
    gt_disc(fcx - 0.6f, fcy - 0.8f, R - 0.4f, GT_LIGHT, 16);
    gt_disc(fcx + 0.4f, fcy + 0.6f, R - 1.6f, GT_FILL, 16);
    for (y = -56; y <= 56; y++)
        for (x = -56; x <= 56; x++) {
            float d = fm_sqrtf((float)(x * x + y * y)), a, t;
            a = fm_clampf(ra + aw * 0.5f + 0.5f - d, 0.0f, 1.0f) * fm_clampf(d - (ra - aw * 0.5f) + 0.5f, 0.0f, 1.0f);
            if (a <= 0.0f)
                continue;
            t = turns((float)y, (float)x) - st;
            if (t < 0.0f)
                t += 1.0f;
            t /= sweep;
            if (t > 1.0f)
                continue;
            px_blend(cx + x, cy + y, RGB(168, 170, 169), (uint32_t)(a * 16.0f + 0.5f));      /* the empty groove */
            if (centred) {
                px_blend(cx + x, cy + y, blend(GT_RED, GT_RED_L, (int)(16.0f * (1.0f - fm_fabsf(t - 0.5f) * 2.0f))), (uint32_t)(a * 16.0f + 0.5f));
                continue;
            }
            if (t > hi || t < lo)
                continue;
            px_blend(cx + x, cy + y, blend(GT_B1, GT_B2, (int)(16.0f * (hi > lo ? (t - lo) / (hi - lo) : 0.0f))), (uint32_t)(a * 16.0f + 0.5f));
        }
    if (hi > lo && !centred) {
        float a0 = (st + sweep * lo) * FM_TWO_PI, a1 = (st + sweep * hi) * FM_TWO_PI;
        gt_disc(fcx + ra * fm_cosf(a0), fcy + ra * fm_sinf(a0), aw * 0.5f, GT_B1, 16);
        gt_disc(fcx + ra * fm_cosf(a1), fcy + ra * fm_sinf(a1), aw * 0.5f, GT_B2, 16);
    }
    for (k = 3; k >= 1; k--)
        gt_disc(fcx + 0.8f, fcy + 1.6f, 30.0f + (float)k * 1.2f, GT_SH, 3);         /* the inner button */
    gt_disc(fcx, fcy, 30.0f, GT_DARK, 16);
    gt_disc(fcx - 0.4f, fcy - 0.6f, 29.2f, GT_LIGHT, 16);
    gt_disc(fcx + 0.3f, fcy + 0.5f, 28.0f, RGB(214, 215, 214), 16);
    ang = (st + sweep * v) * FM_TWO_PI;
    tx = fcx + ra * fm_cosf(ang);
    ty = fcy + ra * fm_sinf(ang);
    for (k = 3; k >= 1; k--)
        gt_disc(tx + 1.0f, ty + 1.8f, 7.0f + (float)k * 1.0f, GT_SH, 3);
    gt_disc(tx, ty, 7.0f, RGB(250, 250, 250), 16);
}
static void gt_page_zoom(void)
{
    char num[16], unit[8], ub[8], nmb[28];
    pref_t r;
    const char *nm;
    const gt_face_t *nf = &GTF_TITLE, *vf = &GTF_ZVAL;
    int dim;
    int32_t uw = 0, avail8;
    put_s(nmb, readout_name(&r));                                               /* (caps_of / readout_name share two static buffers: copy the name first) */
    nm = nmb;
    if (r.kind == R_NONE)
        return;
    pref_value_of(r, pref_shown(r), num, unit);
    put_s(num, caps_of(num));
    put_s(ub, caps_of(unit));
    if (gt_tw8(nf, nm) > 8 * 96)
        nf = &GTF_BAR;
    dim = extra_off(r);
    gt_knob_big(68, 104, pref_frac(r), is_pan(r));
    if (dim)
        gt_disc(68.5f, 104.5f, 64.0f, C_BG, 10);
    if (ub[0])
        uw = gt_tw8(&GTF_KNOB3, ub) / 8 + 4;
    avail8 = (232 - 134 - uw) * 8;
    if (gt_tw8(vf, num) > avail8)
        vf = &GTF_KNOB3;                                                        /* too wide for the big face (a long name): the next size */
    if (gt_tw8(vf, num) > avail8)
        vf = &GTF_BAR;
    gt_text(nf, 186.0f, 72.0f, nm, dim ? GT_SH : RGB(90, 92, 94), 1);
    gt_text(vf, 232.0f - (float)uw, 106.0f, num, dim ? GT_SH : GT_INK, 2);
    if (ub[0])
        gt_text(&GTF_KNOB3, 232.0f, 108.0f, ub, RGB(110, 112, 114), 2);
}

/* wip577: the slider's big rectangle (as the old skin's draw_slider_box): over the knobs' place an inset panel with the NAME (grey) and the VALUE (big, readable);
 * OSC/NOISE MIX shows both halves (50.00/50.00). The sliders above it stay. */
static void gt_slider_box(void)
{
    char t1[24], t2[32], nb[32], vb[32], ub[8];
    pref_t r = pg.r[cur_page()][ui.touched];
    const gt_face_t *nf = &GTF_TITLE, *vf = &GTF_ZVAL;
    int32_t avail8 = 218 * 8, uw = 0;
    int dim = extra_off(r), mix = is_mixsl(r);
    slider_readout_text(r, pref_shown(r), t1, t2);                              /* (a held step shows its own locked value) */
    put_s(nb, t1);
    if (mix) {
        put_s(vb, t2);
        ub[0] = 0;
    } else {
        char num[16], unit[8];
        const char *wl, *wc, *wr;
        slider_words(r, &wl, &wc, &wr);                                         /* the name = the word the slider shows on the page (OSC FREQ, FILTER FREQ, QUANTIZATION ...) */
        put_s(nb, caps_of(wc));
        pref_value_of(r, pref_shown(r), num, unit);
        put_s(vb, caps_of(num));
        put_s(ub, caps_of(unit));
    }
    cv_rect(0, 124, 240, 66, C_BG);
    gt_inset_panel(6, 126, 234, 188, 9.0f);
    if (gt_tw8(nf, nb) > avail8)
        nf = &GTF_KNOB3;
    if (gt_tw8(nf, nb) > avail8)
        nf = &GTF_BAR;
    if (ub[0])
        uw = gt_tw8(&GTF_KNOB3, ub) / 8 + 4;
    avail8 = (218 - uw) * 8;
    if (gt_tw8(vf, vb) > avail8)
        vf = &GTF_KNOB3;
    if (gt_tw8(vf, vb) > avail8)
        vf = &GTF_BAR;
    gt_text(nf, 120.0f, 139.5f, nb, dim ? GT_SH : RGB(90, 92, 94), 1);
    if (ub[0]) {
        float tw8 = (float)gt_tw8(vf, vb) / 8.0f, x0 = 120.0f - ((float)uw + tw8) * 0.5f;
        gt_text(vf, x0, 166.0f, vb, dim ? GT_SH : GT_INK, 0);
        gt_text(&GTF_KNOB3, x0 + tw8 + 4.0f, 168.0f, ub, RGB(110, 112, 114), 0);
    } else
        gt_text(vf, 120.0f, 166.0f, vb, dim ? GT_SH : GT_INK, 1);
}

static void draw_quarters(void)
{
    int row, c, pgi = cur_page();
    if (mixer_tall()) {                               /* wip224: the 8 sliders, tall; the zoom is not drawn here */
        for (row = 0; row < 2; row++) {
            cv_begin(240, Q_H, C_BG);
            if (grigio()) {                           /* wip564: GRIGIOTONIC: the GLO mixer in his drawing */
                cv_oy = -row * Q_H;
                gt_page_gmix(drum_mix_page());
                cv_oy = 0;
            } else
                draw_mix_tall(row * Q_H, drum_mix_page());
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
        }
        cv_begin(240, 12, C_BG);
        draw_footer_y(1);                             /* wip371: the blue hint under the mixer */
        cv_commit(2, 0, MAIN_Y + 2 * Q_H);
        return;
    }
    if (chk_page() || dck_page()) {                   /* wip328: the CHOKE screen takes the whole area, the footer under it (wip350: and the DUCK screen) */
        for (row = 0; row < 2; row++) {
            cv_begin(240, Q_H, C_BG);
            if (dck_page())
                draw_dck(row * Q_H);
            else
                draw_chk(row * Q_H);
            gt_back_tri(row * Q_H);                   /* wip594: the big way-back triangle (the skin) */
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
        }
        cv_begin(240, 12, C_BG);
        draw_footer_y(1);
        cv_commit(2, 0, MAIN_Y + 2 * Q_H);
        return;
    }
    if (ui.view == V_GLO && ui.overlay == O_MEM) {    /* wip274: MEMORY: the bucket over the whole area, the footer under it */
        for (row = 0; row < 2; row++) {
            cv_begin(240, Q_H, C_BG);
            draw_mem(row * Q_H);
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
        }
        cv_begin(240, 12, C_BG);
        draw_footer_y(1);
        cv_commit(2, 0, MAIN_Y + 2 * Q_H);
        return;
    }
    if (ui.seqpat && ui.view == V_SEQ) {              /* SEQ, 2nd press: only the number of the pattern, big (the next one small, when cued) */
        for (row = 0; row < 2; row++) {
            cv_begin(240, Q_H, C_BG);
            cv_oy = -row * Q_H;
            draw_patinfo(2 * Q_H);
            cv_oy = 0;
            if (row == 1) {                               /* wip111: K1 = SONG ON / OFF, bottom left corner (ON = purple, OFF = grey) */
                const char *ct = seq.chain_on ? "SONG ON" : "SONG OFF";
                int32_t cw = tw(&FONT_B, ct) + 16;
                rbox(4, Q_H - 26, cw, 22, 8, seq.chain_on ? C_TBOTH : C_LINE);
                cv_text(12, vc(&FONT_B, Q_H - 26, 22), &FONT_B, ct, seq.chain_on ? C_WHITE : C_GRAY);
            }
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
        }
        cv_begin(240, 12, C_BG);                       /* wip112: the footer of this page (it was left over from the last page before) */
        draw_footer_y(1);
        cv_commit(2, 0, MAIN_Y + 2 * Q_H);
        return;
    }
    for (row = 0; row < 2; row++) {
        cv_begin(240, Q_H, C_BG);
        if (gt_zoom_on()) {                                                     /* wip576: the knob zoom in the skin */
            cv_oy = -row * Q_H;
            gt_page_zoom();
            cv_oy = 0;
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
            continue;
        }
        if (gt_big_page()) {   /* wip545 / wip552 / wip553 / wip554 / wip555: GRIGIOTONIC: the MIX and OSC pages in their own drawing, the whole area, no zoom rectangle */
            cv_oy = -row * Q_H;
            if (fx_comp_page())
                gt_page_comp();
            else if (gt_perf())
                gt_page_perf();
            else if (ui.view == V_VEL)
                gt_page_vel();
            else if (ui.view == V_GLOB)
                gt_page_glob();
            else if (ui.view == V_OSC)
                gt_page_osc();
            else if (ui.view == V_PMOD)
                gt_page_pmod();
            else if (ui.view == V_NOISE)
                gt_page_noise();
            else if (ui.view == V_NEV)
                gt_page_nenv();
            else if (ui.view == V_MIX2) {
                static const char *const L2[4] = {"LVL", "PAN", "LCUT", "HCUT"};
                static const char *const L3[4] = {"REV", "DLY", "THRS", ""};
                if (cur_page() == 1)
                    gt_page_mix("SENDS", L3);
                else
                    gt_page_mix("MIXER 2", L2);
            } else {
                static const char *const L1[4] = {"MIX", "EQF", "DIST", "EQG"};
                gt_page_mix("MIXER", L1);
            }
            if (gt_slbox_on())                              /* wip577: a slider's knob turned: the big rectangle over the knobs */
                gt_slider_box();
            gt_big_hint();                                  /* wip570 */
            cv_oy = 0;
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
            continue;
        }
        /* K1 of SEL / ENV / LFO is an icon: its zoom is the circle (draw_readout_h), never a dial */
        if (ui.touched >= 0 && !slider_cell(ui.touched) && !proj.set.zoom_off && ui.view != V_SONG && !(ui.view == V_FX && pgi == 0) && !gt_fx_knobs()) {   /* wip380: the SONG screen: no zoom, the turned cell is only highlighted */   /* (the PERFORM page: no zoom) */   /* GLO > ZOOM KNOB = NO: the cell is only highlighted */
            /* a knob was turned: the zoom (big dial, name, value) over the whole area */
            cv_oy = -row * Q_H;
            draw_readout_h(2 * Q_H);
            cv_oy = 0;
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
            continue;
        }
        if (slider_cell(ui.touched) && row == ((ui.view == V_GLOB || ui.view == V_NEV || ui.view == V_VEL) && ui.touched >= 2 ? 0 : ui.view == V_NOISE ? 0 : 1)) {
            /* a slider was moved: the big rectangle over the knobs' place (bottom half); GLOBAL's third slider
             * lives in the bottom half, so for it the rectangle goes to the top half (its animation stays seen) */
            draw_slider_box(pg.r[pgi][ui.touched]);
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
            continue;
        }
        if (row == 0 && ui.view == V_OSC) {       /* ENV: the waveform icon (top centre) and the Osc Freq slider (middle) */
            pref_t wr = pg.r[pgi][0];
            const x0x_param_t *wd = pref_desc(wr);
            int wv = pref_shown(wr), tch = ui.touched == 0;
            if (wd) {
                draw_wave_icon(120, 29, 28, wv);
                cv_text(10, vc(&FONT_XS, 18, 9), &FONT_XS, "WAVEFORM", tch ? C_WHITE : C_GRAY);
                text_r(230, vc(&FONT_B, 18, 14), &FONT_B, wd->names && wv <= wd->max ? wd->names[wv] : "", tch ? C_WHITE : C_HI);
            }
            draw_slider_cell(1, 58);
            cv_commit(4, 0, MAIN_Y);
            continue;
        }
        if (row == 1 && ui.view == V_NOISE) {     /* LFO: the Filter Freq slider at the bottom */
            draw_slider_cell(1, 26);
            cv_commit(5, 0, MAIN_Y + Q_H);
            continue;
        }
        if (row == 0 && ui.view == V_MIX1) {      /* two horizontal sliders, stacked, in the top half */
            draw_slider_cell(0, 0);
            draw_slider_cell(1, Q_H / 2);
            cv_commit(4, 0, MAIN_Y);
            continue;
        }
        if (ui.view == V_GLOB) {                  /* GLOBAL PARAMETERS: three sliders, 1 and 2 in the top half, 3 in the bottom */
            if (row == 0) {
                draw_slider_cell(0, 0);
                draw_slider_cell(1, Q_H / 2);
            } else {
                draw_slider_cell(2, 0);
                draw_slider_cell(3, Q_H / 2);
            }
            cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
            continue;
        }
        for (c = 0; c < 2; c++) {
            int i = row * 2 + c, x0 = c * 120, cx = x0 + 60, val, touched = ui.touched == i;
            pref_t r;
            if (i < 0)
                continue;
            r = pg.r[pgi][i];
            const x0x_param_t *d = pref_desc(r);
            char num[16], unit[8];
            uint16_t col = drum_mix_page() >= 0 ? PART_COL[drum_mix_page()] : part_col();
            const felucca_font_t *nf = &FONT_S;
            if (ui.view == V_NEV && i == 1) {                 /* wip328: K2 = the CHOKE button: blue when the selected track is in the group, grey when not */
                int in = (seq.choke >> ui.sel[0]) & 1, k;
                float sc = 0.055f, ox = (float)cx + 0.5f, oy = 38.5f;     /* wip333: his icon (two bars, a disc cut by a slanted stroke), black on blue (in) / grey (not in) */
                disc(ox, oy, 34.0f, in ? C_SLBLUE : C_GRAY);
                rbox(cx - 22, 38 - 21, 44, 5, 2, C_BLACK);               /* the top bar */
                rbox(cx - 22, 38 + 17, 44, 5, 2, C_BLACK);               /* the bottom bar */
                for (k = 0; k <= 40; k++) {                              /* the slanted stroke: from the lower left to the upper right, round ends */
                    float t = (float)k / 40.0f;
                    disc(ox + (-9.6f + 19.2f * t) , oy + (11.9f - 23.2f * t), 1.8f, C_BLACK);
                }
                disc(ox - 0.2f, oy + 0.0f, 7.0f, C_BLACK);               /* the big disc in the middle */
                text_c(cx, vc(&FONT_S, 75, 11), &FONT_S, "CHOKE GROUP", in ? C_WHITE : C_GRAY);
                cell_value(cx, vc(&FONT_B, 88, 14), in ? "IN" : "NOT IN", "", in ? C_SLBLUE : C_GRAY);
                continue;
            }
            if (ui.view == V_VEL && i == 3) {                 /* wip326: K4 = RANDOM: the blue RND icon (blinks while it works) */
                int busy = (int32_t)(rnd_until - plat_ms()) > 0;
                disc((float)cx + 0.5f, 38.5f, 34.0f, (!busy || (ui.frame & 4u)) ? C_SLBLUE : dim(C_SLBLUE, 4));   /* wip330: the blue disc with RNDM written inside (blinks while it works) */
                text_c(cx, vc(&FONT_M, 38 - 10, 20), &FONT_M, "RNDM", C_BLACK);   /* wip333: black letters */
                text_c(cx, vc(&FONT_S, 75, 11), &FONT_S, "RANDOM", busy ? C_WHITE : C_GRAY);
                cell_value(cx, vc(&FONT_B, 88, 14), "TURN", "", busy ? C_WHITE : C_HI);
                continue;
            }
            if (ui.view == V_PMOD && i == 3) {                /* wip610: K4 = RANDOM ALL: the purple disc (blinks while it works) */
                int busy = (int32_t)(rnd_all_until - plat_ms()) > 0;
                disc((float)cx + 0.5f, 38.5f, 34.0f, (!busy || (ui.frame & 4u)) ? GT_PURPLE : dim(GT_PURPLE, 4));
                text_c(cx, vc(&FONT_M, 38 - 10, 20), &FONT_M, "RNDM", C_BLACK);
                text_c(cx, vc(&FONT_S, 75, 11), &FONT_S, "RANDOM ALL", busy ? C_WHITE : C_GRAY);
                cell_value(cx, vc(&FONT_B, 88, 14), "TURN", "", busy ? C_WHITE : C_HI);
                continue;
            }
            if (!d)
                continue;
            if (extra_off(r))                                 /* wip302: CLASSIC: greyed */
                col = C_LINE;
            if (touched)
                box(x0 + 4, 2, 112, Q_H - 4, dim(col, 4));
            val = pref_get(r);
            if (lock_step() >= 0) {
                int lv = lock_of(r, lock_step());
                if (lv >= 0)
                    val = lv;
                if (lockable(r))
                    dot(x0 + 108, 10, 3, lv >= 0 ? C_AMB : C_LINE);
            } else if (has_motion(r)) {
                int mv = motion_now(r);
                dot(x0 + 108, 10, 3, mv >= 0 ? C_WHITE : C_DIM);
                if (mv >= 0)
                    val = mv;
            }
            if (ui.view == V_NOISE && i == 0) {
                draw_filter_icon(cx, 38, 34, val);
            } else if (ui.view == V_PMOD && i == 0) {
                draw_mod_icon(cx, 38, 34, val, 1);
            } else if (ui.view == V_NEV && i == 0) {
                draw_env_icon(cx, 38, 34, val);
            } else if (r.kind == R_SCIN) {                    /* wip356: the sidechain INPUT is a big number (the size of the knob), not a knob */
                char nb[4];
                int ch = caps(&FONT_L)->bot - caps(&FONT_L)->top + 1, k = 32 / ch < 1 ? 1 : 32 / ch;
                put_i(nb, val + 1);
                big_text(cx - big_w(&FONT_L, nb, k) / 2, 4 + (68 - ch * k) / 2 - caps(&FONT_L)->top * k, &FONT_L, nb, extra_off(r) ? C_DIM : touched ? C_WHITE : C_HI, k);
            } else if (d->names && d->max < 12) {
                int n = d->max + 1, k, pw = n > 6 ? 6 : 12, gap = 3, w0 = n * (pw + gap) - gap;
                for (k = 0; k < n; k++)
                    box(cx - w0 / 2 + k * (pw + gap), 24, pw, 20, k == val ? col : C_LINE);
            } else {
                seg_knob(cx, 38, 34, 28, d->max ? (float)val / (float)d->max : 0.0f, bipolar_of(r), 1);
            }
            if (extra_off(r))                                 /* wip305: CLASSIC: a shade over the knob */
                shade_rect(x0 + 8, 2, 104, 72);
            if (tw(nf, caps_of(pref_name(r))) > 112)
                nf = &FONT_XS;
            text_c(cx, vc(nf, 75, 11), nf, caps_of(pref_name(r)), extra_off(r) ? C_DIM : touched ? C_WHITE : C_GRAY);
            pref_value_of(r, val, num, unit);
            if (r.kind != R_SCIN)                             /* wip356: the input's number is already big above */
                cell_value(cx, vc(&FONT_B, 88, 14), num, unit, extra_off(r) ? C_DIM : touched ? C_WHITE : C_HI);
        }
        if (ui.view == V_MIX2 && pgi == 1 && row == 1)       /* wip356: SENDS: the dot right in the middle between SIDECHAIN INPUT and INTENSITY: red pulsing = the selected track is ducked, purple = it is the input */
            sc_dot(120, 38, 7, ui.sel[0]);
        cv_commit(4 + row, 0, MAIN_Y + row * Q_H);
    }
    cv_begin(240, 12, C_BG);
    draw_footer_y(1);
    cv_commit(2, 0, MAIN_Y + 2 * Q_H);
}

/* ===================================================================== LEDs === */
/* wip615: BLINKS BY TIME, at fixed rates (his words): the muted track's key 150 BPM, the soloed track's key 200 BPM, the current pattern's key (FUNC held, PERFORM) 110 BPM.
 * leds() builds the steady lights once per UI frame and only NOTES which keys blink (lq.mute / solo / cur); leds_out() adds them by the clock and sends the lights.
 * ui_leds_tick() calls leds_out() between the frames (the FM-1 draws few frames a second: frame-counted blinks came out uneven). */
static struct { uint32_t b, k, d, f, mute, solo, cur; } lq;
static void leds_out(void)
{
    uint32_t ms = plat_ms(), k = lq.k;
    if (ms % 400u < 200u)
        k |= lq.mute;                                /* 150 BPM */
    if (ms % 300u < 150u)
        k |= lq.solo;                                /* 200 BPM */
    if (ms % 545u < 273u)
        k |= lq.cur;                                 /* 110 BPM */
    k |= ui.keys;
    plat_keys_dim(lq.d & ~k);
    plat_keys_faint(lq.f & ~k & ~lq.d);
    plat_glow(proj.set.keyled == 1);
    plat_play_red(ui.rec);                           /* PLAY green; red while recording */
    plat_leds(lq.b, k);
}
void ui_leds_tick(void)
{
    static uint32_t last;
    uint32_t ms = plat_ms();
    if (ms - last < 5u)
        return;
    last = ms;
    leds_out();
}
static void leds(void)
{
    uint32_t b = 0, k = 0, d = 0, f = 0, i;
    lq.mute = lq.solo = lq.cur = 0;
    int blink = (ui.frame & 8u) != 0;
    int slow = (ui.frame & 16u) != 0;                 /* wip230: the muted track's key blinks slowly (as the FUNC lights do) */
    if (seq.playing && eng_step[TRK_DRUM] % 4 == 0)
        b |= 1u << B_PLAY;
    if (rec_blink_on())                                /* wip684: the same test as the REC dot of the ARP chip */
        b |= 1u << B_REC;
    if (ui.view == V_HOME || ui.view == V_SONG || (ui.view == V_MIX2 && cur_page() == 0) || (ui.view == V_FX && cur_page() == 0) || ui.view == V_GLOB || (seq.song_on && blink))   /* wip695: PERFORM is HOME's 2nd page now */
        b |= 1u << B_HOME;
    if (ui.view == V_SEQ && ui.arp)
        b |= 1u << B_ARP;                              /* wip251 */
    else if (ui.view == V_SEQ)
        b |= 1u << B_SEQ;
    if (ui.cpm && tonic_part() && ui.overlay != O_ASK) {   /* wip422: COPY-PASTE MODE: the SEQ button blinks (the way out) */
        b &= ~(1u << B_SEQ);
        if (blink)
            b |= 1u << B_SEQ;
    }
    if (ui.view == V_PART || ui.view == V_NEV)
        b |= 1u << B_EDIT;
    if (ui.view == V_PMOD)
        b |= 1u << B_SEL;
    if (ui.view == V_OSC)
        b |= 1u << B_ENV;
    if ((ui.view == V_FX && cur_page() != 0) || ui.view == V_MIX1 || (ui.view == V_MIX2 && cur_page() == 1))   /* wip695: SENDS is FX's 2nd page now */
        b |= 1u << B_FX;
    if (ui.view == V_MIX || ui.view == V_NOISE)
        b |= 1u << B_LFO;
    if (ui.view == V_GLO || ui.view == V_VEL)
        b |= 1u << B_GLO;                              /* wip290: VEL is GLO's first page */
    if (ui.overlay != O_NONE)
        b |= 1u << B_SEL;
    if (ui.view == V_HOME || ui.view == V_SONG ? ui.pbank : ui.spage)
        b |= 1u << B_OCTUP;
    if (ui.dirty && blink && ui.view == V_GLO)
        b |= 1u << B_SAVE;
    if (proj.set.keyled) {                           /* bright: on / chosen; dim: there, not chosen */
        if (ui.cpm && tonic_part() && ui.overlay != O_ASK) {   /* wip362: COPY-PASTE MODE: every white key blinks, the chosen source stays lit (steady) */
            for (i = 0; i < 16; i++) {
                if (i == (unsigned)ui.cpsrc || ((ui.cpdone >> i) & 1u))
                    k |= 1u << WHITE_KEY[i];
                else if (blink) {                                       /* wip422: a pattern with notes blinks bright, an EMPTY one blinks dim */
                    if (pattern_used(&proj.pat[ui.pbank * 16 + i]))
                        k |= 1u << WHITE_KEY[i];
                    else
                        d |= 1u << WHITE_KEY[i];
                }
            }
        } else if (tonic_part() && (ui.mod_held & 4u) && ui.overlay != O_ASK) {
            /* FUNC held, any Tonic page: the black keys show the tracks that have notes (steady; a muted one blinks slowly;
             * an empty one is off), the white keys the patterns that have notes (steady; the one waiting to start blinks).
             * Everything else is dark until FUNC is let go. */
            const dpart_t *d0 = &cur_pat()->drum[PART_909];
            int cu = seq_cue_of(&seq, PART_909) - ui.pbank * 16;
            k |= 1u << BLACK_KEY[10];                    /* FUNC itself */
            for (i = 0; i < NDRUM; i++)
                if (sm_any(d0->hit[i])) {
                    if (!track_muted(PART_909, (int)i))
                        k |= 1u << BLACK_KEY[i];
                    else
                        lq.mute |= 1u << BLACK_KEY[i];
                }
            for (i = 0; i < 16; i++)
                if (pattern_used(&proj.pat[ui.pbank * 16 + i]))
                    k |= 1u << WHITE_KEY[i];
            if (cu >= 0 && cu < 16) {                    /* the cued pattern: blinks (used or not) until it starts */
                k &= ~(1u << WHITE_KEY[cu]);
                if (blink)
                    k |= 1u << WHITE_KEY[cu];
            }
            {                                            /* wip282: FUNC held: the CURRENT pattern's key blinks slowly (where we are) */
                int cp = seq.ppat[PART_909] - ui.pbank * 16;
                if (cp >= 0 && cp < 16 && cp != cu) {
                    k &= ~(1u << WHITE_KEY[cp]);
                    lq.cur |= 1u << WHITE_KEY[cp];
                }
            }
            if (ui.blink_pat >= 0 && ui.blink_pat < 16 && (int32_t)(plat_ms() - ui.blink_until) < 0) {   /* a paste just went here: */
                k &= ~(1u << WHITE_KEY[ui.blink_pat]);                                                /* its key blinks fast a moment */
                if (ui.frame & 2u)
                    k |= 1u << WHITE_KEY[ui.blink_pat];
            }
        } else if (ui.overlay == O_ASK && ui.ask_act == ACT_CHAIN && tonic_part()) {   /* wip628: the SONG PATTERNS screen (empty song): white keys = patterns, with notes LIT, empty DARK (the keys write patterns 1-16 of the first bank) */
            for (i = 0; i < 16; i++)
                if (pattern_used(&proj.pat[i]))
                    k |= 1u << WHITE_KEY[i];
        } else if (ui.view == V_SEQ && ui.arp && tonic_part()) {   /* wip251: the ARP page: the two C keys dim (C = the voice's own pitch at octave 0) */
            d |= 1u << WHITE_KEY[KEY_WHITE[ARP_C_KEY]];
            d |= 1u << WHITE_KEY[KEY_WHITE[ARP_C_KEY + 12]];
            if (ui.sel[PART_909] < NDRUM_TONIC && slow)   /* wip418: the CURRENT track's black key (1-8) blinks slowly: the keys are all piano keys here, this says which voice they play */
                k |= 1u << BLACK_KEY[ui.sel[PART_909]];
        } else if (ui.view == V_HOME || ui.view == V_SONG) {
            const song_t *sg = song();
            int bar = ui.view == V_SONG && ui.song_sel < sg->len;
            int pp = (bar ? sg->bar[ui.song_sel].pat[PART_909] : seq.ppat[PART_909]) - ui.pbank * 16;
            int cu = seq_cue_of(&seq, PART_909) - ui.pbank * 16;
            for (i = 0; i < 16; i++)                  /* patterns with notes */
                if (pattern_used(&proj.pat[ui.pbank * 16 + i])) {
                    if (ui.view == V_SONG && tonic_part())   /* wip624: the SONG screen: a pattern with notes is LIT, an empty one is dark */
                        k |= 1u << WHITE_KEY[i];
                    else
                        d |= 1u << WHITE_KEY[i];
                }
            if (pp >= 0 && pp < 16 && (ui.view == V_HOME || bar)) {
                if (ui.view == V_SONG && tonic_part()) {  /* wip624: the cursor bar's pattern blinks (where the next white key writes over) */
                    k &= ~(1u << WHITE_KEY[pp]);
                    if (blink)
                        k |= 1u << WHITE_KEY[pp];
                } else
                    k |= 1u << WHITE_KEY[pp];
            }
            if (ui.view == V_HOME && seq_cue_of(&seq, PART_909) >= 0 && blink && cu >= 0 && cu < 16)
                k |= 1u << WHITE_KEY[cu];
            for (i = 0; i < NPARTS; i++) {            /* the parts: playing bright, muted dim */
                int muted = ui.view == V_HOME ? part_muted((int)i) : bar ? (int)(sg->bar[ui.song_sel].mute >> i & 1u) : 1;
                if (muted)
                    d |= 1u << BLACK_KEY[i];
                else
                    k |= 1u << BLACK_KEY[i];
            }
        } else if (((ui.view == V_MIX && !msp_on()) || ui.view == V_MIX2 || ui.view == V_MIX1 || ui.view == V_PMOD || ui.view == V_OSC || ui.view == V_NOISE || ui.view == V_NEV || ui.view == V_VEL) && (cur_page() == 0 || drum_mix_page() >= 0)) {
            int dm = drum_mix_page(), n = dm >= 0 ? NDRUM : NPARTS, on = dm >= 0 ? ui.sel[dm] : ui.mixsel;
            if (dm >= 0) {                               /* the Tonic's sound pages (FX 1st press, SEL, ENV, LFO, EDIT, HOME 1st press): */
                const dpart_t *dd = &cur_pat()->drum[dm];   /* the white keys show the selected track's trigs and the playhead, as on SEQ */
                int ph = eng_step[TRK_DRUM + dm];
                for (i = 0; i < 16; i++) {
                    int s = step_of((int)i), lit = s < dd->len && sm_get(dd->hit[ui.sel[dm]], s);
                    if (seq.playing && s == ph)
                        lit = !lit;
                    if (lit && !blink && step_flag_blink(dd, ui.sel[dm], s))   /* wip429 */
                        lit = 0;
                    if (lit)
                        k |= 1u << WHITE_KEY[i];
                }
            }
            for (i = 0; i < (uint32_t)n && i < 11u; i++) {  /* the knobs' channel bright (blinking: muted), */
                int mu = dm >= 0 ? track_muted(dm, (int)i) : part_muted((int)i);   /* the others dim, muted dark */
                if ((int)i == on) {
                    if (!mu)
                        k |= 1u << BLACK_KEY[i];
                    else
                        lq.mute |= 1u << BLACK_KEY[i];
                } else if (!mu)
                    d |= 1u << BLACK_KEY[i];
            }
        } else if (dck_page()) {                      /* wip350: the DUCK screen: black keys 1-8 lit = ducked or the input */
            for (i = 0; i < NDRUM_TONIC; i++)
                if (((proj.set.sc_mask >> i) & 1u) || (int)i == proj.set.sc_in)
                    k |= 1u << BLACK_KEY[i];
        } else if (chk_page()) {                      /* wip328: the CHOKE screen: black keys 1-8 are lit when the track is in the group */
            for (i = 0; i < NDRUM_TONIC; i++)
                if ((seq.choke >> i) & 1u)
                    k |= 1u << BLACK_KEY[i];
        } else if ((ui.view == V_FX && cur_page() == 0) || (ui.seqpat && ui.view == V_SEQ && tonic_part()) || (ui.view == V_MIX && msp_on())) {   /* PERFORM, the pattern page and the GLO mixer (wip607): black keys 1-8 are the mute buttons: lit = the track plays, dark = muted */
            for (i = 0; i < NDRUM_TONIC; i++)
                if (!track_muted(PART_909, (int)i) && (!msp_on() || !seq.vsolo || (seq.vsolo & (1u << (PART_909 * NDRUM + i)))))   /* (wip271: on the MUTE / SOLO page a voice the solo silences is dark) */
                    k |= 1u << BLACK_KEY[i];
                else if (msp_on() && track_muted(PART_909, (int)i))   /* wip615: a muted voice's key blinks at 150 BPM, by the clock (lq.mute) */
                    lq.mute |= 1u << BLACK_KEY[i];
            if (msp_on()) {                               /* wip272 (both PERFORM pages): the mode's key is lit: FUNC = MUTE, FILL = SOLO */
                k |= 1u << BLACK_KEY[ui.msmode || ui.view == V_MIX ? 9 : 10];   /* wip630: the GLO mixer is always SOLO: FILL lit */
                {                                         /* wip375: the white keys = the 16 patterns: empty = no light, with notes = low light, the CURRENT one blinks slowly, the cued (next) one blinks (fast) */
                    int cp = seq.ppat[PART_909] - ui.pbank * 16;
                    int cu = pat_next() - ui.pbank * 16;
                    for (i = 0; i < 16; i++)
                        if (pattern_used(&proj.pat[ui.pbank * 16 + i]))
                            d |= 1u << WHITE_KEY[i];
                    if (cp >= 0 && cp < 16) {
                        d &= ~(1u << WHITE_KEY[cp]);
                        lq.cur |= 1u << WHITE_KEY[cp];
                    }
                    if (cu >= 0 && cu < 16 && cu != cp) {
                        d &= ~(1u << WHITE_KEY[cu]);
                        if (blink)
                            k |= 1u << WHITE_KEY[cu];
                    }
                }
            }
        } else if (ui.view == V_MIX || ui.view == V_FX) {
            /* the master and the effects: the black keys have nothing to choose; the white ones show the steps (wip204) */
            if (step_hold_view()) {
                const dpart_t *d0 = &cur_pat()->drum[ui.part];
                int ph = eng_step[TRK_DRUM + ui.part];
                for (i = 0; i < 16; i++) {
                    int s = step_of((int)i), on = s < d0->len && sm_get(d0->hit[ui.sel[ui.part]], s);
                    if (seq.playing && s == ph)
                        on = !on;
                    if (on && !blink && step_flag_blink(d0, ui.sel[ui.part], s))   /* wip429 */
                        on = 0;
                    if (on)
                        k |= 1u << WHITE_KEY[i];
                }
            }
        } else if (is_drum()) {
            const dpart_t *d0 = &cur_pat()->drum[ui.part];
            const uint32_t *bits = ((ui.btn & (1u << B_ENV)) || ac_on()) ? d0->accent : d0->hit[ui.sel[ui.part]];
            int ph = eng_step[TRK_DRUM + ui.part];
            for (i = 0; i < 16; i++) {
                int s = step_of((int)i), on = s < d0->len && sm_get(bits, s);
                if (seq.playing && s == ph)
                    on = !on;
                if (on && !blink && bits == d0->hit[ui.sel[ui.part]] && step_flag_blink(d0, ui.sel[ui.part], s))   /* wip429: ACCENT / FILL held: the trigs with that flag blink */
                    on = 0;
                if (on)
                    k |= 1u << WHITE_KEY[i];
            }
            for (i = 0; i < NDRUM && i < 11u; i++) {    /* the track bright (blinking: muted), the others */
                int mu = track_muted(ui.part, (int)i);  /* with hits dim, muted dark */
                if (!ac_on() && (int)i == ui.sel[ui.part]) {
                    if (!mu)
                        k |= 1u << BLACK_KEY[i];
                    else
                        lq.mute |= 1u << BLACK_KEY[i];
                } else if ((int)i != ui.sel[ui.part] && sm_any(d0->hit[i]) && !mu)
                    d |= 1u << BLACK_KEY[i];
            }
        } else if (ui.part == PART_BRK) {
            int ph = eng_step[TRK_BRK];
            for (i = 0; i < 16; i++) {
                int on = (cur_pat()->brk.steps >> i) & 1u;
                if (seq.playing && (int)i == ph)
                    on = !on;
                if (on)
                    k |= 1u << WHITE_KEY[i];
            }
            for (i = 0; i < 8; i++)                   /* the slice pads */
                d |= 1u << BLACK_KEY[i];
        }
        /* wip231 / wip232: white keys 1, 5, 9, 13 glow FAINT (fainter than dim) = the 4 / 4 beats, on every page but PERFORM (the pattern pages HOME / SONG, FUNC held and a question show patterns there) */
        if (!proj.set.beat_off && !(ui.view == V_FX && cur_page() == 0) && ui.view != V_HOME && ui.view != V_SONG && ui.overlay != O_ASK &&
            !(tonic_part() && (ui.mod_held & 4u)) && !(ui.cpm && tonic_part()))   /* wip422: not in the COPY-PASTE MODE (the keys blink there) */
            for (i = 0; i < 16; i += 4)
                f |= 1u << WHITE_KEY[i];
    }
    if (tonic_part() && seq.vsolo && !(ui.mod_held & 4u) && ui.overlay != O_ASK && !(ui.view == V_SEQ && ui.arp)) {   /* wip264 (wip282: also on the PERFORM pages): the soloed voice's key blinks fast */
        for (i = 0; i < NDRUM_TONIC; i++)
            if (seq.vsolo & (1u << (PART_909 * NDRUM + i))) {
                k &= ~(1u << BLACK_KEY[i]);
                d &= ~(1u << BLACK_KEY[i]);
                lq.solo |= 1u << BLACK_KEY[i];    /* wip615: a soloed voice's key blinks at 200 BPM, by the clock */
            }
    }
    {                                                /* wip376: HIT FLASH (like the Digitakt): when a track's trig plays, its black key blinks for that hit, on every page except ARP (wip378) */
        static uint8_t hc[NDRUM_TONIC];              /* wip584: the hit counters seen last; a hit = the counter moved (every hit counts, at any frame rate) */
        static uint32_t fu[NDRUM_TONIC];             /* the flash lasts until this time (ms), not a number of frames: the same look on a slow page and a fast one */
        static uint8_t fl[NDRUM_TONIC], hinit;
        uint32_t now = plat_ms();
        for (i = 0; i < NDRUM_TONIC; i++) {
            uint8_t c = eng_hit_cnt[i];
            if (!hinit)
                hc[i] = c;                           /* the first look: no flash for hits from before */
            if (c != hc[i]) {
                hc[i] = c;
                fu[i] = now + 90u;                   /* a new hit: about 90 ms */
            }
            fl[i] = (uint8_t)((int32_t)(fu[i] - now) > 0);
        }
        hinit = 1;
        if (tonic_part() && !(ui.view == V_SEQ && ui.arp) && seq.playing && !msp_on())   /* wip378: every page except ARP; wip611: and not where the 8 black keys show MUTE / SOLO (PERFORM, the pattern page, the GLO mixer): there they stay steady */
            for (i = 0; i < NDRUM_TONIC; i++)
                if (fl[i] && !(lq.solo & (1u << BLACK_KEY[i]))) {
                    if (k & (1u << BLACK_KEY[i]))
                        k &= ~(1u << BLACK_KEY[i]);   /* a key that is lit (the selected track) goes dark for the hit */
                    else {
                        d &= ~(1u << BLACK_KEY[i]);
                        k |= 1u << BLACK_KEY[i];      /* a dim or dark key lights up for the hit */
                    }
                }
    }
    lq.b = b;
    lq.k = k;
    lq.d = d;
    lq.f = f;
    leds_out();
}

/* ===================================================================== frame === */
/* ============================================================ screen reader === */
/* The screen as text for a screen reader on the computer (announce.c; web/reader): a screen or a page
 * once it settles, with its knobs; a knob's value as it turns; what a button did (say); the help cards;
 * questions; a list's rows; steps put on and off; play and stop. WHERE (the reader asks, or it has just
 * started listening) is everything on the screen. Nothing is built while no reader listens. */
static struct {
    uint32_t ctx;                      /* the screen, hashed: a change is said */
    char knob[ANN_MAX + 1];            /* the readout said last */
    int16_t list_sel, list_v;
    int8_t playing, pat;
    uint8_t started;
} sp X0X_POOL;
static char sp_buf[ANN_MAX + 1] X0X_POOL;
static uint32_t sp_n;

static void sp_cat(const char *s)
{
    while (*s && sp_n < ANN_MAX)
        sp_buf[sp_n++] = *s++;
    sp_buf[sp_n] = 0;
}
static void sp_int(int v)
{
    char b[12];
    put_i(b, v);
    sp_cat(b);
}
static void sp_value(pref_t r)                        /* "64", "-6 DB", "OFF" */
{
    char num[16], unit[8];
    pref_value(r, num, unit);
    sp_cat(num);
    if (unit[0]) {
        sp_cat(" ");
        sp_cat(unit);
    }
}
static void sp_row(pref_t r)                          /* a list row or a knob: "DECAY 64" */
{
    char t[28];
    if (r.kind == R_NONE)
        return;
    if (r.kind == R_HDR) {
        hdr_name(r.a, t);
        sp_cat(t);
        return;
    }
    sp_cat(caps_of(pref_name(r)));
    if (r.kind == R_ACT)
        return;
    sp_cat(" ");
    sp_value(r);
}

/* where we are: the screen, the knobs' owner, the four knobs */
static void sp_screen(void)
{
    char t[28];
    int i, pgi = cur_page();
    if (part_view()) {
        sp_cat(PART_N[ui.part]);
        if (ui.view == V_SEQ)
            sp_cat(ui.arp && tonic_part() ? " ARP" : " SEQ");
    } else {
        sp_cat(view_title());
    }
    if (pg.n) {
        uint32_t k = 0;
        knob_tag(t);
        while (k < sp_n && t[k] == sp_buf[k])
            k++;
        if (k < sp_n || (t[k] && t[k] != ' ')) {     /* "909 KIT" after "909": not "909. 909 KIT" */
            sp_cat(". ");
            sp_cat(t);
        } else {
            sp_cat(t + k);
        }
        for (i = 0; i < 4; i++)
            if (pg.r[pgi][i].kind != R_NONE) {
                sp_cat(i ? ", " : ": ");
                sp_row(pg.r[pgi][i]);
            }
    }
}

/* the steps the white keys show, on: "STEPS 1-16 ON: 1 5 9 13" */
static void sp_steps(void)
{
    int c, any = 0, len = steps_len();
    if (!steps_view() || !is_drum())
        return;
    sp_cat(". STEPS ");
    sp_int(ui.spage * 16 + 1);
    sp_cat("-");
    sp_int(ui.spage * 16 + 16);
    sp_cat(" ON:");
    for (c = 0; c < 16; c++) {
        int s = step_of(c), on;
        if (s >= len)
            break;
        on = sm_get(cur_pat()->drum[ui.part].hit[ui.sel[ui.part]], s);
        if (on) {
            sp_cat(" ");
            sp_int(s + 1);
            any = 1;
        }
    }
    if (!any)
        sp_cat(" NONE");
}

/* a card's line, as a sentence after the last: a line going on from the one before (indented, or after a
 * comma or a colon) joins it; two things on one line (three spaces apart) are two sentences */
static void sp_line(const char *s)
{
    char p = sp_n ? sp_buf[sp_n - 1] : 0;
    if (*s == ' ' || p == ',' || p == ':')
        sp_cat(" ");
    else
        sp_cat(". ");
    while (*s == ' ')
        s++;
    while (*s && sp_n < ANN_MAX) {
        if (s[0] == ' ' && s[1] == ' ') {
            while (*s == ' ')
                s++;
            sp_cat(". ");
            continue;
        }
        sp_buf[sp_n++] = *s++;
    }
    sp_buf[sp_n] = 0;
}

/* a list row with where it is: "TUNE 66, 2 OF 14" (the section names not counted) */
static void sp_list_row(void)
{
    int i, k = 0, n = 0;
    pref_t r = list_rows[ui.list_sel];
    for (i = 0; i < ui.list_n; i++)
        if (list_rows[i].kind != R_HDR) {
            n++;
            if (i <= ui.list_sel)
                k = n;
        }
    sp_row(r);
    if (r.kind == R_ACT)
        sp_cat(", ACTION");
    sp_cat(", ");
    sp_int(k);
    sp_cat(" OF ");
    sp_int(n);
}

/* what is in front: a help card, a question, a list, or the screen */
static void sp_front(int all)
{
    sp_n = 0;
    sp_buf[0] = 0;
    if (ui.help) {
        const char *l[HELP_LINES];
        int n = help_lines(ui.help - 1, l), k;
        sp_cat(BTN_LABEL[ui.help - 1]);
        sp_cat(" BUTTON");
        for (k = 0; k < n; k++)
            sp_line(l[k]);
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_SAVEAS) {
        if (ui.ask_arg == SV_HERE) {
            sp_cat("SAVE HERE? PLACE ");
            sp_int(ui.sv_here);
            sp_cat(" ");
            sp_cat(ui.sv_pn[0] ? ui.sv_pn : "UNTITLED");
            sp_cat(ui.sv_here == ui.preset_cur ? "" : ui.sv_occ[0] ? ". REPLACES " : ". EMPTY PLACE");
            sp_cat(ui.sv_here == ui.preset_cur ? "" : ui.sv_occ);
            sp_cat(". ALGORITHM: NEW NAME. SELECT: OTHER SPOT. OCT+: YES, SAVE NOW. OCT-: CANCEL");
        } else if (ui.ask_arg == SV_CHOOSE) {
            sp_cat("CHOOSE SLOT. OCT+: CHOOSE A SLOT. OCT-: NO, EXIT SAVE");
        } else if (ui.ask_arg == 2) {
            sp_cat("SAVE IN SLOT ");
            sp_int(ui.sv_here);
            sp_cat(ui.sv_occ[0] ? ". REPLACES " : ". EMPTY");
            sp_cat(ui.sv_occ);
            sp_cat(". ALGORITHM: OTHER SLOT. OCT+: CONFIRM. OCT-: BACK");
        } else {
            sp_cat(ui.sv_rename ? "RENAME " : ui.ask_arg ? "NAME " : "SAVE AS ");
            sp_cat(ui.sv_name);
            if (ui.ask_arg == 1 && proj.set.name_manual) {   /* wip324 */
                char cs[2] = {name_cand(), 0};
                sp_cat(ui.nm_len < NM_MAX ? ". LETTER " : "");
                sp_cat(ui.nm_len < NM_MAX ? cs : "");
                sp_cat(". KNOB 4: LETTER. OCT+: OK. OCT-: DELETE. SAVE: DONE");
            } else {
            sp_cat(". ALGORITHM: ANOTHER NAME. ");
            sp_cat(ui.sv_rename ? "OCT+: SAVE. OCT-: CANCEL" : ui.ask_arg ? "OCT+: SAVE. OCT-: BACK" : "OCT+: OK. OCT-: CANCEL");
            }
        }
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_STOPFIRST) {
        sp_cat("STOP FIRST. THE SEQUENCER IS PLAYING. PLAY: STOP, THEN OCT+ SAVES. OCT-: BACK");
    } else if (ui.overlay == O_ASK && ui.ask_act == ACT_CHAIN) {
        int i;
        sp_cat("PATTERNS. PRESS KEYS 1 TO 16. ");
        if (ui.ch_n) {
            sp_cat("SONG: ");
            for (i = 0; i < ui.ch_n; i++) {
                sp_int(ui.ch_pat[i] + 1);
                sp_cat(" ");
            }
        }
        sp_cat(". OCT+: OK. OCT-: DELETE LAST");
    } else if (ui.overlay == O_ASK) {
        sp_cat(ui.ask_q[0]);
        if (ui.ask_q[1][0]) {
            sp_cat(" ");
            sp_cat(ui.ask_q[1]);
        }
        sp_cat(ui.ask_act == ACT_PASTE_TNPAT ? ". OCT+: YES. OCT-: NO" : octask() ? ". OCT+: CONFIRM. OCT-: GO BACK" : ui.ask_act == ACT_QSAVE ? ". OCT+: YES. OCT-: EXIT" : ui.ask_act == ACT_PRESETGO ? ". OCT+: YES. OCT-: NO" : ui.ask_act == ACT_PRESET ? ". OCT+: UPLOAD. OCT-: GO BACK" : ". SEL: YES. HOME: NO");
    } else if (ui.overlay == O_MEM) {
        sp_cat("PROJECT MEMORY. ");
        { char t[8]; put_i(t, project_mem_pct()); sp_cat(t); }
        sp_cat(" PERCENT USED. GLO: SETTINGS. HOME: BACK");
    } else if (ui.overlay == O_LIST) {
        int i, h = -1;
        sp_cat(list_title);
        sp_cat(" LIST. ");
        for (i = ui.list_sel; i >= 0 && h < 0; i--)   /* the section it is in */
            if (list_rows[i].kind == R_HDR)
                h = i;
        if (h >= 0) {
            sp_row(list_rows[h]);
            sp_cat(". ");
        }
        sp_list_row();
    } else {
        sp_screen();
        if (all)
            sp_steps();
    }
}

/* the readout as said ("STEP 5 BD  TUNE 37"); empty when there is none */
static void sp_readout(void)
{
    pref_t r;
    const char *nm;
    sp_n = 0;
    sp_buf[0] = 0;
    if (ui.touched < 0 || (int32_t)(ui.touch_until - plat_ms()) <= 0 || ui.overlay != O_NONE)
        return;
    nm = readout_name(&r);
    if (lock_step() >= 0 && ui.touched < 4) {
        sp_cat("STEP ");
        sp_int(lock_step() + 1);
        sp_cat(" ");
    }
    sp_cat(nm);
    sp_cat(" ");
    sp_value(r);
}

static uint32_t sp_ctx_sig(void)
{
    uint32_t h = 2166136261u;
#define MIXIN(v) (h = (h ^ (uint32_t)(v)) * 16777619u)
    MIXIN(ui.view);
    MIXIN(ui.part);
    MIXIN(cur_page());
    MIXIN(ui.overlay);
    MIXIN(ui.help);
    MIXIN(ui.sel[ui.part]);
    MIXIN(ui.mixsel);
    MIXIN(ui.overlay == O_ASK ? ui.ask_act : 0);
    MIXIN(ui.overlay == O_LIST ? (uint32_t)list_title[0] << 8 | (uint32_t)list_title[1] : 0);
#undef MIXIN
    return h;
}

/* a step a key just put on or took off: "STEP 5 ON", "STEP 5 ON C3", "STEP 5 ACCENT" */
static void sp_step(int s, int accent)
{
    char t[32], *q;
    if (!ann_on())
        return;
    q = put_i(put_s(t, "STEP "), s + 1);
    if (is_drum()) {
        const dpart_t *d = &cur_pat()->drum[ui.part];
        int v = ui.sel[ui.part];
        if (accent == 3)
            put_s(q, sm_get(d->vfill[v], s) ? " FILL" : " NO FILL");
        else if (accent == 2)
            put_s(q, sm_get(d->vacc[v], s) ? " ACCENT" : " NO ACCENT");
        else if (accent)
            put_s(q, sm_get(d->accent, s) ? " ACCENT" : " NO ACCENT");
        else
            put_s(q, sm_get(d->hit[ui.sel[ui.part]], s) ? " ON" : " OFF");
    }
    ann_say(t, 0, ANN_NOW);
}

static void speak(void)
{
    uint32_t now = plat_ms(), c;
    int where = ann_where();
    if (!ann_on()) {
        sp.started = 0;
        return;
    }
    if (!sp.started || where) {                       /* everything on the screen */
        sp.started = 1;
        sp.ctx = sp_ctx_sig();
        sp_readout();
        put_s(sp.knob, sp_buf);
        sp.list_sel = ui.list_sel;
        sp.playing = (int8_t)seq.playing;
        sp.pat = (int8_t)seq.ppat[part_view() ? ui.part : PART_909];
        sp_front(1);
        ann_say(sp_buf, 0, ANN_NOW);
        return;
    }
    if (seq.playing != sp.playing) {
        sp.playing = (int8_t)seq.playing;
        ann_say(seq.playing ? "PLAY" : "STOP", 0, ANN_NOW);
    }
    {
        int pat = seq.ppat[part_view() ? ui.part : PART_909];
        if (pat != sp.pat) {
            char t[8];
            sp.pat = (int8_t)pat;
            put_i(put_s(t, "P"), pat + 1);
            ann_later(t, 0, 150u, now);
        }
    }
    c = sp_ctx_sig();
    if (c != sp.ctx) {                                /* a new screen, page, card or list: said when it settles */
        sp.ctx = c;
        sp_readout();                                 /* (a readout left from the page before is not news) */
        put_s(sp.knob, sp_buf);
        sp.list_sel = ui.list_sel;
        sp.list_v = ui.overlay == O_LIST ? (int16_t)pref_get(list_rows[ui.list_sel]) : 0;
        sp_front(0);
        ann_later(sp_buf, ANN_NOW, 200u, now);
        return;
    }
    if (ui.overlay == O_LIST && ui.list_sel < ui.list_n) { /* a row moved to, or its value changed */
        pref_t r = list_rows[ui.list_sel];
        int v = r.kind == R_ACT || r.kind == R_HDR ? 0 : pref_get(r);
        if (ui.list_sel != sp.list_sel || v != sp.list_v) {
            sp.list_sel = ui.list_sel;
            sp.list_v = (int16_t)v;
            sp_n = 0;
            if (ui.list_sel > 0 && list_skip(ui.list_sel - 1)) {   /* the first row of a section: its name first */
                sp_row(list_rows[ui.list_sel - 1]);
                sp_cat(". ");
            }
            sp_list_row();
            ann_later(sp_buf, ANN_NOW, 120u, now);
        }
        return;
    }
    sp_readout();                                     /* a knob turning */
    if (sp_n && strcmp_s(sp_buf, sp.knob))
        ann_later(sp_buf, ANN_NOW, 120u, now);
    put_s(sp.knob, sp_buf);
}


#ifdef X0X_HOST
/* every screen, page, list row, value name and help card as a screen reader gets them (the "speakdump" command):
 * KIND <tab> the text sent (fm1-reader/tools/audit.mjs puts it in words with web/words/ and checks it) */
static void sp_print(const char *kind, const char *t) { printf("%s\t%s\n", kind, t); }
static void speak_dump(void)
{
    static const uint8_t views[] = {V_PART, V_SEQ, V_FX, V_MIX, V_HOME, V_SONG, V_PERF, V_MIX2, V_MIX1, V_GLOB, V_PMOD, V_OSC, V_NOISE, V_NEV, V_VEL};
    ui_t keep = ui;
    int v, p, q, i, b, x;
    for (v = 0; v < (int)sizeof views; v++)
        for (p = 0; p < NPARTS; p++) {
            ui.view = views[v];
            ui.part = (uint8_t)p;
            ui.overlay = O_NONE;
            ui.help = 0;
            if (!part_view() && p)
                continue;
            build_pages();
            for (q = 0; q < pg.n; q++) {
                *page_of(ui.view) = (uint8_t)q;
                sp_front(1);
                sp_print("SCREEN", sp_buf);
                for (i = 0; i < 4; i++) {
                    pref_t r = pg.r[q][i];
                    if (r.kind == R_NONE)
                        continue;
                    ui.touched = (int8_t)i;
                    ui.touch_until = plat_ms() + 1000u;
                    sp_readout();
                    sp_print("KNOB", sp_buf);
                    ui.touched = -1;
                }
            }
            *page_of(ui.view) = 0;
            open_list_of_pages();
            for (i = 0; i < ui.list_n; i++) {
                const x0x_param_t *d = pref_desc(list_rows[i]);
                if (list_skip(i))
                    continue;                                /* (a section's name: never selected) */
                ui.list_sel = (uint8_t)i;
                sp_front(0);
                sp_print("ROW", sp_buf);
                if (d && d->names && list_rows[i].kind != R_ACT)
                    for (x = 0; x <= d->max; x++) {
                        char num[16], unit[8];
                        pref_value_of(list_rows[i], x, num, unit);
                        put_s(put_s(put_s(put_s(put_s(sp_buf, caps_of(pref_name(list_rows[i]))), " "), num), " "), unit);
                        sp_print("VALUE", sp_buf);
                    }
            }
            ui.overlay = O_NONE;
            for (b = 0; b < NB; b++)
                for (x = 0; x < 2; x++) {
                    ui.rec = (uint8_t)x;
                    ui.help = (uint8_t)(b + 1);
                    sp_front(0);
                    sp_print("HELP", sp_buf);
                }
            ui.help = 0;
            ui.rec = 0;
        }
    ui.view = V_GLO;
    open_global();
    for (i = 0; i < ui.list_n; i++) {
        const x0x_param_t *d = pref_desc(list_rows[i]);
        if (list_skip(i))
            continue;
        ui.list_sel = (uint8_t)i;
        sp_front(0);
        sp_print("ROW", sp_buf);
        if (d && d->names && list_rows[i].kind != R_ACT)
            for (x = 0; x <= d->max; x++) {
                char num[16], unit[8];
                pref_value_of(list_rows[i], x, num, unit);
                put_s(put_s(put_s(put_s(put_s(sp_buf, caps_of(pref_name(list_rows[i]))), " "), num), " "), unit);
                sp_print("VALUE", sp_buf);
            }
    }
    ui = keep;
    build_pages();
}
#endif

void ui_init(void)
{
    int i;
    for (i = 0; i < NKEYS; i++) {
        if (KEY_WHITE[i] >= 0)
            WHITE_KEY[KEY_WHITE[i]] = (uint8_t)i;
        if (KEY_BLACK[i] >= 0)
            BLACK_KEY[KEY_BLACK[i]] = (uint8_t)i;
    }
    corners_init();
    ui.view = V_SEQ;                                /* switch on and a loaded preset: the SEQ page */
    ui.prev_view = V_SEQ;
    ui.part = PART_909;
    ui.held_step = -1;
    ui.brk_held = -1;
    ui.hold_step = -1;
    ui.clip = -1;
    ui.tc_src = -1;
    ui.pc_view = -1;
    ui.pst_trk = 0;
    ui.blink_pat = -1;
    ui.cpm = 0;
    ui.cpexit = 0;
    ui.cpf = 0;
    ui.cpsrc = -1;
    ui.touched = -1;
    ui.overlay = O_NONE;
    ui.outline_ok = 0;
    palette_set(proj.set.palette);
    ui.preset_cur = proj.set.preset_cur > NPRESETS ? 0 : proj.set.preset_cur;   /* wip309: the place loaded before power-off (0 = none); wip654: a place 31-40 of the old 40 is gone: none */
    for (i = 0; i < 6; i++)
        blit_hash[i] = 0;
    build_pages();
    undo_start();                                   /* the project as loaded is where undo stops */
}

/* a Tonic preset (tnp.h) loaded as the working project: stopped only (as a restore). 0 = loaded (the name on the
 * screen); 10 = playing, STOP FIRST; else project_preset_unpack's refusal, nothing changed. It is a change like any
 * other: the autosave keeps it (the compressor, reverb and delay come from the preset itself: its saved ones, or the factory ones; only SAVE keeps changes to them). */
int app_load_preset(const uint8_t *b, uint32_t n)
{
    return load_preset_ex(b, n, -1);                   /* a preset on its own (the web page's paste, the simulator): no second spot */
}

static int load_preset_ex(const uint8_t *b, uint32_t n, int ext_slot)
{
    char nm[TNP_NAME_LEN + 1];
    uint32_t mark = 0;
    int rc, i;
    if (seq.playing)
        return 10;
    rc = project_preset_unpack(b, n);
    if (rc)
        return rc;
    if (ext_slot >= 0)
        mark = project_pair_mark(b, n);                /* wip659: the main spot's CRC, taken before pbuf (= b) is reused for the second spot */
    for (i = 0; i < (int)TNP_NAME_LEN; i++)
        nm[i] = (char)b[16 + i];
    nm[TNP_NAME_LEN] = 0;
    engine_apply_sound(&proj.sound);
    seq.chain_on = project_chain_tidy() > 0 ? 1 : 0;   /* wip250: a preset plays through its chains only when it has some (flags on patterns WITH notes); an empty slot or a preset without them: SONG OFF, nothing runs away. K1 of the pattern page switches it */
    seq.vmute = (seq.vmute & ~(uint32_t)((1u << NDRUM) - 1u)) | b[11];   /* the preset's own mutes (Tonic voices 1-8) */
    seq.vsolo = 0;                                     /* wip264: a new preset starts without a solo */
    seq.choke = 0;                                     /* wip328: the preset's CHOKE group: bit 0 of each voice's spare byte (an older preset: 0 = none) */
    for (i = 0; i < NDRUM_TONIC; i++)
        seq.choke |= (uint8_t)((b[TNP_HDR + (uint32_t)i * TNP_VOICE + 43u] & 1u) << i);
    engine_tonic_choke(seq.choke);
    if (ext_slot >= 0) {                               /* wip659 (E2c): the place's second spot: its song, motion lanes and ARP notes. b is not read any more below (nm, mutes and choke are out), so pbuf can hold the second spot. None, damaged or of another pair (a save cut by a power-off): song, lanes and notes stay empty, as a preset load made them just now */
        int r = plat_ext_load(ext_slot, pbuf, sizeof pbuf);
        project_ext_unpack(r > 0 ? pbuf : 0, r > 0 ? (uint32_t)r : 0u, mark);
    }
    seq.bpm = (float)proj.set.bpm_x10 / 10.0f;
    project_apply_io();
    engine_motion_reset();
    engine_brk_loops();
    ui.song_sel = 0;
    ui_redraw();
    ui_init();
    ui.dirty = 1;
    ui_say("PRESET ", nm);
    return 0;
}

/* AUTOSAVE: changes are saved by themselves while the pattern is stopped and nothing has been touched
 * for AUTOSAVE_QUIET ms, at most every AUTOSAVE_GAP ms. Stopped only: a flash erase silences the
 * audio. Only the objects that changed are written (project.c), usually one or two sectors. */
#define AUTOSAVE_QUIET 4000u
#define AUTOSAVE_GAP 20000u
#define SETTINGS_QUIET 1500u                           /* wip321: the settings alone: a shorter quiet (so they survive a quick power-off) */
static void autosave(void)
{
    uint32_t now = plat_ms();
    if (save_hold)
        return;
    if (save_wait && !seq.playing && !seq.req_stop) { /* SAVE pressed while playing: now */
        save_project();
        return;
    }
    {
        int full = ui.dirty && (!proj.set.autosave_off || ui.last_autosave);
        int only = !full && ui.set_dirty;               /* wip321: AUTOSAVE off: the settings alone are still kept (never the patterns or the sound) */
        if ((!full && !only) || seq.playing || seq.req_stop || ui.rec || perf_testing() ||
            ui.overlay == O_ASK || now - ui.act_t < (only ? SETTINGS_QUIET : AUTOSAVE_QUIET) || (ui.saved_t && now - ui.saved_t < AUTOSAVE_GAP))
            return;
        ui.saved_t = now;
        if (only) {
            if (project_autosave_settings() == 0) {
                ui.set_dirty = 0;
                say("SETTINGS SAVED", 0);
            }
            return;
        }
    }
    ui.last_autosave = 0;
    switch (project_autosave()) {
    case 0:
        ui.dirty = ui.set_dirty = 0;
        say("AUTOSAVED", 0);
        break;
    case -7:
        warn("MEMORY FULL: NOT SAVED", 0);
        break;
    default:
        break;
    }
}

/* ------------------------------------------------------- WHOLE-SCREEN MESSAGES (wip199) ---
 * A warning (warn(): something did not happen) and every question (wip205: the two editors SAVE AS and
 * CHAIN too) take the whole 240 x 240 screen: no header, no knobs, no legend under it. Plain say()
 * messages stay in the header. A warning shown now wins over a question; the question comes back. */
static int full_kind(void)                            /* 0 none, 1 a warning, 2 a question, 3 the COPY-PASTE MODE (wip423), 4 the TRIG % (wip433), 5 LOW BATTERY (wip605) */
{
    if (ui.lowbatt)
        return 5;
    if (ui.help)
        return 0;
    if (ui.msg_warn && ui.msg_until && (int32_t)(ui.msg_until - plat_ms()) > 0)
        return 1;
    if (pc_on) {                                     /* wip433: TRIG % turned: big on the whole screen */
        if (ui.overlay == O_NONE && held_any_step() >= 0 && ui.msg_until && (int32_t)(ui.msg_until - plat_ms()) > 0)
            return 4;                                /* wip722: only while the step is still held */
        pc_on = 0;
        ui.msg_until = 0;                            /* wip722: let go of the step = the TRIG % is gone at once (also the header text) */
    }
    if (ui.overlay == O_ASK)                         /* wip205: SAVE AS and CHAIN too */
        return 2;
    if (ui.cpm && tonic_part())                      /* wip423: COPY-PASTE MODE: the instructions take the whole screen, big */
        return 3;
    return 0;
}
static int strchr_colon(const char *s)                 /* wip433: a ':' in the text (a 1:2 condition, not a percentage) */
{
    while (*s)
        if (*s++ == ':')
            return 1;
    return 0;
}
static void fs_small(const char *s, int cy, uint16_t col)   /* wip340: the small lines under a big message: one line, or two cut at the space nearest the middle */
{
    char a[40], b[40];
    int n = 0, i, best = -1;
    while (s[n])
        n++;
    if (tw(&FONT_S, s) <= 216 || n > 38) {
        text_c(120, vc(&FONT_S, cy - 10, 20), &FONT_S, s, col);
        return;
    }
    for (i = 0; i < n; i++)
        if (s[i] == ' ' && (best < 0 || (i - n / 2 < 0 ? n / 2 - i : i - n / 2) < (best - n / 2 < 0 ? n / 2 - best : best - n / 2)))
            best = i;
    if (best < 0) {
        text_c(120, vc(&FONT_S, cy - 10, 20), &FONT_S, s, col);
        return;
    }
    for (i = 0; i < best; i++)
        a[i] = s[i];
    a[best] = 0;
    for (i = best + 1; i <= n; i++)
        b[i - best - 1] = s[i];
    text_c(120, vc(&FONT_S, cy - 22, 20), &FONT_S, a, col);
    text_c(120, vc(&FONT_S, cy + 2, 20), &FONT_S, b, col);
}
static void fs_msg(const char *s, int cy, uint16_t col)   /* one or two centred lines, as big as they fit in 216 px (wip340: a newline = the rest in small lines under) */
{
    const felucca_font_t *f = &FONT_M;
    char a[56], b[28];
    int n = 0, i, best = -1;
    while (s[n] && s[n] != '\n')
        n++;
    if (s[n] == '\n') {
        for (i = 0; i < n && i < 55; i++)
            a[i] = s[i];
        a[i] = 0;
        fs_msg(a, 84, col);
        fs_small(s + n + 1, 164, C_GRAY);
        return;
    }
    if (tw(f, s) <= 216) {
        text_c(120, vc(f, cy - 16, 32), f, s, col);
        return;
    }
    for (i = 0; i < n; i++)
        if (s[i] == ' ' && (best < 0 || (i - n / 2 < 0 ? n / 2 - i : i - n / 2) < (best - n / 2 < 0 ? n / 2 - best : best - n / 2)))
            best = i;
    if (best < 0) {
        text_c(120, vc(&FONT_B, cy - 16, 32), tw(&FONT_B, s) <= 216 ? &FONT_B : &FONT_S, s, col);
        return;
    }
    for (i = 0; i < best; i++)
        a[i] = s[i];
    a[best] = 0;
    for (i = best + 1; i <= n; i++)
        b[i - best - 1] = s[i];
    if (tw(f, a) > 216 || tw(f, b) > 216)
        f = &FONT_B;
    text_c(120, vc(f, cy - 36, 32), f, a, col);
    text_c(120, vc(f, cy + 4, 32), f, b, col);
}
static void fs_big(int cy, const felucca_font_t *f, const char *s, uint16_t col, int k)   /* text at k times, centred at (120, cy) */
{
    int ch = caps(f)->bot - caps(f)->top + 1;
    big_text(120 - big_w(f, s, k) / 2, cy - ch * k / 2 - caps(f)->top * k, f, s, col, k);
}
static void fs_editor(void)                           /* wip205: the SAVE AS screens and the CHAIN screen, 240 x 240 */
{
    uint16_t purple = C_TBOTH;
    char t[40], *q;
    const char *l, *r;
    if (ui.ask_act == ACT_SAVEAS) {
        int st = (int)ui.ask_arg;
        if (st == SV_HERE) {                          /* wip319: SAVE HERE? big (as the name was), the number and the name half size under it */
            const char *pn = ui.sv_pn[0] ? ui.sv_pn : "UNTITLED";
            q = t;
            if (ui.sv_here < 10)
                *q++ = '0';
            put_i(q, ui.sv_here);
            fs_big(30, &FONT_M, "SAVE HERE?", C_WHITE, big_w(&FONT_M, "SAVE HERE?", 2) <= 216 ? 2 : 1);   /* wip708: moved up: two big hint lines under the name */
            fs_big(66, &FONT_L, t, C_WHITE, 1);
            fs_big(92, &FONT_M, pn, C_HI, 1);
            if (ui.sv_here != ui.preset_cur) {        /* another place: who goes (amber), or an empty place */
                char u[40];
                q = put_s(u, ui.sv_occ[0] ? "REPLACES " : "EMPTY PLACE");
                if (ui.sv_occ[0])
                    put_s(q, ui.sv_occ);
                text_c(120, vc(&FONT_S, 108, 18), tw(&FONT_S, u) <= 216 ? &FONT_S : &FONT_XS, u, ui.sv_occ[0] ? C_AMB : C_GRAY);
            }
            text_c(120, vc(&FONT_B, 128, 22), tw(&FONT_B, "ALGORITHM: NEW NAME") <= 216 ? &FONT_B : &FONT_S, "ALGORITHM: NEW NAME", C_WHITE);   /* wip708: the two hints, FONT_B white (was XS gray) */
            text_c(120, vc(&FONT_B, 150, 22), tw(&FONT_B, "SELECT: OTHER SPOT") <= 216 ? &FONT_B : &FONT_S, "SELECT: OTHER SPOT", C_WHITE);
        } else if (st == SV_CHOOSE) {
            fs_big(86, &FONT_M, "CHOOSE SLOT", C_WHITE, big_w(&FONT_M, "CHOOSE SLOT", 2) <= 216 ? 2 : 1);
        } else if (st == 2) {                         /* the slot: the number big, what is in it under */
            text_c(120, vc(&FONT_B, 18, 30), &FONT_B, "SLOT", C_GRAY);
            put_i(t, ui.sv_here);
            fs_big(86, &FONT_L, t, C_WHITE, big_w(&FONT_L, t, 2) <= 200 ? 2 : 1);
            q = put_s(t, "TURN ALGORITHM  ");
            put_s(q, ui.sv_occ[0] ? ui.sv_occ : "EMPTY");
            text_c(120, vc(&FONT_S, 138, 20), tw(&FONT_S, t) <= 216 ? &FONT_S : &FONT_XS, t, ui.sv_occ[0] ? C_AMB : C_GRAY);
        } else if (st == 1 && proj.set.name_manual) {  /* wip324: the letter editor: the accepted letters white, the turning one blinks */
            const felucca_font_t *f = &FONT_M;
            int k = ui.sv_rename ? 1 : big_w(f, "MMMMMMMM", 2) <= 216 ? 2 : 1;   /* wip325: RENAME is the big title, the letters normal size */
            int ch = caps(f)->bot - caps(f)->top + 1;
            int y = (ui.sv_rename ? 100 : 86) - ch * k / 2 - caps(f)->top * k;
            int on = (int)((plat_ms() / 400u) & 1u);
            char cs[2];
            if (ui.sv_rename)
                fs_big(40, &FONT_M, "RENAME", C_WHITE, big_w(&FONT_M, "RENAME", 2) <= 216 ? 2 : 1);
            else
                text_c(120, vc(&FONT_B, 18, 30), &FONT_B, "NAME", C_GRAY);
            name_shown(t);
            cs[0] = name_cand();
            cs[1] = 0;
            q = t;                                    /* the whole width counts, so the letters do not move while it blinks */
            {
                int x0 = 120 - big_w(f, q, k) / 2;
                if (ui.nm_len)
                    big_text(x0, y, f, ui.sv_name, C_WHITE, k);
                if (ui.nm_len < NM_MAX && on)
                    big_text(x0 + big_w(f, ui.sv_name, k), y, f, cs, C_HI, k);
            }
            if (ui.nm_len && (on || ui.nm_len >= NM_MAX))        /* wip431: both messages bigger (PRESS SAVE in the name's font, TURN K4 one size up) */
                fs_big(124, &FONT_M, "PRESS SAVE", C_WHITE, 1);
            fs_big(152, &FONT_M, "TURN KNOB 4", C_GRAY, 1);          /* wip432: the same big font, KNOB 4 spelled out */
        } else {                                      /* the name: as big as it fits */
            if (ui.sv_rename && st == 1) {                /* wip325: RENAME big (FONT_M, x2 as the SAVE HERE? title), the name under it at normal size */
                fs_big(40, &FONT_M, "RENAME", C_WHITE, big_w(&FONT_M, "RENAME", 2) <= 216 ? 2 : 1);
                fs_big(100, &FONT_M, ui.sv_name, C_HI, 1);
            } else {
                text_c(120, vc(&FONT_B, 18, 30), &FONT_B, st == 0 ? "SAVE AS" : ui.sv_rename ? "RENAME" : "NAME", st == 0 ? C_WHITE : C_GRAY);
                fs_big(86, &FONT_M, ui.sv_name, st == 0 ? C_HI : C_WHITE, big_w(&FONT_M, ui.sv_name, 2) <= 216 ? 2 : 1);
            }
            text_c(120, vc(&FONT_B, 138, 32), tw(&FONT_B, "ALGORITHM: NEW NAME") <= 216 ? &FONT_B : &FONT_S, "ALGORITHM: NEW NAME", C_WHITE);   /* wip248: bigger, white */
        }
        l = st == 1 && proj.set.name_manual && ui.nm_len ? "OCT-  DELETE" : st == SV_HERE ? "OCT-  CANCEL" : st == SV_CHOOSE ? "OCT-  NO, EXIT" : st == 0 || ui.sv_rename ? "OCT-  CANCEL" : "OCT-  BACK";
        r = st == 1 && proj.set.name_manual ? "OCT+  OK" : st == SV_HERE ? "OCT+  YES" : st == SV_CHOOSE ? "OCT+  CHOOSE SLOT" : st == 2 ? "OCT+  CONFIRM" : st == 1 ? "OCT+  SAVE" : "OCT+  OK";
    } else {                                          /* CHAIN: the chain as numbered squares, 8 to a row, the last 16 */
        int i, first = ui.ch_n > 16 ? ui.ch_n - 16 : 0;
        text_c(120, vc(&FONT_B, 14, 30), &FONT_B, "PATTERNS", C_WHITE);
        if (!ui.ch_n)
            text_c(120, vc(&FONT_B, 80, 30), &FONT_B, "PRESS 1-16", C_GRAY);
        for (i = first; i < ui.ch_n; i++) {
            int n = i - first, x = 13 + (n & 7) * 27, y = 54 + (n >> 3) * 40;
            char b[4];
            put_i(b, ui.ch_pat[i] + 1);
            box(x, y, 24, 34, i == ui.ch_n - 1 ? purple : C_GRAY);
            text_c(x + 12, vc(&FONT_S, y, 34), &FONT_S, b, C_BLACK);
        }
        put_i(put_s(t, "STEPS: "), ui.ch_n);
        text_c(120, vc(&FONT_S, 138, 20), &FONT_S, t, C_GRAY);
        if (ui.ch_n)                                    /* wip221: the hint for FUNC + a white key */
            text_c(120, vc(&FONT_XS, 157, 14), &FONT_XS, "FUNC + KEY: LAST OFF", C_GRAY);
        l = ui.ch_n ? "OCT-  DEL" : "OCT-  CANCEL";
        r = "OCT+  OK";
    }
    frame(14, 176, 100, 34, C_GRAY);
    text_c(64, vc(&FONT_B, 176, 34), btnfont(l, 92), l, C_GRAY);
    box(126, 176, 100, 34, purple);
    text_c(176, vc(&FONT_B, 176, 34), btnfont(r, 92), r, C_BLACK);
    if (ui.ask_act == ACT_SAVEAS)       /* wip266: FUNC + SAVE in an empty place: HOME leaves */
        text_c(120, vc(&FONT_S, 212, 24), &FONT_S, "[HOME] EXIT", C_GRAY);
}
static void fs_body(int kind)
{
    uint16_t purple = kind == 3 ? C_SLBLUE : kind == 5 ? RGB(222, 58, 58) : C_TBOTH;   /* wip423: the copy-paste screen is in our blue like the hints; wip605: LOW BATTERY is red */
    cv_rect(0, 0, 240, 4, purple);
    cv_rect(0, 236, 240, 4, purple);
    cv_rect(0, 0, 4, 240, purple);
    cv_rect(236, 0, 4, 240, purple);
    if (kind == 1) {
        fs_msg(ui.msg, 120, C_WHITE);
        return;
    }
    if (kind == 5) {                                  /* wip605: LOW BATTERY: big and red, what to do under it, how to leave */
        fs_msg("LOW BATTERY", 70, RGB(222, 58, 58));
        text_c(120, vc(&FONT_B, 104, 30), &FONT_B, "(PLUG YOUR DEVICE)", C_WHITE);
        text_c(120, vc(&FONT_B, 172, 30), &FONT_B, "PRESS HOME TO EXIT", C_SLBLUE);
        return;
    }
    if (kind == 4) {                                  /* wip433: TRIG %: the value as big as it fits, the step under it */
        char st[16];
        int cond = strchr_colon(pc_big);                 /* a 1:2 condition: FONT_M has the colon, FONT_L (the numbers font) has not */
        const felucca_font_t *f = cond ? &FONT_M : &FONT_L;
        int k = big_w(f, pc_big, 3) <= 216 ? 3 : big_w(f, pc_big, 2) <= 216 ? 2 : 1;
        put_i(put_s(st, "STEP "), pc_step);
        text_c(120, vc(&FONT_B, 22, 30), &FONT_B, cond ? "TRIG CONDITION" : "TRIG %", C_GRAY);
        fs_big(114, f, pc_big, C_WHITE, k);
        text_c(120, vc(&FONT_B, 184, 30), &FONT_B, st, C_GRAY);
        return;
    }
    if (kind == 3) {                                  /* wip423: two big instructions, one above the other */
        fs_msg("PRESS PATTERN TO PASTE", 64, C_SLBLUE);
        fs_msg("PRESS SEQ TO EXIT", 172, C_SLBLUE);
        return;
    }
    if (ui.ask_act == ACT_SAVEAS || ui.ask_act == ACT_CHAIN) {
        fs_editor();
        return;
    }
    {
        const char *l, *r;
        int stop = ui.ask_act == ACT_STOPFIRST, pre = ui.ask_act == ACT_PRESET || ui.ask_act == ACT_PRESETGO;
        if (pre) {                                        /* the number and name big and purple, the question small (F6) */
            const felucca_font_t *pf = tw(&FONT_M, ui.ask_q[1]) <= 216 ? &FONT_M : tw(&FONT_B, ui.ask_q[1]) <= 216 ? &FONT_B : &FONT_S;
            text_c(120, vc(&FONT_B, 24, 30), &FONT_B, ui.ask_q[0], C_GRAY);
            text_c(120, vc(pf, 70, 56), pf, ui.ask_q[1], purple);
            if (ui.ask_act == ACT_PRESET)
                text_c(120, vc(&FONT_S, 138, 20), &FONT_S, "PRESETS KNOB: ANOTHER ONE", C_GRAY);
        } else if (stop) {
            fs_msg("STOP FIRST", 78, C_WHITE);
            text_c(120, vc(&FONT_B, 112, 30), &FONT_B, "BEFORE SAVING", C_GRAY);
        } else if (ui.ask_act == ACT_QSAVE) {         /* wip308: the whole title in the big font ('?' now in it), the slot name big and purple */
            const felucca_font_t *pf = tw(&FONT_M, ui.ask_q[1]) <= 216 ? &FONT_M : tw(&FONT_B, ui.ask_q[1]) <= 216 ? &FONT_B : &FONT_S;
            text_c(120, vc(&FONT_M, 40, 32), &FONT_M, ui.ask_q[0], C_WHITE);
            if (ui.ask_q[1][0])
                text_c(120, vc(pf, 90, 40), pf, ui.ask_q[1], purple);
        } else {
            fs_msg(ui.ask_q[0], ui.ask_q[1][0] ? 78 : 100, C_WHITE);
            if (ui.ask_q[1][0])
                text_c(120, vc(&FONT_B, 116, 30), tw(&FONT_B, ui.ask_q[1]) <= 216 ? &FONT_B : &FONT_S, ui.ask_q[1], C_GRAY);
        }
        l = stop ? "OCT-  BACK" : ui.ask_act == ACT_QSAVE ? "OCT-  EXIT" : ui.ask_act == ACT_PRESETGO || ui.ask_act == ACT_PASTE_TNPAT ? "OCT-  NO" : ui.ask_act == ACT_PRESET || octask() ? "OCT-  GO BACK" : "HOME  NO";
        r = stop ? "PLAY  STOP" : (ui.ask_act == ACT_PRESETGO || ui.ask_act == ACT_QSAVE || ui.ask_act == ACT_PASTE_TNPAT) ? "OCT+  YES" : ui.ask_act == ACT_PRESET ? "OCT+  UPLOAD" : octask() ? "OCT+  CONFIRM" : "SEL  YES";
        frame(14, 176, 100, 34, C_GRAY);
        text_c(64, vc(&FONT_B, 176, 34), &FONT_B, l, C_GRAY);
        box(126, 176, 100, 34, purple);
        text_c(176, vc(&FONT_B, 176, 34), &FONT_B, r, C_BLACK);
        if (stop && ui.sv_fixed)                  /* wip266: FUNC + SAVE screens: HOME leaves */
            text_c(120, vc(&FONT_S, 212, 24), &FONT_S, "[HOME] EXIT", C_GRAY);
    }
}
static void draw_fullscreen(int kind)
{
    static const uint8_t reg[3] = {0, 4, 5};
    static const uint8_t y0[3] = {0, 104, 208};
    static const uint8_t hh[3] = {104, 104, 32};
    int k;
    for (k = 0; k < 3; k++) {
        cv_begin(240, hh[k], C_BG);
        cv_oy = -(int)y0[k];
        fs_body(kind);
        cv_oy = 0;
        cv_commit(reg[k], 0, y0[k]);
    }
}

/* wip386 (MIDI step S3): THE CC TABLE. A control change is a knob turned from outside: the CC number = the parameter (the same
 * numbers for every track), the track = the MIDI channel (CC MODE = CHANNELS: channel base + t = track t + 1) or the SELECTED track
 * (CC MODE = SELECTED, on the Tonic's channel). The value goes through pref_set(), the SAME setter a knob uses, so the screen, the
 * project and the autosave follow; a CC does not zoom, does not lock a step and is not recorded as motion; the classic engine's
 * extras (sends, cuts) are ignored like their greyed knobs. CC 0-127 = the knob's 0-127; a 3-way switch splits the range in three. */
#define CC_LOCUT 250
#define CC_HICUT 251
static const struct { uint8_t cc, i; } CC_TAB[] = {
    {7, TNP_LEVEL}, {10, TN_POT_PAN},
    {20, TNP_OSCWAVE}, {21, TNP_OSCFREQ}, {22, TNP_OSCATK}, {23, TNP_OSCDCY},
    {24, TNP_MODMODE}, {25, TNP_MODAMT}, {26, TNP_MODRATE},
    {27, TNP_NFILMOD}, {28, TNP_NFILQ}, {29, TNP_NFILFRQ},
    {30, TNP_NENVMOD}, {31, TNP_NENVATK}, {102, TNP_NENVDCY},
    {103, TNP_MIX}, {104, TNP_DIST}, {105, TNP_EQFREQ}, {106, TNP_EQGAIN},
    {107, TNP_OSCVEL}, {108, TNP_NVEL}, {109, TNP_MODVEL},
    {110, CC_LOCUT}, {111, CC_HICUT}, {112, TN_POT_RSND}, {113, TN_POT_DSND},
};
static void cc_apply(int track, int cc, int val)
{
    unsigned k;
    pref_t r;
    const x0x_param_t *d;
    int v;
    for (k = 0; k < sizeof CC_TAB / sizeof CC_TAB[0] && CC_TAB[k].cc != cc; k++)
        ;
    if (k == sizeof CC_TAB / sizeof CC_TAB[0])
        return;                                        /* not in the table: ignored */
    r = CC_TAB[k].i == CC_LOCUT ? PR(R_SHLO, track, 0, 0) : CC_TAB[k].i == CC_HICUT ? PR(R_SHHI, track, 0, 0) : PR(R_ENG, T_909, track, CC_TAB[k].i);
    d = pref_desc(r);
    if (!d || extra_off(r))
        return;                                        /* the classic engine: no message, a CC flood must not talk */
    v = d->max >= 127 ? val : val * ((int)d->max + 1) / 128;
    if (pref_get(r) != v)                              /* an unchanged value marks nothing (a controller sends the same value often) */
        pref_set(r, v);
}

static uint16_t mq_n;                                  /* wip384: MIDI queue, control changes taken */
static uint8_t mq_cc, mq_val;
int ui_midi_seen(int what) { return what == 0 ? mq_n : what == 1 ? mq_cc : mq_val; }
static void midi_queue_run(void)                       /* wip384 (S1): the queue's reader; S3 puts the CC table here */
{
    uint32_t pkt;
    int guard = 32;
    int rem_on = proj.set.midi_ch[4] >= 1 && proj.set.midi_ch[4] <= 16;
    if (!rem_on)
        rkeys = 0;                                     /* remote switched off with keys down: all up */
    while (guard-- && engine_midi_pop(&pkt)) {
        uint32_t st = (pkt >> 8) & 0xFF, d1 = (pkt >> 16) & 0x7F, d2 = (pkt >> 24) & 0x7F, ty = st & 0xF0;
        if (rem_on && (st & 0x0F) == (uint32_t)proj.set.midi_ch[4] - 1u && (ty == 0x90 || ty == 0x80 || (ty == 0xB0 && (d1 == 120 || d1 == 123)))) {
            if (ty == 0xB0) {
                rkeys = 0;                             /* all notes off */
            } else if (d1 >= REMOTE_NOTE0 && d1 < REMOTE_NOTE0 + NKEYS) {
                uint32_t m = 1u << (d1 - REMOTE_NOTE0);
                if (ty == 0x90 && d2)
                    rkeys |= m;                        /* note on = key down, note off (or velocity 0) = key up */
                else
                    rkeys &= ~m;
            }
            continue;
        }
        if (ty != 0xB0)
            continue;
        mq_n++;
        mq_cc = (uint8_t)d1;
        mq_val = (uint8_t)d2;
        {                                              /* wip386: which track: the channel block wins, then the Tonic's channel = the selected track */
            int ch = (int)(st & 0x0Fu), tr = -1, mode = proj.set.cc_mode, base = proj.set.cc_base;
            if ((mode & 2) && ch >= base && ch < base + 8)
                tr = ch - base;
            else if ((mode & 1) && seq.ch[0] == ch)
                tr = ui.sel[PART_909];
            if (tr >= 0 && tr < 8)
                cc_apply(tr, (int)d1, (int)d2);
        }
    }
}

void ui_frame(void)
{
    uint32_t i;
    midi_queue_run();                                  /* wip385: first, so the remote keys are in this frame's input() */
    batt_update();                                     /* wip605 */
    input();
    note_groups();                                     /* wip696 */
    if (ui.preset_pend && !seq.playing && !seq.req_stop) {   /* the preset chosen while playing: the sequencer has stopped, load it */
        int id = ui.preset_pend - 1;
        ui.preset_pend = 0;
        run_action(ACT_PRESET, id);
    }
    if (ui.ca_hold) {                                  /* wip632: FUNC + PLAY held 3 s since CLEAR PATTERN? came: it turns into CLEAR ALL PATTERNS? */
        if (ui.overlay != O_ASK || ui.ask_act != ACT_CLEAR_TNPAT || !(ui.btn & (1u << B_PLAY)) || !(ui.keys & (1u << BLACK_KEY[10])))
            ui.ca_hold = 0;
        else if ((int32_t)(plat_ms() - ui.down_t[B_PLAY]) >= 3000) {
            ui.ca_hold = 0;
            ask(ACT_CLEAR_ALL, 0, "CLEAR ALL PATTERNS?", "[OCT-] NO [OCT+] YES");
        }
    }
    undo_tick();
    autosave();
    perf_test_tick();
    if (ui.view == V_PERF && !perf_testing())
        perf_window(0);
    for (i = 0; i < NPARTS; i++)
        eng_peak[i] = (uint16_t)(eng_peak[i] - (eng_peak[i] >> 3));
    for (i = 0; i < NKIT * NDRUM; i++) {              /* the drum tracks' hits fall as the part meters do */
        volatile uint8_t *h = &eng_hit[i / NDRUM][i % NDRUM];
        *h = (uint8_t)(*h - (*h >> 3) - (*h ? 1 : 0));
    }
    {   /* entering or leaving the quarter layout: the whole screen is sent again */
        static int was_q;
        int q = quarter_view(), k;
        if (q != was_q) {
            was_q = q;
            for (k = 0; k < 6; k++)
                blit_hash[k] = 0;
        }
    }
    if (proj.set.autoscroll && seq.playing && is_drum() && (ui.view == V_PART || ui.view == V_SEQ) && ui.held_step < 0) {   /* AUTOSCROLL: the step page follows the playhead */
        int pgp = eng_step[TRK_DRUM + ui.part] / 16;
        if (pgp > NSTEPS / 16 - 1)
            pgp = NSTEPS / 16 - 1;
        ui.spage = (uint8_t)pgp;
    } else if (is_drum() && (ui.view == V_PART || ui.view == V_SEQ) && ui.spage >= spage_pages()) {
        ui.spage = (uint8_t)(spage_pages() - 1);      /* wip583: never an empty page: a 16-step loop cannot stay on 17-32 (SEQ, ARP, EDIT; autoscroll off, or stopped) */
    }
    {   /* wip199: a warning / a question takes the whole screen; entering or leaving it, every region is sent again */
        static int was_fs;
        int fk = full_kind(), k;
        if ((fk != 0) != was_fs) {
            was_fs = fk != 0;
            for (k = 0; k < 6; k++)
                blit_hash[k] = 0;
        }
        if (fk)
            draw_fullscreen(fk);
        else {
            draw_header();
            draw_main();
            draw_knobs();
        }
    }
    leds();
    speak();
    ann_tick(plat_ms());
    ui.frame++;
}

void ui_input_only(void) { input(); }
int ui_dirty(void) { return ui.dirty != 0; }
int ui_lowbatt(void) { return ui.lowbatt != 0; }
int ui_msmode(void) { return ui.msmode; }
void ui_say(const char *a, const char *b) { say(a, b); }
void ui_warn(const char *a, const char *b) { warn(a, b); }

/* ---- backup and restore over USB (backup.c) ---- */
/* a backup, stopped: what is not saved yet saved first (the flash is what it sends); ui_dirty() after it says
 * whether it worked */
void ui_flush(void)
{
    if (!seq.playing && !seq.req_stop && ui.dirty && project_autosave() == 0) {
        ui.dirty = ui.set_dirty = 0;
        save_wait = 0;
    }
}
void ui_hold_saves(int on) { save_hold = (uint8_t)(on != 0); }
void ui_save_now(void) { save_project(); }
void ui_redraw(void)
{
    int i;
    lcd_fill(0, 0, 240, 240, C_BG);
    for (i = 0; i < 6; i++)
        blit_hash[i] = 0;
}
/* a restore wrote the project: all of it from the flash again, as at power-on (stopped: a restore is refused while
 * playing); undo starts over from it */
void app_restored(void)
{
    project_flash_changed();
    if (project_load() != 0)
        project_defaults();                      /* (as at power-on: a project that does not load whole) */
    engine_apply_sound(&proj.sound);
    seq.bpm = (float)proj.set.bpm_x10 / 10.0f;
    seq.accent_q7 = proj.set.accent_q7;
    project_apply_io();
    engine_motion_reset();
    engine_brk_loops();
    save_wait = 0;
    ui.dirty = ui.set_dirty = 0;
    ui.song_sel = 0;
    ui_redraw();
    ui_init();
}
