import numpy as np
exec(open('mixfit.py').read().split("print(\"envelope dB")[0])
N=2048*4
def hamp(x,k,f0=250.0):
    seg=x.mean(1)[:int(0.03*44100)]; seg=seg*np.hanning(len(seg)); X=np.abs(np.fft.rfft(seg,N)); fr=np.fft.rfftfreq(N,1/44100)
    c=f0*k; m=(fr>c-70)&(fr<c+70); return X[m].max()
mine={g:np.fromfile(f"mx/eq{g}/shot3.f32",dtype=np.float32).reshape(-1,2) for g in (0,40)}
r=ref['osc100']
print(" harm  freq   ref/mine(eq0) dB  rel. to fundamental")
base=20*np.log10(hamp(r,1)/hamp(mine[0],1))
rows=[]
for k in (1,3,5,7,9,11,13,15,17,19,21):
    d=20*np.log10(hamp(r,k)/hamp(mine[0],k)); rows.append((250*k,d-base))
    print(f"{k:4d} {250*k:6.0f}   {d:7.1f}          {d-base:7.1f}")
# fit RBJ peaking EQ gain/Q to the relative boosts
def peak_db(f,fc,G,Q,fs=44100):
    A=10**(G/40); w=2*np.pi*fc/fs; al=np.sin(w)/(2*Q); c=np.cos(w)
    b=np.array([1+al*A,-2*c,1-al*A]); a=np.array([1+al/A,-2*c,1-al/A]); z=np.exp(-1j*2*np.pi*f/fs)
    H=(b[0]+b[1]*z+b[2]*z**2)/(a[0]+a[1]*z+a[2]*z**2); return 20*np.log10(abs(H))
fs_=np.array([f for f,_ in rows]); ys=np.array([d for _,d in rows])
best=None
for G in np.arange(5,60,1.0):
    for Q in np.arange(0.2,6,0.05):
        for off in (0,):
            e=((peak_db(fs_,1764.7,G,Q)-peak_db(250.0,1764.7,G,Q)-ys)**2).mean()
            if best is None or e<best[0]: best=(e,G,Q)
print("best fit RBJ peaking at 1764.7 Hz: gain %.0f dB, Q %.2f, rms err %.2f dB"%(best[1],best[2],best[0]**.5))
print("model vs measured:"," ".join(f"{peak_db(f,1764.7,best[1],best[2]):.1f}/{y:.1f}" for f,y in zip(fs_,ys)))
