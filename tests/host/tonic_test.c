/* host test: Rostoma preset -> 8 one-shots; tonic.c (chunked, 8 voices) vs libpo32's one-shot render */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "dsp/tonic.h"
extern po32_status_t po32_patch_parse_mtdrum_text(const char*, size_t, po32_patch_params_t*);
static const char *KEYS[21] = {"OscWave","OscFreq","OscAtk","OscDcy","ModMode","ModRate","ModAmt","NFilMod","NFilFrq","NFilQ","NEnvMod","NEnvAtk","NEnvDcy","Mix","DistAmt","EQFreq","EQGain","Level","OscVel","NVel","ModVel"};
static void wav(const char *fn, const float *x, int n, int ch)
{
    FILE *f = fopen(fn, "wb"); uint32_t sr = 44100, db = n * ch * 2, v;
    uint16_t s;
    fwrite("RIFF", 1, 4, f); v = 36 + db; fwrite(&v, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    s = 1; fwrite(&s, 2, 1, f); s = ch; fwrite(&s, 2, 1, f); fwrite(&sr, 4, 1, f); v = sr * ch * 2; fwrite(&v, 4, 1, f);
    s = ch * 2; fwrite(&s, 2, 1, f); s = 16; fwrite(&s, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&db, 4, 1, f);
    for (int i = 0; i < n * ch; i++) { float a = x[i] * 32767.f; if (a > 32767) a = 32767; if (a < -32768) a = -32768; int16_t q = (int16_t)a; fwrite(&q, 2, 1, f); }
    fclose(f);
}
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb"); static char buf[100000]; size_t n = fread(buf, 1, sizeof buf - 1, f); buf[n] = 0; fclose(f);
    int mixmode = argc > 3 ? atoi(argv[3]) : 0;
    char *dp = strstr(buf, "DrumPatches:");
    static tonic_t T; tonic_init(&T, 44100);
    static po32_patch_params_t PP[8];
    for (int v = 0; v < 8; v++) {
        char key[16]; snprintf(key, sizeof key, "\t\t%d: {", v + 1);
        char *s = strstr(dp, key), *e = strstr(s, "\n\t\t}"), txt[4096] = ""; double pan = 0;
        *e = 0;
        for (int k = 0; k < 21; k++) {
            char pat[32], *l; snprintf(pat, sizeof pat, "\t\t\t%s = ", KEYS[k]);
            l = strstr(s, pat); if (!l) { printf("missing %s\n", KEYS[k]); continue; }
            l += strlen(pat); char *le = strpbrk(l, "\r\n"); char val[64]; snprintf(val, sizeof val, "%.*s", (int)(le - l), l);
            if (k == 13 && mixmode) snprintf(val, sizeof val, "%.6f", 100.0 - atof(val));
            strcat(txt, KEYS[k]); strcat(txt, ": "); strcat(txt, val); strcat(txt, "\n");
        }
        char *pp = strstr(s, "\t\t\tPan = "); if (pp) pan = atof(pp + 9);
        *e = '\n';
        po32_status_t st = po32_patch_parse_mtdrum_text(txt, strlen(txt), &PP[v]);
        if (getenv("ZMOD") && v == atoi(getenv("ZMOD"))) PP[v].ModAmt = 0.5f;
        if (getenv("EQS")) { int ev; float sc; if (sscanf(getenv("EQS"), "%d:%f", &ev, &sc) == 2 && ev == v) PP[v].EQGain = 0.5f + (PP[v].EQGain - 0.5f) * sc; }
        if (getenv("DISTZ") && atoi(getenv("DISTZ")) == v) PP[v].DistAmt = 0.0f;
        if (getenv("EQ0")) PP[v].EQGain = 0.5f;
        if (getenv("EQDB") && v == 2) PP[v].EQGain = 0.5f + (float)atof(getenv("EQDB")) / 80.0f;
        if (getenv("DIST0")) PP[v].DistAmt = 0.0f;
        if (getenv("MIXOV")) { int mv; float mval; if (sscanf(getenv("MIXOV"), "%d:%f", &mv, &mval) == 2 && mv == v) PP[v].Mix = mval; }
        float *fp = &PP[v].OscWave;
        for (int i = 0; i < 21; i++) tonic_set(&T, v, i, (uint16_t)(fp[i] * 65535.0f + 0.5f));
        tonic_set_pan(&T, v, (int)(64 + pan * 0.63 + 0.5));
        printf("voice %d parse=%d freq=%.4f dcy=%.3f ndcy=%.3f mix=%.3f lvl=%.3f pan=%.1f\n", v + 1, (int)st, PP[v].OscFreq, PP[v].OscDcy, PP[v].NEnvDcy, PP[v].Mix, PP[v].Level, pan);
    }
    /* each drum as a one-shot: engine (one at a time, 3 s) vs libpo32 render */
    po32_synth_t sy; po32_synth_init(&sy, 44100);
    int N = 44100 * 3; static float ref[44100 * 3], L[44100 * 3], R[44100 * 3], st2[44100 * 6];
    for (int v = 0; v < 8; v++) {
        memset(L, 0, sizeof L); memset(R, 0, sizeof R); size_t rl = 0;
        po32_synth_render(&sy, &PP[v], 96, 3.0f, ref, N, &rl);
        for (int q = 0; q < 8; q++) T.active[q] = 0;
        tonic_trigger(&T, v, argc>4?atoi(argv[4]):96);
        /* odd block sizes on purpose */
        int pos = 0, bl[] = {64, 100, 256, 33, 512}, bi = 0;
        while (pos < N) { int m = bl[bi++ % 5]; if (pos + m > N) m = N - pos; tonic_render_st(&T, L + pos, R + pos, m); pos += m; }
        double pl, pr; float gl = T.pan_l[v], gr = T.pan_r[v]; (void)gr;
        double err = 0, pk = 0, pkr = 0, rms = 0; int last = 0;
        for (int i = 0; i < N; i++) { double d = L[i] / (gl * T.vgain[v]) - ref[i]; if (i < (int)rl) { err += d * d; } if (fabs(ref[i]) > pkr) pkr = fabs(ref[i]); if (fabs(ref[i]) > 1e-3) last = i; rms += ref[i] * ref[i]; }
        (void)pl; (void)pr; (void)pk;
        printf("  v%d: peak %.3f, rms %.4f, audible to %.2f s, max diff vs libpo32 %.2e\n", v + 1, pkr, sqrt(rms / N), last / 44100.0, sqrt(err / N));
        char fn[128]; snprintf(fn, sizeof fn, "%s/shot%d.f32", argv[2], v + 1); { static float st[44100*6]; for(int i=0;i<N;i++){st[2*i]=L[i];st[2*i+1]=R[i];} FILE*o=fopen(fn,"wb"); fwrite(st,4,2*N,o); fclose(o);}
    }
    return 0;
}
