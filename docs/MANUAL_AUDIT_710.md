# Manual audit (handoff 710, 26 Nov 2026): is every manual up to date with the new functions and styles?

Checked: docs/MANUAL.md (English) line by line against handoffs 695 to 709, then the same places in .es .it .fr .pt .de, the web page (web/emu/index.html) and docs/NEW.md / FEATURES.md. NOTHING was edited: public texts wait for his OK (his rule). Every piece below is small and is done for all 6 languages at once.

## What is already right (no work)
- FX = MIX 1 > SENDS, HOME = MIX 2 > PERFORM > GLOBAL, "Back where you left it" (695-698).
- 30 places, 2 demo presets (675), RANDOM ALL VOICES, global accent, sidechain compressor, battery + LOW BATTERY, song LOOP, mute / solo.
- Saving: ONE screen SAVE HERE?, Saving section and Saving cheat sheet at the end of chapter 5, Song mode cheat sheet, in all 6 languages (708-709).

## What is OUT OF DATE (the same in all 6 languages)

| Piece | From | Where the manual is wrong or silent | What it should say |
|---|---|---|---|
| **P1 SEQ / ARP knobs (DONE in 711)** | 699, 704 | MANUAL.md l.61 (Knobs: "ALGORITHM turns to a second page or picks a voice"), l.227 (SEQ row: "SELECT: the step pages ... ALGORITHM: the voice"), l.229 (ARP row: knobs "FIXED LEN, OCTAVE, PAGE, -"; "ALGORITHM: the voice. SELECT: the step pages"), l.363 ("On the SEQ page ALGORITHM picks the voice"), l.370 ("SELECT turns the step pages"). Web page: ARP table (ALGORITHM = The voice), panel labels "SELECT = PAGE / STEP PAGE", "ALGORITHM = 2nd PAGE / VOICE". | SEQ and ARP: **SELECT = the track** (SEQ: 1 to 8 then AC; ARP: 1 to 8), **ALGORITHM = the step page** (only with AUTOSCROLL off). ARP knobs: K1 FIXED LEN, **K2 AUTOSCROLL (same setting as SEQ)**, K3 PAGE, **K4 OCTAVE**; OCT- / OCT+ still move it. Unchanged: step held + SELECT = chance, HOME + SELECT = tempo, the pattern page keeps its old SELECT. |
| **P2 Swing (DONE in 712)** | 700 | l.227 / l.371 ("Swing belongs to the pattern"), l.178 GLOBAL row, l.206 SETTINGS rows list, GLO settings table (no SWING row). | Swing is **global by default** (one value for all patterns). New GLO setting **SWING: GLOBAL / PER PATTERN** (after NAME PROJECTS). PER PATTERN = each pattern has its own. Old projects: pattern 1's swing goes to all. |
| **P3 Pattern bank (beta) (DONE in 713)** | 685-689 | Nothing: no "bank" anywhere. l.18 and l.359 say "16 patterns per project"; the Button combinations table has no FUNC + SELECT row; FUNC held (l.123) says patterns 1 to 16. | **FUNC + SELECT** = pattern bank 1 (patterns 1-16) / bank 2 (17-32) on SEQ, ARP, SONG and HOME. Beta: a saved project still keeps only patterns 1-16. |
| **P4 Top bar and chips (DONE in 714)** | 687, 690-693, 703, 706, 707 | Nothing about the three chips, the big TRACK chip on SEQ, the blinking REC dot, the OCT help card, the bigger battery. | Top bar on every page: **[P1] [PATTERN BANK n] [TRACK n]** (red in bank 2; in song mode the first chip = S + the bar). SEQ and ARP show the big TRACK chip; the REC dot + REC sit in it while recording and blink like the REC button. No page name in the bar. Battery is bigger. |
| **P5 Version 1.8f texts (NEW.md + FEATURES.md DONE in 715; web What's new 1.8f WAITS for his OK)** | 707-709 | docs/NEW.md: newest entry is 1.8e (no 1.8f); the old CHOOSE SLOT save is still described there and in FEATURES.md. The web button says "What's new in 1.8e"; "What's new in 1.8f" is not written. | Add a 1.8f block to NEW.md; fix the save lines; write the web "What's new in 1.8f" (his OK of the list of 9 items is pending since 709: swap, ARP K2 / K4, swing, top bar, big track chip, bigger battery, simple SAVE, Saving section, cheat sheets). |
| **P6 Cheat sheet (PDF / web) (DONE in 716)** | 709 | Song mode + Saving cheat sheets are in (709). It has no SEQ / ARP row, so P1 changes nothing there; add the FUNC + SELECT bank row only if he wants it (P3). | Check after P1 to P4, rebuild with tools/make_pdfs.py. |

## Order and size
Each piece = about 1 file family (6 manuals + the web page) and one command. Suggested order: P1, P2, P3, P4, P5, P6. After each piece: `bash ~/tonic_site.sh` and look at the manual.

## Notes
- Translations (es it fr pt de) are written by Claude: a native read is still open.
- The Contents has no links on sub-entries (as before).
- VERSION 1.8f, manual 1.8f2 today. Each piece raises the manual number by one (1.8f3, 1.8f4 ...).
