#!/usr/bin/env python3
"""Grigiotonic NOISE ENVELOPE page (EDIT, first press) mockup (picture only, no firmware code).
Screen = 240 x 240 units, drawn 3x supersampled and saved at 1080 x 1080.
Run: python3 tools/grigiotonic_nenv_mockup.py [out.png]
"""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops

OUT = sys.argv[1] if len(sys.argv) > 1 else "docs/grigiotonic_nenv1_mockup.png"
FD = "/usr/share/fonts/truetype/google-fonts/"
FINAL = 1080
SS = 3
S = FINAL * SS
U = S / 240.0

BG = (197, 198, 197)          # the logo grey
INK = (62, 64, 66)
BLUE1 = (28, 140, 214)
BLUE2 = (70, 200, 240)
LIGHT = (233, 234, 233)
DARK = (146, 148, 147)

def px(v): return v * U
def font(name, size): return ImageFont.truetype(FD + name, int(size * U))

img = Image.new("RGB", (S, S), BG)

# ---------- helpers ----------
def blank(): return Image.new("L", (S, S), 0)

def shifted(mask, dx, dy):
    out = Image.new("L", (S, S), 0)
    out.paste(mask, (int(px(dx)), int(px(dy))))
    return out

def blurred(mask, r): return mask.filter(ImageFilter.GaussianBlur(px(r)))

def tint(layer_mask, color, alpha=1.0):
    global img
    m = layer_mask.point(lambda v: int(v * alpha))
    img.paste(Image.new("RGB", (S, S), color), mask=m)

def raised(mask, d=2.2, blur=2.4, a_light=0.95, a_dark=0.75):
    tint(blurred(shifted(mask, -d, -d), blur), LIGHT, a_light)
    tint(blurred(shifted(mask, d, d), blur), DARK, a_dark)

def inset(mask, d=1.6, blur=1.6, a_dark=0.75, a_light=0.9):
    inv = ImageChops.invert(mask)
    dk = ImageChops.multiply(blurred(shifted(inv, d, d), blur), mask)
    lt = ImageChops.multiply(blurred(shifted(inv, -d, -d), blur), mask)
    tint(dk, DARK, a_dark)
    tint(lt, LIGHT, a_light)

def pill_mask(x0, y0, x1, y1):
    m = blank(); ImageDraw.Draw(m).rounded_rectangle(
        [px(x0), px(y0), px(x1), px(y1)], radius=px((y1 - y0) / 2), fill=255)
    return m

def disc_mask(cx, cy, r):
    m = blank(); ImageDraw.Draw(m).ellipse(
        [px(cx - r), px(cy - r), px(cx + r), px(cy + r)], fill=255)
    return m

def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))

def text(xy, s, f, fill=INK, anchor="mm"):
    ImageDraw.Draw(img).text((px(xy[0]), px(xy[1])), s, font=f, fill=fill, anchor=anchor)

# ---------- top bar ----------
d = ImageDraw.Draw(img)
d.rounded_rectangle([px(2), px(2), px(58), px(17)], radius=px(3), fill=(168, 216, 244))
text((30, 9.6), "TRACK 1", font("Poppins-Medium.ttf", 8.6), fill=(28, 30, 34))
text((66, 9.6), "NOISE ENV", font("Poppins-Medium.ttf", 8.6), fill=BLUE1, anchor="lm")
text((124, 9.6), "P1", font("Poppins-Medium.ttf", 8.6), fill=INK)
text((214, 9.6), "125", font("Poppins-Medium.ttf", 8.6), fill=BLUE1)
d.line([px(60), px(19), px(240), px(19)], fill=(176, 178, 177), width=int(px(0.5)))

# ---------- sliders (visual reference: they follow knob 1 and knob 2) ----------
PL, PR = 8, 232
LX0, LX1 = PL + 21, PR - 20          # the thin line / thumb travel
TW, TH = 43, 21

def slider(y0, y1, val, left, mid, right):
    m = pill_mask(PL, y0, PR, y1)
    tint(m, (201, 202, 201))
    inset(m, d=1.7, blur=1.9)
    cy = (y0 + y1) / 2
    ImageDraw.Draw(img).line([px(LX0), px(cy), px(LX1), px(cy)], fill=(52, 54, 56), width=int(px(0.9)))
    cx = LX0 + val * (LX1 - LX0)
    tm = pill_mask(cx - TW / 2, cy - TH / 2, cx + TW / 2, cy + TH / 2)
    raised(tm, d=1.5, blur=1.6, a_light=0.5, a_dark=0.85)
    # vertical blue gradient inside the thumb
    grad = Image.new("RGB", (S, S), BLUE1)
    gd = ImageDraw.Draw(grad)
    t0, t1 = int(px(cy - TH / 2)), int(px(cy + TH / 2))
    for yy in range(t0, t1):
        gd.line([0, yy, S, yy], fill=lerp((52, 176, 232), (22, 128, 206), (yy - t0) / max(1, t1 - t0)))
    img.paste(grad, mask=tm)
    # ridges
    for k in (-1, 0, 1):
        x = cx + k * 3.7
        ImageDraw.Draw(img).line([px(x + .5), px(cy - 4.4), px(x + .5), px(cy + 4.4)], fill=(120, 205, 245), width=int(px(1.0)))
        ImageDraw.Draw(img).line([px(x - .3), px(cy - 4.4), px(x - .3), px(cy + 4.4)], fill=(18, 108, 178), width=int(px(0.9)))
    ly = y1 + 8.5
    fs = font("Poppins-Regular.ttf", 8.4)
    text((PL + 1, ly), left, fs, anchor="lm")
    text((PR - 1, ly), right, fs, anchor="rm")
    text(((PL + PR) / 2, ly - 0.5), mid, font("Poppins-Regular.ttf", 10.6))

# example values: ENV MODE order = EXP / LIN / CLAP (as now); CHOKE on/off; ATK, DCY
MODE_SEL = 0
CHOKE_ON = True
V_MODE, V_ATK, V_DCY = MODE_SEL * 0.5, 0.10, 0.55

TITLE_Y = 28.0   # same page-title place on every page
text((120, TITLE_Y), "NOISE ENVELOPE", font("Poppins-Regular.ttf", 11.0))

def decay_pts(x0, y_top, y_base, w, k, n=40):
    return [(x0 + w * (i / n), y_base - (y_base - y_top) * math.exp(-k * i / n)) for i in range(n + 1)]

def env_pts(kind, cx, cy, w, h):
    """The three envelope icons, drawn like his picture (EXP spike + curve, LIN triangle, CLAP burst + curve)."""
    L, T, B = cx - w, cy - h, cy + h
    if kind == 0:   # EXP: fast rise, then an exponential curve
        pts = [(L, B), (L + 0.20 * 2 * w, T)]
        pts += decay_pts(L + 0.20 * 2 * w, T, B, 2 * w * 0.80, 4.2)[1:]
        return [pts]
    if kind == 1:   # LIN: straight up, straight down
        return [[(L, B), (L + 0.34 * 2 * w, T), (cx + w, B)]]
    # CLAP: a burst of fast zigzag spikes, then a decaying tail
    x = L; pts = [(x, B)]
    for j in range(4):
        pts += [(x + 1.6, T + 1.2), (x + 3.2, B - 0.5 * h)]
        x += 3.2
    pts += [(x + 1.6, T)]
    tail = decay_pts(x + 1.6, T, B, 2 * w - (x + 1.6 - L), 4.0)
    pts += tail[1:]
    return [pts]

BR, BY = 25.0, 60.0   # same size as the Pitch Mod buttons
def lit_disc(bx, by, r):
    m = disc_mask(bx, by, r)
    tint(blurred(m, 3.0), (120, 205, 245), 0.55)
    raised(m, d=1.8, blur=2.0, a_light=0.7, a_dark=0.6)
    grad = Image.new("RGB", (S, S), BLUE2)
    gd = ImageDraw.Draw(grad)
    for k in range(40, 0, -1):
        t = k / 40.0
        rr = r * t
        gd.ellipse([px(bx - rr), px(by - rr), px(bx + rr), px(by + rr)],
                   fill=lerp((214, 244, 254), (92, 190, 240), t ** 1.6))
    img.paste(grad, mask=m)
    ring = blank(); ImageDraw.Draw(ring).ellipse([px(bx - r), px(by - r), px(bx + r), px(by + r)],
                                                  outline=255, width=int(px(2.2)))
    tint(ring, (60, 178, 238))

for i, bx in enumerate((48, 120, 192)):
    m = disc_mask(bx, BY, BR)
    if i == MODE_SEL:
        lit_disc(bx, BY, BR)
    else:
        raised(m, d=2.0, blur=2.2, a_light=0.9, a_dark=0.8)
        tint(m, (222, 223, 222))
    for pts in env_pts(i, bx, BY + 0.5, 11.2, 11.0):
        ImageDraw.Draw(img).line([(px(x), px(y)) for x, y in pts], fill=(36, 38, 40), width=int(px(1.4)), joint="curve")
text((120, 93.5), "ENVELOPE MODE", font("Poppins-Regular.ttf", 10.0))

# ---------- the CHOKE button (his icon): a rounded square, lights blue when active ----------
def rrect_mask(cx, cy, w, h, r):
    m = blank(); ImageDraw.Draw(m).rounded_rectangle([px(cx - w / 2), px(cy - h / 2), px(cx + w / 2), px(cy + h / 2)], radius=px(r), fill=255)
    return m

def choke_icon(cx, cy, k=1.0, col=(36, 38, 40)):
    dd = ImageDraw.Draw(img)
    for sx in (-1, 1):   # the two walls
        dd.line([px(cx + sx * 9.6 * k), px(cy - 8.4 * k), px(cx + sx * 9.6 * k), px(cy + 8.4 * k)], fill=col, width=int(px(1.3 * k)))
    dd.line([px(cx - 6.6 * k), px(cy - 5.6 * k), px(cx + 6.6 * k), px(cy + 5.6 * k)], fill=col, width=int(px(1.5 * k)))
    rr = 2.6 * k
    dd.ellipse([px(cx - rr), px(cy - rr), px(cx + rr), px(cy + rr)], fill=col)

CHX, CHY, CHS = 120.0, 118.0, 34.0
cm = rrect_mask(CHX, CHY, CHS, CHS, 8.0)
if CHOKE_ON:
    tint(blurred(cm, 3.0), (120, 205, 245), 0.55)
    raised(cm, d=1.6, blur=1.8, a_light=0.7, a_dark=0.6)
    grad = Image.new("RGB", (S, S), BLUE2)
    gd = ImageDraw.Draw(grad)
    t0, t1 = int(px(CHY - CHS / 2)), int(px(CHY + CHS / 2))
    for yy in range(t0, t1):
        gd.line([0, yy, S, yy], fill=lerp((214, 244, 254), (92, 190, 240), ((yy - t0) / max(1, t1 - t0)) ** 1.2))
    img.paste(grad, mask=cm)
    ring = blank(); ImageDraw.Draw(ring).rounded_rectangle([px(CHX - CHS / 2), px(CHY - CHS / 2), px(CHX + CHS / 2), px(CHY + CHS / 2)],
                                                            radius=px(8.0), outline=255, width=int(px(2.0)))
    tint(ring, (60, 178, 238))
else:
    raised(cm, d=1.8, blur=2.0, a_light=0.9, a_dark=0.8)
    tint(cm, (222, 223, 222))
choke_icon(CHX, CHY)
text((120, 142.0), "CHOKE", font("Poppins-Regular.ttf", 8.6))

# ---------- knobs: 4 in a row, as big as the width allows ----------
KD = 56.0                       # knob diameter (4 x 56 + gaps = 240)
GAP = (240 - 4 * KD - 2 * 3.0) / 3
KY = 179.0
LABEL_PT = 14.6                 # ALL 3-letter labels (same size on every page; WAV is the widest and sets it)
LABEL_PT4 = 11.8                # 4-letter labels (DIST): slightly smaller
SWEEP = 330.0                   # arc starts at 6 o'clock, runs clockwise

def knob(cx, cy, label, val, switch=None):
    R = KD / 2
    outer = disc_mask(cx, cy, R)
    tint(outer, (203, 204, 203))
    raised(outer, d=1.7, blur=1.9, a_light=0.9, a_dark=0.8)
    # faint groove for the full range of the arc
    ra, aw = R - 4.2, 4.0
    gm = blank(); gdw = ImageDraw.Draw(gm)
    box = [px(cx - ra), px(cy - ra), px(cx + ra), px(cy + ra)]
    gdw.arc(box, 90, 90 + SWEEP, fill=255, width=int(px(aw)))
    for a in (90, 90 + SWEEP):
        ex, ey = cx + ra * math.cos(math.radians(a)), cy + ra * math.sin(math.radians(a))
        gdw.ellipse([px(ex - aw / 2), px(ey - aw / 2), px(ex + aw / 2), px(ey + aw / 2)], fill=255)
    tint(gm, (189, 191, 190), 1.0)
    # blue value arc, darker blue -> light cyan at the tip
    n = max(2, int(SWEEP * val))
    dd = ImageDraw.Draw(img)
    for i in range(n):
        a0 = 90 + i * (SWEEP * val) / n
        a1 = 90 + (i + 1.6) * (SWEEP * val) / n
        col = lerp(BLUE1, BLUE2, (i / n) ** 1.3)
        dd.arc(box, a0, min(a1, 90 + SWEEP * val), fill=col, width=int(px(aw)))
    def cap(a, col):
        ex, ey = cx + ra * math.cos(math.radians(a)), cy + ra * math.sin(math.radians(a))
        dd.ellipse([px(ex - aw / 2), px(ey - aw / 2), px(ex + aw / 2), px(ey + aw / 2)], fill=col)
    cap(90, BLUE1)
    # white thumb
    ang = 90 + SWEEP * val
    tx, ty = cx + ra * math.cos(math.radians(ang)), cy + ra * math.sin(math.radians(ang))
    tm = disc_mask(tx, ty, 3.6)
    tint(blurred(shifted(tm, 0.6, 1.0), 1.0), (120, 122, 121), 0.8)
    tint(tm, (250, 250, 250))
    # inner button with the label
    rin = R - 8.9
    im_ = disc_mask(cx, cy, rin)
    tint(im_, (206, 207, 206))
    raised(im_, d=1.2, blur=1.3, a_light=0.8, a_dark=0.55)
    inset(disc_mask(cx, cy, rin - 0.1), d=0.5, blur=0.7, a_dark=0.15, a_light=0.15)
    if label == "ICON":
        choke_icon(cx, cy + 0.2, k=1.12)
    else:
        text((cx, cy + 0.3), label, font("Poppins-Medium.ttf", LABEL_PT if len(label) <= 3 else LABEL_PT4))

labels = ["MODE", "ICON", "ATK", "DCY"]     # K2 = the CHOKE icon (on / off)
vals = [V_MODE, 1.0 if CHOKE_ON else 0.0, V_ATK, V_DCY]
G4 = (240 - 4 * KD - 2 * 3.0) / 3
for i in range(4):
    knob(3.0 + KD / 2 + i * (KD + G4), KY, labels[i], vals[i])

# ---------- footer hint ----------
text((120, 229.5), "BLACK: TRACK   WHITE: STEP   FUNC+BLK: MUTE", font("Poppins-Medium.ttf", 9.6), fill=BLUE1)

# ---------- RNDM = RANDOM ALL: purple button in the free 4th place ----------
PURPLE = (166, 106, 224)          # the firmware purple (C_TBOTH, accent + fill)
def rndm_button(cx, cy):
    R = KD / 2
    outer = disc_mask(cx, cy, R)
    tint(outer, (203, 204, 203))
    raised(outer, d=1.7, blur=1.9, a_light=0.9, a_dark=0.8)
    ra, aw = R - 4.2, 4.0
    gm = blank(); ImageDraw.Draw(gm).ellipse([px(cx - ra - aw / 2), px(cy - ra - aw / 2), px(cx + ra + aw / 2), px(cy + ra + aw / 2)], fill=255)
    ImageDraw.Draw(gm).ellipse([px(cx - ra + aw / 2), px(cy - ra + aw / 2), px(cx + ra - aw / 2), px(cy + ra - aw / 2)], fill=0)
    tint(gm, PURPLE, 1.0)                                   # full purple ring instead of the blue arc
    rin = R - 8.9
    im_ = disc_mask(cx, cy, rin)
    raised(im_, d=1.2, blur=1.3, a_light=0.8, a_dark=0.55)
    grad = Image.new("RGB", (S, S), PURPLE)
    gd = ImageDraw.Draw(grad)
    t0, t1 = int(px(cy - rin)), int(px(cy + rin))
    for yy in range(t0, t1):
        gd.line([0, yy, S, yy], fill=lerp((190, 136, 238), (142, 84, 206), (yy - t0) / max(1, t1 - t0)))
    img.paste(grad, mask=im_)
    text((cx, cy + 0.3), "RNDM", font("Poppins-Medium.ttf", LABEL_PT4), fill=(255, 255, 255))

# NO RNDM on this page (his decision): the 4th place stays empty

img = img.resize((FINAL, FINAL), Image.LANCZOS)
img.save(OUT)
print("saved", OUT, img.size)
