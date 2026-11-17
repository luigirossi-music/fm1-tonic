#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dsp/tonic.h"
#include "dsp/drum909.h"
extern po32_status_t po32_patch_parse_mtdrum_text(const char*, size_t, po32_patch_params_t*);
static const char *KEYS[21] = {"OscWave","OscFreq","OscAtk","OscDcy","ModMode","ModRate","ModAmt","NFilMod","NFilFrq","NFilQ","NEnvMod","NEnvAtk","NEnvDcy","Mix","DistAmt","EQFreq","EQGain","Level","OscVel","NVel","ModVel"};
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static tonic_t T; static drum909_t D9; 
int main(int argc,char**argv){
    FILE *f=fopen(argv[1],"rb"); static char buf[200000]; size_t n=fread(buf,1,sizeof buf-1,f); buf[n]=0; fclose(f);
    char *dp=strstr(buf,"DrumPatches:"); tonic_init(&T,44100);
    for(int v=0;v<8;v++){ char key[16]; snprintf(key,sizeof key,"\t\t%d: {",v+1); char*s=strstr(dp,key),*e=strstr(s,"\n\t\t}"),txt[4096]=""; static po32_patch_params_t PP; *e=0;
        for(int k=0;k<21;k++){ char p[32],*l; snprintf(p,sizeof p,"\t\t\t%s = ",KEYS[k]); l=strstr(s,p); if(!l)continue; l+=strlen(p); char*le=strpbrk(l,"\r\n"); char val[64]; snprintf(val,sizeof val,"%.*s",(int)(le-l),l); strcat(txt,KEYS[k]);strcat(txt,": ");strcat(txt,val);strcat(txt,"\n"); }
        *e='\n'; po32_patch_parse_mtdrum_text(txt,strlen(txt),&PP); float*fp=&PP.OscWave; for(int i=0;i<21;i++) tonic_set(&T,v,i,(uint16_t)(fp[i]*65535.f+.5f)); }
    drum909_init(&D9);
    int secs=20, N=44100*secs, B=64; static float l[64],r[64],rv[64],dl[64],t1[64],t2[64];
    volatile float sink=0; double t0;
    /* Tonic: all 8 drums every 1/8 s (a dense pattern) */
    t0=now(); for(int i=0;i<N;i+=B){ if(((i/B)%(44100/8/B))==0) for(int v=0;v<8;v++) tonic_trigger(&T,v,100); memset(l,0,sizeof l);memset(r,0,sizeof r); tonic_render_st(&T,l,r,B); sink+=l[0]; }
    double tt=now()-t0;
    t0=now(); for(int i=0;i<N;i+=B){ if(((i/B)%(44100/8/B))==0) for(int v=0;v<DR_NUM;v++) drum909_trigger(&D9,v,.9f); memset(l,0,sizeof l);memset(r,0,sizeof r);memset(rv,0,sizeof rv);memset(dl,0,sizeof dl); drum909_render_st(&D9,l,r,rv,dl,B); sink+=l[0]; }
    double t9=now()-t0;
    printf("host time for %d s of audio, everything triggered 8x per second (worst case):\n Tonic 8 voices: %.3f s  (%.1f %% of real time)\n 909 11 voices: %.3f s  (%.1f %%)\n",secs,tt,100*tt/secs,t9,100*t9/secs);
    { double worst=0,tot=0; int cnt=0; for(int rep=0;rep<2000;rep++){ for(int v=0;v<8;v++){ double a=now(); tonic_trigger(&T,v,100); double d=now()-a; tot+=d; cnt++; if(d>worst)worst=d; } memset(l,0,sizeof l);memset(r,0,sizeof r); tonic_render_st(&T,l,r,B); }
      double ab=0,wb=0; for(int rep=0;rep<4000;rep++){ for(int v=0;v<8;v++) tonic_trigger(&T,v,100); double a=now(); memset(l,0,sizeof l);memset(r,0,sizeof r); tonic_render_st(&T,l,r,B); double d=now()-a; ab+=d; if(d>wb)wb=d; }
      printf(" Tonic trigger: avg %.1f us, worst %.1f us per voice; one 64-sample block with all 8 voices sounding: avg %.1f us, worst %.1f us (block = %.0f us of audio)\n",1e6*tot/cnt,1e6*worst,1e6*ab/4000,1e6*wb,64/44100.0*1e6); }
    /* Tonic sparse: preset pattern a at 121 BPM (real use) */
    return 0; }
