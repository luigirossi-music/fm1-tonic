#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# X0X in the browser: the host simulator compiled to WebAssembly (Emscripten), plus its page.
#   [X0X_VERSION=0.4-beta] web/emu/build.sh  ->  build/emu/{index.html, worklet.js, x0x.wasm}
# Same sources and flags as host/build_host.sh (-ffp-contract=off, like the device).
set -e
cd "$(dirname "$0")/../.."
mkdir -p build/gen build/emu
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
[ -f build/gen/x0x_drum_tables.h ] || python3 tools/gen_drum_tables.py build/gen/x0x_drum_tables.h >/dev/null
python3 tools/gen_words.py firmware/words.tsv web/words build/gen/ann_words.h >/dev/null
python3 tools/gen_preset_bank.py build/gen/tonic_bank.h "${X0X_PRESET_BANK:-presets}" >/dev/null   # the factory bank: presets/*.tnp (X0X_PRESET_BANK=other folder)
START="${X0X_START_PRESET:-$(ls presets/01_*.tnp 2>/dev/null | head -n 1)}"   # the starting project: presets/01_*.tnp
python3 tools/gen_preset_bank.py --start build/gen/tonic_start.h $START >/dev/null
U="firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c firmware/src/dsp/drum_tables.c firmware/src/dsp/breaks.c
   firmware/src/dsp/fxbus.c firmware/src/dsp/master.c firmware/src/seq/sequencer.c
   firmware/src/seq/pattern.c firmware/src/seq/motion.c firmware/src/app/engine.c"
# shellcheck disable=SC2086
emcc -O2 -ffp-contract=off -std=gnu99 -Wall -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
    -DX0X_HOST -DX0X_WEB "-DX0X_VERSION=\"$(printf %s "${X0X_VERSION:-DEV}" | tr a-z A-Z)\"" -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    --no-entry -sSTANDALONE_WASM -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=33554432 -sFILESYSTEM=0 \
    -o build/emu/x0x.wasm web/emu/x0x_web.c $U
cp web/emu/index.html web/emu/worklet.js build/emu/
mkdir -p build/emu/downloads && cp web/emu/downloads/*.pdf web/emu/downloads/*.html build/emu/downloads/   # the cheat sheet (PDF) and the multilingual manual (HTML): tools/make_pdfs.py makes them. wip438: the .html was NOT copied before, so the manual link on the site was a 404
mkdir -p build/emu/fonts && cp web/emu/fonts/DotGothic16-subset.woff web/emu/fonts/OFL.txt build/emu/fonts/   # the panel labels: the webapp's font (SIL OFL)
echo "emu: build/emu ($(wc -c < build/emu/x0x.wasm) B wasm)"
