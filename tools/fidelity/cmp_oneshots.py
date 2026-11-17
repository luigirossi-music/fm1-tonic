import numpy as np, wave, sys
D="r2/Rostona Export 2.0/Rostoma Hieroglycepha (121) track %d %s.wav"
def rd(f):
    w=wave.open(f); b=w.readframes(w.getnframes()); a=np.frombuffer(b,dtype=np.uint8).reshape(-1,3)
    v=(a[:,0].astype(np.int32)|(a[:,1].astype(np.int32)<<8)|(a[:,2].astype(np.int32)<<16)); v=np.where(v>=1<<23,v-(1<<24),v)
    return (v/(1<<23)).reshape(-1,2)
def on(x):
    a=np.abs(x).max(1); i=np.nonzero(a>2e-4)[0]; return i[0] if len(i) else 0
def cent(x):
    X=np.abs(np.fft.rfft(x[:16384]*np.hanning(min(len(x),16384)))) if len(x)>=16384 else np.abs(np.fft.rfft(x)); f=np.fft.rfftfreq((len(X)-1)*2,1/44100); return (f*X).sum()/X.sum()
for kind,vel in (("clean",96),("accent",127)):
    print("==",kind,"vs mine vel",vel)
    for t in range(1,9):
        r=rd(D%(t,kind)); o=on(r); r=r[o:]
        m=np.fromfile(f"tt/o{vel}/shot{t}.f32",dtype=np.float32).reshape(-1,2)
        n=min(len(r),len(m),44100*3); r=r[:n]; m=m[:n]
        w=13230
        rr=np.sqrt((r[:w]**2).mean()); mm=np.sqrt((m[:w]**2).mean())
        rm=r.mean(1); mmn=m.mean(1); c=np.corrcoef(rm,mmn)[0,1]
        # tail: time where envelope (20ms rms) falls below -60dB of its peak
        def dec(x):
            e=np.sqrt(np.convolve(x**2,np.ones(882)/882,'same')); pk=e.max(); i=np.nonzero(e>pk*1e-3)[0]; return i[-1]/44100
        print(f"t{t}: onset {o/44.1:6.1f}ms peak ref {np.abs(r).max():.3f} mine {np.abs(m).max():.3f} | rms0.3s diff {20*np.log10((rr+1e-12)/(mm+1e-12)):6.1f} dB | corr {c:6.3f} | -60dB tail ref {dec(rm):.2f}s mine {dec(mmn):.2f}s | centroid ref {cent(rm):6.0f} mine {cent(mmn):6.0f}")
