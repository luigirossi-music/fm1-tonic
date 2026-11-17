import numpy as np,struct,glob
def rdf(f):
    d=open(f,'rb').read(); j=d.find(b'data'); n=struct.unpack('<I',d[j+4:j+8])[0]; return np.frombuffer(d[j+8:j+8+n],dtype=np.float32).reshape(-1,2).astype(np.float64)
P=sorted(glob.glob('ryg/*/*.wav'))
R={}
for f in P:
    k='0' if 'Fader 0' in f else ('-6' if 'Fader -6' in f else '-12'); R[k]=rdf(f)
