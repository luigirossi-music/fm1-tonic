#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""fw_head_crc: CRC-16 of the first 16 KB (the "head") of the flash.bin inside official .fwsc files.

The FM-1 failure screen of firmware 1.3 ("ERROR -5 HEAD DIFFERS") shows CRC DEV (the head of
the unit) and CRC PKG (the head of the package). Give this script the official packages
(V14, V15, V16 ...) and see which one has the same CRC as CRC DEV.

  python3 fw_head_crc.py FM-1_V14.fwsc FM-1_V15.fwsc FM-1_V16.fwsc [--dev D023]

Same CRC as ota.c uses: CRC-16 poly 0x1021, init 0, over bytes 0..0x3FFF of flash.bin.
Reads files only; writes nothing, touches no device.
"""
import argparse
import hashlib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fm1_rescue import Fail, crc16, package_flash_image  # noqa: E402

HEAD = 0x4000


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--dev", help="CRC DEV shown on the FM-1 screen, hex, e.g. D023")
    a = ap.parse_args()
    dev = int(a.dev, 16) if a.dev else None
    for f in a.files:
        raw = open(f, "rb").read()
        sha = hashlib.sha256(raw).hexdigest()
        try:
            flash = package_flash_image(raw)
        except Fail as e:
            print(f"{os.path.basename(f)}: {e}")
            continue
        head = flash[:HEAD]
        c = crc16(head)
        mark = ""
        if dev is not None:
            mark = "   <== SAME AS YOUR UNIT" if c == dev else "   (not your unit's)"
        print(f"{os.path.basename(f)}: CRC HEAD {c:04X}  head 0x20..0x23 {head[0x20:0x24].hex()}  "
              f"sha256 {sha[:12]}{mark}")


if __name__ == "__main__":
    main()
