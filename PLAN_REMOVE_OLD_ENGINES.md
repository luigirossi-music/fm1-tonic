# PLAN: remove the old X0X engines (808, break sampler, user sample slots)  -  written 4 Nov 2026 (wip113)
The user: "we need to plan to start to remove everything about old x0x engines, 808 and the other, the sampler".
Goal: the FM1 Tonic only (PART_909 = the Tonic, Microtonic-style: dsp/tonic.c + dsp/po32/po32_synth.c). ONE small step per chat. Never break a saved project or the 8 factory presets without an explicit format job.

## What exists today (survey, wip112 tree)
| Piece | Where | Size / refs | State |
| 808 drum DSP | dsp/drum808.c/.h | 1773 + 212 lines | DELETED in wip114 (step 2). Was DEAD in the firmware: engine.c never calls drum808_*. Was still compiled (3 build lists). STEP 1 below took it out of the builds (wip113). Files + their unit tests (tests/host/run_drum808.sh, drum808_test.c, drum_pan_test.c, tonic_bench.c) still in the repo |
| 909 DSP (the old one) | dsp/drum909.c/.h, drum909_dsp.h | 854 + 122 + 310 | NOT compiled in the firmware. drum909.h is only included for constants (project.c DR_SD; fxbus.h includes drum909_dsp.h). Reference tests tests/host/drum909_*.c |
| Break sampler | dsp/breaks.c/.h, engine.c (PART_BRK, TRK_BRK, T_BRK, ~47 refs), ui.c (~44 refs), project.c, sequencer.c (TRK_BRK, MUTE_BRK), main_fm1.c | 708 + 133 lines + built-in loop bank (tools/gen_break_bank.py, gen_builtin_break.*) | LIVE: still rendered, still has a part/UI |
| User sample slots | ui.c (~32 refs "slot"), engine.c, project.c, main_fm1.c (fl_plain_window_init, flash XIP), web/x0x_breaks.*, tools/import_breaks.py, upload_breaks.py, sampleio.py, fm1_sample_upload.py | | LIVE: flash area for samples + upload tools |
| Part / track model | engine.h `enum { PART_909, PART_808, PART_BRK, NPARTS }`, `T_909,T_808,T_BRK,T_FX,T_MIX,T_MST`; pattern.h NKIT 2; sequencer.h NTRACKS (NKIT+1), MUTE_BRK | | Saved projects and presets index these arrays: renumbering = FORMAT CHANGE |
| UI | ui.c PART_808 6 refs, PART_BRK 5, MIX pages with 3 parts, MIDI channels per part (seq.ch[NTRACKS]) | | |
| Tests | 66 scenarios in tests/scenarios; many are old 303/808/break ones (29 stale failures, list in HANDOFF_111) | | delete the stale ones that only test removed things |

## The steps (each = ONE chat, small, files at the end; do them in this order)
 1. DONE (wip113): drum808.c no longer compiled (tools/build.py O2_UNITS, web/emu/build.sh, host/build_host.sh). Host build + scenarios unchanged. NOT verified here: the firmware build (no JieLi toolchain in the chat) and the emulator build (no emcc): the user's `bash ~/tonic.sh` is the emulator check; the real firmware build is on his Mac.
 2. DONE (wip114): deleted dsp/drum808.c/.h, tests/host/run_drum808.sh, drum808_test.c, drum_pan_test.c (it tested the old 909 + 808 pans: also the old 909 half is gone, step 3 does not need to touch it), the drum-pan line and `drum808` of tests/run_tests.sh, the 808 row of LICENSING.md; tonic_bench.c lost its 808 part (kept: Tonic + old 909 timing); comments in pattern.h / tonic.h reworded. WAS: delete the 808 sources and their tests: dsp/drum808.c/.h, tests/host/run_drum808.sh, drum808_test.c, sc808*, the drum808 lines of tests/run_tests.sh (lines ~38 and ~41), drum_pan_test.c and tonic_bench.c (they use it: edit, do not delete blindly). Also NKIT comment "1 = 808". No format change.
 3. CORRECTED in wip115 (the survey of step 3 found two live dependencies on the "old 909" files, so they can NOT simply be deleted now):
    - dsp/drum909_dsp.h is used by the LIVE FX bus (fxbus.h includes it; fxbus.c uses d9_biquad_*, d9_shape_*, d9_exp_small, D9_SR ...). It stays; a later clean-up may rename it (e.g. dsp_common.h).
    - dsp/drum909.c + drum909.h are used at BUILD time by tools/gen_builtin_break.sh / gen_builtin_break.c (they render the built-in break loop from the 909 voices). They go together with the break sampler: STEP 5 (delete gen_builtin_break.*, gen_break_bank.py, the loop bank, then drum909.c/.h, tests/host/drum909_*, run_drum909.sh, the run_tests `drum909` loop).
    - DONE (wip115): project.c no longer includes drum909.h (its DR_SD became REV1_VOICE = 1, same value). ui.c has its own DR_ enum for the demo pattern (not from drum909.h).
 4. Hide the 808 and BREAK parts in the UI: no way to reach PART_808 / PART_BRK (part keys, MIX lists, MIDI channel list, song page parts, help text, spoken words). Data stays as is. Check the Tonic-only screens first (ui.seqpat, PERFORM, MIX).
 5. Stop rendering / processing the break sampler in engine.c (brk render, s_brkstop, breaks_live MIDI, MUTE_BRK use); breaks.c still linked. Then remove breaks.c/.h + the built-in loop bank (gen_break_bank, gen_builtin_break) from the 3 build lists, then the files.
 6. Remove the sample slots: ui.c slot screens, engine.c slot code, main_fm1.c flash window init, project.c slot fields; web x0x_breaks.* and the sample tools; flash layout (check the storage map / OTA package first: the loader and the backup / restore format (tools/x0x_backup.py, tests x0x_backup_test.py) must keep reading old backups).
 7. THE FORMAT JOB (last, bigger, plan file first): shrink NPARTS / NKIT / NTRACKS / T_* to the Tonic only, a project format version bump with migration of saved projects and the 8 factory presets (tests migrate.x0x, migrate0102.x0x, project.c), USB restore of old backups.
 8. Clean up: the stale scenarios, docs (README, FEATURES.md, manual figures), web installer / editor text that mention 808 / BREAK / sampler.

## Rules for every step
 - Grep first (who calls it), remove the call sites, then the code; the host build must have 0 errors and the Tonic scenarios must still pass (tonic_seqpat, tonic_perfpat, tonic_seq, tonic_fill, tonic_chain, tonic_fxmix, tonic_edit, tonic_zoom, help, knobs, guard).
 - Never change an array index / enum value that is saved (see step 7) before the format job.
 - ASK the user before deleting anything he may still want to keep (e.g. the MIDI-in sampler play, the break upload page of the web tools, old backups support).

## Questions for the user (ask at step 4 / 6 / 7, not now)
 - Is the sampler (break loops + user slots) gone for good, or only hidden for now?
 - Old projects / backups that contain 808 or BREAK data: must they still load (just ignoring that part)? Probably yes.
 - Should the web tools (x0x_breaks, sample upload) disappear too?
