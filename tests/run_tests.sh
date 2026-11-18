#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# X0X host tests: the maths, the sequencer, each engine
# against its reference implementation, and the whole firmware app in the simulator
# (tests/scenarios/*.x0x: UI, patterns, persistence, MIDI clock), with screenshots and
# audio in build/scenarios/. Needs the reference sources (see tests/host/run_*.sh).
#   tests/run_tests.sh
set -u
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
FAIL=0
OUT=build/host
mkdir -p "$OUT" build/scenarios
run() {
    name="$1"; shift
    if "$@" > "$OUT/$name.log" 2>&1; then
        echo "  ok   $name"
    else
        echo "  FAIL $name (see $OUT/$name.log)"
        tail -15 "$OUT/$name.log" | sed 's/^/       /'
        FAIL=1
    fi
}
W="-O2 -ffp-contract=off -Wall -Wextra -Wdouble-promotion -Werror"
run fastmath sh -c "$CC -O2 -ffp-contract=off -Wall -Wextra -Werror -o $OUT/fastmath_test tests/host/fastmath_test.c -lm && $OUT/fastmath_test"   # its reference is double on purpose
seq_test() {
    $CC -O2 -ffp-contract=off -Wall -Wextra -Wno-unused-function -o $OUT/seq_test tests/host/seq_test.c \
        firmware/src/seq/sequencer.c firmware/src/seq/pattern.c || return 1
    $OUT/seq_test
}
run sequencer seq_test
run encoder sh -c "$CC -O2 -w -Ifirmware/hal -o $OUT/encoder_test tests/host/encoder_test.c && $OUT/encoder_test"
run uac sh -c "$CC -O2 -w -Ifirmware/src -Ifirmware/hal -o $OUT/uac_test tests/host/uac_test.c -lm && $OUT/uac_test"
run trs sh -c "$CC -O2 -w -Ifirmware/src -Ifirmware/hal -o $OUT/trs_test tests/host/trs_test.c && $OUT/trs_test"
run undo sh -c "$CC -O2 -Wall -Wextra -Wno-unused-function -o $OUT/undo_test tests/host/undo_test.c && $OUT/undo_test"
run bta-sim sh -c "$CC -std=c99 -O2 -Wall -Wextra -Werror -Ifirmware/src/bt -o $OUT/bta_sim_test tests/host/bta_sim_test.c firmware/src/bt/plat_bta_sim.c && $OUT/bta_sim_test"   # the Bluetooth audio stand-in (host only)
run bta-crypto sh -c "$CC -O2 -Wall -Wextra -o $OUT/bta_crypto_test tests/host/bta_crypto_test.c && $OUT/bta_crypto_test"   # the Classic pairing maths (SAFER+, E1 / E3, SSP f1-f3, SHA-256) against the Bluetooth spec's sample data
run bta-sbc sh -c "$CC -O2 -w -Itests/host/ref/sbc_dec -o $OUT/bt_sbc_test tests/host/bt_sbc_test.c tests/host/ref/sbc_dec/*.c -lm && $OUT/bt_sbc_test"   # our SBC encoder against AOSP's decoder (a decoder that is not ours): every frame taken, the tones back
run bta-syntax sh tests/host/bt_syntax_check.sh   # the Classic stack compiles (syntax only, glue stubbed) against the radio headers in firmware/src/bt/hal/
run tonic-cut sh -c "$CC -O2 -ffp-contract=off -std=c99 -Wall -Wextra -Wno-unused-function -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o $OUT/tonic_cut_test tests/host/tonic_cut_test.c firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c -lm && $OUT/tonic_cut_test"   # wip318: LOW CUT / HIGH CUT have no OFF: no step when a cut is turned in a hit
run tonic-choke sh -c "$CC -O2 -ffp-contract=off -std=c99 -Wall -Wextra -Wno-unused-function -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o $OUT/tonic_choke_test tests/host/tonic_choke_test.c firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c -lm && $OUT/tonic_choke_test"   # wip328: the CHOKE group
run tonic-sidechain sh -c "$CC -O2 -ffp-contract=off -std=c99 -Wall -Wextra -Wno-unused-function -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o $OUT/tonic_sidechain_test tests/host/tonic_sidechain_test.c firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c -lm && $OUT/tonic_sidechain_test"   # wip348: the SIDECHAIN (the input's sound ducks the mask)
run tonic-note sh -c "$CC -O2 -ffp-contract=off -std=c99 -Wall -Wextra -Wno-unused-function -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o $OUT/tonic_note_test tests/host/tonic_note_test.c firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c -lm && $OUT/tonic_note_test"   # wip251: a note = the pitch moved by semitones for one hit
run lz sh -c "$CC $W -o $OUT/lz_test tests/host/lz_test.c && $OUT/lz_test"
run motion sh -c "$CC $W -o $OUT/motion_test tests/host/motion_test.c firmware/src/seq/motion.c && $OUT/motion_test"
run stereo sh -c "$CC -O2 -ffp-contract=off -w -Ifirmware/src/dsp -Ifirmware/src -Ibuild/gen -o $OUT/stereo_test tests/host/stereo_test.c firmware/src/dsp/master.c firmware/src/dsp/fxbus.c -lm && $OUT/stereo_test"
run master sh -c "$CC $W -Wno-double-promotion -o $OUT/master_test tests/host/master_test.c firmware/src/dsp/master.c -lm && $OUT/master_test"
for t in breaks; do
    [ -f "tests/host/run_$t.sh" ] && run "$t" sh "tests/host/run_$t.sh"
done
run storage sh -c "$CC -O2 -o $OUT/storage_test tests/storage_test.c && $OUT/storage_test"
# backup and restore over USB (firmware/src/backup.c) on storage.c over a simulated flash: the stream's frames, a
# restore refused, cut or damaged changing nothing, SEL restoring it all, a power cut at each write: the old project
# or the new, whole. Then tools/x0x_backup.py against a simulated X0X, on that image
run backup sh -c "$CC -std=c99 -Wall -Wextra -Werror -Wno-unused-function -O2 -o $OUT/backup_test tests/backup_test.c && $OUT/backup_test $OUT/backup.x0b"
run x0x-backup python3 tests/x0x_backup_test.py $OUT/backup.x0b
# the web backup page (web/backup/index.html): the same bytes as x0x_backup.py's on that conversation, replayed;
# then a simulated X0X: a backup and a restore byte for byte, STOP FIRST, HOME, a damaged or another firmware's file
# not sent
if command -v node >/dev/null 2>&1; then
    run backup-page sh -c "python3 tests/x0x_backup_record.py $OUT/backup.x0b $OUT/backup_transcript.json && \
        node tests/host/backup_page_test.mjs $OUT/backup_transcript.json"
fi
# the emulator page's preset backup (web/emu/index.html PB.presets) against a simulated FM-1 (cmds 0x44..0x47)
if command -v node >/dev/null 2>&1; then
    run presets-page node tests/host/presets_page_test.mjs
fi
# the update path, against the firmware package (Felucca's tests; needs ./build.sh)
if [ -f build/fm1-tonic.fwsc ]; then
    run ota-entry sh -c "$CC -o $OUT/ota_test tests/ota_test.c && $OUT/ota_test build/fm1-tonic.fwsc"
    if [ -z "${AC79_SDK:-}" ]; then
        echo "  skip update-loader (needs AC79_SDK, as the build)"
    else
    run update-loader sh -c "head -c 200000 build/fm1-tonic.bin > $OUT/old_app.bin && \
        python3 tools/fm1pkg_make.py $OUT/old_app.bin build/loader/ota.bin $OUT/old.fwsc >/dev/null && \
        $CC -o $OUT/ldr_test tests/ldr_test.c && $OUT/ldr_test $OUT/old.fwsc build/fm1-tonic.fwsc"
    fi
    run installer python3 tests/install_test.py
else
    echo "  skip update-path tests (no build/fm1-tonic.fwsc: run ./build.sh)"
fi
run upload-images sh -c "python3 tests/host/make_test_loops.py build/test_loops && \
    python3 tools/upload_breaks.py --dry-run build/test_upload build/test_loops/loop*.wav"
command -v node >/dev/null 2>&1 && run breaks-web node web/test_breaks.mjs   # the browser uploader: upload_breaks.py's bytes
run host-build sh host/build_host.sh
if command -v emcc >/dev/null 2>&1 && command -v node >/dev/null 2>&1; then   # the browser build, when it can be built
    printf 'wav emu_ref.wav\nwait 500\ntapkey b0\ntapkey w0\ntapkey w8\ntap PLAY\nwait 2960\nwavstop\n' > build/host/emu_ref.x0x
    run emu sh -c "sh web/emu/build.sh >/dev/null 2>&1 && build/host/x0x_host build/host/emu_ref.x0x build/host >/dev/null && \
        node tests/host/emu_test.mjs build/emu/x0x.wasm build/host/emu_ref.wav"
    run emu-slots node tests/host/emu_slots_test.mjs build/emu/x0x.wasm build/test_loops/loop1.wav
fi
# the Tonic preset path (tools/tnp_pack.c): pack the Rostoma .mtpreset (not shipped: put it at ROSTOMA) with libpo32 (LIBPO32),
# then scenarios/preset.x0x loads that .tnp; without them that scenario is skipped
LIBPO32="${LIBPO32:-../libpo32-main}"
ROSTOMA="${ROSTOMA:-tests/fixtures/Rostoma_Hieroglycepha__121_.mtpreset}"
rm -f build/rostoma.tnp
if [ -f "$ROSTOMA" ] && [ -f "$LIBPO32/core/src/po32_patch_import.c" ]; then
    run tnp-pack sh -c "$CC -O2 -Wall -Wextra -Werror -Ifirmware/src -I$LIBPO32/core/include -o $OUT/tnp_pack tools/tnp_pack.c \
        $LIBPO32/core/src/po32_patch_import.c $LIBPO32/core/src/po32.c -lm && $OUT/tnp_pack $ROSTOMA build/rostoma.tnp"
else
    echo "(no Rostoma preset at $ROSTOMA or no libpo32 at $LIBPO32: the preset scenario is skipped)"
fi
run tnp-synth sh -c "python3 tests/make_test_tnp.py build/test_fill.tnp && python3 tests/make_test_tnp.py build/test_12.tnp TWELVE 1200 12 && python3 tests/make_test_tnp.py build/test_chain.tnp CHAINTEST 1200 4 0,1,3"    # a small preset (fills, accents, mutes) that needs no libpo32
# the starting point: a second host build with a start preset built in, run on its own scenario (tonic_start)
run host-start sh -c "X0X_START_PRESET=build/test_fill.tnp X0X_HOST_OUT=build/host/x0x_host_start sh host/build_host.sh >/dev/null && sh host/build_host.sh >/dev/null"
mkdir -p build/scenarios/tonic_start
run scenario-tonic_start build/host/x0x_host_start tests/scenarios/tonic_start.x0x build/scenarios/tonic_start
# the real factory bank (presets/): a host build with it and its first preset as the starting point
run host-factory sh -c "X0X_PRESET_BANK=presets X0X_START_PRESET=presets/01_Marchiav.tnp X0X_HOST_OUT=build/host/x0x_host_factory sh host/build_host.sh >/dev/null && sh host/build_host.sh >/dev/null"
mkdir -p build/scenarios/tonic_factory
run scenario-tonic_factory build/host/x0x_host_factory tests/scenarios/tonic_factory.x0x build/scenarios/tonic_factory
mkdir -p build/scenarios/tonic_bank8
run scenario-tonic_bank8 build/host/x0x_host_factory tests/scenarios/tonic_bank8.x0x build/scenarios/tonic_bank8   # the real 8 factory presets
for s in tests/scenarios/*.x0x; do
    n=$(basename "$s" .x0x)
    [ "$n" = tonic_start ] && continue
    [ "$n" = tonic_factory ] && continue
    [ "$n" = tonic_bank8 ] && continue
    [ "$n" = preset ] && [ ! -f build/rostoma.tnp ] && continue
    mkdir -p "build/scenarios/$n"
    run "scenario-$n" build/host/x0x_host "$s" "build/scenarios/$n"
done
[ $FAIL -eq 0 ] && echo "all tests passed" || echo "TESTS FAILED"
exit $FAIL
