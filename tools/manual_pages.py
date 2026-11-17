#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""wip381: the page cards of the manual's chapter 1A (BUTTONS AND PAGES).
Each card = the FM1 Tonic panel (docs/img/fm1-tonic-panel.svg) with the REAL screenshot of the page on the screen
and the labels of THAT page: dark = what the control does by itself, RED = what it does with FUNC held,
ORANGE = what it does with FILL held (like the orange print on the Digitakt, here red / orange).
The screenshots are docs/img/pages/shot-<id>.png (made by the host simulator, see tests/scenarios/tonic_pagecards.x0x).
    python3 tools/manual_pages.py            -> docs/img/pages/<id>.svg
"""
import base64, re, sys, pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
IMG = ROOT / "docs" / "img"
PLAIN, RED, ORANGE, GREY = "#202020", "#cc1a1a", "#d96c00", "#6a6a6a"

# the label positions of the panel (from fm1-tonic-panel.svg)
KX = {"K1": 610, "K2": 714, "K3": 818, "K4": 922}
BLK_ACC, BLK_FILL, BLK_FUNC = 726.0, 782.5, 895.5
WHITE0, WHITE_STEP = 76.25, 56.5

def esc(t):
    return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

def txt(x, y, t, col=PLAIN, size=10.5, weight=700, anchor="middle"):
    return (f'<text x="{x}" y="{y}" text-anchor="{anchor}" font-size="{size}" font-weight="{weight}" '
            f'fill="{col}">{esc(t)}</text>\n')

def card(c):
    s = (IMG / "fm1-tonic-panel.svg").read_text()
    # 1. the placeholder text on the screen goes; the real screenshot takes its place (240 x 240, in the 256 x 256 frame)
    for ph in ("SCREEN", "TRACK n  PAGE NAME", "4 big values = K1 K2 K3 K4"):
        s = re.sub(r'<text [^>]*>%s</text>\n?' % re.escape(ph), "", s)
    png = base64.b64encode((IMG / "pages" / ("shot-%s.png" % c["shot"])).read_bytes()).decode()
    over = f'<image x="300" y="52" width="240" height="240" href="data:image/png;base64,{png}"/>\n'
    # 2. the sub-labels of the knobs and the buttons: this page's
    subs = {"VOLUME": c.get("master", ("VOLUME", PLAIN)),
            "PAGE / STEP PAGE": c.get("select", ("-", GREY)),
            "2nd PAGE / VOICE": c.get("algo", ("-", GREY)),
            "PRESET (40)": c.get("presets", ("PRESET (40)", PLAIN)),
            "NO / DOWN": c.get("octm", ("OCTAVE DOWN", PLAIN)),
            "YES / UP": c.get("octp", ("OCTAVE UP", PLAIN))}
    for i, k in enumerate(("K1", "K2", "K3", "K4")):
        subs["VALUE %d" % (i + 1)] = c["k"][i][:2] if i < len(c["k"]) else ("-", GREY)
    for old, (new, col) in subs.items():
        s = re.sub(r'(<text [^>]*?)fill="#3a3a3a"([^>]*>)%s</text>' % re.escape(old),
                   lambda m: m.group(1) + 'fill="%s"' % col + m.group(2) + esc(new) + "</text>", s, count=1)
    # 3. extra lines
    o = over
    o += txt(500, 20, c["title"], PLAIN, 15)
    # legend: three coloured words
    o += (f'<text x="500" y="35" text-anchor="middle" font-size="10.5" font-weight="700">'
          f'<tspan fill="{PLAIN}">BLACK = ITSELF   </tspan><tspan fill="{RED}">RED = WITH FUNC HELD   </tspan>'
          f'<tspan fill="{ORANGE}">ORANGE = WITH FILL HELD</tspan></text>\n')
    for i, k in enumerate(("K1", "K2", "K3", "K4")):                         # under each knob
        for j, (t, col) in enumerate(c["k"][i][2:] if i < len(c["k"]) else ()):
            o += txt(KX[k], 146 + 10 * j, t, col, 9, 600)
    for (x, lines) in c.get("under_enc", []):                                  # under MASTER / SELECT
        for j, (t, col) in enumerate(lines):
            o += txt(x, 146 + 10 * j, t, col, 9, 600)
    for j, (t, col) in enumerate(c.get("blk", [])):                            # under black keys 1-8
        o += txt(358, 442 + 13 * j, t, col, 10.5)
    for j, (t, col) in enumerate(c.get("acc", [])):
        o += txt(BLK_ACC, 442 + 12 * j, t, col, 9.5)
    for j, (t, col) in enumerate(c.get("fill", [])):
        o += txt(BLK_FILL, 442 + 12 * j, t, col, 9.5)
    for j, (t, col) in enumerate(c.get("func", [])):
        o += txt(BLK_FUNC, 442 + 12 * j, t, col, 9.5)
    ws = c.get("white", [])
    s = re.sub(r'<text [^>]*>WHITE KEYS 1-16 = STEPS 1-16[^<]*</text>\n?', "", s)
    for j, (t, col) in enumerate(ws):
        o += txt(500, 662 + 13 * j, t, col, 11)
    # the button that opens this page: a thick outline
    hb = c.get("button")
    if hb:
        bx = {"FX": (578, 170), "SEL": (642, 170), "ENV": (706, 170), "LFO": (770, 170), "EDIT": (834, 170), "GLO": (898, 170),
              "HOME": (578, 244), "SAVE": (642, 244), "ARP": (706, 244), "SEQ": (770, 244), "PLAY": (834, 244), "REC": (898, 244)}[hb]
        o += f'<rect x="{bx[0]-2}" y="{bx[1]-2}" width="62" height="{54}" rx="8" fill="none" stroke="#2146c8" stroke-width="3"/>\n'
    return s.replace("</svg>", o + "</svg>")

CARDS = {}
exec((ROOT / "tools" / "manual_pages_data.py").read_text())   # CARDS[id] = {...}

def main():
    out = IMG / "pages"
    out.mkdir(exist_ok=True)
    for cid, c in CARDS.items():
        (out / (cid + ".svg")).write_text(card(c))
        print(cid)

if __name__ == "__main__":
    main()
