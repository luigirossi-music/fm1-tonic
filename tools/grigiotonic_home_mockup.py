#!/usr/bin/env python3
"""Grigiotonic HOME pages (1st press = MIXER 2, 2nd press = SENDS). Based on the MIX page script.
Run: python3 tools/grigiotonic_home_mockup.py home1|home2|home3 out.png
OLD DOC: Grigiotonic MIX page mockup (picture only, no firmware code).
Screen = 240 x 240 units, drawn 3x supersampled and saved at 1080 x 1080.
Run: python3 tools/grigiotonic_mix_mockup.py [out.png]
"""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops

PAGE = sys.argv[1] if len(sys.argv) > 1 else "home1"
OUT = sys.argv[2] if len(sys.argv) > 2 else "docs/grigiotonic_%s_mockup.png" % PAGE
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
text((66, 9.6), {"home1": "MIX 2", "home2": "SENDS", "home3": "GLOBAL"}[PAGE], font("Poppins-Medium.ttf", 8.6), fill=BLUE1, anchor="lm")
text((124, 9.6), "P1", font("Poppins-Medium.ttf", 8.6), fill=INK)
text((214, 9.6), "125", font("Poppins-Medium.ttf", 8.6), fill=BLUE1)
d.line([px(60), px(19), px(240), px(19)], fill=(176, 178, 177), width=int(px(0.5)))

# ---------- sliders (visual reference: they follow knob 1 and knob 2) ----------
PL, PR = 8, 232
LX0, LX1 = PL + 21, PR - 20          # the thin line / thumb travel
TW, TH = 43, 21

def slider(y0, y1, val, left, mid, right):
    TH = min(21, (y1 - y0) - 5)
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
    ly = y1 + (8.5 if y1 - y0 > 22 else 7.0)
    fs = font("Poppins-Regular.ttf", 8.4)
    text((PL + 1, ly), left, fs, anchor="lm")
    text((PR - 1, ly), right, fs, anchor="rm")
    text(((PL + PR) / 2, ly - 0.5), mid, font("Poppins-Regular.ttf", 10.6))

if PAGE == "home1":      # HOME 1st press: LEVEL, PAN, LOW CUT, HIGH CUT of the selected track
    TITLE, LABELS = "MIXER 2", ["LVL", "PAN", "LCUT", "HCUT"]
    VALS = [0.80, 0.50, 0.12, 0.45]                     # example values (level default 101/127, pan centre)
    slider(38, 64, VALS[0], "0", "LEVEL", "+10DB")
    slider(92, 118, VALS[1], "L", "PAN", "R")
elif PAGE == "home2":    # HOME 2nd press: REVERB SEND, DELAY SEND, SIDECHAIN INPUT (a NUMBER 1-8), SIDECHAIN THRESHOLD
    TITLE, LABELS = "SENDS", ["REV", "DLY", "3", "THRS"]   # K3 shows the input track NUMBER instead of a label
    VALS = [0.30, 0.15, 2 / 7.0, 0.40]                  # input track 3 of 8
    slider(38, 64, VALS[0], "0", "REVERB SEND", "100%")
    slider(92, 118, VALS[1], "0", "DELAY SEND", "100%")
else:                    # HOME 3rd press: GLOBAL = QUANTIZATION, SWING, FILL RATE (numbers on the knob), TEMPO (number)
    TITLE, LABELS = "GLOBAL", ["QNT", "SWG", "4X", "125"]  # K3 = fill rate as a number, K4 = tempo as a number
    VALS = [0.50, 0.25, 0.50, (125 - 20) / 255.0]
    slider(35, 55, VALS[0], "1/8", "QUANTIZATION", "1/32")
    slider(73, 93, VALS[1], "0%", "SWING", "100%")
    slider(111, 131, VALS[2], "2X", "FILL RATE", "8X")

TITLE_Y = 28.0   # same page-title place on every page
text((120, TITLE_Y), TITLE, font("Poppins-Regular.ttf", 11.0))

# ---------- knobs: 4 in a row, as big as the width allows ----------
KD = 56.0                       # knob diameter (4 x 56 + gaps = 240)
GAP = (240 - 4 * KD - 2 * 3.0) / 3
KY = 179.0
LABEL_PT = 14.6                 # ALL 3-letter labels (same size on every page; WAV is the widest and sets it)
LABEL_PT4 = 11.8                # 4-letter labels (DIST): slightly smaller
SWEEP = 330.0                   # arc starts at 6 o'clock, runs clockwise

def knob(cx, cy, label, val):
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
    pt = (LABEL_PT) if label[0].isdigit() else (LABEL_PT if len(label) <= 3 else LABEL_PT4)
    text((cx, cy + 0.3), label, font("Poppins-Medium.ttf", pt))

labels, vals = LABELS, VALS
for i in range(4):
    knob(3.0 + KD / 2 + i * (KD + GAP), KY, labels[i], vals[i])

# ---------- footer hint ----------
text((120, 229.5), "BLACK: TRACK   WHITE: STEP   FUNC+BLK: MUTE", font("Poppins-Medium.ttf", 9.6), fill=BLUE1)

img = img.resize((FINAL, FINAL), Image.LANCZOS)
img.save(OUT)
print("saved", OUT, img.size)
