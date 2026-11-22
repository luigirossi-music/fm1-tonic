# FM1 TONIC

Nach dem Vorbild von Microtonic VST von Sonic Charge ist FM1 TONIC ein Drum-Synthesizer mit 8 Stimmen, in dem jeder Klang von Grund auf synthetisiert wird, ganz ohne Samples. Er verbindet eine tiefe Percussion-Engine mit einem Sequencer von 1 bis 64 steps (mit accents, fills und 16 Patterns pro Projekt), einem integrierten Mixer mit level und pan, Reverb, Delay und einem Master-Kompressor. Mit 40 project slots (8 Werks-Presets + 32 eigene Slots) und einem Workflow für den Live-Einsatz geht er mühelos von knackigen, rhythmischen Beats zu sich entwickelnden Ambient-Texturen.

## Merkmale

**Sound**

- Drum-Synthesizer mit 8 Stimmen: Jeder Track hat seinen eigenen oscillator, pitch mod, noise, mix, EQ und seine eigene Verzerrung.
- Level und pan pro Track sowie reverb send und delay send pro Track (im Enhanced-Engine zusätzlich low cut und high cut).
- Enhanced- oder Classic-Engine (Classic = nur die grundlegenden Klangeinstellungen, ohne die Extras des FM1 TONIC).
- Track-Randomizer: ein neuer Zufallssound für den gewählten Track.
- Choke groups (zum Beispiel geschlossene und offene Hi-Hat).
- Sidechain-Kompressor: Jeder Track kann die anderen ducken.

**Sequencer**

- Sequencer mit 64 steps, 16 Patterns pro Projekt. Feste Länge von 8, 16, 32 oder 64 steps oder freie Länge von 1 bis 64.
- Song mode.
- Globaler accent und accent pro Track, fill mit retrig (2x, 4x, 8x) und trig % (chance) auf jedem step.
- Parameter lock (begrenzt: 3 knobs pro Track in jedem Pattern).
- Record automation (begrenzt: 3 knobs pro Track).
- Live-Aufnahme, quantized oder unquantized. Swing, quantization und Tempo.
- Keyboard (ARP): Spiele einen Track wie ein Keyboard und nimm die Noten in die steps auf (ein Piano Roll).
- Kopieren und Einfügen eines Patterns, eines Tracks und eines Patterns on the fly, während der Song läuft.
- Mute und solo.
- Undo und redo.

**Mixen und Performen**

- Master-Mixer: level und pan jedes Tracks (der rote Punkt markiert die Mitte).
- Master-Kompressor.
- Performance-Seite: 2 Master-Effekte (reverb und delay), ein low cut und ALL FX OFF.
- Memory-Seite: wie voll der Projektspeicher ist.

**Projekte und Speichern**

- 40 project slots: 8 Werks-Presets und 32 eigene.
- Autosave und quick save.
- Projektname: GENERATE oder MANUAL. Projekt umbenennen.
- Backup und Wiederherstellung über die Webseite. Export und Import von `.mtpreset`-Presets.
- Presets funktionieren in beide Richtungen: lade `.mtpreset`-Presets von Patternarium und Microtonic und exportiere deine zurück für das Microtonic VST (getestet; der Klang ist sehr ähnlich, nicht identisch).

**Display, Lichter und Einstellungen**

- 4/4-LED-Anzeige (weiße Tasten 1, 5, 9 und 13), Tastenlicht an oder aus.
- Zoom knobs (das große Rad, wenn ein knob gedreht wird), Farbthema.
- Ausgangslautstärke NORMAL oder LOUD. USB audio.

**MIDI**

- MIDI-Kanalwahl, notes out an oder aus, clock out an oder aus.
- Control changes (CC): 26 Sound-Parameter, pro gewähltem Track oder pro MIDI-Kanal.
- Remote-Panel: Spiele die eigenen Tasten des FM-1 von einem externen Keyboard oder einem Digitakt.

## 1. Das Panel

![Das Panel des FM1 TONIC: knobs MASTER, SELECT, PRESETS, ALGORITHM und KNOB 1 bis 4; Buttons FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- und OCT+; schwarze Tasten TRACK 1 bis 8, ACCENT, FILL, FUNC; weiße Tasten 1 bis 16](img/fm1-tonic-panel.svg "")

- **Knobs.** MASTER ist die Lautstärke. SELECT wechselt die Seite (in einer Liste: rauf und runter). PRESETS wählt eines der 40 Presets. KNOB 1 bis 4 stellen die vier Werte der Seite ein, die auf dem Display zu sehen ist (der gedrehte wird größer angezeigt). ALGORITHM wechselt zu einer zweiten Seite oder wählt eine Stimme, je nach Seite (die Seitentabellen sagen es). Ein kleines blaues Dreieck in einer unteren Ecke des Displays bedeutet, dass hinter ALGORITHM eine zweite Seite liegt: Dreieck rechts = ALGORITHM nach rechts drehen, Dreieck links = nach links drehen, um zurückzukehren.
- **Schwarze Tasten.** 1 bis 8 sind TRACK 1 bis 8, 9 ist ACCENT, 10 ist FILL, 11 ist FUNC.
- **Weiße Tasten 1 bis 16** sind die 16 steps des gewählten Tracks. Ein Pattern kann bis zu 64 steps haben, die jeweils 16 auf einmal angezeigt werden (siehe SEQ).
- **OCT- / OCT+** verschieben die Tonhöhe des gewählten Tracks eine Oktave nach unten oder oben (auf ARP: die Oktave des Keyboards). Bei einer Frage ist OCT+ = YES, OCT- = NO.

### Inhalt

1. Das Panel {#s1}
2. Signalweg {#s2}
3. Seiten, Button für Button {#s3}
4. Spielen {#s4}
5. Presets und Speichern {#s5}
6. Lichter, MIDI und USB {#s6}
7. Installieren, Backup, Wiederherstellen {#s7}
8. Die Engine {#s8}
  - Klangrezepte
9. Credits {#s9}

---

**So liest du die Seitentabellen.** Jeder Button hat eine Tabelle mit einer Zeile pro Seite. Normaler Text ist, was das Bedienelement allein tut. {r:Rot} ist, was es bei gehaltener **FUNC** tut, {o:orange} ist, was es bei gehaltener **FILL** tut (wie beim Digitakt).

**Was auf jeder Seite funktioniert** (die folgenden Tabellen wiederholen es nicht):

| Tasten | Was sie tun |
|---|---|
| Schwarze Taste 1 bis 8 | Wählt den Track und spielt ihn (auch während der Wiedergabe). |
| Weiße Taste 1 bis 16 | Die steps: einmal tippen = trig an, nochmal tippen = aus. Ein Pattern hat bis zu 64 steps, jeweils 16 angezeigt. |
| Weiße Taste gehalten + ACCENT / FILL | Dieser step bekommt einen accent / ein fill (nochmal = wieder entfernt). |
| {r:FUNC + schwarze Taste 1 bis 8} / {o:FILL + schwarze Taste 1 bis 8} | Track stumm (mute) / solo (nochmal = zurück). |
| {r:FUNC + weiße Taste} | Wechselt das Pattern (während der Wiedergabe: am Ende des Takts). |
| {r:FUNC + ACCENT} / {r:FUNC + FILL} / {r:FUNC + PLAY} | Pattern kopieren / einfügen (fragt PASTE PATTERN?) / löschen (fragt CLEAR PATTERN?). OCT+ = ja, OCT- = nein. |
| {r:FUNC + schwarze Taste 1 bis 8 gehalten, dann ACCENT / FILL / PLAY} | Den Sound dieses Tracks kopieren / einfügen / das Pattern dieses Tracks löschen (fragt nach). |
| {r:FUNC + SEQ} | Song mode an / aus (fragt nach). |
| {r:FUNC + ARP} | Löscht den Song (fragt nach). |
| {r:FUNC + SAVE} / {r:FUNC + HOME} | Quick save in den geladenen Platz / benennt das Projekt um. |
| {r:FUNC + REC} | QUANTIZED REC / UNQUANTIZED REC. |
| ENV + ein knob | Dieser knob springt auf seinen Standardwert zurück. |
| GLO + ACCENT / GLO + FILL | Undo / redo (auf jeder Seite). |
| REC + PLAY, dann Tasten und knobs | Live-Aufnahme: Die schwarzen Tasten 1 bis 8 schreiben trigs, die knobs schreiben Automation (3 knobs pro Track). |
| PLAY | Startet und stoppt das Pattern. |
| OCT- / OCT+ | Tonhöhe des Tracks eine Oktave tiefer / höher. Bei einer Frage: NO / YES. |

Ausnahmen: Auf PERFORM, MUTE / SOLO und der Pattern-Seite sind die schwarzen Tasten Mute-Buttons; auf ARP ist jede Taste eine Keyboard-Taste. Sie stehen alle in der nächsten Liste.

**Wo die Tasten etwas anderes tun.** Die Tasten des Panels (schwarze Tasten 1 bis 8 = Tracks, ACCENT, FILL, FUNC, weiße Tasten 1 bis 16 = steps) und die Kurzbefehle oben gelten auf allen Seiten dieses Handbuchs, außer auf den Seiten dieser Liste, wo die Tasten eine andere Aufgabe haben:

| Seite | Was die Tasten dort tun |
|---|---|
| **ARP** | Alle Tasten, weiße und schwarze, sind Keyboard-Tasten, die die gewählte Stimme spielen. |
| **PERFORM, MUTE / SOLO, PATTERN** (SEQ, 2. Druck) | Die schwarzen Tasten 1 bis 8 sind Mute-Buttons (im SOLO-Modus Solo-Buttons), sie wählen den Track nicht. Die weißen Tasten 1 bis 16 wählen das nächste Pattern. FUNC und FILL wechseln mit einem Tipp zwischen MUTE- und SOLO-Modus. |
| **DUCK** (SENDS, ALGORITHM nach rechts) | Die schwarzen Tasten 1 bis 8 wählen, welche Tracks geduckt werden. |
| **CHOKE** (EDIT, ALGORITHM nach rechts) | Die schwarzen Tasten 1 bis 8 wählen, welche Tracks in der choke group sind. |
| **SONG- und PATTERNS-Bildschirme** (Song mode) | Die weißen Tasten 1 bis 16 schreiben ein Pattern in den Takt. FILL löscht den Takt. |
| **Kopieren-Einfügen-Modus** (nach FUNC + ACCENT) | Die weißen Tasten 1 bis 16 sind die Plätze zum Einfügen. |
| **FUNC gehalten** | Die weißen Tasten 1 bis 16 sind die Patterns 1 bis 16; die Tasten leuchten dort, wo ein Pattern oder ein Track Noten hat. |

**Dinge, die es nur auf manchen Seiten gibt.**

| Seite | Tasten | Was es tut |
|---|---|---|
| Sound-Seiten: MIX 1, SEL, ENV, LFO, EDIT, VEL | Weiße Taste gehalten + knob 1 bis 4 | **P-lock**: Dieser step bekommt seinen eigenen Wert des knobs. Weiße Taste gehalten + SAVE + derselbe knob entfernt ihn. Ein Track speichert 3 p-lock-knobs pro Pattern. |
| Alle Seiten außer PERFORM, HOME, SONG, COMP und den Listen | Weiße Taste gehalten + SELECT | Die **chance** dieses steps: 5 bis 100 %, dann 1:2, 2:2, 1:3 ... 8:8. |
| ENV | OCT- / OCT+ | Die Frequenz des oscillators eine Oktave tiefer / höher. |
| EDIT | {r:FUNC + knob 2}, {r:FUNC, Tipp auf EDIT}, EDIT oder LFO gehalten + {r:FUNC + ACCENT / FILL / PLAY} | Tritt in die choke group ein / aus; DELETE AUTOMATION?; kopiert / fügt ein / setzt diese Track-Seite zurück. |
| SEQ | Knobs, SELECT, ALGORITHM, {o:FILL + SEQ} | Siehe die SEQ-Tabelle in Kapitel 3. |
| GLO > VEL | {r:FUNC + knob 4} | Ein neuer Zufallssound für den gewählten Track. |
| GLOBAL (HOME, 3. Druck) | Knob 4 (oder HOME gehalten + SELECT) | Das Tempo. |

## 2. Signalweg

![Die Engine des FM1 TONIC: Jeder der 8 Tracks hat einen Pitch-Modulator und einen oscillator, einen Noise-Filter und eine Noise-Hüllkurve, einen Mix, eine Verzerrung und einen EQ, danach cuts, level und pan; die 8 Tracks laufen durch den Sidechain, den Master-Low-Cut und den Kompressor zur Master-Lautstärke; Reverb und Delay sind Send-Effekte](img/fm1-engine.svg "Der ganze Signalweg. Das kleine Wort in jedem Kasten ist der Button, der diese Seite öffnet.")

Jeder Track erzeugt seinen Sound von Grund auf mit der Drum-Engine des FM1 TONIC: ein Pitch-Modulator und oscillator, ein Noise-Filter und eine Noise-Hüllkurve, ein Mix, eine Verzerrung und ein EQ, danach die cuts, das level und das pan des Tracks; die 8 Tracks laufen durch den Sidechain, den Master-Low-Cut und den Kompressor zu MASTER, mit Reverb und Delay als Send-Effekten. Kapitel 8 erklärt jede Einstellung.

**Enhanced und classic.** GLO > settings > Tonic Engine steht standardmäßig auf **enhanced**. In **classic** sind die Extras des FM1 TONIC (die cuts der Tracks, die sends, der Sidechain) grau und tun nichts, der Sound nutzt also nur die Einstellungen, die eine `.mtpreset`-Datei tragen kann. Ihre Werte bleiben erhalten und kommen mit enhanced zurück. Level und pan bleiben auch in classic verfügbar. Der Modus ist global und wird nicht in einem Preset gespeichert.

## 3. Seiten, Button für Button

Eine Seite mit **Track**-Werten (knobs 1 bis 4 der Sound-Seiten) gehört zum gewählten Track: Wechsle den Track mit einer schwarzen Taste 1 bis 8, und die Werte ändern sich. EDIT oder LFO gehalten + {r:FUNC + ACCENT / FILL / PLAY} kopiert, fügt ein und setzt **diese Seite** des Tracks zurück (nur Track-Seiten).

### FX

Tipp = MIX 1 → PERFORM → MIX 1. Halten (eine halbe Sekunde) = REVERB. Von der Seite eines anderen Buttons aus kehrt FX zur letzten FX-Seite zurück, die du verlassen hast.

| Seite | Knob 1 · 2 · 3 · 4 | Auch auf dieser Seite |
|---|---|---|
| **MIX 1** (1. Druck) | OSC- / NOISE-Mix · EQ FREQ (20 Hz-20 kHz) · DIST · EQ GAIN (64 = flat) | Weiße Taste gehalten + knob: p-lock. SAVE + knob: vergisst diesen Lock oder diese Bewegung. |
| **PERFORM** (2. Druck) | REVERB D/W · DELAY D/W (0-100 %, 0 beim Einschalten) · MASTER LOW CUT (20-663 Hz) · ALL FX (rechts an, links aus) | Die große Zahl ist das laufende Pattern. SELECT: die nächsten FX-Seiten (hält an den Enden an). ALGORITHM nach rechts: MUTE / SOLO. Schwarze Tasten 1 bis 8: mute. Weiße Tasten 1 bis 16: nächstes Pattern. {r:FUNC Tipp}: MUTE-Modus, oder alle unmuten. {o:FILL Tipp}: SOLO-Modus, oder alle Solos löschen. Keine p-locks. |
| **MUTE / SOLO** (PERFORM, ALGORITHM nach rechts) | wie PERFORM | Acht Kästchen statt der Zahl: blau = der Track spielt. Schwarze Tasten 1 bis 8 schalten stumm (MUTE-Modus) oder fügen ein Solo hinzu / entfernen es (SOLO-Modus, mehrere gleichzeitig). ALGORITHM nach links: zurück. |
| **REVERB** (FX halten) | DECAY · TONE | ALGORITHM nach rechts: REVERB 2. |
| **REVERB 2** | LOW CUT (von dem, was in den Reverb geht) · RETURN (level im Mix) | ALGORITHM nach links: zurück. |
| **DELAY** (HOME zusätzlich halten) | TIME (1/32 ... 1/2, mit T- und punktierten Werten) · FEEDBACK · TONE · TAPE (das erste Achtel = sauber digital, danach Tape mit mehr Verschleiß) | ALGORITHM nach rechts: DELAY 2. |
| **DELAY 2** | LOW CUT · RETURN · PING-PONG (off / on) | ALGORITHM nach links: zurück. |

ALL FX aus schaltet die beiden D/W-Sends von PERFORM und den Master-Low-Cut stumm; die eigenen sends der Tracks arbeiten weiter. Mutes und Solos teilen sich PERFORM und MUTE / SOLO; Solos werden nicht gespeichert und enden, wenn ein Preset geladen wird. Die Effektseiten sind global (alle Tracks teilen sie): SELECT läuft durch PERFORM, REVERB, DELAY, REVERB 2, DELAY 2.

### HOME

Tipp = MIX 2 → SENDS → GLOBAL → MIX 2. Halten = DELAY.

| Seite | Knob 1 · 2 · 3 · 4 | Auch auf dieser Seite |
|---|---|---|
| **MIX 2** | LEVEL · PAN · LOW CUT (20-663 Hz, nie aus) · HIGH CUT (20 kHz bis hinunter zu 1 kHz) | Die beiden cuts sind in classic grau. |
| **SENDS** | REVERB SEND · DELAY SEND · SIDECHAIN THRESHOLD (arbeitet wie der Threshold des Kompressors: ganz rechts = 100 % = nichts wird geduckt, nach links drehen für mehr Ducking) · SIDECHAIN INPUT (Track 1 bis 8, große Zahl) | ALGORITHM nach rechts: der DUCK-Bildschirm (unten). Die sends arbeiten, egal was D/W von PERFORM und ALL FX sagen. |
| **DUCK** (SENDS, ALGORITHM nach rechts) | - | Schwarze Tasten 1 bis 8: blau = geduckt, grau = nicht, lila = der Input. Der Input kann nicht geduckt werden. Nichts wird geduckt, bevor du wählst. ALGORITHM nach links oder HOME: zurück. |
| **GLOBAL** | QUANTIZATION (1/8 ... 1/32) · SWING · FILL RATE (2x, 4x, 8x) · TEMPO | - |

Das Ducking folgt dem Sound des Input-Tracks (eine lange Kick duckt lange; ein stummgeschalteter Input duckt nicht) und duckt auch die sends der Tracks. Ein roter Punkt zeigt einen geduckten Track, ein lila Punkt den Input; der Punkt folgt dem Input-Track bei jedem Schlag, egal wie der Threshold steht (mit dem knob ganz rechts wird nichts geduckt und der Punkt bleibt dunkel). Ein Sidechain für alle Tracks, mit dem Preset gespeichert.

### SEL, ENV, LFO, EDIT (die Stimmen-Seiten)

Alle vier sind Track-Seiten: p-locks und chance funktionieren auf ihnen.

| Button | Knob 1 · 2 · 3 · 4 | Auch auf dieser Seite |
|---|---|---|
| **SEL** PITCH MOD | SHAPE (DECAY, SINE, RANDOM) · AMOUNT (64 = none) · RATE · - | |
| **ENV** OSCILLATOR | WAVE (SINE, TRI, SAW) · FREQUENCY (20 Hz-20 kHz) · ATTACK · DECAY | OCT- / OCT+ verschieben die Frequenz um eine Oktave. |
| **LFO** NOISE FILTER | MODE (LP, BP, HP) · Q · FREQUENCY · - | |
| **EDIT** NOISE ENVELOPE | MODE (EXP, LIN, CLAP) · CHOKE button · ATTACK · DECAY | {r:FUNC + knob 2 nach rechts}: der Track tritt in die choke group ein; nach links: verlässt sie. Knob 2 allein zeigt nur eine Meldung. ALGORITHM nach rechts: der CHOKE-Bildschirm. {r:FUNC, Tipp auf EDIT}: DELETE AUTOMATION? (OCT+ ja, OCT- nein). |
| **CHOKE** (EDIT, ALGORITHM nach rechts) | - | Schwarze Tasten 1 bis 8: blau = in der Gruppe. Ein Hit eines Tracks der Gruppe stoppt die anderen sofort, so wie eine geschlossene Hi-Hat eine offene stoppt. ALGORITHM nach links: zurück. Mit dem Preset gespeichert. |

DELETE AUTOMATION entfernt die p-locks und die aufgenommene Bewegung des gewählten Tracks im laufenden Pattern (andere Tracks, andere Patterns und chances bleiben; Undo funktioniert). FUNC + EDIT gehalten und dann ACCENT, FILL oder PLAY zu drücken ist das Kopieren der Seite, kein Löschen.

### GLO

Tipp = VEL → MIXER → COMP → MEMORY → SETTINGS → VEL.

| Seite | Knob 1 · 2 · 3 · 4 | Auch auf dieser Seite |
|---|---|---|
| **VEL** | OSC · NOISE · PITCH MOD (wie stark die Velocity jedes davon bewegt) · RANDOM | {r:FUNC + knob 4}: der gewählte Track bekommt einen neuen zufälligen Sound (nur die 21 Sound-Einstellungen). Knob 4 allein zeigt nur einen Hinweis. Track-Seite: p-locks funktionieren. |
| **MIXER** | LEVEL der vier angezeigten Tracks | Acht hohe Slider; SELECT wechselt zwischen den Tracks 1 bis 4 / 5 bis 8 und schaltet, wie der blaue Hinweis auf dem Bildschirm sagt, zwischen level / pan um (pan Mitte = rot). Schwarze Tasten 1 bis 8 wählen einen Track und seine Gruppe. Ein Hit lässt seinen Slider aufleuchten. {r:FUNC + schwarze Taste 1 bis 8}: mute. Ein roter Punkt unter einem Slider = geduckt, lila = Sidechain-Input. Chance funktioniert hier. |
| **COMP** | ATTACK · RELEASE · THRESHOLD · MAKE-UP | Master-Kompressor, Ratio 4:1, eine Nadel zeigt die Pegelabsenkung. Standard: nichts komprimiert (0.0 dB / 0.0 dB). Wird nur von SAVE behalten. |
| **MEMORY** | - | Eine Batterie zeigt, wie voll der Projektspeicher ist (Patterns, Song, Motion, ARP-Noten). Lila ab 90 %, FULL bei 100 %: Ein Speichern wird dann abgelehnt. |
| **SETTINGS** | Liste (SELECT bewegt) | Zeilen: clock out, notes out, Tonic MIDI channel, output level, USB audio, key light, 4/4 LED, colour theme, zoom knob (yes / no), Tonic Engine, name projects, save, autosave, clear pattern, factory reset, performance, cores, about, battery. Die nicht offensichtlichen werden gleich unten erklärt. Änderungen werden etwa anderthalb Sekunden, nachdem du aufhörst, etwas zu berühren, von selbst behalten (der Sequencer muss gestoppt sein). |

#### GLO settings: die nicht offensichtlichen

Die Einstellungen stehen in der GLO-Liste (SELECT bewegt rauf und runter, ALGORITHM ändert den Wert). Die Standardwerte sind fett.

| Einstellung | Auswahl | Was sie tut |
|---|---|---|
| **OUTPUT** | NORMAL / **LOUD** | LOUD (die Werkseinstellung) macht die Ausgänge des FM-1 um 6 dB heißer (Vollaussteuerung des Codecs). NORMAL behält 6 dB Headroom: Nutze es, wenn der Sound zu laut ist, und achte auf den Kompressor und den Limiter. |
| **USB AUDIO** | **FOLLOWS KNOB** / FULL | FOLLOWS KNOB: Der Computer empfängt, was die Kopfhörer hören, nach dem MASTER-knob. FULL: Der Computer empfängt immer den vollen Pegel, und MASTER ändert nur die eigenen Ausgänge des FM-1 (Lautsprecher, Kopfhörer, Line out). Nutze FULL, um mit vollem Pegel aufzunehmen und trotzdem leise abzuhören. |
| **4/4 LED** | OFF / **ON** | ON (die Werkseinstellung): Die weißen Tasten 1, 5, 9 und 13 leuchten schwach, um die vier Schläge des Takts zu markieren (nicht auf PERFORM). |
| **ZOOM KNOB** | **YES** / NO | YES: Der knob, den du drehst, wird über dem Bildschirm vergrößert. NO: Er wird nur hervorgehoben, sodass die ganze Seite sichtbar bleibt. Der SONG-Bildschirm zoomt nie. |
| **NAME PROJECTS** | **GENERATE** / MANUAL | GENERATE erfindet den Namen eines gespeicherten Projekts (ALGORITHM = ein anderer). MANUAL: Der Namensbildschirm ist ein Buchstaben-Editor (siehe Kapitel 5). |
| **TONIC ENGINE** | **ENHANCED** / CLASSIC | CLASSIC macht die Extras des FM1 TONIC (Track-cuts, sends, Sidechain) grau und stumm, sodass der Sound nur die Einstellungen nutzt, die eine `.mtpreset`-Datei tragen kann (Kapitel 2 und 8). |
| **2ND CORE** | **ON** / OFF | Der Chip des FM-1 hat zwei Kerne, und Tonic teilt das Sound-Rendering zwischen ihnen auf. Es gilt ab dem nächsten Einschalten. Wenn der FM-1 mit zwei Kernen jemals abstürzt, schaltet er sich selbst auf OFF; schalte es hier wieder auf ON. |

### SEQ und ARP

| Seite | Knob 1 · 2 · 3 · 4 | Auch auf dieser Seite |
|---|---|---|
| **SEQ** (1. Druck) | FIXED LEN (8, 16, 32, 64) · AUTOSCROLL · FREE LEN (1 bis 64) · SWING | Weiße Tasten: die steps. SELECT: die step-Seiten (je 16; AUTOSCROLL aus): die Seite (1-16, 17-32 ...) erscheint kurz groß auf dem dritten knob, danach ist der knob wieder FREE LEN. Die Seite bleibt nie auf steps stehen, die es nicht gibt: wird der Loop kürzer als die Seite, auf der du warst, springt sie auf die letzte Seite zurück, die die Länge hat. ALGORITHM: die Stimme (1 bis 8, dann AC = der globale accent). Einen step halten + ACCENT / FILL: Flags (accent = dunkelblau, fill = gelb, beide = grün). {r:FUNC + SEQ}: Song mode an / aus (mit OCT+ bestätigen). {o:FILL + SEQ}: der SONG-Bildschirm. |
| **PATTERN** (SEQ, 2. Druck) | SONG ON / OFF | Schwarze Tasten 1 bis 8 schalten stumm, weiße Tasten 1 bis 16 wählen das nächste Pattern. Eine dunkle Taste = leeres Pattern, schwaches Licht = hat Noten, langsames Blinken = das aktuelle, schnelles Blinken = das nächste (vorgemerkt). {r:FUNC Tipp} und {o:FILL Tipp} wie auf PERFORM. Nochmal SEQ: zurück. |
| **ARP** | FIXED LEN · OCTAVE (auch OCT- / OCT+, -3 bis +3) · PAGE (1-16, 17-32 ...) · - | Der SEQ-Bildschirm, aber jede Taste (weiß und schwarz) spielt die gewählte Stimme wie ein Keyboard. Die C-Tasten (gedimmt) spielen die eigene Tonhöhe der Stimme in Oktave 0. ALGORITHM: die Stimme. SELECT: die step-Seiten, angezeigt auf dem dritten knob. Es wird nichts aufgenommen. SEQ: zurück. {r:FUNC + ARP}: den Song löschen (DELETE SONG?). |

Ein Pattern zu verlängern kopiert, was da ist, in den neuen Teil (8 auf 16 kopiert die steps 1 bis 8 nach 9 bis 16); es zu verkürzen kürzt es nur.

### PLAY, REC, SAVE

| Button | Was er tut |
|---|---|
| **PLAY** | Play und Stop. {r:FUNC + PLAY}: das Pattern löschen (es fragt nach). |
| **REC** | REC, dann PLAY: Live-Aufnahme. Jede schwarze Taste 1 bis 8 schreibt einen trig ihres Tracks auf den nächsten step; jeder Sound-knob, den du drehst, wird als Automation aufgenommen. Nicht auf PERFORM und der Pattern-Seite. {r:FUNC + REC}: QUANTIZED REC (Standard) / UNQUANTIZED REC (der Hit behält sein genaues Timing, innerhalb des steps); während der Wiedergabe schaltet es auch die Aufnahme ein. |
| **SAVE** | Siehe Kapitel 5. {r:FUNC + SAVE}: Quick save. {r:FUNC + HOME}: umbenennen. |

### Button-Kombinationen

Jede Kombination aus Buttons und Tasten, gruppiert nach dem Button, den du zuerst hältst. Die schwarze Taste 9 ist ACCENT, 10 ist FILL, 11 ist FUNC. {r:Rot} = FUNC gehalten, {o:orange} = FILL gehalten. Die letzte Spalte sagt, wo es funktioniert. **„Jede Seite“ heißt jede Seite außer ARP und den beiden PERFORM-Seiten**: Auf ARP sind FUNC, ACCENT und FILL Keyboard-Tasten, und auf PERFORM sind FUNC und FILL die Tasten für den MUTE- und SOLO-Modus, ihre Kombinationen funktionieren dort also nicht. Die einzige Ausnahme ist FUNC + PLAY (das Pattern löschen), das auch auf ARP mit gehaltener schwarzer Taste 11 funktioniert. GLO + ACCENT / FILL und die Kombinationen, die nur aus Buttons und knobs bestehen (HOME + SELECT, ENV + ein knob ...), funktionieren.

**FUNC gehalten (schwarze Taste 11)**

| Tasten | Was es tut | Wo |
|---|---|---|
| {r:FUNC + schwarze Taste 1 bis 8} | Schaltet den Track stumm (nochmal = unmute). Es wirkt, wenn du die Taste loslässt. | Jede Seite |
| {r:FUNC + weiße Taste 1 bis 16} | Wechselt zu diesem Pattern (während der Wiedergabe: am Ende des Takts). | Jede Seite; nicht auf dem SONG-Bildschirm oder im Song mode (der Song entscheidet) |
| {r:FUNC + ACCENT} | Kopiert das Pattern. Der Bildschirm sagt PRESS PATTERN TO PASTE / PRESS SEQ TO EXIT, jede Pattern-Taste blinkt (ein leeres Pattern schwächer) und SEQ blinkt: Drücke eine Pattern-Taste, und die Kopie landet dort, dieses Pattern wird zum aktuellen. Tippe vor der Wahl FUNC allein, um für viele Einfügungen im Modus zu bleiben. SEQ verlässt den Modus und öffnet die Sequencer-Seite; {r:FUNC + ACCENT} nochmal oder jeder andere Button verlässt ihn ebenfalls. | Jede Seite |
| {r:FUNC + FILL} | Fügt das kopierte Pattern ein, wenn du FILL loslässt. Es fragt PASTE PATTERN? (OCT+ = ja, OCT- = nein). | Jede Seite |
| {r:FUNC + FILL gehalten + weiße Taste} | Fügt das kopierte Pattern sofort in diesen Platz ein (ohne Frage). | Jede Seite |
| {r:FUNC + PLAY} | Löscht das Pattern (es fragt CLEAR PATTERN?). Es startet nicht die Musik. | Jede Seite, auch ARP (dort schwarze Taste 11 halten, eine Klaviertaste auf ARP: PLAY startet / stoppt nicht, solange sie gehalten wird) |
| {r:FUNC + schwarze Taste 1 bis 8 gehalten, dann ACCENT} | Kopiert den Sound dieses Tracks (nicht seine steps). | Jede Seite |
| {r:FUNC + schwarze Taste 1 bis 8 gehalten, dann FILL} | Fügt den kopierten Sound in diesen Track ein (mute, Lautstärke und pan auch; der Track wird dadurch nicht stummgeschaltet). Keine Frage. | Jede Seite |
| {r:FUNC + schwarze Taste 1 bis 8 gehalten, dann PLAY} | Löscht das Pattern dieses Tracks (es fragt CLEAR TRACK PATTERN?). | Jede Seite |
| {r:FUNC + EDIT oder LFO gehalten, dann ACCENT} | Kopiert diese Seite des gewählten Tracks (nur ihre Parameter). | Seiten mit Track-Parametern (nicht GLO, HOME, SEQ) |
| {r:FUNC + EDIT oder LFO gehalten, dann FILL} | Fügt sie in den gewählten Track ein, auf derselben Seite. | Seiten mit Track-Parametern (nicht GLO, HOME, SEQ) |
| {r:FUNC + EDIT oder LFO gehalten, dann PLAY} | Setzt diese Seite des Tracks auf ihre Werte beim Einschalten zurück. Keine Frage; Undo nimmt es zurück. | Seiten mit Track-Parametern (nicht GLO, HOME, SEQ) |
| {r:FUNC + EDIT}, einmal getippt | DELETE AUTOMATION?: entfernt die p-locks und die aufgenommene Bewegung des gewählten Tracks im laufenden Pattern (es fragt nach). | Jede Seite |
| {r:FUNC + SEQ} | Song mode an / aus (es fragt: OCT+ = ja). | Jede Seite |
| {r:FUNC + ARP} | DELETE SONG?: löscht den ganzen Song und verlässt den Song mode (es fragt nach). | Jede Seite |
| {r:FUNC + HOME} | Benennt das Projekt um und speichert es am selben Platz. | Jede Seite |
| {r:FUNC + SAVE} | Quick save in den geladenen Platz. Keine Frage. | Jede Seite |
| {r:FUNC + REC} | Schaltet zwischen QUANTIZED REC / UNQUANTIZED REC um. Während der Wiedergabe schaltet es auch die Aufnahme ein. | Jede Seite |
| {r:FUNC + OCT- / OCT+} | Nichts. | |
| {r:FUNC + knob 2 (nach rechts / links drehen)} | Der gewählte Track tritt in die choke group ein / verlässt sie. | Nur EDIT, Seite NOISE ENVELOPE |
| {r:FUNC + knob 4} | Ein neuer zufälliger Sound für den gewählten Track. | Nur GLO > VEL |

**FILL gehalten (schwarze Taste 10)**

| Tasten | Was es tut | Wo |
|---|---|---|
| {o:FILL + schwarze Taste 1 bis 8} | Schaltet diesen Track solo (eine andere Taste verschiebt das Solo, dieselbe Taste beendet es). | Jede Seite |
| {o:FILL + SEQ} | Zurück zum SONG-Bildschirm von jeder Seite aus (im Song mode; sonst NOT IN SONG MODE). | Jede Seite |
| {o:FILL}, gedrückt und allein losgelassen | Löscht den Takt unter dem Cursor (keine Frage). | Nur der SONG-Bildschirm |
| {o:FILL} + eine weiße Taste gehalten (oder erst FILL, dann weiße Tasten) | Dieser step bekommt einen fill (der Track spielt darüber 2x, 4x oder 8x); nochmal = aus. | Seiten, auf denen weiße Tasten steps sind |
| {o:FILL} gehalten | Die trigs des gewählten Tracks, die einen fill tragen, blinken auf den weißen Tasten (die anderen trigs bleiben an). | Seiten, auf denen weiße Tasten steps sind |
| {r:FUNC + FILL} | Einfügen. Siehe die FUNC-Liste oben. | |

**ACCENT gehalten (schwarze Taste 9)**

| Tasten | Was es tut | Wo |
|---|---|---|
| Weiße Taste gehalten + ACCENT (oder erst ACCENT, dann weiße Tasten) | Dieser step bekommt einen accent (laut); nochmal = aus. | Seiten, auf denen weiße Tasten steps sind |
| ACCENT gehalten | Die trigs des gewählten Tracks, die einen accent tragen, blinken auf den weißen Tasten (die anderen trigs bleiben an). | Seiten, auf denen weiße Tasten steps sind |
| {r:FUNC + ACCENT} | Kopieren. Siehe die FUNC-Liste oben. | |

**GLO gehalten**

| Tasten | Was es tut | Wo |
|---|---|---|
| GLO + ACCENT | Undo. | Jede Seite, auch ARP |
| GLO + FILL | Redo. | Jede Seite, auch ARP |

**HOME gehalten**

| Tasten | Was es tut | Wo |
|---|---|---|
| HOME + SELECT | Das Tempo. | Jede Seite |
| HOME allein gehalten (eine halbe Sekunde) | Die DELAY-Seite. | Jede Seite |
| HOME + weiße Taste | Nichts. | |

**FX gehalten**

| Tasten | Was es tut | Wo |
|---|---|---|
| FX allein gehalten (eine halbe Sekunde) | Die REVERB-Seite. | Jede Seite |

**ENV gehalten**

| Tasten | Was es tut | Wo |
|---|---|---|
| ENV + ein knob | Dieser knob geht auf seinen Wert beim Einschalten zurück. | Seiten mit knobs |
| ENV + weiße Taste | Der accent dieses steps an / aus. | Seiten, auf denen weiße Tasten steps sind |
| ENV + schwarze Taste 1 bis 8 | Spielt diesen Track laut. Bei der Aufnahme: Der step bekommt einen accent. | Jede Seite |

**SAVE gehalten**

| Tasten | Was es tut | Wo |
|---|---|---|
| SAVE + ein knob | Die aufgenommene Bewegung dieses knobs wird vergessen. | Seiten mit knobs |
| Weiße Taste gehalten + SAVE + derselbe knob | Entfernt den p-lock dieses steps für den knob. | Sound-Seiten |

**Eine weiße Taste gehalten (ein step)**

| Tasten | Was es tut | Wo |
|---|---|---|
| Weiße Taste gehalten + knob 1 bis 4 | **P-lock**: Dieser step bekommt seinen eigenen Wert des knobs (3 gelockte knobs pro Track und Pattern). | Sound-Seiten: MIX 1, SEL, ENV, LFO, EDIT, VEL |
| Weiße Taste gehalten + SELECT | Die chance dieses steps: 5 bis 100 %, dann 1:2, 2:2, 1:3 ... 8:8. | Jede Seite außer PERFORM, HOME, SONG, COMP und den Listen |
| Weiße Taste gehalten + ACCENT / FILL | Accent- / fill-Flag. | Siehe ACCENT und FILL oben |

**Transport und anderes**

| Tasten | Was es tut | Wo |
|---|---|---|
| REC + PLAY, dann Tasten und knobs | Live-Aufnahme: Die schwarzen Tasten 1 bis 8 schreiben trigs, die knobs schreiben Automation. | Nicht auf PERFORM und der Pattern-Seite |
| OCT- / OCT+ | Die Tonhöhe des gewählten Tracks eine Oktave tiefer / höher. Bei einer Frage: NO / YES. Auf ARP: die Oktave des Keyboards. | Jede Seite |
| Ein Button allein gehalten (etwa eine Sekunde) | Eine Karte sagt, was der Button auf dieser Seite tut, einschließlich seiner Kombinationen. | Jeder Button außer FX und HOME |

## 4. Spielen

- **Steps.** Wähle einen Track, drücke eine weiße Taste, um einen step zu setzen, drücke sie nochmal, um ihn zu entfernen. **Accent** (laut) und **fill** (der Track spielt vier Mal über diesen step): Halte die weiße Taste und drücke ACCENT und / oder FILL (nochmal = Flag aus), oder halte zuerst ACCENT / FILL und drücke weiße Tasten.
- **Patterns.** {r:FUNC + weiße Taste} wechselt das Pattern. {r:FUNC + ACCENT} kopiert, {r:FUNC + FILL} fügt ein (halte FILL und drücke eine weiße Taste, um in diesen Platz einzufügen), {r:FUNC + PLAY} löscht. Das Einfügen des Patterns fragt **PASTE PATTERN?** (OCT+ = ja, OCT- = nein), und das Löschen fragt CLEAR PATTERN?. Mit zusätzlich gehaltener schwarzer Taste 1 bis 8 kopieren, fügen sie ein und löschen nur diesen Track; das Einfügen des Sounds eines Tracks oder einer Seite, das Einfügen mit FILL und einer weißen Taste und der Kopieren-Einfügen-Modus fügen sofort ein, ohne Frage.
- **Kopieren-Einfügen im Fluss.** Nach {r:FUNC + ACCENT} sagt der ganze Bildschirm in großen blauen Buchstaben **PRESS PATTERN TO PASTE** und **PRESS SEQ TO EXIT**. Jede Pattern-Taste blinkt (ein Pattern mit Noten blinkt hell, ein LEERES Pattern blinkt schwächer, das kopierte bleibt an) und der Button **SEQ** blinkt ebenfalls. Drücke eine Pattern-Taste, und die Kopie landet dort, dieses Pattern wird zum aktuellen, und der Modus endet. Um an vielen Stellen einzufügen, **tippe vor der Wahl FUNC allein**: Der Modus bleibt, und das aktuelle Pattern ändert sich nicht. Um den Modus zu verlassen, drücke **SEQ**: Es öffnet die **Sequencer-Seite**, nicht die Pattern-Seite (der nächste SEQ-Druck ist wieder normal). Jeder andere Button verlässt ihn ebenfalls, ebenso nochmal {r:FUNC + ACCENT} (der Bildschirm sagt COPY MODE OFF). {r:FUNC + ACCENT} halten und eine weiße Taste drücken = dieses Pattern ist die Kopie. Nicht verfügbar auf PERFORM und der Pattern-Seite, wo FUNC der MUTE-Modus ist.
- **Mute und Solo.** {r:FUNC + schwarze Taste 1 bis 8} schaltet stumm (nochmal = unmute). {o:FILL + schwarze Taste 1 bis 8} schaltet diesen Track solo (eine andere Taste verschiebt das Solo, dieselbe Taste beendet es). Solos werden nicht gespeichert und enden, wenn ein Preset geladen wird. Nicht auf ARP.
- **P-locks und chance.** Halte eine weiße Taste und drehe auf einer Stimmen-Seite einen knob: Dieser step bekommt seinen eigenen Wert, jeder andere step behält den Sound. Während der Wiedergabe hörst du es, wenn der step das nächste Mal drankommt. Um einen zu entfernen: Halte den step, halte SAVE, drehe denselben knob. Ein Track hält 3 gelockte knobs pro Pattern (der Bildschirm sagt MAX 3 PER TRACK, MOTION FULL, wenn kein Platz mehr ist). Halte einen step und drehe SELECT: seine chance, von 100 % in 5-%-Schritten abwärts, dann 1:2, 2:2, 1:3 ... 8:8. Während du drehst, füllt die chance den ganzen Bildschirm (TRIG % und der step), groß, sodass du sie sofort ablesen kannst.
- **Chains.** Ein Pattern kann so markiert werden, dass es am Ende ins nächste läuft (nach Pattern 8 kommt 1). Ein Flag zählt nur bei einem Pattern mit Noten. Ein Preset mit Chains wird spielend durch sie hindurch geladen (SONG ON); Knob 1 der Pattern-Seite von SEQ schaltet es aus, und das Pattern läuft in der Schleife.
- **Song mode.** {r:FUNC + SEQ}, mit OCT+ bestätigen (OCT- = zurück). Wenn es noch keinen Song gibt, öffnet sich der PATTERNS-Bildschirm: Weiße Tasten 1 bis 16 fügen Patterns hinzu, OCT- (oder FUNC + eine weiße Taste) nimmt das letzte weg, OCT+ ist OK. PLAY spielt einen Takt pro Pattern, in der Schleife. Dieselbe Geste verlässt den Song mode. Auf dem **SONG-Bildschirm** (eine Zeile pro Takt) ist knob 1 = der Takt, knob 2 = sein Pattern, knob 3 = Wiederholungen (x1 bis x8); weiße Tasten schreiben ein Pattern in den Takt unter dem Cursor und rücken weiter; FILL, gedrückt und losgelassen, löscht den Takt sofort (der zuletzt gelöschte Takt = der Song wird gelöscht und der Song mode endet). {r:FUNC + ARP} löscht den ganzen Song.

## 5. Presets und Speichern

**Presets funktionieren in beide Richtungen.** Der FM-1 lädt `.mtpreset`-Presets von **Patternarium** und von **Microtonic**, und die Presets, die du auf dem FM1 TONIC machst, lassen sich zurück nach `.mtpreset` exportieren und im **Microtonic VST** verwenden. Das wurde getestet: Die exportierten Presets öffnen sich und spielen in Microtonic. Der Klang ist sehr, sehr nah am Original, nicht identisch. Import und Export laufen über die Webseite (die Backup-Seite: „Restore my presets“ und „Export my presets as .mtpreset“). Die Slots 1 bis 8 lassen sich nicht exportieren. Was der FM1 TONIC hinzugefügt hat (ARP-Noten, späte Treffer, Track-cuts und -sends, Kompressor, reverb, delay, mute), hat in einer `.mtpreset` keinen Platz und bleibt draußen.

Drehe **PRESETS** durch die 40 Plätze: 01 bis 08 sind Werks-Presets, 09 bis 40 gehören dir (ein leerer Platz lädt mit den Sounds eines Werks-Presets und ohne Pattern: 09 klingt wie 01, 10 wie 02, und so weiter im Kreis). Eine Frage kommt zuerst: OCT+ lädt es, OCT- behält deinen Sound. Das Laden wird während der Wiedergabe abgelehnt.

- **SAVE** fragt SAVE HERE? mit dem geladenen Platz. OCT+ speichert dort und behält den Namen. ALGORITHM wählt einen anderen Platz (1 bis 40; Bernstein zeigt, wer ersetzt würde). OCT- geht zu CHOOSE SLOT, dann zum Slot-Bildschirm, dann zum Namensbildschirm (ALGORITHM = ein anderer erfundener Name; OCT+ speichert). HOME verlässt ohne Speichern. Wenn seit dem Einschalten nichts geladen wurde, beginnt SAVE bei CHOOSE SLOT.
- {r:FUNC + SAVE} speichert per Quick save in den geladenen Platz, ohne nach einem Platz zu fragen, auch während der Wiedergabe. Ein leerer Platz speichert sofort als UNTITLED 01, 02 ...; ein voller fragt OVERWRITE SLOT? (OCT+ ja).
- {r:FUNC + HOME} benennt das geladene Projekt um (ALGORITHM = ein anderer Name, OCT+ speichert, OCT- oder HOME bricht ab). Wenn NAME PROJECTS auf MANUAL steht (GLO settings), wird der Namensbildschirm zu einem Buchstaben-Editor: Knob 4 schaltet durch die Zeichen, OCT+ (der Bildschirm sagt OK) übernimmt den Buchstaben, OCT- löscht den letzten, dann drücke SAVE (höchstens 8 Buchstaben; der Bildschirm zeigt PRESS SAVE und TURN KNOB 4 in großen Buchstaben).
- **Was gespeichert wird.** Bei AUTOSAVE ON wird dein Arbeitsprojekt 4 Sekunden, nachdem du aufhörst, etwas zu berühren, gespeichert, außer dem Kompressor, dem Reverb und dem Delay: Diese werden nur von SAVE behalten, im Preset. PERFORM startet beim Einschalten immer sauber (D/W 0 %, Master-Low-Cut 20 Hz, ALL FX an). AUTOSAVE ist ab Werk OFF: Drücke SAVE (oder FUNC + SAVE), um deine Arbeit zu behalten. Die Einstellungen werden trotzdem von selbst behalten. Schalte AUTOSAVE in GLO > SETTINGS auf ON, wenn das Projekt von selbst behalten werden soll. Werkseinstellungen: clock out und notes out aus, Tonic MIDI channel 10, remote MIDI channel aus, CC mode aus (CC CH BASE 1-8), output LOUD, USB audio folgt dem knob, 4/4 LED an, theme TONIC, zoom knob yes, name projects GENERATE, Tonic Engine ENHANCED, 2nd core ON. Ein neues Gerät oder ein FACTORY RESET bekommt sie; ein Gerät, das schon ein gespeichertes Projekt hält, behält seine eigenen Einstellungen.

## 6. Lichter, MIDI und USB

- **Lichter.** PLAY leuchtet bei jedem 4. step (rot während der Aufnahme). REC: aus, wenn nicht aufgenommen wird; dauerhaft an im Stillstand (quantized) und im Takt blinkend während der Wiedergabe; zwei schnelle und ein langsames Blinken bei unquantized. Der Button der aktuellen Seite leuchtet. Weiße Tasten zeigen die steps des gewählten Tracks (der laufende step kippt um); 1, 5, 9 und 13 können schwach leuchten (GLO > 4/4 LED). Schwarze Tasten 1 bis 8: der gewählte Track hell (blinkend, wenn stumm), Tracks mit Noten gedimmt, und während der Wiedergabe blinkt die Taste eines Tracks bei jedem seiner Hits (nicht auf ARP). Mit gehaltener FUNC zeigen die Tasten, welche Patterns und Tracks Noten haben.
- **MIDI.** Über USB oder den TRS MIDI IN spielt Kanal 10 standardmäßig die Stimmen mit General-MIDI-Drum-Noten in der Reihenfolge der Stimmen: 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51 (Velocity = Lautstärke). Eine ankommende Clock wird befolgt; Start, Stop und Continue steuern das Pattern. GLO settings: TONIC MIDI CH (oder OFF), REMOTE MIDI CH (oder OFF, siehe unten), CC MODE und CC CH BASE (Control Changes, siehe unten), CLOCK OUT (Clock und Start / Stop), NOTES OUT (das Pattern als Noten).
- **Remote-Panel.** GLO > REMOTE MIDI CH (standardmäßig OFF) lässt einen zweiten MIDI-Kanal die eigenen Tasten des FM-1 spielen. Die 27 Tasten des Panels sind die 27 Noten von F3 bis G5 (MIDI-Noten 53 bis 79, mittleres C = 60), angeordnet wie eine Klaviertastatur: Die erste Taste, F3, ist Note 53, die letzte, G5, ist Note 79. Ein Note on drückt die Taste, ein Note off lässt sie los, eine lange Note hält die Taste also gedrückt. Alles, was eine Taste auf dem Panel tut, funktioniert genauso: Schwarze Tasten 1 bis 8 wählen oder muten die Tracks, weiße Tasten 1 bis 16 sind die steps oder die Patterns, und die schwarzen Tasten 9, 10 und 11 sind ACCENT (Note 73), FILL (Note 75) und FUNC (Note 78). Ein Akkord ist in jeder Notenreihenfolge in Ordnung: ACCENT, FILL und FUNC werden immer zuerst angewendet, FUNC + ACCENT + eine weiße Taste funktioniert also, wenn die drei Noten zusammen ankommen. All Notes Off (Controller 123) lässt jede Taste los. Noten auf dem Remote-Kanal spielen nicht die Stimmen; gib ihm einen anderen Kanal als TONIC MIDI CH. Noten außerhalb von 53 bis 79 werden ignoriert.

| Taste | Noten |
| --- | --- |
| Weiße Tasten 1 bis 16 | 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 |
| Schwarze Tasten 1 bis 8 (Tracks) | 54, 56, 58, 61, 63, 66, 68, 70 |
| Schwarze Taste 9 = ACCENT, 10 = FILL, 11 = FUNC | 73, 75, 78 |

### Control Changes (CC)

Ein Control Change dreht einen knob von außen: Die CC-Nummer ist der **Parameter** (dieselben Nummern für jeden Track) und der **Track** wird auf zwei Arten gewählt. GLO > **CC MODE** entscheidet (standardmäßig OFF: Nichts reagiert auf CC, bis du es einschaltest):

| CC MODE | Was es tut |
| --- | --- |
| OFF | Jeder Control Change wird ignoriert. |
| SELECTED | Ein CC auf dem Kanal des Tonic (TONIC MIDI CH) bewegt den Parameter des **gewählten** Tracks. |
| CHANNELS | Ein Block aus 8 Kanälen, einer pro Track: Der erste Kanal von GLO > **CC CH BASE** ist Track 1, der nächste Track 2, und so weiter (Base 1 = Kanäle 1-8). Jeder Track kann jederzeit bewegt werden, ohne ihn vorher wählen zu müssen. |
| BOTH | Beide zusammen. Liegt der Kanal des Tonic im Block, gewinnt auf diesem Kanal der Block. |

Noten spielen weiterhin die Stimmen auf TONIC MIDI CH, und das Remote-Panel behält seinen eigenen Kanal, gib dem Block also Kanäle, die diese beiden nicht nutzen (der Standardblock 1-8 lässt 10 und 11 frei). Ein CC bewegt den Wert wie der knob, sodass Bildschirm, SAVE und Autosave folgen; er öffnet nicht das große Rad, er lockt keinen step und wird nicht aufgenommen. Ein CC 0-127 ist das 0-127 des knobs; ein Dreifach-Schalter (Wellenform, Filtermodus, Hüllkurvenmodus) teilt den Bereich in drei. In der CLASSIC-Engine ignorieren die Extras (low cut, high cut, reverb send, delay send) CC wie ihre grauen knobs.

| CC | Parameter | CC | Parameter |
| --- | --- | --- | --- |
| 7 | LEVEL | 29 | NOISE FILTER FREQ |
| 10 | PAN | 30 | NOISE ENV MODE |
| 20 | OSC WAVE | 31 | NOISE ENV ATTACK |
| 21 | OSC FREQ | 102 | NOISE ENV DECAY |
| 22 | OSC ATTACK | 103 | MIX |
| 23 | OSC DECAY | 104 | DIST |
| 24 | PITCH MOD SHAPE | 105 | EQ FREQ |
| 25 | PITCH MOD AMOUNT | 106 | EQ GAIN |
| 26 | PITCH MOD RATE | 107 | VELOCITY OSC |
| 27 | NOISE FILTER MODE | 108 | VELOCITY NOISE |
| 28 | NOISE FILTER Q | 109 | VELOCITY PITCH MOD |
| 110 | LOW CUT | 112 | REVERB SEND |
| 111 | HIGH CUT | 113 | DELAY SEND |

### Einen Digitakt oder Digitone anschließen

Die USB-Buchse des FM-1 und die des Digitakt oder Digitone sind beide „Device“-Buchsen, sie können also nicht miteinander sprechen. Nutze den MIDI OUT (5-polig) des Digitakt oder Digitone in den TRS MIDI IN des FM-1, mit einem Adapter von 5-poligem MIDI auf 3,5-mm-TRS-MIDI (Typ A oder B: Wenn nichts ankommt, probiere den anderen Typ).

1. Am FM-1: GLO settings, TONIC MIDI CH = 10 (die Stimmen) und REMOTE MIDI CH = 11 (das Panel), oder zwei beliebige verschiedene Kanäle.
2. Am Digitakt oder Digitone: Nutze zwei MIDI-Tracks und stelle ihre Kanäle auf diese beiden Zahlen. Der erste spielt die Stimmen mit den Drum-Noten (36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51). Der zweite ist das Remote-Panel: Zeichne die Noten der Tabelle oben in sein step-Raster oder den Piano Roll.
3. Ein MIDI-Track spielt bis zu 4 Noten in einem step, mit einer Länge für alle: Die Notenlänge ist, wie lange die Tasten gehalten werden, eine lange Note für FUNC mit anderen Noten auf späteren steps ist also FUNC + diese Tasten, und ein Akkord aus FUNC + ACCENT + einer weißen Taste ist ein step.
4. Um den Sound von den CC-Lanes des Digitakt oder Digitone aus zu bewegen: Stelle GLO > CC MODE = CHANNELS, mache einen MIDI-Track pro Tonic-Track auf den Kanälen 1 bis 8 (oder ab CC CH BASE) und weise seinen CC-Lanes die Nummern der Tabelle oben zu; jeder MIDI-Track hat nur eine begrenzte Zahl von CC-Lanes (16 beim Digitakt II, weniger beim ersten Digitakt), wähle also pro Track die Parameter, die du automatisieren willst.
5. Um dem Tempo und Start / Stop des Digitakt oder Digitone zu folgen, schalte seine MIDI-Clock und den Transport-Ausgang ein; der FM-1 folgt einer ankommenden Clock.

### Ein MIDI-Keyboard anschließen

Der FM-1 kann nicht der Host eines USB-Keyboards sein. Nutze ein Keyboard mit einem 5-poligen MIDI OUT (mit einem Adapter von 5-polig auf TRS in den TRS MIDI IN), oder schicke ein USB-Keyboard über einen Computer oder eine USB-MIDI-Host-Box an die USB-Buchse des FM-1.

1. Am FM-1: GLO settings, REMOTE MIDI CH = der Kanal, auf dem das Keyboard sendet (1 ist der übliche).
2. Spiele die Noten F3 bis G5 (53 bis 79). Halte eine Taste für FUNC (Note 78, F#5), ACCENT (Note 73, C#5) oder FILL (Note 75, D#5) und drücke die anderen: Kopieren, Einfügen, Pattern-Wechsel, mute und solo funktionieren wie auf dem Panel. Ein Keyboard mit 49 Tasten deckt das ganze Panel ab; bei einem kleineren verschiebe es mit den Oktav-Buttons.

- **USB-Audio.** Dasselbe Kabel trägt, was die Kopfhörer hören, nach MASTER, als 2-Kanal-Eingang mit 44,1 kHz, ohne Treiber (GLO > USB AUDIO = FULL sendet den vollen Pegel, was auch MASTER sagt). Im ersten Moment, nachdem eine App ihn öffnet, kann es knacken.

## 7. Installieren, Backup, Wiederherstellen

- **Ausprobieren:** Der [Emulator](https://luigirossi-music.github.io/fm1-tonic/emu/) führt denselben Code im Browser aus.
- **Installieren oder aktualisieren:** Öffne den [Web-Installer](https://luigirossi-music.github.io/fm1-tonic/install/) in Chrome oder Edge, verbinde den FM-1 mit einem USB-Datenkabel, drücke Install und ziehe ihn nicht ab, während er schreibt. Eine andere Firmware behält dein Projekt nicht: Speichere es vorher in eine Datei. „Back to the stock firmware“ unten auf derselben Seite spielt die Firmware von M-VAVE zurück. Das Installieren anderer Firmware geschieht auf eigenes Risiko.
- **Backup:** Die [Backup-Seite](https://luigirossi-music.github.io/fm1-tonic/backup/) speichert das Projekt in eine `.x0b`-Datei (Back up), lädt es zurück (Restore: den FM-1 stoppen, dann RESTORE FROM USB? mit SEL beantworten), exportiert Presets als `.mtpreset`-Dateien und sichert / stellt die Plätze 9 bis 40 als `.tnp`-Dateien wieder her. Kommandozeile: `python3 tools/x0x_backup.py backup my-song.x0b` (auch `restore`, `info`).
- **Wenn etwas schiefgeht:** OCT- und OCT+ fünf Sekunden halten = Update-Modus; sie beim Einschalten halten = Hardware-Kalibrierung. Nach einem Absturz startet ein roter CRASH-Bildschirm den FM-1 neu; stürzt er beim Start zweimal ab, geht er in den SAFE MODE (kein Sound, USB funktioniert: im Installer neu installieren; PLAY versucht einen normalen Start). Wenn auch das scheitert, spielt das Mac-Rescue-Skript in `tools/fm1_rescue.sh` die Firmware von M-VAVE zurück (es prüft jede Datei, die es schreibt; es nochmal auszuführen ist sicher).
- **Wenn ein Update abgelehnt wird.** Wenn der FM-1 die Firmware nicht wechselt (zum Beispiel bei „Back to the stock firmware“), zeigt sein Update-Bildschirm 8 Sekunden lang FAILED, eine Fehlernummer mit einem kurzen Grund (zum Beispiel ERROR -5 HEAD DIFFERS) und den Schritt, an dem er stehen blieb, und dann startet der FM-1 wieder FM1 TONIC. ERROR -5 heißt, dass der Startbereich (der „Head“) deines FM-1 nicht der der Firmware ist, zu der du wechseln willst. Bei diesem Fehler zeigt der Bildschirm außerdem 40 Sekunden lang das erste abweichende Byte, wie viele Bytes abweichen und beide Prüfsummen (CRC). Notiere diese Zahlen, wenn du um Hilfe bittest.
- **Ein eigener Bootloader.** Wenn du einen eigenen Bootloader auf den FM-1 gespielt hast (zum Beispiel m-bootloader, mit M-Flasher), kann der Rückweg zur offiziellen Firmware von M-VAVE von der Webseite aus mit ERROR -5 HEAD DIFFERS abgelehnt werden. Spiele zuerst den Standard-Bootloader von M-VAVE zurück (in M-Flasher: FLASH BOOTLOADER, dann STOCK (FROM .FWSC) mit dem offiziellen Paket), und wechsle dann zur Standard-Firmware zurück. Beim FM-1 des Autors hat damit wieder alles funktioniert.
- **Neuere FM-1-Geräte.** Manche neueren FM-1 haben einen anderen Flash-Chip (ID B36014 statt 856014). FM1 TONIC akzeptiert beide, sodass Speichern, Presets, Backup und Restore auch dort funktionieren (noch nicht an einem Gerät mit dem neuen Chip getestet).
- **Vollständiger Text** der Kapitel über Installation, Backup, Rescue und Parameter: `docs/reference/MANUAL_full_wip381.md`.

## 8. Die Engine

Jeder der 8 Tracks ist ein vollständiger Drum-Synthesizer: die Engine des FM1 TONIC, nichts gesampelt. Die 8 Tracks sind identisch und jeder hat dieselben **26 Einstellungen**, pro Track gehalten und mit dem Preset gespeichert. Dieses Kapitel ist die Referenz für alle. Wo eine Einstellung liegt, steht als Button · knob.

```
pitch mod ─► OSCILLATOR ─┐
                         ├─► MIX ─► EQ ─► DIST ─► LEVEL ─┐
NOISE ─► filter ─► env ──┘                               │
          ┌──────────────────────────────────────────────┘
          └─► LOW CUT ─► HIGH CUT ─┬─► PAN ─► master
                                   └─► REVERB / DELAY SEND
```

Ein Hit klingt, bis seine Hüllkurven ausgeklungen sind. Ein neuer Hit auf demselben Track schneidet den noch klingenden ab (Drums sind monophon), und ein Track in der choke group wird von den anderen abgeschnitten (3 ms Fade).

**Die knobs sind Kurven, keine Geraden.** Frequenzen, Zeiten und Q sind exponentiell: Die Mitte des knobs ist nicht die Mitte des Werts. Auf dem Bildschirm erscheinen Frequenzen in Hz oder kHz, der pitch mod AMOUNT in Halbtönen (sm) und pan als L / C / R, der OSC/NOISE MIX als zwei Prozentwerte (100.00/00.00 = nur oscillator, 00.00/100.00 = nur Noise); jeder andere knob zeigt eine einfache Zahl von 0 bis 127. Diese Tabelle zeigt, was die Zahlen bedeuten.

| Knob | 0 | 32 | 64 | 96 | 127 |
|---|---|---|---|---|---|
| **Frequency** (oscillator, Noise-Filter, EQ) | 20 Hz | 114 Hz | 650 Hz | 3,7 kHz | 20 kHz |
| **Decay** (oscillator, Noise, pitch-mod-Abfall) | 10 ms | 57 ms | 0,33 s | 1,8 s | 10 s |
| **Attack** (oscillator, Noise) | 0 | 20 ms | 0,16 s | 1 s | 5 s |
| **Filter Q** | 0,1 | 1,8 | 33 | 600 | 10000 |

Ein Decay ist die Zeit, die der Sound braucht, um bis zur Stille abzufallen (60 dB tiefer).

### Oscillator: der Ton

| Einstellung | Wo | Bereich | Was sie tut |
|---|---|---|---|
| **WAVE** | ENV · knob 1 | SINE, TRI, SAW | SINE ist rein und weich, TRI hohl, SAW hell und schnarrend. |
| **FREQUENCY** | ENV · knob 2 | 20 Hz bis 20 kHz | Die Tonhöhe. Kick etwa 40 bis 70 Hz, Toms 80 bis 250 Hz, Snare-Körper 150 bis 250 Hz. OCT- / OCT+ verschieben sie um eine Oktave. |
| **ATTACK** | ENV · knob 3 | 0 bis 5 s | 0 beginnt sofort. Erhöhe es für ein Anschwellen. Das erste Viertel des knobs liegt unter 20 ms. |
| **DECAY** | ENV · knob 4 | 10 ms bis 10 s | Wie lange er klingt: kurz = Klick oder Tick, lang = Boom. |

### Pitch mod: die Tonhöhe bewegt sich, während der Sound spielt

| Einstellung | Wo | Bereich | Was sie tut |
|---|---|---|---|
| **SHAPE** | SEL · knob 1 | DECAY, SINE, RANDOM | Die Art der Bewegung (unten). |
| **AMOUNT** | SEL · knob 2 | 64 = keine; bis zu 8 Oktaven in jede Richtung | Über 64 geht die Tonhöhe nach oben, unter 64 nach unten. Bei DECAY beginnt der Sound so weit entfernt und gleitet zur eingestellten Tonhöhe. |
| **RATE** | SEL · knob 3 | hängt von SHAPE ab | DECAY: wie lange das Gleiten dauert, 10 ms bis 10 s (Kick-Drop = kurz, Laser = lang). SINE: die Geschwindigkeit des Wabberns, 0 bis 2000 Hz; nur die ersten paar Stufen sind ein Vibrato, höher ist FM (Glocken, Zaps), und eine höhere Rate lässt ihn auch länger anhalten. RANDOM: wie schnell die Tonhöhe zufällig wandert, 20 Hz bis 20 kHz; langsam driftet sie, schnell wird sie metallisch. |

### Noise: der Knack und das Zischen

Das Noise ist weißes Rauschen durch einen Filter und eine Hüllkurve. Der oscillator und das Noise sind getrennt: Der MIX entscheidet, wie viel von jedem du hörst.

| Einstellung | Wo | Bereich | Was sie tut |
|---|---|---|---|
| **FILTER MODE** | LFO · knob 1 | LP, BP, HP | LP behält die Tiefen (Wumms, dunkel), HP die Höhen (Hat, Zischeln), BP ein Band um die Frequenz. |
| **FILTER Q** | LFO · knob 2 | 0,1 bis 10000 | Resonanz. Niedrig = sanft, um knob 64 = eine starke Spitze, hoch = der Filter klingt als Ton mit Tonhöhe: BP mit hohem Q und kurzer Hüllkurve ist ein gestimmtes Ping (Tom, Cowbell, Block). |
| **FILTER FREQUENCY** | LFO · knob 3 | 20 Hz bis 20 kHz | Wo der Filter arbeitet. |
| **ENVELOPE MODE** | EDIT · knob 1 | EXP, LIN, CLAP | EXP: ein schneller Anstieg und ein natürlicher Abfall, wie beim oscillator. LIN: ein geradliniger Anstieg und Abfall. CLAP: ein paar kurze Bursts, wie Hände. |
| **ENVELOPE ATTACK** | EDIT · knob 3 | 0 bis 5 s | EXP und LIN: der Anstieg. CLAP: wie lange das Noise auf vollem Pegel bleibt, bevor die Bursts beginnen. |
| **ENVELOPE DECAY** | EDIT · knob 4 | 10 ms bis 10 s | Der Abfall. Bei CLAP legt er auch die Lücke zwischen den Bursts fest: kurz = ein schnelles Schnarren, lang = einzelne Claps. |

### Mix, EQ, Verzerrung

| Einstellung | Wo | Bereich | Was sie tut |
|---|---|---|---|
| **OSC/NOISE MIX** | FX · knob 1 | 100.00/00.00 bis 00.00/100.00 | Der Bildschirm zeigt den Anteil des oscillators und den des Noise in Prozent. 100.00/00.00 = nur oscillator, 49.61/50.39 (die Mitte des knobs) = beide mit vollem Pegel, 00.00/100.00 = nur Noise. Ein neuer Track beginnt bei 100.00/00.00: kein Noise, bis du es bewegst. |
| **EQ FREQUENCY** | FX · knob 2 | 20 Hz bis 20 kHz | Die Mitte des einen EQ-Bands. |
| **DIST** | FX · knob 3 | 0 bis 127 | Verzerrung, sehr steil: Das erste Viertel ist eine sanfte Wärme, hinter der Mitte ist sie kräftig, ganz oben ein Fuzz. Sie ist unsymmetrisch, sodass sie Lautheit und Biss hinzufügt. |
| **EQ GAIN** | FX · knob 4 | 64 = flat; 40 dB in jede Richtung | Anhebung oder Absenkung des Bands (breit, eine Glocke). Der EQ kommt **vor** der Verzerrung, erst anzuheben und dann zu verzerren ergibt also einen schwereren Sound als umgekehrt. |

### Velocity: accents, die den Sound verändern

Der Sequencer spielt einen normalen step mit etwa einem Drittel der vollen Velocity und einen accent-step mit voller. Die drei VELOCITY-knobs (GLO · VEL, knobs 1 bis 3) sagen, wie stark ein Teil ihr folgt.

| Einstellung | Wo | Was sie tut |
|---|---|---|
| **OSC** | GLO · knob 1 | 0 = der oscillator ignoriert die Velocity. Höher = bei normalen steps wird er leiser, accents bleiben voll. |
| **NOISE** | GLO · knob 2 | Dasselbe für das Noise. |
| **PITCH MOD** | GLO · knob 3 | Dasselbe für den Betrag des pitch mod: Bei normalen steps schrumpft die Bewegung. |

Sie ist steil: Ein kleiner Wert kürzt einen normalen step schon stark, und ab etwa knob 95 verliert ein normaler step diesen Teil ganz. Wenn alle drei auf 0 stehen, klingt ein accent genau wie ein normaler step: Der accent ist nur über diese knobs zu hören. Zum Beispiel hat eine Snare mit hohem VELOCITY OSC ihren Ton nur bei accents und spielt bei normalen steps nur Noise.

### Output: level, pan, cuts, sends

| Einstellung | Wo | Bereich | Was sie tut |
|---|---|---|---|
| **LEVEL** | HOME · knob 1 | Stille bis etwa +10 dB (Standard 101 = etwa -2,5 dB) | Die Lautstärke des Tracks. Die Stimme clippt bei Vollaussteuerung: Wenn es knistert, senke sie. |
| **PAN** | HOME · knob 2 | 0 links, 64 Mitte, 127 rechts | Position im Stereobild. |
| **LOW CUT** | HOME · knob 3 | 20 bis 663 Hz | Nimmt die Tiefen weg (12 dB pro Oktave). Bei 20 Hz tut er nichts Hörbares. |
| **HIGH CUT** | HOME · knob 4 | 20 kHz hinunter bis 1 kHz | Nimmt die Höhen weg (12 dB pro Oktave). Ganz rechts = offen. |
| **REVERB SEND** | HOME, 2. Druck · knob 1 | 0 bis 127 | Wie viel vom Track in den Reverb geht. |
| **DELAY SEND** | HOME, 2. Druck · knob 2 | 0 bis 127 | Dasselbe für das Delay. |

Die sends gehen nach den cuts und dem Sidechain ab, vor dem pan, und arbeiten, egal was D/W von PERFORM und ALL FX sagen.

### Rund um die Stimme

Gemeinsam für alle Tracks:

- **Choke group** (EDIT, ALGORITHM nach rechts): Ein Hit eines Tracks der Gruppe stoppt die anderen sofort (geschlossene Hi-Hat stoppt offene Hi-Hat).
- **Sidechain** (HOME, 2. Druck): Ein Input-Track lässt die Tracks, die du auf dem DUCK-Bildschirm wählst, ducken, solange er klingt; der Threshold-knob arbeitet wie der des Kompressors: ganz rechts = aus, nach links duckt mehr.
- **Master Low Cut** (PERFORM · knob 3, 20 bis 663 Hz): ein Low Cut auf dem ganzen Mix, vor dem Kompressor.
- **Kompressor** (GLO · COMP): 4:1, Soft Knee, Attack 0,1 bis 100 ms, Release 10 bis 1500 ms, Threshold -48 bis 0 dB, Make-up 0 bis 24 dB. Ein Limiter danach hält den Ausgang unter der Vollaussteuerung.
- **Reverb** (FX halten): Decay, Tone, Low Cut, Return-Pegel; ein körniger Raum aus der Frühzeit der Digitaltechnik. **Delay** (HOME halten): tempo-synchrone Zeit (1/32 bis 1/2), Feedback, Tone, TAPE (sauber digital bis abgenutztes Band), Low Cut, Return, Ping-Pong.

### Enhanced und classic

22 der 26 Einstellungen trägt ein Patternarium-Preset (die 21 Sound-Einstellungen, level inklusive, plus pan). Vier sind **Extras des FM1 TONIC**: LOW CUT, HIGH CUT, REVERB SEND und DELAY SEND, ebenso wie die choke group, der Sidechain, die Master-Sektion, p-locks, chance und Microtiming. In **classic** (GLO > settings > TONIC ENGINE) sind die vier Extras grau und stumm, der Sidechain auch, sodass das, was du baust, nur die Einstellungen nutzt, die eine `.mtpreset`-Datei trägt; die Werte kommen in enhanced zurück. Ein geladenes Patternarium-Preset behält seine genauen Werte, bis du diesen knob drehst.

### Klangrezepte

Das sind Ausgangspunkte: Ändere immer nur einen Wert und höre hin.

#### Kick

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 20 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +37.04 sm | RATE 25 | - |
| **ENV** | WAVE SINE | FREQUENCY 56 Hz | ATTACK 0 | DECAY 72 |
| **LFO** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **EDIT** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

AMOUNT ist die Tiefe des Abfalls und RATE seine Geschwindigkeit. Für einen Klick stelle MIX 40 auf FX, dann LFO: MODE HP, FREQUENCY 2.9 kHz, und EDIT: DECAY 15. Für einen längeren Boom erhöhe ENV DECAY.

#### Snare

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 15 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +12.85 sm | RATE 13 | - |
| **ENV** | WAVE TRI | FREQUENCY 196 Hz | ATTACK 0 | DECAY 46 |
| **LFO** | MODE HP | Q 22 | FREQUENCY 2.0 kHz | - |
| **EDIT** | MODE EXP | so lassen | ATTACK 0 | DECAY 55 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Mehr Knack: Erhöhe MIX. Mehr Körper: Senke MIX. Für leise Ghost Notes stelle GLO NOISE auf 30: Normale steps spielen das Noise leiser, und accents spielen es voll.

#### Geschlossene Hi-Hat

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **ENV** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **LFO** | MODE HP | Q 22 | FREQUENCY 7.1 kHz | - |
| **EDIT** | MODE EXP | so lassen | ATTACK 0 | DECAY 25 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

OSC/NOISE MIX 00.00/100.00 bedeutet nur Noise. Senke die LFO-Frequenz für eine dunklere Hi-Hat.

#### Offene Hi-Hat

Dasselbe wie die geschlossene Hi-Hat, mit **EDIT DECAY 68**. Lege die beiden Hi-Hats in eine **choke group**, damit die geschlossene die offene stoppt: Halte auf jedem der beiden Tracks FUNC und drehe EDIT knob 2 nach rechts.

#### Clap

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **ENV** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.1 kHz | - |
| **EDIT** | MODE CLAP | so lassen | ATTACK 0 | DECAY 60 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Mit diesen Werten bekommst du 3 bis 4 schnelle Bursts in etwa einer Zehntelsekunde. EDIT DECAY niedriger = ein strafferes Schnarren, höher = langsamere, getrenntere Claps und ein längerer Ausklang (75 bis 85). Ein wenig Reverb send (HOME, 2. Druck) macht daraus einen Raum-Clap.

#### Tom

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +17.39 sm | RATE 20 | - |
| **ENV** | WAVE SINE | FREQUENCY 120 Hz | ATTACK 0 | DECAY 65 |
| **LFO** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **EDIT** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Nutze denselben Sound auf anderen Tracks mit einer anderen FREQUENCY (80 Hz für ein Floor Tom, 200 Hz für ein hohes Tom).

#### Zap

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +47.62 sm | RATE 62 | - |
| **ENV** | WAVE SAW | FREQUENCY 1.0 kHz | ATTACK 0 | DECAY 75 |
| **LFO** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **EDIT** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Die Tonhöhe fällt in etwa 0,3 s um 4 Oktaven. Schicke ihn ans Delay (HOME, 2. Druck) für einen klassischen Laser.

#### Rimshot

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 30 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +11.34 sm | RATE 10 | - |
| **ENV** | WAVE TRI | FREQUENCY 552 Hz | ATTACK 0 | DECAY 25 |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.8 kHz | - |
| **EDIT** | MODE EXP | so lassen | ATTACK 0 | DECAY 22 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Ein kurzer, trockener Klick (etwa 45 ms). Erhöhe DIST für einen härteren Rim; schiebe den MIX nach links für mehr Ton.

#### Sub bass

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 35 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +6.80 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 41 Hz | ATTACK 6 | DECAY 80 |
| **LFO** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **EDIT** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Eine tiefe Note von etwa 0,9 s mit einem kleinen Tonhöhenabfall am Anfang und etwas DIST, damit man sie auf kleinen Lautsprechern hört. Ändere FREQUENCY (30 bis 60 Hz) für eine andere Note, oder kürze DECAY für eine lange Kick.

#### Conga

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 82.68/17.32 | EQ FREQ 20 Hz | DIST 10 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +5.29 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 231 Hz | ATTACK 0 | DECAY 54 |
| **LFO** | MODE BP | Q 30 | FREQUENCY 1.6 kHz | - |
| **EDIT** | MODE EXP | so lassen | ATTACK 0 | DECAY 20 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Ein runder Ton mit einem sehr kurzen Noise-Schlag (insgesamt etwa 0,2 s). Erhöhe FREQUENCY (zum Beispiel 330 Hz) auf einem anderen Track für eine hohe Conga.

#### Crash

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **ENV** | nicht hörbar (OSC/NOISE MIX 00.00/100.00), so lassen | | | |
| **LFO** | MODE HP | Q 10 | FREQUENCY 5.1 kHz | - |
| **EDIT** | MODE EXP | so lassen | ATTACK 0 | DECAY 100 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Ein Becken aus reinem Noise, das etwa 2,3 s dauert. Senke DECAY (85 ergibt etwa 1 s) für ein Splash, oder erhöhe die LFO FREQUENCY für einen dünneren Sound. Schicke es ein wenig in den Reverb (HOME, 2. Druck).

#### Clave

| Button | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT 0.00 sm | RATE 0 | - |
| **ENV** | WAVE SINE | FREQUENCY 2.5 kHz | ATTACK 0 | DECAY 28 |
| **LFO** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **EDIT** | nicht hörbar (OSC/NOISE MIX 100.00/00.00), so lassen | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | so lassen |

Ein trockener, hoher Ton von etwa 50 ms ohne Tonhöhenbewegung (AMOUNT 0.00 sm). Ändere FREQUENCY (2 bis 3 kHz) für eine andere Stimmung.

**Tipps.** Um einen Teil allein zu hören, drehe den MIX ganz nach links (100.00/00.00, nur oscillator) oder ganz nach rechts (00.00/100.00, nur Noise). Damit accents den Sound verändern und nicht nur die Lautstärke, erhöhe die GLO-knobs. Um einen Wert nur auf einem step auszuprobieren, halte den step und drehe den knob (p-lock, Kapitel 4). Ein Sound, der dir gefällt, wird von SAVE behalten (Kapitel 5).

## 9. Credits

**FM1 TONIC wird von Luigi Rossi gemacht**, dem Entwickler und kreativen Kopf hinter dieser Firmware: die Tonic Drum Machine, ihr Sequencer, die Bildschirme, die Presets und die Web-Tools (Emulator, Installer, Backup). Es ist freie Software unter der GNU General Public License, Version 3; wenn du die Firmware weitergibst, musst du ihren Quellcode weitergeben.

**Luigi Rossi online:** [linktr.ee/luigirossi](https://linktr.ee/luigirossi)

LICENSING.md listet Herkunft und Lizenz jeder Datei auf. Sie baut auf der Arbeit von folgenden auf:

- **libpo32** von Eric Lewis (MIT): der Ausgangspunkt des Drum-Synthese-Codes des FM1 TONIC. Nichts wird gesampelt.
- **X0X** von Charles Vestal, die Firmware, aus der FM1 TONIC gewachsen ist, und die Webseiten (Emulator, Installer, Backup); X0X ist ein Fork von **Felucca** von Leo Kuroshita (@kurogedelic), Hügelton Instruments: die Plattform (Hardware-Schicht, USB, Update-Loader, Installer, Speicher) und das Rescue-Board FM-1-transporter.
- Reverb und Delay sind von **9W9** von athousanddetails portiert, das auf **ER-99** von Matthew Cieplak (GPL-3.0) basiert. Der Master-Kompressor folgt Giannoulis, Massberg und Reiss; der Master-Filter ist das TPT-State-Variable-Filter von Vadim Zavalishin.
- **Melodee** von keremimo und **Jangada** von zednaked: was sie über den zweiten Kern des FM-1 herausgefunden haben.
- Schriften: **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov), **DotGothic16** (The DotGothic16 Project Authors), alle unter der SIL Open Font License.

M-VAVE und FM-1 sind Marken ihrer jeweiligen Inhaber. FM1 TONIC steht in keiner Verbindung zu M-VAVE, Sonic Charge oder Luigi Rossi und wird von ihnen nicht unterstützt.
