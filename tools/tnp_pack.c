/* SPDX-License-Identifier: GPL-3.0-only */
/* tnp_pack: Microtonic .mtpreset (text) -> TNP1 binary (firmware/src/app/tnp.h).
 * usage: tnp_pack in.mtpreset out.tnp [NAME]   (NAME replaces the name written inside the preset)
 * Build (needs libpo32, MIT, for the parameter normalisation, the same code the engine was fitted with):
 *   gcc -O2 -I firmware/src -I <libpo32>/core/include -o build/tnp_pack tools/tnp_pack.c \
 *       <libpo32>/core/src/po32_patch_import.c <libpo32>/core/src/po32.c -lm */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "po32.h"
#include "app/tnp.h"

extern po32_status_t po32_patch_parse_mtdrum_text(const char *, size_t, po32_patch_params_t *);
static const char *KEYS[21] = {"OscWave", "OscFreq", "OscAtk", "OscDcy", "ModMode", "ModRate", "ModAmt", "NFilMod", "NFilFrq", "NFilQ", "NEnvMod",
                               "NEnvAtk", "NEnvDcy", "Mix", "DistAmt", "EQFreq", "EQGain", "Level", "OscVel", "NVel", "ModVel"};
enum { RATE_16, RATE_16T, RATE_32, RATE_8T };   /* pattern.h */

static uint8_t out[TNP_SIZE(TNP_NPAT_MAX)];
static char dtxt[8][4096];
static double dpan[8];
static int have_pat[TNP_NPAT_MAX];

static void p16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void p32(uint8_t *p, uint32_t v) { p16(p, v & 0xFFFFu); p16(p + 2, v >> 16); }

static void bits(const char *s, uint32_t *w, int len)
{
    int i;
    w[0] = w[1] = 0;
    for (i = 0; i < len && i < 64 && s[i] && s[i] != '\r' && s[i] != '\n'; i++)
        if (s[i] == '#') w[i >> 5] |= 1u << (i & 31);
}

int main(int argc, char **argv)
{
    static char buf[300000];
    char path[8][48], *line, *save = 0;
    int depth = 0, npat = 0, i, v;
    double tempo = 120, swing = 0, fillrate = 0, mast = 0;
    char name[64] = "", rate[3000] = "1/16";
    int mutes = 0;
    FILE *f;
    size_t n;
    if (argc < 3) { fprintf(stderr, "usage: tnp_pack in.mtpreset out.tnp\n"); return 2; }
    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot read %s\n", argv[1]); return 1; }
    n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = 0;
    if (strncmp(buf, "MicroTonicPresetV3", 18)) { fprintf(stderr, "%s: not a MicroTonicPresetV3 file\n", argv[1]); return 1; }
    memset(out, 0, sizeof out);
    for (line = strtok_r(buf, "\r\n", &save); line; line = strtok_r(0, "\r\n", &save)) {
        char *s = line, *eq, key[48], val[3000];
        size_t l;
        while (*s == ' ' || *s == '\t') s++;
        l = strlen(s);
        while (l && (s[l - 1] == ' ' || s[l - 1] == '\t')) s[--l] = 0;
        if (!l) continue;
        if (s[0] == '}') { if (depth > 0) depth--; continue; }
        if (l >= 2 && s[l - 1] == '{') {                       /* "name: {" (or "Name: {") opens a block */
            char *c = strchr(s, ':');
            size_t k = c ? (size_t)(c - s) : l - 1;
            if (k >= sizeof path[0]) k = sizeof path[0] - 1;
            if (depth < 8) { memcpy(path[depth], s, k); path[depth][k] = 0; depth++; }
            continue;
        }
        eq = strstr(s, " = ");
        if (!eq) continue;
        snprintf(key, sizeof key, "%.*s", (int)(eq - s), s);
        snprintf(val, sizeof val, "%s", eq + 3);
        if (depth == 1) {                                      /* inside MicroTonicPresetV3 */
            if (!strcmp(key, "Tempo")) tempo = atof(val);
            else if (!strcmp(key, "Swing")) swing = atof(val);
            else if (!strcmp(key, "FillRate")) fillrate = atof(val);
            else if (!strcmp(key, "MastVol")) mast = atof(val);
            else if (!strcmp(key, "Name")) snprintf(name, sizeof name, "%.*s", (int)strcspn(val + (val[0] == '"'), "\""), val + (val[0] == '"'));
            else if (!strcmp(key, "StepRate")) snprintf(rate, sizeof rate, "%s", val);
        } else if (depth == 3 && !strcmp(path[1], "DrumPatches")) {      /* [0] preset, [1] DrumPatches, [2] voice */
            v = atoi(path[2]) - 1;
            if (v < 0 || v > 7) continue;
            if (!strcmp(key, "Pan")) dpan[v] = atof(val);
            for (i = 0; i < 21; i++)
                if (!strcmp(key, KEYS[i])) { char t[3100]; snprintf(t, sizeof t, "%s: %s\n", key, val); strcat(dtxt[v], t); }
        } else if (depth == 2 && !strcmp(path[1], "Mutes")) {
        } else if (depth == 3 && !strcmp(path[1], "Patterns")) {         /* [2] = pattern letter */
            int pi = path[2][0] - 'a';
            if (pi < 0 || pi >= (int)TNP_NPAT_MAX) continue;
            if (!strcmp(key, "Length")) { out[TNP_HDR + 8u * TNP_VOICE + (unsigned)pi * TNP_PAT] = (uint8_t)atoi(val); have_pat[pi] = 1; }
            else if (!strcmp(key, "Chained")) out[TNP_HDR + 8u * TNP_VOICE + (unsigned)pi * TNP_PAT + 1] = !strcmp(val, "true");
        } else if (depth == 4 && !strcmp(path[1], "Patterns")) {         /* [3] = voice: Accents / Fills / Triggers */
            int pi = path[2][0] - 'a', pv = atoi(path[3]) - 1;
            uint8_t *pp;
            uint32_t w[2];
            int len;
            if (pi < 0 || pi >= (int)TNP_NPAT_MAX || pv < 0 || pv > 7) continue;
            pp = out + TNP_HDR + 8u * TNP_VOICE + (unsigned)pi * TNP_PAT;
            len = pp[0] ? pp[0] : 16;
            bits(val, w, len);
            if (!strcmp(key, "Triggers")) { p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv, w[0]); p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv + 4, w[1]); }
            else if (!strcmp(key, "Accents")) { p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv + 8, w[0]); p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv + 12, w[1]); }
            else if (!strcmp(key, "Fills")) { p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv + 16, w[0]); p32(pp + TNP_PAT_HDR + 24u * (unsigned)pv + 20, w[1]); }
        }
        if (depth == 2 && !strcmp(path[1], "Mutes")) {}
        (void)mutes;
    }
    for (i = 0; i < (int)TNP_NPAT_MAX; i++) if (have_pat[i]) npat = i + 1;
    if (!npat) { fprintf(stderr, "no patterns found\n"); return 1; }
    if (npat > (int)TNP_NPAT_LOAD) { fprintf(stderr, "(%d patterns found: the firmware keeps the first %u, so only those are packed)\n", npat, TNP_NPAT_LOAD); npat = (int)TNP_NPAT_LOAD; }
    for (i = 0; i < npat; i++) if (!have_pat[i]) { fprintf(stderr, "pattern %c missing (patterns must run a, b, c ...)\n", 'a' + i); return 1; }
    for (v = 0; v < 8; v++) {
        po32_patch_params_t PP;
        const float *fp = &PP.OscWave;
        uint8_t *o = out + TNP_HDR + (unsigned)v * TNP_VOICE;
        int pan;
        if (po32_patch_parse_mtdrum_text(dtxt[v], strlen(dtxt[v]), &PP) != PO32_OK) { fprintf(stderr, "drum %d: parameters did not parse\n", v + 1); return 1; }
        for (i = 0; i < 21; i++) p16(o + 2 * i, (uint32_t)(fp[i] * 65535.0f + 0.5f));
        pan = (int)(64 + dpan[v] * 0.63 + (dpan[v] < 0 ? -0.5 : 0.5));
        o[42] = (uint8_t)(pan < 0 ? 0 : pan > 127 ? 127 : pan);
    }
    memcpy(out, TNP_MAGIC, 4);
    p16(out + 4, TNP_SIZE((unsigned)npat));
    p16(out + 6, (uint32_t)(tempo * 10 + 0.5));
    out[8] = (uint8_t)(swing < 0 ? 0 : swing > 100 ? 100 : (int)(swing + 0.5));
    out[9] = (uint8_t)npat;
    if (!strcmp(rate, "1/16")) out[10] = RATE_16;
    else if (!strcmp(rate, "1/32")) out[10] = RATE_32;
    else if (!strcmp(rate, "1/8T")) out[10] = RATE_8T;
    else if (!strcmp(rate, "1/16T")) out[10] = RATE_16T;
    else { fprintf(stderr, "warning: step rate %s is not supported, 1/16 used\n", rate); out[10] = RATE_16; }
    p16(out + 12, (uint32_t)(fillrate * 100 + 0.5));
    p16(out + 14, (uint32_t)(int16_t)(mast * 10 + (mast < 0 ? -0.5 : 0.5)));
    if (argc > 3 && argv[3][0]) snprintf(name, sizeof name, "%s", argv[3]);   /* a name given on the command line wins */
    if (strlen(name) > 8) {                                    /* wip320: 8 letters at most (as the made-up names of SAVE): drop a trailing " (121)" tag, then cut */
        char *tag = strrchr(name, '(');
        if (tag && tag > name && tag[-1] == ' ') tag[-1] = 0;
        if (strlen(name) > 8) name[8] = 0;
        for (size_t k = strlen(name); k > 0 && name[k - 1] == ' '; k--) name[k - 1] = 0;
    }
    snprintf((char *)out + 16, TNP_NAME_LEN, "%s", name);
    f = fopen(argv[2], "wb");
    if (!f) { fprintf(stderr, "cannot write %s\n", argv[2]); return 1; }
    fwrite(out, 1, TNP_SIZE((unsigned)npat), f);
    fclose(f);
    printf("%s: \"%s\" tempo %.1f swing %.2f, %d patterns, %u bytes\n", argv[2], name, tempo, swing, npat, (unsigned)TNP_SIZE((unsigned)npat));
    return 0;
}
