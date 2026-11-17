# the page cards (used by manual_pages.py): P = black (by itself), R = red (FUNC held), O = orange (FILL held)
P, R, O, G = PLAIN, RED, ORANGE, GREY
def kn(name, *lines):
    return (name, P) + tuple((t, P) for t in lines)
NONE_K = ("-", G)
_trk = [("KEYS 1-8: SELECT + PLAY THE TRACK", P), ("FUNC + KEY = MUTE", R), ("FILL + KEY = SOLO", O)]
_copy = dict(acc=[("+ COPY", R), ("PATTERN", R)], fill=[("+ PASTE", R), ("PATTERN", R)],
             func=[("+ PLAY", R), ("CLEARS IT", R)])   # red = with FUNC held: FUNC + ACCENT copies, FUNC + FILL pastes, FUNC + PLAY clears
CARDS["fx-1-mix"] = dict(
    title="FX BUTTON · 1st PRESS · MIX 1", shot="fx-1-mix", button="FX",
    k=[kn("MIX", "OSC <> NOISE"), kn("EQ FREQ", "20 Hz - 20 kHz"), kn("DIST", "AMOUNT"), kn("EQ GAIN", "64 = FLAT")],
    select=("STEP HELD: CHANCE", P), blk=_trk, **_copy,
    white=[("WHITE KEY HELD + TURN K1-K4 = P-LOCK THAT STEP (SELECT = ITS CHANCE)", P),
           ("ENV + TURN = DEFAULT    SAVE + TURN = FORGET MOTION    REC + PLAY + TURN = RECORD", P),
           ("FUNC + WHITE KEY = CHANGE PATTERN", R)])
_perf_k = [kn("REVERB D/W", "0 - 100 %"), kn("DELAY D/W", "0 - 100 %"), kn("LOW CUT", "MASTER 20 - 663 Hz"),
           kn("ALL FX", "RIGHT ON / LEFT OFF")]
CARDS["fx-2-perform"] = dict(
    title="FX BUTTON · 2nd PRESS · PERFORM", shot="fx-2-perform", button="FX", k=_perf_k,
    select=("TURN: NEXT PAGES", P), algo=("> MUTE / SOLO BOXES", P),
    blk=[("KEYS 1-8: MUTE (SOLO IF SOLO MODE)", P), ("THE BIG NUMBER = THE PATTERN", P)],
    func=[("TAP: MUTE MODE", R), ("OR UNMUTE ALL", R)], fill=[("TAP: SOLO MODE", O), ("OR CLEAR SOLOS", O)],
    white=[("WHITE KEY = NEXT PATTERN (AT THE END OF THE PLAYING ONE)", P),
           ("ENV + TURN = DEFAULT    REC + PLAY + TURN = RECORD    (NO P-LOCKS HERE)", P)])
CARDS["fx-2b-mutesolo"] = dict(
    title="FX BUTTON · 2nd PRESS · ALGORITHM RIGHT · MUTE / SOLO", shot="fx-2b-mutesolo", button="FX", k=_perf_k,
    select=("TURN: NEXT PAGES", P), algo=("< BACK TO PERFORM", P),
    blk=[("BLUE BOX = THE TRACK PLAYS", P), ("MUTE MODE: KEY MUTES / UNMUTES", P),
         ("SOLO MODE: KEY ADDS / TAKES OUT A SOLO", P)],
    func=[("TAP: MUTE MODE", R), ("OR UNMUTE ALL", R)], fill=[("TAP: SOLO MODE", O), ("OR CLEAR SOLOS", O)],
    white=[("WHITE KEY = NEXT PATTERN (AT THE END OF THE PLAYING ONE)", P)])
_eff_w = [("WHITE KEY HELD + SELECT = THAT STEP'S CHANCE (NO P-LOCKS ON EFFECTS)", P),
          ("ENV + TURN = DEFAULT    SAVE + TURN = FORGET MOTION    REC + PLAY + TURN = RECORD", P),
          ("PRESS FX = BACK TO MIX 1", P)]
CARDS["fx-reverb"] = dict(
    title="FX BUTTON · HOLD (½ s) · REVERB", shot="fx-reverb", button="FX",
    k=[kn("DECAY", "LENGTH"), kn("TONE", "DARK <> BRIGHT"), NONE_K, NONE_K],
    select=("TURN: NEXT PAGES", P), algo=("> REVERB 2", P), blk=_trk[:1], white=_eff_w)
CARDS["fx-reverb2"] = dict(
    title="FX BUTTON · HOLD · ALGORITHM RIGHT · REVERB 2", shot="fx-reverb2", button="FX",
    k=[kn("LOW CUT", "REVERB INPUT"), kn("RETURN", "REVERB LEVEL"), NONE_K, NONE_K],
    select=("TURN: NEXT PAGES", P), algo=("< REVERB", P), blk=_trk[:1], white=_eff_w)
CARDS["fx-delay"] = dict(
    title="DELAY · HOLD HOME (OR SELECT FROM REVERB)", shot="fx-delay", button="HOME",
    k=[kn("TIME", "1/32 ... 1/2."), kn("FEEDBACK", "REPEATS"), kn("TONE", "DARK <> BRIGHT"), kn("TAPE", "CLEAN <> WORN")],
    select=("TURN: NEXT PAGES", P), algo=("> DELAY 2", P), blk=_trk[:1], white=_eff_w)
CARDS["fx-delay2"] = dict(
    title="DELAY 2 · ALGORITHM RIGHT ON DELAY", shot="fx-delay2", button="HOME",
    k=[kn("LOW CUT", "DELAY INPUT"), kn("RETURN", "DELAY LEVEL"), kn("PING-PONG", "OFF / ON"), NONE_K],
    select=("TURN: NEXT PAGES", P), algo=("< DELAY", P), blk=_trk[:1], white=_eff_w)
