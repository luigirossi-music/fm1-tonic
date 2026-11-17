import numpy as np, wave, glob
R="ref/Rostoma_Hieroglycepha__121__Exports/"
def rd(f):
    w=wave.open(f); n=w.getnframes(); sw=w.getsampwidth(); b=w.readframes(n)
    if sw==3:
        a=np.frombuffer(b,dtype=np.uint8).reshape(-1,3).astype(np.int32)
        v=a[:,0]|(a[:,1]<<8)|(a[:,2]<<16); v=np.where(v>=1<<23,v-(1<<24),v); x=v/(1<<23)
    elif sw==2: x=np.frombuffer(b,dtype=np.int16)/32768.
    else: x=np.frombuffer(b,dtype=np.float32)
    print(f.split('/')[-1], w.getframerate(), sw*8, w.getnchannels(), n/w.getframerate())
    return x.reshape(-1,w.getnchannels())
def trim(x):
    a=np.abs(x).max(1); i=np.nonzero(a>2e-4)[0]; return x[i[0]:], i[0]
def mine(m,v=3): return np.fromfile(f"mx/m{m}/shot{v}.f32",dtype=np.float32).reshape(-1,2)
files={'osc100':'drum3 mix osc100.wav','osc75':'drum3 mix osc75.wav','50':'drum3 mix 50.wav','noise75':'drum3 mix noise75.wav','noise100':'drum3 mix noise100.wav','orig_eq0dist0':'drum3_eq0_dist0.wav'}
refs={}
for k,f in files.items():
    x,o=trim(rd(R+f)); refs[k]=x; print(k,'onset ms',o/44.1,'peak',np.abs(x).max(),'rms0.3s',np.sqrt((x[:13230]**2).mean()))
for m in ['0','0.25','0.5','0.75','1']:
    x=mine(m); print('mine mix',m,'peak',np.abs(x).max(),'rms0.3s',np.sqrt((x[:13230]**2).mean()))
print('---- cross-check other drum 3 refs')
def rms(x,n=13230): return np.sqrt((x[:n]**2).mean())
E="ref/Rostona Export 2.0/Rostoma Hieroglycepha (121) track %d %s.wav"
for k in ('accent','clean'):
    x,o=trim(rd(E%(3,k))); print('track3',k,'onset',o/44.1,'peak',np.abs(x).max(),'rms',rms(x))
for f in ('drum3_eq0.wav','drum3_eq0_dist0.wav'):
    x,o=trim(rd(R+f)); print(f,'onset',o/44.1,'peak',np.abs(x).max(),'rms',rms(x), 'len',len(x)/44100)
print('---- orig-mix compare')
m=np.fromfile("mx/orig/shot3.f32",dtype=np.float32).reshape(-1,2)
r,_=trim(rd(R+'drum3_eq0_dist0.wav'))
print('mine orig mix eq0dist0 peak',np.abs(m).max(),'rms0.3',rms(m),' ref peak',np.abs(r).max(),'rms0.3',rms(r))
def env(x,w=441): 
    e=np.sqrt(np.convolve(x.mean(1)**2,np.ones(w)/w,'same')); return e
er,em=env(r),env(m)
for t in (0.005,0.02,0.05,0.1,0.2,0.4,0.7):
    i=int(t*44100); print(f"t={t:5.3f}s ref {20*np.log10(er[i]+1e-9):6.1f} dB  mine {20*np.log10(em[i]+1e-9):6.1f} dB")
