#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""What the emulator page offers for download: the cheat sheet (PDF) and the multilingual user manual (HTML; wip438: no PDF manual any more).

  python3 tools/make_pdfs.py

Sources (the only copies of the text): the cheat sheet block of web/emu/index.html (its cheat-body div, inside the Read more of the Credits) and docs/MANUAL.md
(-> docs/manual.html by tools/manual_html.py, run first). Output: web/emu/downloads/. Needs Python
Playwright with Chromium (the page is printed). Run it after changing either source."""
import re
import subprocess
import sys
from pathlib import Path
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "web" / "emu" / "downloads"
FOOT = ('<div style="font:7pt sans-serif;color:#555;width:100%;padding:0 12mm;display:flex;justify-content:space-between">'
        '<span>FM1 Tonic · made by Luigi Rossi · free software, GPL-3.0</span><span><span class="pageNumber"></span> / <span class="totalPages"></span></span></div>')

CHEAT_CSS = """
@page { size: A4; margin: 12mm 12mm 14mm; }
* { box-sizing: border-box; }
body { margin: 0; font: 8pt/1.3 -apple-system, "Helvetica Neue", Arial, sans-serif; color: #111; background: #fff; }
h1 { font-size: 17pt; margin: 0 0 1mm; } h1 b { color: #3d6f9e; }
.sub { margin: 0 0 3mm; color: #444; }
h3 { font-size: 10.5pt; margin: 4mm 0 1mm; border-left: 1.2mm solid #3d6f9e; padding-left: 1.6mm; break-after: avoid; }
h4 { font-size: 7.5pt; margin: 2.5mm 0 .5mm; color: #555; text-transform: uppercase; letter-spacing: .05em; break-after: avoid; }
p.note, p.exc { margin: 0 0 1.5mm; color: #444; }
table { border-collapse: collapse; width: 100%; margin: 0 0 1mm; }
th { text-align: left; font-size: 6.8pt; text-transform: uppercase; letter-spacing: .05em; color: #555; padding: 0 2mm 1mm 0; border-bottom: .6pt solid #888; }
td { padding: 1mm 2mm 1mm 0; border-bottom: .3pt solid #ccc; vertical-align: top; }
tr { break-inside: avoid; }
table.t1 td:first-child { width: 42%; } table.g td.pg { width: 26%; border-right: .3pt solid #ccc; } table.g td.ky { width: 30%; }
table.g tr.gs td { border-top: .6pt solid #999; }
.r { color: #c0281c; font-weight: 600; } .o { color: #a85a00; font-weight: 600; }
"""


def cheat_html():
    s = (ROOT / "web" / "emu" / "index.html").read_text(encoding="utf-8")
    m = re.search(r'<div class="cheat-body">(.*?)</div>\n    </div>\n  </details>', s[s.index('<details class="cheat"'):], re.S)
    return ('<!doctype html><meta charset="utf-8"><title>FM1 Tonic cheat sheet</title><style>' + CHEAT_CSS + '</style>'
            '<h1><b>FM1</b> Tonic cheat sheet</h1><p class="sub">How the firmware works today, for the M-VAVE FM-1. The user manual has the details.</p>'
            + m.group(1))


def main():
    subprocess.run([sys.executable, str(ROOT / "tools" / "manual_html.py")], check=True)
    OUT.mkdir(parents=True, exist_ok=True)
    # wip399: the same multilingual manual as the standalone file (images inside), so the web page offers exactly it
    subprocess.run([sys.executable, str(ROOT / "tools" / "manual_html.py"), "--multi", str(OUT / "FM1-Tonic-manual.html"), "--embed"], check=True)
    # wip438: the manual's number (docs/MANUAL_VERSION) goes into the page's links and into the saved file's name
    mv = (ROOT / "docs" / "MANUAL_VERSION").read_text().strip()
    ip = ROOT / "web" / "emu" / "index.html"
    t = ip.read_text(encoding="utf-8")
    t = re.sub(r"(<!--mv-->)[\d.]+(<!--/mv-->)", rf"\g<1>{mv}\g<2>", t)
    t = re.sub(r"FM1-Tonic-manual-[\d.]+\.html", f"FM1-Tonic-manual-{mv}.html", t)
    ip.write_text(t, encoding="utf-8")
    with sync_playwright() as p:
        b = p.chromium.launch()
        pg = b.new_page()
        pg.set_content(cheat_html())
        pg.pdf(path=str(OUT / "FM1-Tonic-cheat-sheet.pdf"), format="A4", print_background=True, display_header_footer=True,
               header_template="<span></span>", footer_template=FOOT, margin={"top": "12mm", "bottom": "14mm", "left": "12mm", "right": "12mm"})
        b.close()
    for f in sorted(OUT.glob("*.pdf")):
        print(f, f.stat().st_size, "bytes")


if __name__ == "__main__":
    main()
