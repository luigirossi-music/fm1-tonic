import wave,numpy as np,subprocess,sys
from scipy.signal import butter,sosfilt
def rd24(f):
    w=wave.open(f); a=np.frombuffer(w.readframes(w.getnframes()),dtype=np.uint8).reshape(-1,3).astype(np.int32)
    x=(a[:,0]|(a[:,1]<<8)|(a[:,2]<<16)); x=np.where(x>=1<<23,x-(1<<24),x)/float(1<<23); return x.reshape(-1,2)
def rdf(f):
    d=open(f,'rb').read(); i=d.find(b'data'); return np.frombuffer(d[i+8:],dtype=np.float32).reshape(-1,2)
ref=rd24('Rostoma_Hieroglycepha__121__Exports/Rostoma_Hieroglycepha__121__Exports/Rostoma_Hieroglycepha__121__Export 2 bars render.wav')
sos=butter(4,[1500,12000],btype='band',fs=44100,output='sos')
def env(x):
    y=sosfilt(sos,x.mean(1)); e=np.abs(y); k=np.ones(44)/44; e=np.convolve(e,k,'same'); return np.diff(e,prepend=0).clip(0)
re_=env(ref[:88200])
res=[]
for swk in [0,0.5,1,1.5,2,2.5,3,4,5,6,8]:
    subprocess.run(['env',f'SWK={swk}','/tmp/pr','/mnt/user-data/uploads/Rostoma_Hieroglycepha__121_.mtpreset','/tmp/e.wav','a','1'],capture_output=True)
    e=rdf('/tmp/e.wav'); e=np.concatenate([e,np.zeros((88200-len(e),2),dtype=np.float32)])[:88200]
    ee=env(e); best=max(((np.dot(re_[max(0,l):88200+min(0,l)],ee[max(0,-l):88200-max(0,l)])/np.sqrt((re_**2).sum()*(ee**2).sum()),l) for l in range(-60,61)))
    res.append((swk,best)); print('SWK %.1f  corr %.3f at lag %d'%(swk,best[0],best[1]))
