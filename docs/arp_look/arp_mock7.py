from PIL import Image, ImageDraw, ImageFont, ImageFilter
K=6; OUT=3
base=Image.open('arp_page.png').convert('RGB')
BG=(197,198,197); BLUE=(24,112,200); LB=(120,185,245); GREY=(172,174,180); DK=(150,154,160); AMB=(236,150,20); CHIP=(172,218,246)
def F(n,b=True): return ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans%s.ttf'%('-Bold' if b else ''),int(n*K))
X0,Y0,PX,PY=28,22,12.5,14.6
# (row,col): note on; accents; playhead
notes=[(0,0),(1,2),(2,4),(3,6),(4,8),(3,10),(2,12),(1,14),(0,15)]
acc={0,4,8}            # indices of notes (steps) that carry an accent
play=6
def cx(c): return (X0+c*PX+5.5)*K
def cy(r): return (Y0+r*PY+6.5)*K
def canvas():
    im=base.resize((240*K,240*K),Image.NEAREST); d=ImageDraw.Draw(im)
    d.rectangle([0,19*K,240*K,156*K],fill=BG); d.rectangle([100*K,143*K,240*K,171*K],fill=BG)
    d.rectangle([0,0,112*K,18*K],fill=BG)
    d.rounded_rectangle([2*K,2*K,86*K,16*K],radius=4*K,fill=CHIP)
    d.text((44*K,9.3*K),'PATTERN BANK 1',font=F(7.5),fill=(15,35,70),anchor='mm')
    return im
def keys(d):
    wh=[0,2,4,5,7]
    for r in range(8):
        y=Y0+r*PY
        if r in wh:
            d.rounded_rectangle([3*K,y*K,23*K,(y+13)*K],radius=2*K,fill=(252,252,252),outline=(110,110,115),width=K//2)
            d.text((19*K,(y+6.5)*K),"CDEFG"[wh.index(r)],font=F(6.5),fill=(120,120,125),anchor='mm')
    for r in (1,3,6):
        y=Y0+r*PY
        d.rounded_rectangle([3*K,(y-0.5)*K,15*K,(y+13.5)*K],radius=2*K,fill=(34,34,40))
def bottom(d):
    d.rounded_rectangle([112*K,144*K,232*K,170*K],radius=7*K,fill=CHIP)
    d.text((172*K,157.6*K),"TRACK 1",font=F(17),fill=(15,35,70),anchor='mm')
def playhead(im):
    ov=Image.new('RGBA',im.size,(0,0,0,0)); o=ImageDraw.Draw(ov)
    o.rounded_rectangle([(cx(play)-6*K),19.5*K,(cx(play)+6*K),141*K],radius=5*K,fill=(255,255,255,70))
    return Image.alpha_composite(im.convert('RGBA'),ov).convert('RGB')
def glow(im,pts):
    ov=Image.new('RGBA',im.size,(0,0,0,0)); o=ImageDraw.Draw(ov)
    for (x,y) in pts: o.ellipse([x-11*K,y-11*K,x+11*K,y+11*K],fill=(60,150,255,150))
    ov=ov.filter(ImageFilter.GaussianBlur(5*K))
    return Image.alpha_composite(im.convert('RGBA'),ov).convert('RGB')


COL={'n':(34,34,44),'a':(25,70,200),'f':(250,215,40),'b':(60,190,90)}
POS={0:0,1:.5,2:1,3:1.5,4:2,5:3,6:3.5,7:4,8:4.5,9:5,10:5.5,11:6,12:7}
def ry(s): return (Y0+(7-POS[s])*PY+6.5)*K          # y of a semitone 0 (C) .. 12 (C')
# (col, semitone in octave, type, octave offset from the visible one)
notes=[(0,0,'a',0),(1,1,'n',0),(3,3,'n',0),(4,4,'n',0),(6,6,'f',0),(7,8,'n',0),(9,10,'n',0),(10,11,'b',0),(12,7,'n',0),(14,3,'n',0),(5,9,'n',1),(11,5,'n',-1)]
def lite(c): return tuple(min(255,int(v*0.5+130)) for v in c)
def pale(c): return tuple(int(v*0.45+215*0.55) for v in c)
def keys2(d):
    names=["C'","B","A","G","F","E","D","C"]
    for r in range(8):
        y=Y0+r*PY
        d.rounded_rectangle([3*K,y*K,23*K,(y+13)*K],radius=2*K,fill=(252,252,252),outline=(110,110,115),width=K//2)
        d.text((19*K,(y+6.5)*K),names[r],font=F(6.5),fill=(120,120,125),anchor='mm')
    for s_ in (1,3,6,8,10):
        yc=ry(s_)/K
        d.rounded_rectangle([3*K,(yc-4.5)*K,15*K,(yc+4.5)*K],radius=2*K,fill=(34,34,44))
def grid(d):
    for r in range(8):
        for c in range(16): d.ellipse([cx(c)-1.6*K,cy(r)-1.6*K,cx(c)+1.6*K,cy(r)+1.6*K],fill=DK)
def tri(d,x,y,up,col):
    q=3*K; pts=[(x,y-q),(x-q,y+q*0.7),(x+q,y+q*0.7)] if up else [(x,y+q),(x-q,y-q*0.7),(x+q,y-q*0.7)]
    d.polygon(pts,fill=col)

SH={1,3,6,8,10}
PLAYCOL=6
def make(style):
    im=canvas(); im=playhead(im); d=ImageDraw.Draw(im)
    if style=='B':     # piano-roll lanes: a faint dark lane at every black-key position
        for s_ in SH:
            yc=ry(s_)/K; d.rectangle([X0*K,(yc-3.3)*K,(X0+16*PX)*K,(yc+3.3)*K],fill=(184,186,190))
    keys2(d); grid(d)
    if style=='A':     # tiny guide dots where the sharps live
        for s_ in SH:
            for c in range(16): d.ellipse([cx(c)-0.9*K,ry(s_)-0.9*K,cx(c)+0.9*K,ry(s_)+0.9*K],fill=(176,178,184))
    xmin=(X0-1)*K; ov=Image.new('RGBA',im.size,(0,0,0,0)); o=ImageDraw.Draw(ov); pts=[]
    for i,(c,sm,t,oc) in enumerate(notes):
        x=cx(c); y=ry(sm); col=COL[t] if oc==0 else pale(COL[t]); L=(3 if i%3 else 2)*PX*K
        pts.append((x,y,col,oc,t,sm,c))
        for k in range(40):
            tt=k/40; xx=x-tt*min(L,x-xmin)
            o.ellipse([xx-3.0*K,y-3.0*K,xx+3.0*K,y+3.0*K],fill=col+(int(120*(1-tt)*(1 if oc==0 else .7)),))
    ov=ov.filter(ImageFilter.GaussianBlur(0.6*K)); im=Image.alpha_composite(im.convert('RGBA'),ov).convert('RGB'); d=ImageDraw.Draw(im)
    for (x,y,col,oc,t,sm,c) in pts:
        sharp=sm in SH; r=(4.2 if (sharp and style=='B') else 5.2)*K
        if oc==0:
            d.ellipse([x-r,y-r,x+r,y+r],fill=col); d.ellipse([x-2.3*K,y-3.3*K,x+0.2*K,y-1.0*K],fill=lite(col))
            if sharp and style=='A': d.text((x+0.2*K,y+0.4*K),"#",font=F(7.5),fill=(255,255,255) if t!='f' else (40,40,40),anchor='mm')
        else:
            d.ellipse([x-r,y-r,x+r,y+r],fill=col,outline=COL[t],width=int(1.2*K))
            d.text((x,y+0.3*K),("+1" if oc>0 else "-1"),font=F(6.5),fill=COL[t],anchor='mm')
        if style=='B' and c==PLAYCOL and oc==0:    # the key of the playing note lights in its colour
            yc=ry(sm)
            if sharp: d.rounded_rectangle([3*K,(yc/K-4.5)*K,15*K,(yc/K+4.5)*K],radius=2*K,fill=col)
            else: d.rounded_rectangle([3*K,(yc/K-6.5)*K,23*K,(yc/K+6.5)*K],radius=2*K,fill=col)
    bottom(d); return im
ims=[make('A'),make('B')]; names=['A  # on the ball + guide dots','B  lanes + smaller sharp balls + key lights']
OUT=2; W=240*OUT; out=Image.new('RGB',(W*2+20,W+44),(255,255,255)); dd=ImageDraw.Draw(out)
fn=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf',22)
for i,(im,n) in enumerate(zip(ims,names)):
    im.resize((720,720),Image.LANCZOS).save('docs/arp_look/arp_v7_%s.png'%'AB'[i])
    out.paste(im.resize((W,W),Image.LANCZOS),(i*(W+20),44)); dd.text((i*(W+20)+6,10),n,font=fn,fill=(0,0,0))
out.save('docs/arp_look/arp_v7_both.png')
