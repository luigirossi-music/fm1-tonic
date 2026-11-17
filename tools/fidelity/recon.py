import numpy as np,wave,re
exec(open('ld.py').read())
def rd24(f):
    w=wave.open(f); a=np.frombuffer(w.readframes(w.getnframes()),dtype=np.uint8).reshape(-1,3).astype(np.int32)
    x=(a[:,0]|(a[:,1]<<8)|(a[:,2]<<16)); x=np.where(x>=1<<23,x-(1<<24),x)/float(1<<23); return x.reshape(-1,2).astype(np.float64)
E1="ref/Rostona Export 2.0/Rostoma Hieroglycepha (121) track %d %s.wav"
def _on(x,th=1e-3):
    i=np.flatnonzero(np.abs(x).max(1)>th); return i[0] if len(i) else 0
SH={}
for k in range(1,9):
    for kind in ('accent','clean'):
        x=rd24(E1%(k,kind)); SH[(k,kind)]=x[max(0,_on(x)-4):]
t=open('/mnt/user-data/uploads/Rostoma_Hieroglycepha__121_.mtpreset').read()
ps=t[t.find('\n\t\ta: {'):]; ps=ps[:ps.find('\n\t\tb: {')]
TR={};AC={}
for v in range(1,9):
    s=ps[ps.find('\n\t\t\t%d: {'%v):]; TR[v]=re.search(r'Triggers = ([#-]+)',s).group(1); AC[v]=re.search(r'Accents = ([#-]+)',s).group(1)
tempo,swing=121.0,4.2172770708833
step=60/tempo/4*44100
N=len(R['-12'])
def build(sw_frac,lag=0,vels=None,skip=()):
    out=np.zeros((N+441000,2))
    for v in range(1,9):
        if v in skip: continue
        for i in range(32):
            idx=i%16
            if TR[v][idx]!='#': continue
            at=int(round(i*step+((i&1)*sw_frac*step))+lag)
            sh=SH[(v,'accent' if AC[v][idx]=='#' else 'clean')]
            if at<0: sh=sh[-at:]; at=0
            m=min(len(sh),len(out)-at)
            if m>0: out[at:at+m]+=sh[:m]
    return out[:N]
def fitg(a,b,skip=441):
    a=a[skip:];b=b[skip:]; g=(a*b).sum()/(b*b).sum(); res=a-g*b; return g,1-((res**2).sum()/(a**2).sum())
