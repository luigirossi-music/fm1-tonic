import numpy as np
exec(open('mixan.py').read().split("files={")[0])
import sys
R="ref/Rostoma_Hieroglycepha__121__Exports/"
fn={'osc100':'drum3 mix osc100.wav','osc75':'drum3 mix osc75.wav','50':'drum3 mix 50.wav','noise75':'drum3 mix noise75.wav','noise100':'drum3 mix noise100.wav'}
raw={k:rd(R+f) for k,f in fn.items()}
N=44100
def tr(x):
    a=np.abs(x).max(1); i=np.nonzero(a>1e-4)[0][0]; return x[i:i+N]
ref={k:tr(v) for k,v in raw.items()}
n=min(len(v) for v in ref.values()); ref={k:v[:n] for k,v in ref.items()}
def env(x,w=441): return np.sqrt(np.convolve(x.mean(1)**2,np.ones(w)/w,'same'))
mo=np.fromfile("mx/m0/shot3.f32",dtype=np.float32).reshape(-1,2)[:n]
mn=np.fromfile("mx/m1/shot3.f32",dtype=np.float32).reshape(-1,2)[:n]
print("envelope dB at times: osc100ref | mine osc | noise100ref | mine noise")
for t in (0.002,0.01,0.03,0.06,0.1,0.2,0.3):
    i=int(t*44100)
    f=lambda e:20*np.log10(e[i]+1e-9)
    print(f"{t:5.3f} {f(env(ref['osc100'])):7.1f} {f(env(mo)):7.1f} | {f(env(ref['noise100'])):7.1f} {f(env(mn)):7.1f}")
# per-channel check
for k in ref: print(k,'L rms',np.sqrt((ref[k][:,0]**2).mean()),'R rms',np.sqrt((ref[k][:,1]**2).mean()))
print('mine L',np.sqrt((mo[:,0]**2).mean()),'R',np.sqrt((mo[:,1]**2).mean()))
# gains
def full_rms(x): return np.sqrt((x**2).mean())
go=full_rms(ref['osc100'])/full_rms(mo); gn=full_rms(ref['noise100'])/full_rms(mn)
print(f"osc-only gain ref/mine: {20*np.log10(go):.2f} dB   noise-only gain ref/mine: {20*np.log10(gn):.2f} dB")
# fit mixed files: ref = a*osc100ref + resid(noise)
o=ref['osc100']
for k in ('osc75','50','noise75','noise100'):
    x=ref[k]; a=(x*o).sum()/(o*o).sum(); res=x-a*o
    b=full_rms(res)/full_rms(ref['noise100'])
    print(f"{k:9s} osc amount rel osc100: {a:6.3f} ({20*np.log10(abs(a)+1e-12):6.1f} dB)   noise amount rel noise100: {b:6.3f} ({20*np.log10(b+1e-12):6.1f} dB)")
print('---- joint least squares  x = a*osc100 + b*noise100')
o=ref['osc100'].ravel(); nz=ref['noise100'].ravel()
for k in ('osc75','50','noise75'):
    x=ref[k].ravel(); A=np.stack([o,nz],1); co,res,_,_=np.linalg.lstsq(A,x,rcond=None)
    fit=A@co; err=np.sqrt(((x-fit)**2).mean())/np.sqrt((x**2).mean())
    print(f"{k:8s} a={co[0]:.3f} ({20*np.log10(abs(co[0])+1e-12):6.1f} dB) b={co[1]:.3f} ({20*np.log10(abs(co[1])+1e-12):6.1f} dB) unexplained {err*100:.1f}% of rms")
print('corr noise100 vs noise75 (first 50ms):',np.corrcoef(ref['noise100'][:2205,0],ref['noise75'][:2205,0])[0,1])
print('---- energy-envelope fit  E_x(t)=a^2*E_osc100(t)+b^2*E_noise100(t)  (5 ms windows, 0-250 ms)')
from scipy.optimize import nnls
W=220
def E(x): 
    m=(x**2).mean(1)[:12*W*5//5*0+ int(0.25*44100)//W*W].reshape(-1,W).mean(1); return m
Eo,En=E(ref['osc100']),E(ref['noise100'])
mo_E,mn_E=E(mo),E(mn)
for k in ('osc75','50','noise75'):
    Ex=E(ref[k]); w=1/np.sqrt(Ex+1e-12)
    A=np.stack([Eo,En],1)*w[:,None]; sol,_=nnls(A,Ex*w)
    print(f"{k:8s} osc {10*np.log10(sol[0]+1e-15):6.1f} dB  noise {10*np.log10(sol[1]+1e-15):6.1f} dB   (re the 100% files)")
# sanity: noise100 vs noise-only should give b=0 dB and a tiny
for k in ('osc100','noise100'):
    Ex=E(ref[k]); w=1/np.sqrt(Ex+1e-12); A=np.stack([Eo,En],1)*w[:,None]; sol,_=nnls(A,Ex*w)
    print('sanity',k,10*np.log10(sol+1e-15))
print('---- spectrum ratio ref/mine by band (dB), first 120 ms')
def spec(x,n=5292):
    x=x.mean(1)[:n]*np.hanning(n); X=np.abs(np.fft.rfft(x,8192))**2; return X
f=np.fft.rfftfreq(8192,1/44100)
bands=[(60,120),(120,250),(250,500),(500,1000),(1000,2000),(2000,4000),(4000,6000),(6000,8000),(8000,11000),(11000,16000)]
def bandE(X,lo,hi): return X[(f>=lo)&(f<hi)].sum()
Xo_r,Xo_m=spec(ref['osc100']),spec(mo); Xn_r,Xn_m=spec(ref['noise100']),spec(mn)
print(" band Hz        osc ref/mine   noise ref/mine")
for lo,hi in bands:
    a=10*np.log10((bandE(Xo_r,lo,hi)+1e-30)/(bandE(Xo_m,lo,hi)+1e-30)); b=10*np.log10((bandE(Xn_r,lo,hi)+1e-30)/(bandE(Xn_m,lo,hi)+1e-30))
    print(f"{lo:6d}-{hi:6d}   {a:8.1f}      {b:8.1f}   (noise ref level {10*np.log10(bandE(Xn_r,lo,hi)+1e-30):6.1f}, osc ref {10*np.log10(bandE(Xo_r,lo,hi)+1e-30):6.1f})")
print('---- noise-only band levels (dB), ref vs mine, 1/3 octave, first 120 ms')
cs=[100*2**(k/3) for k in range(0,22)]
for c in cs:
    lo,hi=c/2**(1/6),c*2**(1/6)
    r=10*np.log10(bandE(Xn_r,lo,hi)+1e-30); m=10*np.log10(bandE(Xn_m,lo,hi)+1e-30)
    print(f"{c:7.0f} Hz  ref {r:6.1f}  mine {m:6.1f}  diff {r-m:6.1f}")
