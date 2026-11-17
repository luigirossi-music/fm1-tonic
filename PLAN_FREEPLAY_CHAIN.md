# PLAN: free-play chain of long presets (HANDOFF_103 queue item 6, the user's TOP PRIORITY) - 4 Nov 2026
## DONE (wip122, 5 Nov 2026): the user chose B. Loading a preset turns CHAIN ON (ui.c app_load_preset). With CHAIN ON (sequencer.c bar_end): a CHAINED pattern runs into the next, the first pattern without the flag ends its chain and the next chain follows (1, 2, 3-4-5-6, 7-8, back to 1 after 8; patterns past 8 loop); PLAY from the middle of a chain starts at the chain's first pattern. CHAIN OFF (K1 of the SEQ 2nd-press page) = a pattern loops as before. Measured on presets 4 and 1: 1,2,...,8,1,... in order. Scenario tests/scenarios/tonic_chain.x0x (rewritten). Note: the sequence equals option A; the flags now only decide where PLAY starts. (Original plan text below.)


## What the code does today (checked in wip108)
- A factory preset (.tnp) stores per pattern a "chained" flag (tnp.h, byte 1 of each pattern). project.c copies it to pat[i].drum[0].chain.
- seq/sequencer.c (~235): when a pattern ends and nothing is cued and no CHAIN range is set, a pattern with chain = 1 runs into the NEXT pattern (after pattern 8 it goes back to 1). A pattern with chain = 0 loops on itself.
- So the flag is already followed: the run stops at the first pattern WITHOUT the flag and loops there.
- Flags in the 8 factory presets (C = chained):
  1 Marchiav:    p3
  2 Omm e Night: none
  3 Stai bell:   p3, p7
  4 We Masanie:  p3, p4, p5, p7   (p3 -> p4 -> p5 -> p6 loops; p7 -> p8 loops; p1, p2 loop alone)
  5 Tiemp e Uerr: none
  6 Waie e not:  p1, p3, p6
  7 Robba bell:  p2, p3, p6, p7
  8 Piccerill:   p1, p3, p6
- Also there: the CHAIN screen (FUNC + OCT+), a chain range (seq_chain a..b) and song mode.

## Open question (ASK, do not guess)
What exactly should happen when the user loads a preset and presses PLAY? Candidates:
 A. Play patterns 1 -> 8 -> 1 -> ... in order, always, whatever the flags say.
 B. Keep the flags, but PLAY starts at the first pattern of the chain and the chain runs to its end, then continues with the next chain (1,2,3-4-5-6,7-8, back to 1).
 C. Something else (the user describes it).
Then: smallest change in sequencer.c / project.c + a scenario (load preset 4, play, expect the pattern numbers in order).
