import numpy as np, sys
exec(open('eq3.py').read().split("D=")[0])
E="ref/Rostona Export 2.0/Rostoma Hieroglycepha (121) track %d %s.wav"
def run(tag):
    print("== EQ Q:",tag)
    tot=[]
    for kind,vel in (("clean",42),("accent",127)):
        row=[]
        for t in range(1,9):
            r=tr(rd(E%(t,kind)))[0]; m=np.fromfile(f"tt/{'o' if tag=='0.7' else 'old'}{vel}/shot{t}.f32",dtype=np.float32).reshape(-1,2)
            n=min(len(r),len(m),44100*2); w=min(n,13230)
            rr=np.sqrt((r[:w]**2).mean()); mm=np.sqrt((m[:w]**2).mean()); row.append(20*np.log10(rr/mm))
        print(f"  {kind:6s} ref-mine dB per drum 1-8:"," ".join(f"{x:6.1f}" for x in row)," | rms of errors %.1f"%np.sqrt(np.mean(np.square(row))))
run("1.0"); run("0.7")
