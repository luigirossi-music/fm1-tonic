#!/usr/bin/env python3
"""Grigiotonic GLO pages, version 2 (all knob-style: tall faders, purple PAN knobs, battery, list). Picture only.
Run: python3 tools/grigiotonic_glo2_mockup.py vel|sends|mix|comp|mem|set out.png
Screen = 240 x 240 units, drawn 3x supersampled and saved at 1080 x 1080.
"""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops

PAGE = sys.argv[1] if len(sys.argv) > 1 else "mix"
OUT = sys.argv[2] if len(sys.argv) > 2 else "docs/grigiotonic_glo_%s_mockup.png" % PAGE
FD = "/usr/share/fonts/truetype/google-fonts/"
FINAL = 1080
SS = 3
S = FINAL * SS
U = S / 240.0
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

PURPLE = (166, 106, 224)          # the firmware purple
PURPLE_L = (200, 160, 244)
RED = (214, 70, 70)
def px(v): return v * U
def font(name, size): return ImageFont.truetype(FD + name, int(size * U))

TITLE_Y = 28.0
KD, KY = 56.0, 179.0
GAP = (240 - 4 * KD - 2 * 3.0) / 3
LABEL_PT, LABEL_PT4, SWEEP = 14.6, 11.8, 330.0
KX = [3.0 + KD / 2 + i * (KD + GAP) for i in range(4)]   # the four knob columns

def rect_alpha(x0, y0, x1, y1, alpha):
    """fade a region toward the background (the group that is NOT under the knobs)"""
    m = blank(); ImageDraw.Draw(m).rectangle([px(x0), px(y0), px(x1), px(y1)], fill=int(255 * alpha))
    img.paste(Image.new("RGB", (S, S), BG), mask=m)

# ---------- top bar ----------
NAMES = {"vel": "VEL", "sends": "SENDS", "mix": "GLO MIX", "comp": "COMP", "mem": "MEMORY", "set": "SETTINGS"}
d = ImageDraw.Draw(img)
d.rounded_rectangle([px(2), px(2), px(58), px(17)], radius=px(3), fill=(168, 216, 244))
text((30, 9.6), "TRACK 1", font("Poppins-Medium.ttf", 8.6), fill=(28, 30, 34))
text((66, 9.6), NAMES[PAGE], font("Poppins-Medium.ttf", 8.6), fill=BLUE1, anchor="lm")
text((124, 9.6), "P1", font("Poppins-Medium.ttf", 8.6), fill=INK)
text((214, 9.6), "125", font("Poppins-Medium.ttf", 8.6), fill=BLUE1)
d.line([px(60), px(19), px(240), px(19)], fill=(176, 178, 177), width=int(px(0.5)))

# ---------- knob (big, or small for the pan overview) ----------
def knob(cx, cy, label, val, kd=KD, pan=False, pt=None, ring=None):
    R = kd / 2; k = kd / KD
    outer = disc_mask(cx, cy, R)
    tint(outer, (203, 204, 203))
    raised(outer, d=1.7 * k, blur=1.9 * k, a_light=0.9, a_dark=0.8)
    ra, aw = R - 4.2 * k, 4.0 * k
    box = [px(cx - ra), px(cy - ra), px(cx + ra), px(cy + ra)]
    gm = blank(); gdw = ImageDraw.Draw(gm)
    gdw.arc(box, 90, 90 + SWEEP, fill=255, width=int(px(aw)))
    for a in (90, 90 + SWEEP):
        ex, ey = cx + ra * math.cos(math.radians(a)), cy + ra * math.sin(math.radians(a))
        gdw.ellipse([px(ex - aw / 2), px(ey - aw / 2), px(ex + aw / 2), px(ey + aw / 2)], fill=255)
    tint(gm, (189, 191, 190), 1.0)
    dd = ImageDraw.Draw(img)
    centred = pan and abs(val - 0.5) < 0.02
    a_c = 90 + SWEEP * 0.5                      # the CENTRE of the pan (the starting point)
    if pan and centred:
        # PAN exactly in the middle: PURPLE appears on BOTH sides of the thumb (and only then)
        for lo, hi in ((a_c - 28, a_c - 5), (a_c + 5, a_c + 28)):
            n = 20
            for i in range(n):
                a0 = lo + i * (hi - lo) / n
                dd.arc(box, a0, a0 + (hi - lo) / n * 1.6, fill=lerp(PURPLE, PURPLE_L, i / n), width=int(px(aw)))
        ang = a_c
    elif pan:
        # PAN to a side: a BLUE trace from the MIDDLE to the thumb
        a_t = 90 + SWEEP * val
        lo, hi = min(a_c, a_t), max(a_c, a_t)
        n = max(2, int(hi - lo))
        for i in range(n):
            a0 = lo + i * (hi - lo) / n
            t = i / n if a_t > a_c else 1 - i / n
            dd.arc(box, a0, min(a0 + (hi - lo) / n * 1.6, hi), fill=lerp(BLUE1, BLUE2, t), width=int(px(aw)))
        ang = a_t
    else:
        n = max(2, int(SWEEP * val))
        for i in range(n):
            a0 = 90 + i * (SWEEP * val) / n
            a1 = 90 + (i + 1.6) * (SWEEP * val) / n
            dd.arc(box, a0, min(a1, 90 + SWEEP * val), fill=lerp(BLUE1, BLUE2, (i / n) ** 1.3), width=int(px(aw)))
        ex, ey = cx + ra * math.cos(math.radians(90)), cy + ra * math.sin(math.radians(90))
        dd.ellipse([px(ex - aw / 2), px(ey - aw / 2), px(ex + aw / 2), px(ey + aw / 2)], fill=BLUE1)
        ang = 90 + SWEEP * val
    tx, ty = cx + ra * math.cos(math.radians(ang)), cy + ra * math.sin(math.radians(ang))
    tm = disc_mask(tx, ty, 3.6 * k)
    tint(blurred(shifted(tm, 0.6 * k, 1.0 * k), 1.0 * k), (120, 122, 121), 0.8)
    tint(tm, (250, 250, 250))
    rin = R - 8.9 * k
    im_ = disc_mask(cx, cy, rin)
    tint(im_, (206, 207, 206))
    raised(im_, d=1.2 * k, blur=1.3 * k, a_light=0.8, a_dark=0.55)
    inset(disc_mask(cx, cy, rin - 0.1), d=0.5, blur=0.7, a_dark=0.15, a_light=0.15)
    if pt is None:
        pt = (LABEL_PT if len(label) <= 3 else LABEL_PT4) * k      # ONE rule everywhere: 3 chars / digits 14.6, 4 letters 11.8 (scaled for small knobs)
    text((cx, cy + 0.3 * k), label, font("Poppins-Medium.ttf", pt))

# ---------- the horizontal slider (his picture) ----------
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


# ---------- fader: groove, BLUE = live volume, WHITE dot = the setting ----------
def fader(cx, y0, y1, level, meter, w=30, color=None, dot=True):
    m = pill_mask(cx - w / 2, y0, cx + w / 2, y1)
    tint(m, (189, 191, 190)); inset(m, d=1.5, blur=1.6)
    gw = 7.0
    yb, yt = y1 - 8.0, y0 + 8.0
    ch = pill_mask(cx - gw / 2, yt - gw / 2, cx + gw / 2, yb + gw / 2)
    tint(ch, (176, 178, 177)); inset(ch, d=0.8, blur=0.9, a_dark=0.5, a_light=0.6)
    if meter > 0.001:
        top = yb - (yb - yt) * meter
        fm = pill_mask(cx - gw / 2, top - gw / 2, cx + gw / 2, yb + gw / 2)
        grad = Image.new("RGB", (S, S), BLUE1); gd = ImageDraw.Draw(grad)
        t0, t1 = int(px(top - gw / 2)), int(px(yb + gw / 2))
        for yy in range(t0, t1):
            gd.line([0, yy, S, yy], fill=lerp(BLUE2, BLUE1, ((yy - t0) / max(1, t1 - t0)) ** 0.8))
        img.paste(grad, mask=fm)
    if dot:
        ty = yb - (yb - yt) * level
        tm = disc_mask(cx, ty, 4.4)
        tint(blurred(shifted(tm, 0.7, 1.2), 1.2), (120, 122, 121), 0.8)
        tint(tm, (250, 250, 250))

def vslider(cx, y0, y1, val, w=28):
    """the STANDARD slider of the skin (pill groove, thin line, blue thumb with 3 ridges) drawn VERTICAL"""
    m = pill_mask(cx - w / 2, y0, cx + w / 2, y1)
    tint(m, (201, 202, 201)); inset(m, d=1.7, blur=1.9)
    ly0, ly1 = y0 + 21, y1 - 20                # the thin line / thumb travel (same margins as the horizontal one)
    ImageDraw.Draw(img).line([px(cx), px(ly0), px(cx), px(ly1)], fill=(52, 54, 56), width=int(px(0.9)))
    cy = ly1 - val * (ly1 - ly0)
    TWv, THv = 22.0, 43.0
    tm = pill_mask(cx - TWv / 2, cy - THv / 2, cx + TWv / 2, cy + THv / 2)
    raised(tm, d=1.5, blur=1.6, a_light=0.5, a_dark=0.85)
    grad = Image.new("RGB", (S, S), BLUE1); gd = ImageDraw.Draw(grad)
    t0, t1 = int(px(cx - TWv / 2)), int(px(cx + TWv / 2))
    for xx in range(t0, t1):
        gd.line([xx, 0, xx, S], fill=lerp((52, 176, 232), (22, 128, 206), (xx - t0) / max(1, t1 - t0)))
    img.paste(grad, mask=tm)
    for k in (-1, 0, 1):
        y = cy + k * 3.7
        ImageDraw.Draw(img).line([px(cx - 4.4), px(y + .5), px(cx + 4.4), px(y + .5)], fill=(120, 205, 245), width=int(px(1.0)))
        ImageDraw.Draw(img).line([px(cx - 4.4), px(y - .3), px(cx + 4.4), px(y - .3)], fill=(18, 108, 178), width=int(px(0.9)))

def footer(s="BLACK: TRACK   WHITE: STEP   FUNC+BLK: MUTE"):
    text((120, 229.5), s, font("Poppins-Medium.ttf", 9.6), fill=BLUE1)

def title(s): text((120, TITLE_Y), s, font("Poppins-Regular.ttf", 11.0))
def small(xy, s, sz=8.4, fill=INK, anchor="mm"): text(xy, s, font("Poppins-Regular.ttf", sz), fill=fill, anchor=anchor)

LEV = [0.80, 0.62, 0.80, 0.45, 0.70, 0.55, 0.80, 0.30]    # example levels (white dots)
MET = [0.55, 0.30, 0.72, 0.0, 0.40, 0.0, 0.66, 0.12]      # example live volume (blue)
PAN = [0.50, 0.20, 0.50, 0.85, 0.35, 0.50, 0.65, 0.50]    # example pans (0.5 = centre)
FX8 = [3.0 + 14.8 + i * (234.0 - 29.6) / 7 for i in range(8)]   # 8 columns across the screen

if PAGE == "mix":
    title("GLOBAL MIXER")
    for i in range(8):
        fader(FX8[i], 36, 150, LEV[i], MET[i], w=22)
    # 8 pan knobs under the 8 faders (track number inside); centre = purple ring, side = blue trace
    for i in range(8):
        knob(FX8[i], 172, str(i + 1), PAN[i], kd=27.0, pan=True)
    # the group the four big knobs of the hardware act on (SELECT switches 1-4 / 5-8)
    gl = pill_mask(FX8[0] - 13, 191, FX8[3] + 13, 193.4); tint(gl, BLUE1)
    for i, col in ((0, PURPLE), (1, RED)):    # purple = sidechain input, red = ducked (as in the firmware)
        ImageDraw.Draw(img).ellipse([px(FX8[i] - 3.6), px(200 - 3.6), px(FX8[i] + 3.6), px(200 + 3.6)], fill=col)
    footer()

elif PAGE in ("vel", "sends"):
    # no knobs: the STANDARD slider (his picture: pill, thin line, blue thumb with 3 ridges) drawn VERTICAL, in the four knob columns
    if PAGE == "vel":
        title("VELOCITY")
        items = [("OSC", 0.60, "60%"), ("NOISE", 0.30, "30%"), ("PITCH MOD", 0.45, "45%")]
    else:
        title("SENDS")
        items = [("REVERB", 0.30, "30%"), ("DELAY", 0.15, "15%"), ("SC INPUT", 2 / 7.0, "TRACK 3"), ("THRESHOLD", 0.40, "40%")]
    for i, (nm, v, vt) in enumerate(items):
        vslider(KX[i], 37, 184, v, w=30)
        small((KX[i], 195), nm, 8.6)
        text((KX[i], 207), vt, font("Poppins-Medium.ttf", 10.0), fill=BLUE1)
    footer()

elif PAGE == "comp":
    title("COMPRESSOR")
    # the gain reduction dial of the old screen (0-20 dB, needle, 15 / 20 in purple, 4:1), in the skin
    pm = rrect = blank(); ImageDraw.Draw(pm).rounded_rectangle([px(8), px(35), px(232), px(148)], radius=px(9), fill=255)
    tint(pm, (189, 191, 190)); inset(pm, d=1.7, blur=1.9)
    CX, CY, RR = 120.0, 141.0, 82.0
    def pol(r, a): return (CX + r * math.cos(math.radians(a)), CY - r * math.sin(math.radians(a)))
    A0, A1 = 148.0, 32.0                      # 0 dB at the left, 20 dB at the right
    def ang(db): return A0 + (A1 - A0) * db / 20.0
    dd = ImageDraw.Draw(img)
    for k in range(0, 21):
        major = k % 5 == 0
        r0, r1 = (RR - 9, RR) if major else (RR - 5, RR)
        col = PURPLE if k >= 15 else (52, 54, 56)
        p0, p1 = pol(r0, ang(k)), pol(r1, ang(k))
        dd.line([px(p0[0]), px(p0[1]), px(p1[0]), px(p1[1])], fill=col, width=int(px(1.5 if major else 0.8)))
        if major:
            q = pol(RR + 8.5, ang(k))
            text(q, str(k), font("Poppins-Medium.ttf", 9.4), fill=PURPLE if k >= 15 else INK)
    GR = 6.0                                   # example: 6 dB of gain reduction
    tip = pol(RR - 6, ang(GR)); base = pol(0, ang(GR))
    nd = blank(); ImageDraw.Draw(nd).line([px(CX), px(CY), px(tip[0]), px(tip[1])], fill=255, width=int(px(1.7)))
    tint(blurred(shifted(nd, 0.6, 1.0), 0.9), (120, 122, 121), 0.6)
    dd.line([px(CX), px(CY), px(tip[0]), px(tip[1])], fill=PURPLE, width=int(px(1.7)))
    hub = disc_mask(CX, CY, 6.0); tint(hub, (206, 207, 206)); raised(hub, d=1.0, blur=1.1, a_light=0.8, a_dark=0.6)
    dd.ellipse([px(CX - 2.6), px(CY - 2.6), px(CX + 2.6), px(CY + 2.6)], fill=(110, 112, 114))
    text((16, 138), "%.0f DB" % GR, font("Poppins-Medium.ttf", 10.5), anchor="lm")
    text((224, 138), "4:1", font("Poppins-Regular.ttf", 10.0), fill=(130, 132, 134), anchor="rm")
    text((16, 43), "GAIN REDUCTION DB", font("Poppins-Regular.ttf", 7.4), fill=(110, 112, 114), anchor="lm")
    setv = [0.25, 0.40, 0.55, 0.30]
    for i, l in enumerate(["ATK", "REL", "THRS", "GAIN"]): knob(KX[i], KY, l, setv[i])
    footer()

elif PAGE == "mem":
    title("MEMORY")
    pct = 0.38
    bx0, bx1, by0, by1 = 78, 162, 44, 188          # the battery body, vertical
    nub = pill_mask(108, 36, 132, 48); tint(nub, (176, 178, 177)); inset(nub, d=0.8, blur=0.9)
    body = pill_mask(bx0, by0, bx1, by1)
    tint(body, (189, 191, 190)); inset(body, d=1.8, blur=2.0)
    ix0, ix1, iy0, iy1 = bx0 + 9, bx1 - 9, by0 + 9, by1 - 9
    inner = pill_mask(ix0, iy0, ix1, iy1)
    tint(inner, (176, 178, 177)); inset(inner, d=1.0, blur=1.1, a_dark=0.5, a_light=0.6)
    top = iy1 - (iy1 - iy0) * pct
    fm = pill_mask(ix0, min(top, iy1 - 24), ix1, iy1)
    grad = Image.new("RGB", (S, S), BLUE1); gd = ImageDraw.Draw(grad)
    t0, t1 = int(px(top)), int(px(iy1))
    for yy in range(t0, t1):
        gd.line([0, yy, S, yy], fill=lerp(BLUE2, BLUE1, ((yy - t0) / max(1, t1 - t0)) ** 0.8))
    img.paste(grad, mask=fm)
    # the purple zone starts at 90 %
    y90 = iy1 - (iy1 - iy0) * 0.90
    ImageDraw.Draw(img).line([px(ix0 + 3), px(y90), px(ix1 - 3), px(y90)], fill=PURPLE, width=int(px(1.6)))
    small((ix1 + 6, y90), "90%", 8.4, fill=PURPLE, anchor="lm")
    small((ix0 - 6, iy1 - 2), "0", 8.4, anchor="rm")
    text((120, 118), "38%", font("Poppins-Medium.ttf", 20), fill=(250, 250, 250) if False else INK)
    small((120, 207), "PROJECT MEMORY", 9.4)
    footer("GLO: SETTINGS   HOME: BACK")

elif PAGE == "set":
    title("SETTINGS")
    rows = [("CLOCK OUT", "OFF"), ("NOTES OUT", "OFF"), ("TONIC MIDI CH", "10"), ("OUTPUT", "LOUD"), ("USB AUDIO", "FOLLOWS KNOB"),
            ("KEY LIGHT", "8"), ("4/4 LED", "ON")]
    sel = 3; y = 36.0; RH = 24.0
    for i, (a, b) in enumerate(rows):
        y0, y1 = y + i * (RH + 2.2), y + i * (RH + 2.2) + RH
        m = pill_mask(8, y0, 224, y1)
        if i == sel:
            tint(blurred(m, 2.5), (120, 205, 245), 0.5)
            raised(m, d=1.4, blur=1.6, a_light=0.6, a_dark=0.6)
            grad = Image.new("RGB", (S, S), BLUE1); gd = ImageDraw.Draw(grad)
            for yy in range(int(px(y0)), int(px(y1))):
                gd.line([0, yy, S, yy], fill=lerp((52, 176, 232), (22, 128, 206), (yy - px(y0)) / px(RH)))
            img.paste(grad, mask=m)
            text((18, (y0 + y1) / 2 + 0.3), a, font("Poppins-Medium.ttf", 10.5), fill=(255, 255, 255), anchor="lm")
            text((214, (y0 + y1) / 2 + 0.3), b, font("Poppins-Medium.ttf", 10.5), fill=(255, 255, 255), anchor="rm")
        else:
            tint(m, (201, 202, 201)); inset(m, d=1.3, blur=1.5)
            text((18, (y0 + y1) / 2 + 0.3), a, font("Poppins-Regular.ttf", 10.5), anchor="lm")
            text((214, (y0 + y1) / 2 + 0.3), b, font("Poppins-Regular.ttf", 10.5), fill=BLUE1, anchor="rm")
    # scroll bar (the list has about 19 rows)
    sb = pill_mask(229, 36, 235, 214); tint(sb, (189, 191, 190)); inset(sb, d=0.8, blur=0.9)
    st = pill_mask(229.2, 36 + 178 * 0.12, 234.8, 36 + 178 * 0.12 + 34); tint(st, BLUE1)
    footer("SELECT: MOVE   ALGO: CHANGE   HOME: BACK")

img = img.resize((FINAL, FINAL), Image.LANCZOS)
img.save(OUT)
print("saved", OUT, img.size)
