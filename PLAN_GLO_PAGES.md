# PLAN: GLO with three pages - written 5 Nov 2026 (wip122), NOT built (the user: "if it is not too expensive"; it is not small, so one chat of its own)
User decision (5 Nov): the 8-track MIXER goes first, then the compressor, then the settings:
  GLO page 1 = MIXER (a volume fader for each of the 8 tracks)
  GLO page 2 = COMP (the master compressor: needle meter + ATTACK, RELEASE, THRESH, MAKEUP; today it is the last page of the FX button, wip121)
  GLO page 3 = SETTINGS (today's GLO list: MIDI, audio, keys, project, system; the tempo / swing / quantization sliders stay on HOME 2nd press = V_GLOB)
Order of work (the user: mixer first): (1) pages mechanism for GLO + MIXER, (2) move COMP from FX to GLO page 2 (draw_comp_meter and the page's 4 refs stay; only where it is reached changes; update tonic_comp / tonic_fxmix scenarios, FEATURES, MANUAL, emulator help text), (3) SETTINGS as page 3.
## What exists
- GLO today = view V_GLO with the O_LIST overlay (open_global() builds the list rows; GLO pressed again closes it, ui.c B_GLO ~3020). It has no pages: pages are a V_FX / V_PART thing (build_pages / add_page, cur_page, the FX button advances them).
- ui.c draw_drum_mix() already draws the Tonic's 8 tracks as vertical level bars with pan and a hit flash (V_MIX, now unreached): reuse it for the fader look.
- Track level = find_ref(T_909, v, "Level") (the "Level" knob on HOME 1st press); T_MIX PART_909 MX_LEVEL is ONE level for the whole Tonic part.
## Questions to ASK first (proposed answers in brackets, so the user can just say "yes")
 a. Fader = each track's own "Level" (T_909 voice v)? [yes]
 b. 8 faders on one page: black keys 1-8 pick the track (as everywhere on the Tonic) and K1 = its LEVEL, K2 = PAN, K3 = REV send, K4 = DLY send, the screen shows all 8 faders with the picked one lit? [yes] or K1-K4 = tracks 1-4 and the page 2 of the mixer = tracks 5-8?
 c. How does GLO move between its 3 pages: GLO pressed again = next page (MIXER, COMP, SETTINGS, back to MIXER)? [yes; HOME leaves GLO]
 d. Does GLO still open on the page you left, or always on MIXER? [always MIXER]
## Cost / risk
 Medium: new GLO page state (ui.glopage), the list overlay only on page 3, key / knob routing for page 1, scenarios tonic_glob / tonic_home2 / help / knobs to re-check, then the COMP move and its tests. One chat for steps 1 + 3, a second one for step 2.
