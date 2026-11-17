#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""A small TNP1 preset made by hand (layout: firmware/src/app/tnp.h), so scenarios can test what a preset carries
(own accents, fills, mutes) without libpo32 or a Microtonic file.   make_test_tnp.py OUT.tnp [NAME [TEMPO_X10 [NPAT [CHAINED_LIST]]]]
Pattern a: track 1 hits on steps 1 and 5 (step 5 accented AND fill), track 2 hits on step 3 with fill only,
track 3 is muted by the preset. Pattern b: track 1 on step 1."""
import struct, sys

def row(steps):
    v = 0
    for s in steps:
        v |= 1 << s
    return struct.pack('<Q', v)          # u32 words step 0-31, 32-63: little-endian u64

def pattern(length, voices, chained=0):
    out = struct.pack('<BBBB', length, chained, 0, 0)
    for v in range(8):
        t, a, f = voices.get(v, ([], [], []))
        out += row(t) + row(a) + row(f)
    return out

pats = [pattern(16, {0: ([0, 4], [4], [4]), 1: ([2], [], [2]), 2: ([0], [], [])}),
        pattern(16, {0: ([0], [], [])})]
while len(pats) < (int(sys.argv[4]) if len(sys.argv) > 4 else 2):
    pats.append(pattern(16, {0: ([0], [], [])}))      # more patterns: a kick on step 1
chained = [int(x) for x in sys.argv[5].split(',')] if len(sys.argv) > 5 else []      # pattern numbers (0-based) flagged CHAINED
for i in chained:
    pats[i] = pats[i][:1] + b'\x01' + pats[i][2:]
total = 40 + 8 * 44 + len(pats) * 196
hdr = b'TNP1' + struct.pack('<HHBBBBHh', total, int(sys.argv[3]) if len(sys.argv) > 3 else 1200, 0, len(pats), 0, 0b100, 100, 0) + (sys.argv[2] if len(sys.argv) > 2 else 'SYNTH FILL TEST').encode().ljust(24, b'\0')
assert len(hdr) == 40
voice = struct.pack('<21H', *([32768] * 21)) + bytes([64, 0])
assert len(voice) == 44
blob = hdr + voice * 8 + b''.join(pats)
assert len(blob) == total
open(sys.argv[1], 'wb').write(blob)
