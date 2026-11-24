#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copies the "What's new" pop-up, its button text and the manual number from one page to another (web only, no firmware):
#   python3 tools/whatsnew_page.py SOURCE_index.html TARGET_index.html
# SOURCE = web/emu/index.html of the tree, TARGET = the published emu/index.html (used by whatsnew_site.sh).
import re, sys

if len(sys.argv) != 3:
    sys.exit("usage: whatsnew_page.py SOURCE.html TARGET.html")
src, dst = sys.argv[1], sys.argv[2]
s = open(src, encoding="utf-8").read()
d = open(dst, encoding="utf-8").read()
A, B = '<div class="uover" id="nover"', '<div class="uover" id="uover"'
for t, n in ((s, src), (d, dst)):
    if A not in t or B not in t:
        sys.exit("the What's new pop-up was not found in " + n)
i, j = s.index(A), s.index(B)
a, b = d.index(A), d.index(B)
d = d[:a] + s[i:j] + d[b:]
btn = re.search(r'id="nbtn"[^>]*>(What&#39;s new in [^<]*)</button>', s)
d, k = re.subn(r'(id="nbtn"[^>]*>)What&#39;s new in [^<]*(</button>)', lambda m: m.group(1) + btn.group(1) + m.group(2), d, count=1)
if k != 1:
    sys.exit("the What's new button was not found in " + dst)
mv = re.search(r"<!--mv-->([\w.]+)<!--/mv-->", s).group(1)
d, k = re.subn(r"(<!--mv-->)[\w.]+(<!--/mv-->)", lambda m: m.group(1) + mv + m.group(2), d)
d = re.sub(r"FM1-Tonic-manual-[\w.]+\.html", "FM1-Tonic-manual-" + mv + ".html", d)
open(dst, "w", encoding="utf-8").write(d)
print(f"What's new: pop-up + button '{btn.group(1)}' + manual {mv} written to {dst}")
