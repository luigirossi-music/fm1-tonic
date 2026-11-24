#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Puts docs/FUTURE_UPDATES.txt into the "Future updates" pop-up of web/emu/index.html
# (between the <!--fu--> and <!--/fu--> marks). Usage: python3 tools/future_updates.py
# Optional: python3 tools/future_updates.py LIST.txt PAGE.html   (another list, another page, e.g. the published one:
# a page without the marks is found by its "Coming in future updates" heading)
import html, os, re, sys

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, "docs", "FUTURE_UPDATES.txt")
page = sys.argv[2] if len(sys.argv) > 2 else os.path.join(root, "web", "emu", "index.html")

items = []
for n, line in enumerate(open(src, encoding="utf-8"), 1):
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    if " | " not in line:
        sys.exit(f"line {n}: missing ' | ' between the title and the text: {line[:60]}")
    t, d = line.split(" | ", 1)
    items.append((t.strip(), d.strip()))

li = "".join(f'<li><b class="ut">{html.escape(t, quote=False)}</b><span class="ud">{html.escape(d, quote=False)}</span></li>' for t, d in items)
s = open(page, encoding="utf-8").read()
new, k = re.subn(r"<!--fu-->.*?<!--/fu-->", lambda m: "<!--fu-->" + li + "<!--/fu-->", s, flags=re.S)
if k != 1:   # an older page without the marks: the <ul> under "Coming in future updates"
    new, k = re.subn(r"(Coming in future updates</b></p><ul>).*?(</ul>)", lambda m: m.group(1) + "<!--fu-->" + li + "<!--/fu-->" + m.group(2), s, count=1, flags=re.S)
if k != 1:
    sys.exit("the Future updates list was not found in " + page)
open(page, "w", encoding="utf-8").write(new)
print(f"Future updates: {len(items)} item(s) written to {page}")
