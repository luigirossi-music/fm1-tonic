import numpy as np, wave
def rd(f):
    w=wave.open(f); b=w.readframes(w.getnframes()); a=np.frombuffer(b,dtype=np.uint8).reshape(-1,3).astype(np.int32)
    v=a[:,0]|(a[:,1]<<8)|(a[:,2]<<16); v=np.where(v>=1<<23,v-(1<<24),v); return (v/(1<<23)).reshape(-1,2)
def tr(x):
    a=np.abs(x).max(1); i=np.nonzero(a>1e-4)[0][0]; return x[i:], i
D="e2/export 2/drum3 osc100 eq%d.wav"
f=np.fft.rfftfreq(8192,1/44100)
def spec(x,n=4410):
    x=x.mean(1)[:n]*np.hanning(n); return np.abs(np.fft.rfft(x,8192))**2
def bE(X,lo,hi): return X[(f>=lo)&(f<hi)].sum()
bands=[(150,350),(500,1000),(1000,1500),(1500,2200),(2200,3000),(3000,4500),(4500,7000),(7000,11000)]
R={}
for g in (0,20,40):
    x,i=tr(rd(D%g)); R[g]=x; print(f"ref eq{g}: onset {i/44.1:.1f} ms peak {np.abs(x).max():.4f} rms(first 100ms) {np.sqrt((x[:4410]**2).mean()):.5f}")
M={}
for g in (0,10,20,30,40):
    m=np.fromfile(f"mx/eq{g}/shot3.f32",dtype=np.float32).reshape(-1,2); M[g]=m
    print(f"mine eq{g}: peak {np.abs(m).max():.4f} rms(first 100ms) {np.sqrt((m[:4410]**2).mean()):.5f}")
print("\nband levels dB (ref):", bands)
for g in R: print(f" ref eq{g}:", " ".join(f"{10*np.log10(bE(spec(R[g]),lo,hi)+1e-30):6.1f}" for lo,hi in bands))
print("\nEQ boost seen in ref = band level minus level of ref eq0 (dB):")
for g in (20,40): print(f" ref +{g}:", " ".join(f"{10*np.log10((bE(spec(R[g]),lo,hi)+1e-30)/(bE(spec(R[0]),lo,hi)+1e-30)):6.1f}" for lo,hi in bands))
print("EQ boost in mine = band level minus mine eq0:")
for g in (10,20,30,40): print(f" mine +{g}:", " ".join(f"{10*np.log10((bE(spec(M[g]),lo,hi)+1e-30)/(bE(spec(M[0]),lo,hi)+1e-30)):6.1f}" for lo,hi in bands))
print("\nneutral (eq0) ref/mine, band by band:", " ".join(f"{10*np.log10((bE(spec(R[0]),lo,hi)+1e-30)/(bE(spec(M[0]),lo,hi)+1e-30)):6.1f}" for lo,hi in bands))
print("total rms ratio ref/mine eq0 (first 100ms): %.2f dB"%(20*np.log10(np.sqrt((R[0][:4410]**2).mean())/np.sqrt((M[0][:4410]**2).mean()))))
