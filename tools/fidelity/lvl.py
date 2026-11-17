import wave,numpy as np,sys
def rd(f):
    w=wave.open(f); a=np.frombuffer(w.readframes(w.getnframes()),dtype=np.uint8).reshape(-1,3).astype(np.int32)
    x=(a[:,0]|(a[:,1]<<8)|(a[:,2]<<16)); x=np.where(x>=1<<23,x-(1<<24),x)/float(1<<23); return x.reshape(-1,2)
E="ref/Rostona Export 2.0/Rostoma Hieroglycepha (121) track %d %s.wav"
def on(x,th=1e-3):
    i=np.flatnonzero(np.abs(x)>th); return i[0] if len(i) else 0
def rms(x,a,b): return np.sqrt((x[a:b]**2).mean())+1e-12
wins=[(0,441),(441,2205),(2205,8820),(8820,22050),(22050,44100)]
tot={}
for kind,vel in (('accent',127),('clean',42)):
    print(kind); allr=[]
    for k in range(1,9):
        r=rd(E%(k,kind)).mean(1); m=np.fromfile(f"tt/o{vel}/shot{k}.f32",dtype=np.float32).reshape(-1,2).mean(1)
        r=r[on(r):]; m=m[on(m):]
        row=[20*np.log10(rms(r,a,b)/rms(m,a,b)) for a,b in wins]; tw=20*np.log10(rms(r,0,22050)/rms(m,0,22050)); allr.append(tw)
        print('  d%d  ref-eng dB  windows(0-10ms,-50ms,-200ms,-.5s,-1s):'%k,' '.join('%6.1f'%x for x in row),'| 0-0.5s: %5.1f'%tw)
    print('  rms of 0-0.5s errors %.2f'%np.sqrt(np.mean(np.square(allr))))
