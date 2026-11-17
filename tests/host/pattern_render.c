/* host tool: render a Microtonic preset's pattern with the Tonic engine -> stereo 32-bit float WAV.
 * usage: pattern_render preset.mtpreset out.wav [pattern letter=a] [bars=2]
 * env: SWK = swing scale (default 1.0), VNORM / VACC = velocities (defaults TN_VEL_NORMAL / TN_VEL_ACCENT) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "dsp/tonic.h"
extern po32_status_t po32_patch_parse_mtdrum_text(const char*, size_t, po32_patch_params_t*);
static const char *KEYS[21] = {"OscWave","OscFreq","OscAtk","OscDcy","ModMode","ModRate","ModAmt","NFilMod","NFilFrq","NFilQ","NEnvMod","NEnvAtk","NEnvDcy","Mix","DistAmt","EQFreq","EQGain","Level","OscVel","NVel","ModVel"};
static void wavf(const char *fn, const float *x, int n)
{
    FILE *f = fopen(fn, "wb"); uint32_t sr = 44100, db = n * 8, v; uint16_t s;
    fwrite("RIFF", 1, 4, f); v = 36 + db; fwrite(&v, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    s = 3; fwrite(&s, 2, 1, f); s = 2; fwrite(&s, 2, 1, f); fwrite(&sr, 4, 1, f); v = sr * 8; fwrite(&v, 4, 1, f);
    s = 8; fwrite(&s, 2, 1, f); s = 32; fwrite(&s, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&db, 4, 1, f);
    fwrite(x, 4, 2 * n, f); fclose(f);
}
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb"); static char buf[200000]; size_t n = fread(buf, 1, sizeof buf - 1, f); buf[n] = 0; fclose(f);
    char pat = argc > 3 ? argv[3][0] : 'a'; int bars = argc > 4 ? atoi(argv[4]) : 2;
    double tempo = atof(strstr(buf, "Tempo = ") + 8), swing = atof(strstr(buf, "Swing = ") + 8);
    double swk = getenv("SWK") ? atof(getenv("SWK")) : 0.5;
    int vn = getenv("VNORM") ? atoi(getenv("VNORM")) : TN_VEL_NORMAL, va = getenv("VACC") ? atoi(getenv("VACC")) : TN_VEL_ACCENT;
    char *dp = strstr(buf, "DrumPatches:");
    static tonic_t T; tonic_init(&T, 44100);
    for (int v = 0; v < 8; v++) {
        char key[16]; snprintf(key, sizeof key, "\t\t%d: {", v + 1);
        char *s = strstr(dp, key), *e = strstr(s, "\n\t\t}"), txt[4096] = ""; double pan = 0; static po32_patch_params_t PP;
        *e = 0;
        for (int k = 0; k < 21; k++) {
            char p[32], *l; snprintf(p, sizeof p, "\t\t\t%s = ", KEYS[k]);
            l = strstr(s, p); if (!l) continue; l += strlen(p);
            char *le = strpbrk(l, "\r\n"); char val[64]; snprintf(val, sizeof val, "%.*s", (int)(le - l), l);
            strcat(txt, KEYS[k]); strcat(txt, ": "); strcat(txt, val); strcat(txt, "\n");
        }
        char *pp = strstr(s, "\t\t\tPan = "); if (pp) pan = atof(pp + 9);
        *e = '\n';
        po32_patch_parse_mtdrum_text(txt, strlen(txt), &PP);
        float *fp = &PP.OscWave;
        for (int i = 0; i < 21; i++) tonic_set(&T, v, i, (uint16_t)(fp[i] * 65535.0f + 0.5f));
        tonic_set_pan(&T, v, (int)(64 + pan * 0.63 + 0.5));
    }
    /* pattern: "\t\t<pat>: {" ... per voice "\t\t\t<v>: {" Accents / Triggers */
    char pk[16]; snprintf(pk, sizeof pk, "\n\t\t%c: {", pat);
    char *ps = strstr(strstr(buf, "Patterns:"), pk), *pe = strstr(ps + 1, "\n\t\t}");
    int len = atoi(strstr(ps, "Length = ") + 9);
    char trig[8][65], acc[8][65];
    for (int v = 0; v < 8; v++) {
        char vk[16]; snprintf(vk, sizeof vk, "\n\t\t\t%d: {", v + 1);
        char *vs = strstr(ps, vk), *a = strstr(vs, "Accents = ") + 10, *t = strstr(vs, "Triggers = ") + 11;
        memset(trig[v], '-', 64); memset(acc[v], '-', 64); trig[v][64] = acc[v][64] = 0;
        for (int i = 0; i < len; i++) { trig[v][i] = t[i]; acc[v][i] = a[i]; }
    }
    (void)pe;
    double step = 60.0 / tempo / 4.0 * 44100.0;
    int total = (int)(bars * 4 * 60.0 / tempo * 44100.0 + 0.5);
    float *L = calloc(total + 44100 * 4, 4), *R = calloc(total + 44100 * 4, 4);
    int nsteps = bars * 16, pos = 0;
    for (int s = 0; s < nsteps; s++) {
        double off = (s & 1) ? swk * swing / 100.0 * step : 0.0;
        int at = (int)(s * step + off + 0.5), idx = s % len;
        if (at > pos) { tonic_render_st(&T, L + pos, R + pos, at - pos); pos = at; }
        for (int v = 0; v < 8; v++) if (trig[v][idx] == '#') tonic_trigger(&T, v, acc[v][idx] == '#' ? va : vn);
    }
    tonic_render_st(&T, L + pos, R + pos, total - pos);
    float *o = malloc(8 * total); double pk2 = 0, rms = 0;
    for (int i = 0; i < total; i++) { o[2*i] = L[i]; o[2*i+1] = R[i]; if (fabs(L[i]) > pk2) pk2 = fabs(L[i]); if (fabs(R[i]) > pk2) pk2 = fabs(R[i]); rms += L[i]*L[i] + R[i]*R[i]; }
    wavf(argv[2], o, total);
    printf("tempo %.1f swing %.2f pattern %c len %d: %d frames, peak %.3f (%.1f dBFS), rms %.1f dBFS\n", tempo, swing, pat, len, total, pk2, 20*log10(pk2 + 1e-9), 10*log10(rms / (2.0 * total) + 1e-12));
    return 0;
}
