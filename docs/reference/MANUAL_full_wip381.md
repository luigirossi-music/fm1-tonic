# FM1 Tonic

FM1 Tonic turns the M-VAVE FM-1 into a drum machine in the style of Microtonic: eight synthesised drum voices (TRACK 1 to TRACK 8), a sequencer with accents and fills (patterns of 8, 16, 32 or 64 steps, or any length from 1 to 64), 16 patterns per preset, a mixer, reverb, delay and a master compressor. Nothing is sampled.

## 1. The controls

![The FM1 Tonic panel: knobs MASTER, SELECT, PRESETS, ALGORITHM and K1-K4; buttons FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- and OCT+; black keys TRACK 1-8, ACCENT, FILL, FUNC; white keys 1-16](img/fm1-tonic-panel.svg "The FM1 Tonic panel with our own labels. The small blue words under each button say the first page it opens (press it again for more pages). Black keys 1-8 are the eight tracks, black key 9 is ACCENT, 10 is FILL, 11 is FUNC; the 16 white keys are the 16 steps (with FUNC held: patterns 1-16).")

**Power-on.** The FM-1 shows the FM1 Tonic welcome screen (the logo and BY LUIGI ROSSI) for about a second and a half, then the version in big letters ("VER. 0.9") for half a second, then the normal screen.

**Knobs.** MASTER is the volume. SELECT turns pages (in a list: moves up and down). PRESETS picks a preset. KNOB 1 to KNOB 4 set the four values of the page on the screen; turn one and it is enlarged on the screen. ENV held + a knob = that knob back to its default.

**White keys 1-16** are the 16 steps shown on the screen. A pattern can have up to 64 steps: they come in four pages of 16 (1-16, 17-32, 33-48, 49-64), and SELECT turns the pages on the SEQ page (see the SEQ row). **Black keys 1-8** are TRACK 1-8: press one to select the track (it sounds, also while the pattern plays). **Black key 9 = ACCENT, black key 10 = FILL, black key 11 = FUNC.**

**OCT+ / OCT-** transpose the selected track's pitch by an octave. In a question, OCT+ is YES and OCT- is NO.

| Button | What it opens |
|---|---|
| HOME | 1st press: MIX PAGE 2 (K1 level, K2 pan, K3 LOW CUT, K4 HIGH CUT of the selected track). The two cuts are real filters like the Low Cut / High Cut 12 dB of Ableton's EQ Eight (wip295: 12 dB per octave, -3 dB at the corner, -12 dB one octave away, -24 dB two octaves away): K3 turned right cuts the lows below a corner that moves from 20 Hz (knob left; there is no OFF: the filter is always in the path, so turning it never clicks) up to 663 Hz; K4 turned left cuts the highs above a corner that moves from 20 kHz (knob right) down to 1 kHz, in steps that are finest at the top (about 20 Hz) and grow as it goes down. The screen shows the corner in whole Hz (19900, 20000 ...). Each track has its own; they are kept by the autosave and by SAVE. 2nd press: its SENDS page (K1 reverb send, K2 delay send: what a track sends goes through the reverb / delay whatever the PERFORM dry / wet and ALL FX say; K3 SIDECHAIN INPUT, K4 SIDECHAIN THRESHOLD, see SIDECHAIN below). Again: GLOBAL PARAMETERS (K1 quantization, K2 swing, K3 fill rate, K4 tempo), then back to MIX PAGE 2 (wip339: the PATTERNS and SONG screens are no longer on the HOME button; pick a pattern with FUNC + a white key, song mode is FUNC + SEQ). Hold HOME (half a second): the DELAY page. |
| FX | 1st press: MIX (K1 osc / noise mix, K2 EQ frequency, K3 distortion, K4 EQ gain). Again: PERFORM, again: MIX (the MUTE / SOLO page is behind ALGORITHM: turn it right). Hold FX (half a second): the REVERB page. |
| SEL | PITCH MOD: K1 shape (decay, sine, random), K2 amount, K3 rate. |
| ENV | OSCILLATOR: waveform, pitch, attack, decay. |
| LFO | NOISE: filter mode, Q, frequency. |
| EDIT | NOISE ENVELOPE: K1 the envelope mode (the blue icon, top left), K2 the CHOKE button (top right), K3 attack, K4 decay. Hold FUNC and turn K2 right: the selected track joins the group (blue), left: it leaves (grey); K2 alone only shows a message. To see who is in the group turn ALGORITHM right: the CHOKE screen opens, with the 8 tracks as boxes (ALGORITHM left = back). Press the black keys 1-8 of the tracks that belong to the choke group: blue = in the group, grey = not in it. A hit of one track of the group stops the others at once (a 3 ms fade), like a closed hi-hat stops an open one. A small blue triangle in the bottom right corner of the screen tells you when a page has a second page behind ALGORITHM (PERFORM, REVERB, DELAY, EDIT): on the first page it is in the bottom RIGHT corner pointing right (turn ALGORITHM right to see the second page); on the second page it moves to the bottom LEFT corner pointing left (turn ALGORITHM left to go back). The triangle fades slowly in and out (no flash), so you notice that another page is there. The CHOKE GROUP button shows blue and IN when the selected track is in the group, grey and NOT IN when it is not. Turn K2 to the left (or press any page button) to go back. The group is saved with the preset (SAVE). |
| SEQ | The steps: K1 FIXED LEN, K2 AUTOSCROLL, K3 FREE LEN, K4 SWING. With AUTOSCROLL off, SELECT turns the white keys through the step pages the pattern length has and round (length 16: no change; 32: 1-16 / 17-32; 48: three pages; 64: four). Again: the pattern number screen (K1: SONG ON / OFF; mute and solo work as on PERFORM: black keys 1-8 mute, tap FUNC = every mute off, tap FILL = SOLO mode / every solo off, white keys = next pattern; the key of an empty pattern is dark, a pattern with notes has a low light, the CURRENT pattern blinks slowly and the next one (cued) blinks faster until it starts); once more: back. FUNC + SEQ (on the other pages): song mode on / off. |
| ARP | The piano: the screen is the SEQ view, but every key (white and black) plays the selected voice at a pitch. ALGO: the voice. K1: FIXED LEN (as on the SEQ page). K2 (or OCT- / OCT+): the keyboard's octave (-3 to +3). SELECT: as on the SEQ page (the step pages). The C keys (light dim) play the voice's own pitch at octave 0; each key to the right is a semitone higher, to the left lower. The sound and its stored pitch do not change. Nothing is recorded yet. SEQ: back to the SEQ page. |
| GLO | VEL (K1 how hard velocity moves the oscillator, K2 the noise, K3 the pitch mod, of the selected track; K4 = RANDOM: hold FUNC and turn K4 to give the selected track a new random sound, only the Microtonic sound parameters, never the volume (LEVEL), the pan or the other extras; K4 alone only shows a reminder). Again: the MIXER. Again: COMP. Again: MEMORY. Again: the settings. Again: back to VEL. |
| PLAY / REC / SAVE | Play and stop; record (FUNC + REC: QUANTIZED REC or UNQUANTIZED REC, see chapter 2); save your own preset (SAVE AS). |

## 1A. Buttons and pages, one by one

This chapter takes the buttons one at a time, with every press, every page behind it and everything the knobs and keys do there. Each page has a **page card**: the panel with the real screen of that page and the labels of that page. Read the colours like the orange print on the Digitakt: **black = what the control does by itself, red = what it does with FUNC held, orange = what it does with FILL held.** The button that opens the page has a blue frame. A light blue triangle in a corner of the screen means there is a second page behind ALGORITHM (right triangle: turn ALGORITHM right; left triangle: turn it left to go back).

**On every page (not repeated below):** the black keys 1-8 select the track and play it; FUNC + a black key mutes it, FILL + a black key solos it (on pages where the keys have another job the card says so); FUNC + a white key changes the pattern; FUNC + ACCENT copies the pattern, FUNC + FILL pastes it, FUNC + PLAY clears it (chapter 2); MASTER is the volume; PRESETS picks one of the 40 presets (it always asks first: OCT+ yes, OCT- no); OCT- / OCT+ move the selected track's pitch one octave down / up (on the ARP page they move the keyboard); ENV + a knob puts that knob back to its default value.

### The FX button

FX has three kinds of pages: the track's **MIX 1** page, **PERFORM**, and the **effects** pages (REVERB, DELAY and their second pages). Pressing FX turns MIX 1 into PERFORM and PERFORM back into MIX 1. Holding FX for half a second opens REVERB. From any other button's page, FX brings you back to the last page of this group that you left (MIX 1 or an effects page).

#### FX, first press: MIX 1

![FX button, first press: MIX 1. K1 mix, K2 EQ freq, K3 dist, K4 EQ gain](img/pages/fx-1-mix.svg "FX, first press. Four values of the selected track: oscillator / noise mix, the EQ frequency, distortion and the EQ gain.")

This page belongs to the **selected track** (its number is in the blue tag at the top of the screen): change the track with a black key and the page shows that track's values.

| Control | What it does |
|---|---|
| K1 MIX | The balance of the track's two sound sources: all the way left only the oscillator, all the way right only the noise. Shown as a slider (OSC ... NOISE). |
| K2 EQ FREQ | The frequency the track's EQ works on, from 20 Hz to 20 kHz. Shown as a slider. |
| K3 DIST | How much distortion the track gets. 0 = none. |
| K4 EQ GAIN | How much the EQ boosts or cuts at that frequency. 64 (the middle) = nothing, to the left it cuts, to the right it boosts. |
| SELECT | Nothing by itself (this page has no second page). With a white key held it sets that step's **chance** (5 to 100 %). |
| ALGORITHM | Nothing on this page. |
| White key held + K1-K4 | **P-lock:** that step gets its own value of that knob; all other steps keep the normal value. While the pattern plays the lock is stored at once. |
| SAVE + a knob | Forgets that knob's p-lock (with the step held) or its recorded motion. |
| REC + PLAY, then turn a knob | Records the knob's movement as automation of this pattern (at most 3 knobs per track; the screen says MAX 3 PER TRACK or MOTION FULL when there is no room). |
| Black keys 1-8 | Select the track and play it. |
| FUNC + a black key | Red on the card: mutes / unmutes that track. |
| FILL + a black key | Orange on the card: solo of that track. |
| EDIT (or LFO) held + FUNC + ACCENT / FILL / PLAY | Copies, pastes and resets **this page** of the track: you can copy the four values of MIX 1 from one track to another. |

#### FX, second press: PERFORM

![FX button, second press: PERFORM. K1 reverb D/W, K2 delay D/W, K3 low cut, K4 all FX](img/pages/fx-2-perform.svg "FX, second press: PERFORM. The big number is the playing pattern; the black keys mute and solo.")

PERFORM is the page to play live with: the whole screen shows the **number of the pattern** that is playing (big; when another one is waiting, its number is shown small), and the four knobs are the effects of the whole machine, not of one track. It never shows a track number as its own page.

| Control | What it does |
|---|---|
| K1 REVERB D/W | How much of the tracks goes into the reverb (dry / wet), 0 to 100 %. At every power-on it is 0 %. This is separate from each track's own reverb send (HOME, second press). |
| K2 DELAY D/W | The same for the delay, 0 to 100 %, 0 % at power-on. |
| K3 LOW CUT | A low cut on the whole mix (the master), from 20 Hz (all the way left, nothing is cut) up to about 660 Hz. Shown in Hz. |
| K4 ALL FX | Right = ON, left = OFF. OFF mutes the two D/W sends and takes the master low cut out of the sound, so you hear the dry mix; the tracks' own sends (HOME, second press) keep working. It is not saved: it is ON at every power-on. |
| SELECT | Turns to the next pages: REVERB, DELAY, REVERB 2, DELAY 2 (see below); it stops at the ends. |
| ALGORITHM | Turn right: the MUTE / SOLO boxes page. Turn left to come back. |
| Black keys 1-8 | **Mute buttons** (not track select): first press mutes the track, second press unmutes it. A muted track's key blinks slowly. In SOLO mode they add / take out solos instead. |
| White keys 1-16 | **Next pattern**: the pattern starts at the end of the one that is playing (stopped: at once). The current pattern's key blinks slowly, the next one blinks faster until it starts; an empty pattern's key is dark, a pattern with notes has a low light. |
| FUNC (tap) | Red on the card: if some tracks are muted, unmutes them all; otherwise switches to MUTE mode (its key lit). |
| FILL (tap) | Orange on the card: if some solos are on, clears them all; otherwise switches between SOLO mode (its key lit) and MUTE mode. |
| REC + PLAY, then turn K1-K4 | Records the movement of the knob as automation, like a sound knob. No p-locks on this page. |
| FX | Back to MIX 1. |

#### FX, second press, ALGORITHM right: MUTE / SOLO

![FX button, MUTE / SOLO page: eight boxes, one per track](img/pages/fx-2b-mutesolo.svg "The MUTE / SOLO page: eight boxes, blue = the track plays.")

The same page as PERFORM with the number replaced by **eight boxes**, one per track; the title says MUTE or SOLO, the mode you are in. A blue box means the track is heard, a dark one that it is silenced (muted, or not in the solo). The knobs, the black keys, the white keys, FUNC and FILL work exactly as on PERFORM; only the picture changes. The mutes and solos are shared by both pages. Turn ALGORITHM left to go back to PERFORM; FX goes to MIX 1.

- In **MUTE mode** (the default, FUNC key lit) a black key mutes or unmutes that track.
- In **SOLO mode** (FILL key lit) a black key adds or takes out a solo; you can have **several solos at once**. Tapping FILL while solos are on clears them all.
- Solos are not saved and end when you load a preset.

#### FX, hold: the effects pages (REVERB, DELAY)

Hold FX (half a second, nothing else pressed): the **REVERB** page opens. These pages are global (the reverb and the delay are shared by all the tracks). SELECT walks through all the FX pages in this order and stops at the ends: PERFORM, REVERB, DELAY, REVERB 2, DELAY 2. ALGORITHM turns between the two pages of one effect (REVERB and REVERB 2, DELAY and DELAY 2). The text under the title always shows the delay time and kind, the feedback and the reverb decay, so you can see both effects from either page. Press FX to go back to MIX 1. The DELAY page also opens by holding HOME.

![FX button, hold: REVERB. K1 decay, K2 tone](img/pages/fx-reverb.svg "REVERB: decay and tone. ALGORITHM right: REVERB 2.")

| Control | What it does |
|---|---|
| K1 DECAY | The length of the reverb tail. |
| K2 TONE | Dark (left) to bright (right). |
| ALGORITHM right | REVERB 2. |

![FX button: REVERB 2. K1 low cut, K2 return](img/pages/fx-reverb2.svg "REVERB 2: the low cut of what goes into the reverb, and the reverb's level in the mix.")

| Control | What it does |
|---|---|
| K1 LOW CUT | Cuts the lows that go into the reverb (keeps the bass clean). |
| K2 RETURN | How loud the reverb comes back into the mix. |
| ALGORITHM left | Back to REVERB. |

![DELAY: K1 time, K2 feedback, K3 tone, K4 tape](img/pages/fx-delay.svg "DELAY: time (note values), feedback, tone and the one-knob TAPE.")

| Control | What it does |
|---|---|
| K1 TIME | The echo time as a note value of the tempo: 1/32, 1/16T, 1/16, 1/8T, 1/16., 1/8, 1/4T, 1/8., 1/4, 1/2T, 1/4., 1/2, 1/2. |
| K2 FEEDBACK | How many repeats. |
| K3 TONE | Dark (left) to bright (right) repeats. |
| K4 TAPE | One knob from clean to worn: the first part of the knob (about the first eighth) is the clean digital delay; after it the delay is a tape echo and the further right you turn, the more it wears. Shown in %. |
| ALGORITHM right | DELAY 2. |

![DELAY 2: K1 low cut, K2 return, K3 ping-pong](img/pages/fx-delay2.svg "DELAY 2: low cut, return and ping-pong.")

| Control | What it does |
|---|---|
| K1 LOW CUT | Cuts the lows that go into the delay. |
| K2 RETURN | How loud the delay comes back into the mix. |
| K3 PING-PONG | OFF / ON: with ON the repeats bounce between left and right. |
| ALGORITHM left | Back to DELAY. |

On the effects pages: a white key held + SELECT sets that step's chance (SELECT then does not change page); ENV + a knob puts it back to its default; REC + PLAY + a knob records the movement as automation; there are no p-locks.

## 2. Playing a pattern

- **Pick a track** with its black key 1-8. **Put a step on** with a white key; tap a step that is on to take it off.
- **Accent** (loud) and **fill** (the track plays four times over that step): keep the white key pressed and press black key 9 (accent) and / or 10 (fill); press again to take the flag off. Or hold 9 / 10 first and press white keys. Colours in the step grid: trig white; accent = dark blue; fill = yellow; accent and fill together = green (the whole step in each case).
- **FIXED LEN** (SEQ, K1): 8, 16, 32 or 64 steps, 16 by default; the white keys show 16 steps at a time. Making the pattern longer copies what is there into the new part (8 to 16 copies steps 1-8 into 9-16). Making it shorter only shortens it. **FREE LEN** (K3) sets any length from 1 to 64.
- **Swing** 0-100 %. **Quantization** (HOME, 2nd press): the step grid, 1/8 to 1/32. **Fill rate**: 2x, 4x or 8x.
- **Change the pattern:** hold FUNC and press a white key 1-16. While playing, the change happens at the end of the bar.
- **Copy, paste, clear** (hold FUNC): + ACCENT (black key 9) copies the pattern, + FILL (black key 10) pastes it (hold FILL and press a white key to paste into that slot), + PLAY clears it (it asks). With a black key 1-8 held as well, the same keys copy, paste and clear that track's sound or steps; with EDIT held they copy, paste and reset the page. HOME + REC is undo. **Copy-paste on the fly (wip362):** after FUNC + ACCENT all the white keys blink: let go and press a white key = the copy is pasted there (no trig) and **that pattern becomes the current one** (while playing it starts at the end of the bar; stopped: at once), and the mode ENDS (wip377). To paste in MANY places instead: before you choose the destination **tap FUNC alone** (press and let go, nothing else pressed): the mode stays on, the blue text then says WHITE KEYS PASTE, every white key you press gets the copy and the current pattern does not change, until a button (SEQ, HOME ...) leaves it. FUNC HELD + a white key is still the pattern change. Hold FUNC + ACCENT and press a white key instead = THAT pattern is the copy (its key stays lit, the others blink; with FUNC + ACCENT alone the playing pattern's key is the one that stays lit). The blue text says PRESS WHITE KEY PASTE and FUNC: PASTE MANY (SEQ: EXIT once FUNC was tapped); in the kept mode a key stops blinking (stays lit) once it has been pasted into. Any button (SEQ, HOME ...) leaves the mode.
- **Mute a track:** hold FUNC and press its black key (again = unmute). On the PERFORM page the black keys 1-8 are mute buttons.
- **Solo a track:** hold FILL (black key 10) and press the track's black key 1-8: only that track plays and its key blinks. FILL + another track's key moves the solo there. To end it, press the soloed track's black key (or FILL + that key again). The solo does not change your mutes, is not saved, and ends when you load a preset. It works on every page except the ARP page (where the keys are a piano); on the PERFORM page the press that ends a solo does not also mute the track.
- **Live recording (REC + PLAY):** press REC, then PLAY (both blink); every black key 1-8 you press writes a trig of that track at the nearest step (a hit in the second half of a step goes to the next one; FUNC + REC switches between QUANTIZED REC, the default, and UNQUANTIZED REC, and while the pattern is playing it also switches recording on, which keeps the step that is playing and plays the hit late inside it, exactly as you pressed it; the exact timing is not saved at power-off yet), and every sound knob you turn is recorded as automation. It works on every page except PERFORM and the pattern page (SEQ, 2nd press), where the black keys 1-8 are mute buttons. On pages without a track key of their own (HOME, SONG, REVERB, DELAY ...) the black keys pick and play the track too.
- **P-locks (parameter locks, per step):** keep a white key pressed and turn a knob: that step gets its own value for that knob, and every other step keeps the track's normal sound. This works on the voice pages: ENV (oscillator), LFO (noise), EDIT (noise envelope), GLO first press (VEL), SEL (pitch mod) and FX (the mix page), for the knobs of the selected track, in the pattern you are editing. Not on the SEQ page, and not on the mixer. While the pattern plays, the lock is stored at once and you hear it the next time that step comes round; while it is stopped you hear it as long as you hold the step. To take a lock away, hold the step, hold SAVE and turn the same knob. There are 160 lanes in total (one per knob, per pattern, per track; recorded knob movements use them too): when none is left the screen says MOTION FULL. Clearing the pattern or the track (FUNC + REC) also clears its p-locks, and copy / paste of a pattern carries them along. Probability: on the SEQ page or on any voice page (ENV, LFO, EDIT, SEL, FX), hold a step and turn SELECT: the step's chance, down from 100 % in 5 % steps, then the cycles 1:2, 2:2, 1:3 … 8:8. Turn back up to 100 % to remove it. A track holds at most 3 locked knobs (lanes) in a pattern; a 4th one shows "MAX 3 PER TRACK" (a knob that already has a lock can still get more steps). **Delete a track's automation (wip367):** hold FUNC, tap EDIT and let it go (nothing else pressed): it asks DELETE AUTOMATION? and OCT+ deletes every lock and recorded knob motion of the selected track in the playing pattern (other tracks, other patterns and the step probabilities stay; the knobs go back to their own values; undo takes it back); OCT- keeps it. If the track has none it only says NO AUTOMATION ON the track. Holding FUNC + EDIT and then pressing ACCENT, FILL or PLAY is the page copy / paste / reset and does NOT delete.
- **Probability (a step's chance):** hold a step (a white key) and turn SELECT: that step plays 5 to 100 % of the time. It works on the SEQ page, the sound pages (ENV, LFO, EDIT, SEL, FX first press), the mixer (GLO second press) and the effects pages after PERFORM (REVERB, DELAY and the others). Not on PERFORM, HOME, SONG, GLOBAL, COMP or the lists, where those keys have other jobs. With no step held, SELECT does what it always did on that page.

## 3. Chains and song mode

A pattern can be flagged as chained: it runs into the next pattern when it ends, and after pattern 8 comes pattern 1. A loaded preset brings its flags and, when it has some, plays through them (SONG ON). A flag only counts on a pattern that has notes: a flag on an empty pattern is forgotten when the preset loads, an empty slot or a preset with no chain loads with SONG OFF (nothing runs on by itself while you edit), and when you clear a pattern (FUNC + PLAY) its flag goes too; with the last flag gone the chain is no longer followed. SEQ's second press and K1 turn it off (SONG OFF): the pattern loops. In song mode SONG OFF does not leave song mode, it pauses the bars and the current bar loops; SONG ON lets the bars go on again.

**Song mode:** hold FUNC and press SEQ, then confirm with OCT+ (OCT- goes back). If you have no song yet, the PATTERNS screen opens: press white keys 1-16 to add patterns (the same one as often as you like), OCT- takes the last one off (so does FUNC + any white key, one more pattern off at every press), OCT+ is OK; then PLAY plays the song, one bar per pattern, and loops it. Hold FUNC and press SEQ again (and confirm with OCT+) to leave song mode: it stops and PLAY plays one pattern again.

**The SONG screen (wip356):** in song mode the bars of the song are listed, one row per bar. It is ONE page of three knobs: K1 = the bar (the cursor), K2 = that bar's pattern, K3 = how many times the bar repeats (x1 to x8); SELECT has no second page to turn to. At the bottom, in blue, it says WHITE KEYS 1-16 ADD ROW and FILL KEY DELETE (only on this screen). White keys 1-16 write that pattern into the bar on the cursor and move the cursor on (past the last bar it adds a new bar). FILL, pressed and let go with nothing else, deletes the bar on the cursor at once, with no question (with the cursor on END the last bar; when the last bar goes, the song is deleted and song mode is left). The screen has no BAR tag at the right of the knob row. (wip380) Turning a knob here shows no zoom (the big dial over the screen): the turned cell is only highlighted, so the bar list stays visible while you choose. From any other page, hold FILL and press SEQ to come back to the SONG screen (outside song mode it only says NOT IN SONG MODE). FILL + SEQ is not available on the ARP page, where FILL is a piano key.

**Delete the song:** hold FUNC and press ARP. The screen asks DELETE SONG? (OCT- no, OCT+ yes). Yes clears the song and leaves song mode.

## 4. The voice (24 values per track)

Wave, Pitch, OscAtk, OscDcy (oscillator); ModMod, ModAmt, ModRat (pitch mod); NFMode, NFFreq, NFQ (noise filter); NEMode, NEAtk, NEDcy (noise envelope); Mix, EQFreq, EQGain, Dist, Level, Pan, and VelOsc, VelNs, VelMod, and the track's two sends, RevSnd (to the reverb) and DlySnd (to the delay). Drums are monophonic: a new hit cuts the same track's previous one. Accent plays at full velocity, a normal step at about a third of it.

### 4.1 The sound engine page by page

The FM1 Tonic sound engine is the Microtonic / PO-32 Tonic drum engine: every track makes its sound from scratch (nothing is sampled) with an oscillator, a noise generator, a pitch modulator, a mixer, a distortion and a one-band EQ. These 21 sound settings are the ones a Microtonic or Patternarium preset carries. Every page below shows the selected track (the top line of the screen says TRACK n and the page name); the four knobs K1 to K4 set the four big values, in the order the page draws them. ENV held + a knob puts that knob back to its default.

| Page (button) | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| ENV: OSCILLATOR | Wave: SINE, TRI or SAW (three-way, turns slowly) | Osc Freq: the pitch, 20 Hz to 20 kHz (OCT- / OCT+ move it by an octave) | Attack | Decay |
| SEL: PITCH MOD | Shape: DECAY, SINE or RANDOM | Amount (64 = none) | Rate | - |
| LFO: NOISE FILTER | Filter mode: LP, BP or HP | Q (resonance) | Filter frequency | - |
| EDIT: NOISE ENVELOPE | Envelope mode: EXP, LIN or CLAP | CHOKE button (see chapter 2 and the EDIT row above) | Attack | Decay |
| FX, 1st press: MIX | Mix of oscillator and noise | EQ frequency | Distortion | EQ gain (64 = flat) |
| GLO, 1st press: VEL | How much velocity changes the oscillator | ... the noise | ... the pitch mod | RANDOM (hold FUNC and turn) |

**The oscillator (ENV).** The oscillator is the tonal part of a drum: the body of a kick, the tone of a tom. Pick the wave (SINE is the softest, SAW the brightest), then its pitch. Attack lets the tone fade in (0 = a hard start) and Decay says how long it rings. A short decay gives a click, a long one a boom.

**The pitch mod (SEL).** The pitch of the oscillator can move while the sound plays. DECAY makes the pitch start high (or low) and glide to the set pitch, the classic kick drop; Amount sets how far, and a negative amount goes the other way. SINE makes the pitch wobble like a vibrato, with Rate as its speed. RANDOM makes it jump about at random, so the sound becomes metallic and unpredictable. At Amount 64 nothing moves.

**The noise (LFO, EDIT).** The noise generator makes the snap of a snare and the hiss of a hi-hat. The LFO button opens its filter: LP keeps the lows, HP keeps the highs, BP only a band around the frequency; Q makes that band sharper and more ringing. The EDIT button opens its envelope: EXP is a quick drop, LIN a straight fade, CLAP is a few short bursts one after the other, like hands clapping. Attack and Decay work as for the oscillator.

**The mix, the distortion and the EQ (FX, 1st press).** MIX blends the two sources: all the way to one side you hear only the oscillator, to the other only the noise. Distortion adds grit and loudness. The EQ is one band: EQ frequency chooses where, EQ gain boosts or cuts it (64 = flat).

**Velocity (GLO, 1st press).** A normal step plays at about a third of full velocity and an accented step at full velocity. The three VEL knobs say how much that difference changes the oscillator, the noise and the pitch mod of the track. At 0 an accent is only louder.

**Level and pan (HOME, 1st press).** K1 the track's level, K2 its pan (64 = centre). Both are part of a Microtonic preset.

**The random sound (GLO, K4).** Hold FUNC and turn K4: the selected track gets a new random sound. Only the 21 sound settings change, never the extra ones below.

### 4.2 What the FM1 Tonic adds: the ENHANCED engine

The 21 sound settings above are all that Microtonic and Patternarium presets contain. The FM1 Tonic adds more. In the settings page (GLO, fifth press) the item **Tonic Engine** is **enhanced** by default; in **classic** the track extras below are greyed out (a shade over the knob, the message CLASSIC ENGINE) and do not turn, and the sidechain is silent. Their values are kept and come back when you return to enhanced. The track's level and pan stay available in classic because Microtonic presets have them. The mode is global (it is not saved inside a preset).

The extras of the enhanced engine:

- **LOW CUT and HIGH CUT** per track (HOME, K3 / K4): two real 12 dB per octave filters. Greyed in classic.
- **Reverb send and delay send** per track (HOME, 2nd press = SENDS, K1 / K2), with the reverb and delay pages (FX button) and the PERFORM page (dry / wet, a filter, ALL FX on / off). The sends are greyed in classic.
- **Sidechain** (SENDS, K3 input track, K4 threshold, and the DUCK screen where you choose which tracks duck). Greyed and silent in classic.
- **Master compressor** (GLO pages): an extra of the FM1 Tonic, not part of a Microtonic preset.
- **Choke group** (EDIT, K2 and the CHOKE screen), **p-locks** (a knob value per step), step **probability** and **microtiming**.
- **MUTE / SOLO, copy / paste / clear, chains and song mode, the ARP piano, live recording** and the **hit flash** on the black keys (chapters 2, 3 and 9).

Why a classic mode exists: a patch you build with the extras on would not carry them if you took it to Microtonic or the PO-32 Tonic. Classic lets you build only sounds that will travel.

## 5. The mixer (GLO)

Eight tall vertical sliders, one per track, over the whole screen (no knob row below). **SELECT** switches between tracks 1-4 and tracks 5-8; **K1-K4** set the level of the four tracks shown, in order. A black key picks a track and its group. A hit lights its slider up to the level. FUNC + black key mutes.

**COMP (GLO, third press):** the master compressor with a needle meter (gain reduction): K1 attack, K2 release, K3 threshold, K4 make-up. The ratio is 4:1. By default nothing is compressed (threshold 0.0 dB, make-up 0.0 dB), and a preset starts like that unless you saved other compressor settings in it. The compressor settings are kept only when you press SAVE (see chapter 7).

## 6. Effects (FX)

- **PERFORM:** K1 reverb dry / wet, K2 delay dry / wet, K3 master LOW CUT (20 Hz .. 663 Hz, the same smooth 12 dB per octave cut as each track's LOW CUT, but on the whole mix only: it does not change the tracks' own low cuts), K4 ALL FX on / off. ALL FX OFF mutes these three (the whole kit's send and the master low cut), not the tracks' own reverb / delay sends of the HOME SENDS page. The white keys pick the next pattern, the black keys 1-8 mute.
- **MUTE / SOLO (PERFORM, turn ALGORITHM right; left = back):** MUTE mode (the default, FUNC lit): black keys 1-8 mute / unmute the tracks. SOLO mode (tap FILL, FILL lit): a black key solos that track, another key moves the solo, the same key again ends it; keep FILL pressed to solo several tracks together. Tap FILL while solos are on: every solo goes away at once. The same for mutes: tap FUNC while some tracks are muted and every mute goes away, the whole pattern plays again (FUNC with no mutes just goes back to MUTE mode). Leaving the page does not change mutes or solos. The white keys still pick the next pattern.
- **REVERB** and **DELAY:** send effects with their own pages; both work as master-out effects. Hold FX (half a second) for REVERB, hold HOME for DELAY (turn ALGORITHM right to go to the second page: REVERB 2 has LOW CUT and RETURN; DELAY 2 has LOW CUT, RETURN and PING-PONG; turn it left to come back); a short press of FX goes back to MIX.

## 7. Presets and saving

Turn **PRESETS** to move through the 40 places (01-08 are the factory presets, 09-40 are yours; an empty place loads with the SOUNDS of a factory preset and no pattern, no song: 09 sounds like 01, 10 like 02 ... 16 like 08, then 17 like 01 again, and so on up to 40 like 08; empty ones are skipped). A question asks first: OCT+ loads it, OCT- keeps your sound. Loading replaces everything you edited and is refused while playing. A loaded preset starts with SONG ON only if it is chained (see chapter 3); otherwise SONG OFF.

**SAVE** first asks **SAVE HERE?** with the number and the name of the place that is loaded now. **OCT+ (YES)** saves into that place at once and keeps its name. Turning **ALGORITHM** on this screen picks another place (1-40): the screen then shows the place number, the project's name and, in amber, who would be replaced (or EMPTY PLACE); OCT+ (YES) saves there at once and that place is the loaded one from then on. **OCT- (NO)** goes to **CHOOSE SLOT**: OCT- is NO, EXIT (leaves the save), OCT+ goes on to the slot screen (it starts at the next empty place after the loaded one, or at the next place when all are full; ALGORITHM turns the places 1-40, round, and shows who is in each one; places 1-8 can be overwritten too; OCT+ confirms, OCT- goes back), then to the name screen (ALGORITHM makes another made-up name; OCT+ saves, OCT- goes back). HOME leaves at any screen without saving. If no place has been loaded since power-on, SAVE starts at CHOOSE SLOT. It works while the sequencer is playing (the memory write can make the sound hiccup for a moment). Your working project is saved by itself 4 seconds after you stop, except the compressor, the reverb and the delay: those are kept only when you press SAVE. SAVE puts them inside the preset together with the patterns and the sounds, so when you load that preset the compressor, reverb and delay are as you left them. When you switch the FM-1 on, the PERFORM page always starts clean: reverb dry / wet 0 %, delay dry / wet 0 %, master LOW CUT 20 Hz, ALL FX on (the compressor comes back as you saved it). A preset with none saved (the factory presets, one made from Microtonic) starts with the compressor at threshold 0.0 dB and make-up 0.0 dB and the reverb and delay off.

**FUNC + SAVE (quick save)** saves the project into the preset place that is loaded now, without asking for a slot, also while the sequencer is playing. If that place is **empty**, it saves at once (no question, no name screens) and names it UNTITLED 01, 02, 03 ... (the lowest number not used by another place). If it holds something, it asks **OVERWRITE SLOT?** with the slot name big under it, OCT- EXIT and OCT+ YES; YES saves at once and keeps the name. HOME also leaves without saving. Empty places are listed as EMPTY, as always.

**FUNC + HOME (rename project)** renames the project that is loaded now and saves it into the same preset place (places 01-08 too; the place number does not change). It shows the name screen alone: ALGORITHM makes another made-up name, OCT+ saves, OCT- or HOME cancels and changes nothing. It also works while the sequencer is playing. If no place has been loaded since power-on it says NO SLOT YET: USE SAVE.

**NAME PROJECTS (GLO settings list, PROJECT group):** GENERATE (default) makes the name for you (ALGORITHM = another one). MANUAL: the name screen of SAVE (when you choose a slot) and of FUNC + HOME (rename) becomes a letter editor. The first letter, A, blinks; turn K4 to cycle the capitals, digits, space and the signs - . + # / % : ?; OCT+ accepts the letter and the next one appears; OCT- deletes the last accepted letter. Once a letter is accepted, PRESS SAVE blinks: press the SAVE button to save under that name. A name has 8 letters at most (OCT+ after the eighth saves). SAVE HERE? (OCT+ YES) keeps the project's name and does not ask for one.

**MEMORY (GLO, fourth press):** an upright battery full of blue liquid shows how full the project memory is (patterns, song, motion, ARP notes) as a percentage. The liquid turns purple from 90% (NEARLY FULL) and blinks FULL at 100%: a save is then refused and the last save stays. The figure is the size of the last save or load, so it moves when the project is saved or autosaved. The knobs do nothing here; GLO goes on to the settings, HOME leaves.

**SIDECHAIN (HOME, 2nd press = SENDS):** a track can duck the others, like a kick that makes the bass pump. **K3 SIDECHAIN INPUT** (bottom left, shown as a big number) chooses the track that does the ducking (1-8, default 1: the kick is not always the same track). **K4 SIDECHAIN THRESHOLD** (bottom right) says how deep the others duck, 0-100 %; at 0 % nothing ducks (default, there is no on / off switch). The duck follows the sound of the input track, so a long kick note ducks for a long time and a short one for a short time; a muted input does not duck. The ducked tracks' reverb and delay sends are ducked too. To choose who is ducked turn **ALGORITHM right** on SENDS: the DUCK screen opens with the 8 tracks as boxes (a blue triangle at the bottom shows it, ALGORITHM left or HOME = back). Press the black keys 1-8: **blue = ducked, grey = not ducked, purple = the sidechain input**. Nothing is ducked until you choose. The input track cannot be ducked: pressing its box says TRACK n IS INPUT: CHOOSE DIFFERENT TRACK. If you turn K3 onto a track that was ducked, it becomes the input (purple); its duck is kept and comes back when the input moves away. A **red dot** shows a ducked track (SENDS: right in the middle between the two sidechain knobs, for the selected track; GLO mixer: under each slider); it is dark at rest and pulses bright red each time the input ducks it, at any tempo. The input track has a **purple dot**. The dots show only with an intensity above 0. The sidechain is the same for every track (one input, one intensity, one group). It is kept by the autosave and by SAVE (a preset made before the sidechain loads with intensity 0). With the **CLASSIC** TONIC ENGINE (GLO settings) the sidechain is switched off: its knobs are greyed and do not turn, the DUCK keys refuse, and there is no sound change; the values are kept and come back with ENHANCED.

## 8. Settings (GLO, fifth press)

CLOCK OUT, NOTES OUT, MIDI channel of the Tonic, output level, USB audio, key light, colour theme, zoom knob (YES / NO), SAVE, autosave, clear pattern, factory reset, performance, cores, about, battery. Rows are grouped under MIDI, AUDIO, KEYS, PROJECT and SYSTEM. A change in these settings is always kept by itself, even with AUTOSAVE off: the sequencer must be stopped and you must stop touching things for about a second and a half; it then says SETTINGS SAVED. With AUTOSAVE off only the settings are kept like this, never your patterns or sounds (those wait for SAVE).

## 9. The lights

PLAY lights on every 4th step (red while recording). The REC LED is off when not recording; with QUANTIZED REC it is steady while stopped and blinks with the beat while playing; with UNQUANTIZED REC it blinks two fast and one slow, over and over. The button of the page you are on is lit. White keys show the steps of the selected track (the playing step flips); white keys 1, 5, 9 and 13 can glow faintly on every page except PERFORM to mark the four beats of the bar (they are off by default; GLO > 4/4 LED switches them on); black keys show the selected track bright (blinking when muted) and tracks with notes dim; while playing, a track's black key blinks for each of its hits, on every page except ARP (wip378). While FUNC is held, the keys show which patterns and tracks have notes.

## 10. Install

**Try it first:** the [emulator](https://luigirossi-music.github.io/fm1-tonic/emu/) runs the same code in the browser, with sound, played with the mouse, a touch screen or the keyboard.

**Install:** open the [web installer](https://luigirossi-music.github.io/fm1-tonic/install/) in Chrome or Edge, connect the FM-1 with a USB data cable and press Install. Don't unplug it while it is writing; when it finishes, the FM-1 restarts into Tonic.

**Update:** the same way. Installing another firmware does not keep your project: each firmware keeps its own things in the same memory, so coming back to Tonic starts with an empty project. To keep it, save it to a file first (see [Back up your project](#back-up-your-project)).

**Go back:** "Back to the stock firmware", at the bottom of the same page, puts M-VAVE's firmware back (download the FM-1 V15 file from the link there, choose it, press the button). M-VAVE's own updater, M-UPGRADE, works too.

> Installing other firmware is at your own risk. If the FM-1 ever won't start, see [Recovering an FM-1 that won't start](#recovering-an-fm-1-that-wont-start).

**From the command line:** build the firmware (see BUILDING.md) or download the `.fwsc` file, install Python 3 with `pip3 install mido python-rtmidi`, and run:

```
python3 tools/fm1_install.py build/fm1-tonic.fwsc
```

## 11. Back up your project

The FM-1 keeps one project. To keep more, or to keep yours while you try other firmware, save it to a file on your computer and load it back later. Open the [backup page](https://luigirossi-music.github.io/fm1-tonic/backup/) in Chrome or Edge, with the FM-1 connected by USB.

**Back up:** press Back up. The project goes to an .x0b file in your downloads: the sounds, all 32 patterns, the song, the knob motion, the tempo and the settings. The FM-1 can keep playing, but changes it hasn't saved yet go in the file only when it is stopped.

**Restore:** stop the FM-1, choose a backup and press Restore. The firmware checks the whole file before anything changes. Then its screen asks RESTORE FROM USB? Press SEL to restore, or HOME to keep what you have. The backup replaces the project on the FM-1, and it loads at once.

**Export as .mtpreset (the page's "Export my presets as .mtpreset"):** reads presets from the FM-1 and saves them as Microtonic .mtpreset files. Export all downloads one zip with a file per saved slot (09_name.mtpreset ...); Export one downloads the slot chosen in the menu. Nothing on the FM-1 changes. Left out, because a .mtpreset has no place for them: ARP notes, late hits, per-track cuts and sends, compressor, reverb, delay and mutes; drum names become "Drum n". Slots 1-8 (factory) cannot be read from the FM-1.

**Presets in slots 9-40 (the page's "Back up my presets" / "Restore my presets"):** Back up downloads a zip with one .tnp file per used slot (09_name.tnp ...). Restore takes that zip, single .tnp files, or Microtonic .mtpreset files (Patternarium downloads): choose one or many files, or a whole folder. Every preset in the list has a slot menu (1-40; 1-8 are the factory presets and cannot be written yet): choose where each one goes. A number 9-40 at the start of the file name (09_name.mtpreset) is chosen for you; "first empty" takes the first empty slot. The page shows the list first and asks before it writes. A Microtonic preset keeps its 8 sounds, tempo, swing and its first 8 patterns.

**From the command line:** with the FM-1 connected, run:

```
python3 tools/x0x_backup.py backup my-song.x0b
python3 tools/x0x_backup.py restore my-song.x0b
python3 tools/x0x_backup.py info my-song.x0b
```

## 12. MIDI and USB audio

When connected over USB, the FM-1 shows up as a MIDI device. It also takes MIDI from the TRS MIDI IN jack, both the same way.

| Input | What it does |
|---|---|
| Channel 10 (default) | Plays the Tonic's voices with General MIDI drum notes, in voice order: 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51. The velocity sets the loudness. |
| Clock | The FM-1 follows an incoming MIDI clock. Start, Stop and Continue start and stop the pattern. |

GLO (the settings list, fifth press) sets the Tonic's channel (TONIC MIDI CH), or OFF to ignore its notes, for example when another box sends you its clock with notes on the same channel. The FM-1 can also send MIDI: clock and start / stop when CLOCK OUT is on, and the pattern as notes on the Tonic's channel when NOTES OUT is on.

### USB audio

Over the same USB cable, the FM-1 also shows up on a computer as an audio input: two channels at 44.1 kHz, with no driver to install. It carries what the headphones hear, after VOLUME, in stereo. Record it in any audio app, or monitor it live through one. The first moment after an app opens the input can click while the FM-1 settles.

## 13. Troubleshooting

**Update mode:** hold OCT- and OCT+ together for five seconds. A countdown appears; let go before it ends to cancel.

**Calibration:** if a button or knob does the wrong thing, hold OCT- and OCT+ while you switch the FM-1 on. The screen shows HARDWARE CALIBRATION and asks you to press each button and turn each knob in turn.

**After a crash:** the FM-1 shows a red CRASH screen and restarts by itself a few seconds later. The next start shows RESTARTED AFTER A CRASH and an address (PC ...) for a moment. If you report the problem, include that address and what you were doing.

**Safe mode:** if the firmware crashes twice in a row while starting, it starts in SAFE MODE instead. There is no sound, but USB works, so the web installer can reach it: reinstall the firmware, or put the stock firmware back from the same page. Press PLAY to try starting normally again. If even safe mode fails twice, the FM-1 drops into the chip's own update mode, and only the rescue below can reach it.

## 14. Recovering an FM-1 that won't start

If the web installer and M-VAVE's updater cannot see the FM-1 (a blank screen after a crash, for example), a script can put M-VAVE's firmware back, on a Mac, with the USB cable you already have.

Open Terminal, paste this line and press Return:

```
bash -c "$(curl -fsSL https://raw.githubusercontent.com/luigirossi-music/fm1-tonic/main/tools/fm1_rescue.sh)"
```

1. If a window asks to install the command line developer tools, click Install. When it has finished, paste the line again.
2. If it asks for a password, type your Mac password (nothing shows as you type) and press Return.
3. When it says it is waiting, plug the FM-1 in and switch it on. It crashes and goes blank, and the script catches it. It checks the chip, saves a backup of the FM-1's memory in the `fm1-rescue` folder in your home folder, and asks before it writes anything.
4. Type **yes** and press Return. Don't unplug anything until it says it is restarting the FM-1. The FM-1 then starts into its original firmware, and you can install the Tonic firmware again from there.

The script downloads M-VAVE's FM-1 V15 firmware and JieLi's flash loader, and checks both against known fingerprints. It writes only the firmware area of the memory, never the bootloader or the FM-1's own settings, and checks everything it writes. Running it again is always safe. If it fails, copy everything in the Terminal window and send it with your report. If the FM-1 never shows up at all, Felucca's [FM-1-transporter](https://github.com/kurogedelic/FM-1-transporter) (a small RP2040 board wired to the USB lines) can still reach it.

## 15. Parameters

Every track has the same 24 settings. Most go from 0 to 127. The default is the value a new track starts with.

| Setting | Range | Default | What it does |
|---|---|---|---|
| Wave | SINE, TRI, SAW | SINE | Oscillator waveform |
| Pitch | 0-127 | 60 | Oscillator pitch |
| OscAtk | 0-127 | 0 | Oscillator attack |
| OscDcy | 0-127 | 78 | Oscillator decay |
| ModMod | DECAY, SINE, RANDOM | DECAY | Pitch modulation shape |
| ModRat | 0-127 | 64 | Pitch modulation rate |
| ModAmt | 0-127 | 64 | Pitch modulation amount (64 = none) |
| NFMode | LP, BP, HP | LP | Noise filter type |
| NFFreq | 0-127 | 127 | Noise filter frequency |
| NFQ | 0-127 | 22 | Noise filter resonance |
| NEMode | EXP, LIN, CLAP | EXP | Noise envelope shape |
| NEAtk | 0-127 | 0 | Noise envelope attack |
| NEDcy | 0-127 | 0 | Noise envelope decay |
| Mix | 0-127 | 0 | Oscillator / noise mix |
| Dist | 0-127 | 0 | Distortion |
| EQFreq | 0-127 | 0 | EQ frequency |
| EQGain | 0-127 | 64 | EQ gain (64 = flat) |
| Level | 0-127 | 101 | Track level |
| VelOsc | 0-127 | 0 | How much velocity changes the oscillator |
| VelNs | 0-127 | 0 | How much velocity changes the noise |
| VelMod | 0-127 | 0 | How much velocity changes the pitch modulation |
| Pan | 0-127 | 64 | Position in the stereo field (64 = centre) |
| RevSnd | 0-127 | 0 | How much of the track goes to the reverb (HOME, K3), whatever the PERFORM dry / wet and ALL FX say |
| DlySnd | 0-127 | 0 | How much of the track goes to the delay (HOME, K4), likewise |

The effects and the master compressor have their own pages: see chapter 6 and the COMP page in chapter 5.

## 16. Credits

**FM1 Tonic is made by Luigi Rossi**, the developer and creative mind behind this firmware: the Tonic drum machine, its sequencer, screens, presets and the web tools (emulator, installer, backup). It stands on the work of the people and projects below, whose code, designs and ideas are in it. Thank you.

FM1 Tonic is free software under the GNU General Public License, version 3. If you share the firmware, you must also share its source code. LICENSING.md lists every file's origin and licence.

**The sound**
- **libpo32** by Eric Lewis (MIT licence): the synthesis engine behind the drum voices, tuned against Microtonic.
- **Microtonic** by Sonic Charge: the drum machine whose way of working FM1 Tonic follows. Microtonic .mtpreset files can be loaded. Nothing is sampled from it.

**The base**
- **X0X** by Charles Vestal: the firmware FM1 Tonic grew from, and the web pages (emulator, installer, backup), kept from X0X and changed for FM1 Tonic. X0X is a fork of Felucca.
- **Felucca** by Leo Kuroshita (@kurogedelic), Hügelton Instruments: the platform (hardware layer, USB, update loader, installer, storage). Its recovery board, FM-1-transporter, is mentioned in chapter 14.

**Code and ideas used**
- The reverb and delay are ported from **9W9** by athousanddetails, which is based on **ER-99** by Matthew Cieplak (GPL-3.0).
- The master compressor follows the design of Giannoulis, Massberg and Reiss; the master filter is Vadim Zavalishin's TPT state-variable filter.
- **Melodee** by keremimo and **Jangada** by zednaked: what they found about the FM-1's second core (how to start it, the mailbox between the cores).

**Fonts**
- **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov) and **DotGothic16** (The DotGothic16 Project Authors), all under the SIL Open Font License.

M-VAVE and FM-1 are trademarks of their owners. Microtonic is a trademark of Sonic Charge. FM1 Tonic is not connected with, or endorsed by, M-VAVE, Sonic Charge or Hügelton Instruments.
