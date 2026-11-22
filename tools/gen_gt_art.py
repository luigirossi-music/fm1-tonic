#!/usr/bin/env python3
"""Grigiotonic artwork for the firmware (wip547): the slider groove, the slider thumb, the knob base and a Poppins
glyph atlas, drawn with the SAME geometry / shadows as tools/grigiotonic_mix_mockup.py (units = pixels of the
240 screen), 8x supersampled, reduced to 1x. Every sprite is RGB565 + 8-bit alpha (alpha found by drawing the
sprite on black and on white). Output: firmware/src/app/gt_art.h (committed; run this only to change the art).
Run: python3 tools/gen_gt_art.py [out.h]
"""
import math, sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops

OUT = sys.argv[1] if len(sys.argv) > 1 else "firmware/src/app/gt_art.h"
FD = "/usr/share/fonts/truetype/google-fonts/"
SS = 8
INK = (62, 64, 66)
BLUE1 = (28, 140, 214)
BLUE2 = (70, 200, 240)
LIGHT = (233, 234, 233)
DARK = (146, 148, 147)


class Cv:
    def __init__(s, w, h, bg):
        s.w, s.h = w, h
        s.W, s.H = w * SS, h * SS
        s.img = Image.new("RGB", (s.W, s.H), bg)

    def px(s, v): return v * SS
    def blank(s): return Image.new("L", (s.W, s.H), 0)

    def shifted(s, m, dx, dy):
        o = s.blank(); o.paste(m, (int(s.px(dx)), int(s.px(dy)))); return o

    def blurred(s, m, r): return m.filter(ImageFilter.GaussianBlur(s.px(r)))

    def tint(s, m, color, alpha=1.0):
        mm = m.point(lambda v: int(v * alpha))
        s.img.paste(Image.new("RGB", (s.W, s.H), color), mask=mm)

    def raised(s, m, d=2.2, blur=2.4, a_light=0.95, a_dark=0.75):
        s.tint(s.blurred(s.shifted(m, -d, -d), blur), LIGHT, a_light)
        s.tint(s.blurred(s.shifted(m, d, d), blur), DARK, a_dark)

    def inset(s, m, d=1.6, blur=1.6, a_dark=0.75, a_light=0.9):
        inv = ImageChops.invert(m)
        dk = ImageChops.multiply(s.blurred(s.shifted(inv, d, d), blur), m)
        lt = ImageChops.multiply(s.blurred(s.shifted(inv, -d, -d), blur), m)
        s.tint(dk, DARK, a_dark)
        s.tint(lt, LIGHT, a_light)

    def pill(s, x0, y0, x1, y1):
        m = s.blank(); ImageDraw.Draw(m).rounded_rectangle(
            [s.px(x0), s.px(y0), s.px(x1) - 1, s.px(y1) - 1], radius=s.px((y1 - y0) / 2), fill=255); return m

    def disc(s, cx, cy, r):
        m = s.blank(); ImageDraw.Draw(m).ellipse(
            [s.px(cx - r), s.px(cy - r), s.px(cx + r) - 1, s.px(cy + r) - 1], fill=255); return m

    def done(s): return np.asarray(s.img.reduce(SS), dtype=np.float64)


def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def matte(draw, w, h):
    """draw(Cv) twice (black / white ground) -> (rgb 0..255 straight colour, alpha 0..255)"""
    a = Cv(w, h, (0, 0, 0)); draw(a); ra = a.done()
    b = Cv(w, h, (255, 255, 255)); draw(b); rb = b.done()
    al = 1.0 - (rb - ra).mean(axis=2) / 255.0
    al = np.clip(al, 0.0, 1.0)
    col = np.where(al[..., None] > 0.004, ra / np.maximum(al[..., None], 0.004), 0.0)
    return np.clip(col, 0, 255), np.clip(np.round(al * 255), 0, 255).astype(np.uint8)


def rgb565(c):
    c = c.astype(np.int32)
    return ((c[..., 0] >> 3) << 11) | ((c[..., 1] >> 2) << 5) | (c[..., 2] >> 3)


# ---------------- sprites ----------------
GM = 4                    # groove margin: the inset shadow needs 'outside' around the pill
GW, GH = 224, 26          # groove
def draw_groove(c, gh=GH):
    m = c.pill(GM, GM, GM + GW, GM + gh)
    c.tint(m, (201, 202, 201))
    c.inset(m, d=1.7, blur=1.9)

GH20 = 20                 # wip557: the GLOBAL page's three sliders: a groove 20 high (mockup rows 35 / 73 / 111) and a thumb 15 high
def draw_groove20(c): draw_groove(c, GH20)

TM = 8                    # thumb margin
TW, TH = 43, 21
SW, SH = TW + 2 * TM, TH + 2 * TM
def draw_thumb(c, TH=TH):
    cx, cy = SW / 2.0, c.H / SS / 2.0
    tm = c.pill(cx - TW / 2.0, cy - TH / 2.0, cx + TW / 2.0, cy + TH / 2.0)
    c.raised(tm, d=1.5, blur=1.6, a_light=0.5, a_dark=0.85)
    grad = Image.new("RGB", (c.W, c.H), BLUE1)
    gd = ImageDraw.Draw(grad)
    t0, t1 = int(c.px(cy - TH / 2.0)), int(c.px(cy + TH / 2.0))
    for yy in range(t0, t1):
        gd.line([0, yy, c.W, yy], fill=lerp((52, 176, 232), (22, 128, 206), (yy - t0) / max(1, t1 - t0)))
    c.img.paste(grad, mask=tm)
    d = ImageDraw.Draw(c.img)
    for k in (-1, 0, 1):
        x = cx + k * 3.7
        d.line([c.px(x + .5), c.px(cy - 4.4), c.px(x + .5), c.px(cy + 4.4)], fill=(120, 205, 245), width=int(c.px(1.0)))
        d.line([c.px(x - .3), c.px(cy - 4.4), c.px(x - .3), c.px(cy + 4.4)], fill=(18, 108, 178), width=int(c.px(0.9)))

TH15 = 15                 # (the mockup: TH = min(21, groove - 5))
SH15 = TH15 + 2 * TM
def draw_thumb15(c): draw_thumb(c, TH15)

KM = 8                    # knob margin
KR = 28.0
KS = 2 * (int(KR) + KM) + 1   # 73, centre at KS / 2 (a pixel centre)
def draw_knob(c, A0=90):                              # A0 = where the groove starts (degrees, 90 = 6 o'clock); 105 = the bipolar (pan) knob, symmetric about 12 o'clock
    cx = cy = KS / 2.0
    R = KR
    outer = c.disc(cx, cy, R)
    c.tint(outer, (203, 204, 203))
    c.raised(outer, d=1.7, blur=1.9, a_light=0.9, a_dark=0.8)
    ra, aw, SWEEP = R - 4.2, 4.0, 330.0
    gm = c.blank(); gdw = ImageDraw.Draw(gm)
    box = [c.px(cx - ra), c.px(cy - ra), c.px(cx + ra), c.px(cy + ra)]
    gdw.arc(box, A0, A0 + SWEEP, fill=255, width=int(c.px(aw)))
    for a in (A0, A0 + SWEEP):
        ex, ey = cx + ra * math.cos(math.radians(a)), cy + ra * math.sin(math.radians(a))
        gdw.ellipse([c.px(ex - aw / 2), c.px(ey - aw / 2), c.px(ex + aw / 2), c.px(ey + aw / 2)], fill=255)
    c.tint(gm, (189, 191, 190), 1.0)
    rin = R - 8.9
    im_ = c.disc(cx, cy, rin)
    c.tint(im_, (206, 207, 206))
    c.raised(im_, d=1.2, blur=1.3, a_light=0.8, a_dark=0.55)
    c.inset(c.disc(cx, cy, rin - 0.1), d=0.5, blur=0.7, a_dark=0.15, a_light=0.15)


BR_, BM = 25.0, 7         # round mode button (OSC wave buttons): radius, margin for the shadow / glow
BS = 2 * (int(BR_) + BM) + 1   # 65, centre at BS / 2 (a pixel centre)
def draw_btn_off(c):
    cx = cy = BS / 2.0
    m = c.disc(cx, cy, BR_)
    c.raised(m, d=2.0, blur=2.2, a_light=0.9, a_dark=0.8)
    c.tint(m, (222, 223, 222))

def draw_btn_on(c):                                   # the lit one: soft blue glow, radial light-blue gradient, blue ring
    cx = cy = BS / 2.0
    m = c.disc(cx, cy, BR_)
    c.tint(c.blurred(m, 3.0), (120, 205, 245), 0.55)
    c.raised(m, d=1.8, blur=2.0, a_light=0.7, a_dark=0.6)
    grad = Image.new("RGB", (c.W, c.H), BLUE2)
    gd = ImageDraw.Draw(grad)
    for k in range(40, 0, -1):
        t = k / 40.0
        rr = BR_ * t
        gd.ellipse([c.px(cx - rr), c.px(cy - rr), c.px(cx + rr), c.px(cy + rr)], fill=lerp((214, 244, 254), (92, 190, 240), t ** 1.6))
    c.img.paste(grad, mask=m)
    ring = c.blank()
    ImageDraw.Draw(ring).ellipse([c.px(cx - BR_), c.px(cy - BR_), c.px(cx + BR_) - 1, c.px(cy + BR_) - 1], outline=255, width=int(c.px(2.2)))
    c.tint(ring, (60, 178, 238))


CS, CW_, CR_ = 49, 34.0, 8.0      # CHOKE button (NOISE ENV): rounded square 34 x 34, radius 8, margin 7.5
def rrect(c, cx, cy, w, h, r):
    m = c.blank(); ImageDraw.Draw(m).rounded_rectangle([c.px(cx - w / 2), c.px(cy - h / 2), c.px(cx + w / 2) - 1, c.px(cy + h / 2) - 1], radius=c.px(r), fill=255); return m

def draw_chk_off(c):
    cx = cy = CS / 2.0
    m = rrect(c, cx, cy, CW_, CW_, CR_)
    c.raised(m, d=1.8, blur=2.0, a_light=0.9, a_dark=0.8)
    c.tint(m, (222, 223, 222))

def draw_chk_on(c):
    cx = cy = CS / 2.0
    m = rrect(c, cx, cy, CW_, CW_, CR_)
    c.tint(c.blurred(m, 3.0), (120, 205, 245), 0.55)
    c.raised(m, d=1.6, blur=1.8, a_light=0.7, a_dark=0.6)
    grad = Image.new("RGB", (c.W, c.H), BLUE2)
    gd = ImageDraw.Draw(grad)
    t0, t1 = int(c.px(cy - CW_ / 2)), int(c.px(cy + CW_ / 2))
    for yy in range(t0, t1):
        gd.line([0, yy, c.W, yy], fill=lerp((214, 244, 254), (92, 190, 240), ((yy - t0) / max(1, t1 - t0)) ** 1.2))
    c.img.paste(grad, mask=m)
    ring = c.blank()
    ImageDraw.Draw(ring).rounded_rectangle([c.px(cx - CW_ / 2), c.px(cy - CW_ / 2), c.px(cx + CW_ / 2) - 1, c.px(cy + CW_ / 2) - 1], radius=c.px(CR_), outline=255, width=int(c.px(2.0)))
    c.tint(ring, (60, 178, 238))


# wip564: the GLO MIXER: the tall fader groove (pill 22 x 114 + the thin channel) and the small pan knob (diameter 27)
FM_ = 4                   # fader margin
FW, FHT = 22, 114
FSW, FSH = FW + 2 * FM_, FHT + 2 * FM_
def draw_fader(c):
    m = c.pill(FM_, FM_, FM_ + FW, FM_ + FHT)
    c.tint(m, (189, 191, 190))
    c.inset(m, d=1.5, blur=1.6)
    gw = 7.0
    cx = FM_ + FW / 2.0
    yt, yb = FM_ + 8.0, FM_ + FHT - 8.0
    ch = c.pill(cx - gw / 2, yt - gw / 2, cx + gw / 2, yb + gw / 2)
    c.tint(ch, (176, 178, 177))
    c.inset(ch, d=0.8, blur=0.9, a_dark=0.5, a_light=0.6)

SKD = 27.0                # small knob diameter = the mockup's kd
SK = SKD / 56.0
SKR = SKD / 2
SKM = 7
SKS = 2 * (int(SKR) + SKM)    # 40, centre at SKS / 2 (a pixel EDGE)
def draw_sknob(c, A0=90):
    cx = cy = SKS / 2.0
    R = SKR
    outer = c.disc(cx, cy, R)
    c.tint(outer, (203, 204, 203))
    c.raised(outer, d=1.7 * SK, blur=1.9 * SK, a_light=0.9, a_dark=0.8)
    ra, aw, SWEEP = R - 4.2 * SK, 4.0 * SK, 330.0
    gm = c.blank(); gdw = ImageDraw.Draw(gm)
    box = [c.px(cx - ra), c.px(cy - ra), c.px(cx + ra), c.px(cy + ra)]
    gdw.arc(box, A0, A0 + SWEEP, fill=255, width=int(c.px(aw)))
    for a in (A0, A0 + SWEEP):
        ex, ey = cx + ra * math.cos(math.radians(a)), cy + ra * math.sin(math.radians(a))
        gdw.ellipse([c.px(ex - aw / 2), c.px(ey - aw / 2), c.px(ex + aw / 2), c.px(ey + aw / 2)], fill=255)
    c.tint(gm, (189, 191, 190), 1.0)
    rin = R - 8.9 * SK
    im_ = c.disc(cx, cy, rin)
    c.tint(im_, (206, 207, 206))
    c.raised(im_, d=1.2 * SK, blur=1.3 * SK, a_light=0.8, a_dark=0.55)
    c.inset(c.disc(cx, cy, rin - 0.1), d=0.5 * SK, blur=0.7 * SK, a_dark=0.15, a_light=0.15)


# ---------------- glyph atlas ----------------
TEXT_GAMMA = 0.7          # < 1 = bolder small text (the mockup was reduced with Lanczos from 3x: crisper, darker)
CHARS = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:+-/%'#,()&*<>=!?"
FACES = [          # name, file, em in screen pixels (the sizes of the mockup)
    ("TITLE", "Poppins-Regular.ttf", 13.0),
    ("MID", "Poppins-Regular.ttf", 13.0),
    ("SIDE", "Poppins-Regular.ttf", 10.6),
    ("KNOB3", "Poppins-Medium.ttf", 14.6),
    ("KNOB4", "Poppins-Medium.ttf", 11.8),
    ("FOOT", "Poppins-Medium.ttf", 9.6),
    ("BAR", "Poppins-Medium.ttf", 8.6),
    ("DIAL", "Poppins-Medium.ttf", 9.4),         # wip566: the compressor dial: scale numbers, the small caption, the dB value, the ratio
    ("DIALS", "Poppins-Regular.ttf", 7.4),
    ("DIALV", "Poppins-Medium.ttf", 10.5),
    ("DIALR", "Poppins-Regular.ttf", 10.0),
    ("KNOBS", "Poppins-Medium.ttf", round(14.6 * 27.0 / 56.0, 2)),     # wip564: the label of the small pan knobs (14.6 scaled with the knob)
    ("PNUM", "Poppins-Medium.ttf", 48.0, "0123456789"),               # wip568: the big pattern number in the PERFORM circle (digits only: the other glyphs are empty, adv 0)
    ("ZVAL", "Poppins-Medium.ttf", 34.0, "0123456789+-.%/ ABCDEFGHIJKLMNOPQRSTUVWXYZ"),         # wip576: the big value of the knob zoom (digits, signs, capitals)
]


def face(fname, em, only=None):
    f = ImageFont.truetype(FD + fname, int(round(em * SS)))
    asc, desc = f.getmetrics()
    mid = (asc - desc) / 2.0 / SS          # the baseline lies 'mid' below the middle anchor
    pad = 4
    glyphs = []
    for ch in CHARS:
        if only is not None and ch not in only:
            glyphs.append((0.0, 0, 0, 0, 0, np.zeros((0, 0), np.uint8)))
            continue
        adv = f.getlength(ch) / SS
        wpx = int(math.ceil(adv)) + 2 * pad + 4
        hpx = int(math.ceil((asc + desc) / SS)) + 2 * pad
        L = Image.new("L", (wpx * SS, hpx * SS), 0)
        base = pad + int(math.ceil(asc / SS))
        ImageDraw.Draw(L).text((pad * SS, base * SS), ch, font=f, fill=255, anchor="ls")
        a = np.asarray(L.reduce(SS), dtype=np.float64)
        ys, xs = np.nonzero(a > 3)
        if len(xs) == 0:
            glyphs.append((adv, 0, 0, 0, 0, np.zeros((0, 0), np.uint8)))
            continue
        x0, x1, y0, y1 = xs.min(), xs.max() + 1, ys.min(), ys.max() + 1
        glyphs.append((adv, x0 - pad, y0 - base, x1 - x0, y1 - y0, np.clip(np.round(255.0 * (a[y0:y1, x0:x1] / 255.0) ** TEXT_GAMMA), 0, 255).astype(np.uint8)))
    return mid, glyphs


def main():
    o = []
    o.append("/* GENERATED by tools/gen_gt_art.py (wip547): Grigiotonic sprites (RGB565 + alpha 0..255) and the Poppins glyph atlas.\n"
             " * Geometry and shadows = tools/grigiotonic_mix_mockup.py. Do not edit by hand. */")
    for name, (w, h, fn) in (("GROOVE", (GW + 2 * GM, GH + 2 * GM, draw_groove)), ("THUMB", (SW, SH, draw_thumb)), ("GROOVE20", (GW + 2 * GM, GH20 + 2 * GM, draw_groove20)), ("THUMB15", (SW, SH15, draw_thumb15)), ("KNOB", (KS, KS, draw_knob)), ("BTNOFF", (BS, BS, draw_btn_off)), ("BTNON", (BS, BS, draw_btn_on)), ("CHKOFF", (CS, CS, draw_chk_off)), ("CHKON", (CS, CS, draw_chk_on)), ("FADER", (FSW, FSH, draw_fader)), ("SKNOB", (SKS, SKS, draw_sknob)), ("KNOBP", (KS, KS, lambda c: draw_knob(c, 105))), ("SKNOBP", (SKS, SKS, lambda c: draw_sknob(c, 105)))):
        col, al = matte(fn, w, h)
        c5 = rgb565(col)
        o.append("#define GTS_%s_W %d\n#define GTS_%s_H %d" % (name, w, name, h))
        o.append("static const uint16_t gts_%s_c[%d] = {%s};" % (name.lower(), w * h, ",".join("%d" % v for v in c5.flatten())))
        o.append("static const uint8_t gts_%s_a[%d] = {%s};" % (name.lower(), w * h, ",".join("%d" % v for v in al.flatten())))
    atlas = []
    tabs = []
    for name, fn, em, *only in FACES:
        mid, gl = face(fn, em, only[0] if only else None)
        rows = []
        for (adv, ox, oy, w, h, bmp) in gl:
            off = len(atlas)
            atlas.extend(int(v) for v in bmp.flatten())
            rows.append("{%d,%d,%d,%d,%d,%d}" % (int(round(adv * 8)), ox, oy, w, h, off))
        tabs.append((name, mid, rows))
    o.append("#define GTF_FIRST 32\n#define GTF_COUNT %d" % len(CHARS))
    o.append("typedef struct { uint8_t adv8; int8_t ox, oy; uint8_t w, h; uint16_t off; } gt_glyph_t;")
    o.append("static const uint8_t gtf_atlas[%d] = {%s};" % (len(atlas), ",".join("%d" % v for v in atlas)))
    # the characters are not contiguous in ASCII: a 96 entry map to the glyph index (255 = none)
    mp = [255] * 96
    for i, ch in enumerate(CHARS):
        mp[ord(ch) - 32] = i
    o.append("static const uint8_t gtf_map[96] = {%s};" % ",".join(str(v) for v in mp))
    for name, mid, rows in tabs:
        o.append("static const gt_glyph_t gtf_%s[GTF_COUNT] = {%s};" % (name.lower(), ",".join(rows)))
        o.append("#define GTF_%s_MID %d   /* baseline below the middle anchor, 1/8 px */" % (name, int(round(mid * 8))))
    open(OUT, "w").write("\n".join(o) + "\n")
    print("wrote", OUT, "atlas bytes", len(atlas))


main()
