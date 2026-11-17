Analysis scripts used in session 3 (run from a folder that holds ref/ with the Microtonic exports, mx/ mq/ sw/ with engine renders).
mixfit.py  - oscillator/noise gains of the 5 drum 3 Mix files; harm.py - Microtonic EQ shape from harmonic levels;
eq3.py, cmp2.py - level comparison engine vs Microtonic exports. Test hooks in tests/host/tonic_test.c: env EQ0, DIST0, MIXOV="v:val", EQDB=<dB, voice 3>.

Session 4 additions:
pattern_render.c (tests/host) - renders pattern a (or b..l) of a .mtpreset with the Tonic engine to a 32-bit float WAV (env SWK = swing scale, VNORM/VACC velocities).
  build: gcc -O2 -I firmware/src -I <libpo32>/core/include -o pr tests/host/pattern_render.c firmware/src/dsp/tonic.c firmware/src/dsp/po32/po32_synth.c <libpo32>/core/src/po32_patch_import.c <libpo32>/core/src/po32.c -lm
lvl.py - onset-aligned engine vs Microtonic one-shot level comparison per drum and time window (needs ref/ and tt/o42, tt/o127).
swing_fit.py - tried to fit the swing law on the 2-bar render; INCONCLUSIVE because that render is hard clipped (54 % of samples at full scale).
Session 4b: ld.py + recon.py rebuild pattern a from the Export 2.0 one-shots (each drum MONOPHONIC: a new hit cuts the previous one) and fit it to the float renders
(ref/ = Export 2.0, ryg/ = the three float renders red_yellow_green). Result: mono model explains 93 % of the render (poly only 50 %), swing delay = 0.5 x swing% of a step.
