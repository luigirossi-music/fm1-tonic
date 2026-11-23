# FM1 TONIC

Ispirato a Microtonic VST di Sonic Charge, FM1 TONIC è un sintetizzatore di batteria a 8 voci in cui ogni suono è sintetizzato da zero, senza alcun sample. Unisce un profondo motore di percussioni a un sequencer da 1 a 64 step (con accent, fill e 16 pattern per progetto), un mixer integrato con level e pan, reverb, delay e un compressore master. Con 40 project slot (8 preset di fabbrica + 32 slot utente) e un flusso di lavoro pensato per il live, passa senza sforzo da beat ritmici e incisivi a texture ambient in evoluzione.

## Caratteristiche

**Suono**

- Sintetizzatore di batteria a 8 voci: ogni track ha il suo oscillator, pitch mod, noise, mix, EQ e distorsione.
- Level e pan per track, e reverb send e delay send per track (più low cut e high cut nell'engine Enhanced).
- Engine Enhanced o Classic (Classic = solo le impostazioni di base del suono, senza gli extra di FM1 TONIC).
- Randomizer di track: un nuovo suono casuale per la track selezionata, oppure per tutte e 8 le track insieme (RANDOM ALL VOICES).
- Choke groups (per esempio hi-hat chiuso e aperto).
- Compressore sidechain: una traccia (l'ingresso, per esempio il kick) fa duck alle tracce che scegli, con un threshold da 0 dB fino a -INF.

**Sequencer**

- Sequencer a 64 step, 16 pattern per progetto. Lunghezza fissa di 8, 16, 32 o 64 step, oppure lunghezza libera da 1 a 64.
- Song mode: battute con ripetizioni (da x1 a x8 o LOOP), che si modificano nella schermata SONG.
- Accent globale (da +1 a +10) e accent per track, fill con retrig (2x, 4x, 8x) e trig % (chance) su ogni step.
- Parameter lock (limitato: 3 knob per track in ogni pattern).
- Record automation (limitato: 3 knob per track).
- Registrazione dal vivo, quantized o unquantized. Swing, quantization e tempo.
- Tastiera (ARP): suona una track come una tastiera e registra le note negli step (un piano roll).
- Copia e incolla di un pattern, di una track, e di un pattern al volo mentre la canzone suona.
- Mute e solo (su PERFORM e sul mixer di GLO; il modo resta attivo quando lasci la pagina).
- Undo e redo.

**Mix ed esecuzione**

- Mixer master: level e pan di ogni track (il punto rosso indica il centro).
- Compressore master.
- Pagina Performance: 2 effetti master (reverb e delay), un low cut e ALL FX OFF.
- Pagina Memory: quanta memoria del progetto è occupata.
- Indicatore della batteria (tre livelli, in alto a destra) e un avviso LOW BATTERY a schermo intero.

**Progetti e salvataggio**

- 40 project slots: 8 preset di fabbrica e 32 tuoi.
- Autosave e quick save.
- Nome del progetto: GENERATE o MANUAL. Rinomina del progetto.
- Backup e ripristino dalla pagina web. Esportazione e importazione di preset `.mtpreset`.
- I preset funzionano nei due sensi: carica preset `.mtpreset` da Patternarium e Microtonic, ed esporta i tuoi di nuovo per il Microtonic VST (provato; il suono è molto simile, non identico).

**Schermo, luci e impostazioni**

- Indicatore LED 4/4 (tasti bianchi 1, 5, 9 e 13), luce dei tasti on oppure off.
- Zoom knobs (il grande quadrante quando giri un knob), tema colore.
- Volume di uscita NORMAL o LOUD. USB audio.

**MIDI**

- Selettore del canale MIDI, notes out on oppure off, clock out on oppure off.
- Control change (CC): 26 parametri del suono, per la track selezionata o per canale MIDI.
- Pannello remoto: suona i tasti dello stesso FM-1 da una tastiera esterna o da un Digitakt.

## 1. Il pannello

![Il pannello di FM1 TONIC: knob MASTER, SELECT, PRESETS, ALGORITHM e KNOB 1 a 4; pulsanti FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- e OCT+; tasti neri TRACK 1 a 8, ACCENT, FILL, FUNC; tasti bianchi 1 a 16](img/fm1-tonic-panel.svg "")

- **Knob.** MASTER è il volume. SELECT cambia pagina (in una lista: su e giù). PRESETS sceglie uno dei 40 preset. KNOB 1 a 4 regolano i quattro valori della pagina sullo schermo (quello che giri viene ingrandito). ALGORITHM passa a una seconda pagina oppure sceglie una voce, a seconda della pagina (le tabelle delle pagine lo indicano). Un piccolo triangolo blu in un angolo in basso dello schermo significa che dietro ALGORITHM c'è una seconda pagina: triangolo destro = gira ALGORITHM a destra, triangolo sinistro = giralo a sinistra per tornare indietro.
- **Tasti neri.** Da 1 a 8 sono TRACK 1 a 8, 9 è ACCENT, 10 è FILL, 11 è FUNC.
- **Tasti bianchi da 1 a 16** sono i 16 step della traccia selezionata. Un pattern può avere fino a 64 step, mostrati 16 alla volta (vedi SEQ).
- **OCT- / OCT+** spostano di un'ottava l'altezza della traccia selezionata (su ARP: l'ottava della tastiera). In una domanda, OCT+ = YES, OCT- = NO. Nella schermata SONG: OCT- cancella la riga, OCT+ aggiunge una riga.

### Indice

1. Il pannello {#s1}
2. Il percorso del segnale {#s2}
3. Le pagine, pulsante per pulsante {#s3}
  - Combinazioni di pulsanti
4. Suonare {#s4}
  - Promemoria del modo song
5. Preset e salvataggio {#s5}
6. Luci, MIDI e USB {#s6}
7. Installare, backup, ripristino {#s7}
8. L'engine {#s8}
  - Ricette di suoni
9. Crediti {#s9}

---

**Come leggere le tabelle delle pagine.** Ogni pulsante ha una tabella, con una riga per pagina. Il testo normale è ciò che il controllo fa da solo. {r:Rosso} è ciò che fa con **FUNC** premuto, {o:arancione} è ciò che fa con **FILL** premuto (alla maniera del Digitakt).

**Ciò che vale in ogni pagina** (le tabelle seguenti non lo ripetono):

| Tasti | Che cosa fa |
|---|---|
| Tasto nero da 1 a 8 | Seleziona la traccia e la suona (anche durante la riproduzione). |
| Tasto bianco da 1 a 16 | Gli step: un tocco = trig attivo, un altro tocco = spento. Un pattern ha fino a 64 step, mostrati 16 alla volta. |
| Tasto bianco tenuto + ACCENT / FILL | Quello step riceve un accent / un fill (di nuovo = tolto). |
| ACCENT o FILL tenuto | I trig della traccia che hanno quell'accent / fill lampeggiano sui tasti bianchi (gli altri trig restano accesi). |
| {r:FUNC + tasto nero da 1 a 8} / {o:FILL + tasto nero da 1 a 8} | Mette la traccia in mute / in solo (di nuovo = torna normale). |
| {r:FUNC + tasto bianco} | Cambia pattern (durante la riproduzione: alla fine della battuta). |
| {r:FUNC + ACCENT} / {r:FUNC + FILL} / {r:FUNC + PLAY} | Copia il pattern / lo incolla (chiede PASTE PATTERN?) / lo cancella (chiede CLEAR PATTERN?). OCT+ = sì, OCT- = no. |
| {r:FUNC + tasto nero da 1 a 8 tenuto, poi ACCENT / FILL / PLAY} | Copia il suono di quella traccia / lo incolla / cancella il pattern di quella traccia (chiede conferma). |
| {r:FUNC + SEQ} | Modo song acceso / spento (chiede conferma). |
| {r:FUNC + ARP} | Cancella la song (chiede conferma). |
| {r:FUNC + SAVE} / {r:FUNC + HOME} | Salvataggio rapido nel posto caricato / rinomina il progetto. |
| {r:FUNC + REC} | QUANTIZED REC / UNQUANTIZED REC. |
| ENV + un knob | Quel knob torna al valore di default. |
| GLO + ACCENT / GLO + FILL | Annulla / ripeti (in qualsiasi pagina). |
| REC + PLAY, poi tasti e knob | Registrazione dal vivo: i tasti neri da 1 a 8 scrivono trig, i knob scrivono automazione (3 knob per traccia). |
| PLAY | Avvia e ferma il pattern. |
| OCT- / OCT+ | L'altezza della traccia un'ottava sotto / sopra. In una domanda: NO / YES. Nella schermata SONG: OCT- cancella la riga, OCT+ aggiunge una riga. |

Eccezioni: su PERFORM, MUTE / SOLO, sulla pagina dei pattern e sul mixer GLO i tasti neri sono pulsanti di mute; su ARP ogni tasto è un tasto di tastiera. Sono tutte nell'elenco seguente.

**Dove i tasti fanno altro.** I tasti del pannello (tasti neri da 1 a 8 = tracce, ACCENT, FILL, FUNC, tasti bianchi da 1 a 16 = step) e le scorciatoie qui sopra valgono in ogni pagina di questo manuale, tranne nelle pagine di questo elenco, dove i tasti hanno un altro compito:

| Pagina | Che cosa fanno lì i tasti |
|---|---|
| **ARP** | Ogni tasto, bianco e nero, è un tasto di tastiera che suona la voce selezionata. |
| **PERFORM, MUTE / SOLO, PATTERN** (SEQ, 2ª pressione), **MIXER GLO** | I tasti neri da 1 a 8 sono pulsanti di mute (di solo nel modo SOLO), non selezionano la traccia. I tasti bianchi da 1 a 16 scelgono il pattern successivo. FUNC e FILL, toccati, passano dal modo MUTE al modo SOLO. |
| **DUCK** (SENDS, ALGORITHM a destra) | I tasti neri da 1 a 8 scelgono quali tracce ricevono il duck. |
| **CHOKE** (EDIT, ALGORITHM a destra) | I tasti neri da 1 a 8 scelgono quali tracce fanno parte del gruppo choke. |
| **Schermate SONG e PATTERNS** (modo song) | I tasti bianchi da 1 a 16 scrivono un pattern nella battuta. FILL cancella la battuta. |
| **Modo copia-incolla** (dopo FUNC + ACCENT) | I tasti bianchi da 1 a 16 sono i posti dove incollare. |
| **FUNC tenuto** | I tasti bianchi da 1 a 16 sono i pattern da 1 a 16; i tasti si accendono dove un pattern o una traccia ha delle note. |

**Cose che esistono solo in alcune pagine.**

| Pagina | Tasti | Che cosa fa |
|---|---|---|
| Pagine del suono: MIX 1, SEL, ENV, LFO, EDIT, VEL | Tasto bianco tenuto + knob da 1 a 4 | **P-lock**: quello step riceve un suo valore del knob. Tasto bianco tenuto + SAVE + lo stesso knob lo toglie. Una traccia conserva 3 knob con p-lock per pattern. |
| Ogni pagina tranne PERFORM, il mixer GLO, HOME, SONG, COMP e le liste | Tasto bianco tenuto + SELECT | La **chance** di quello step: da 5 a 100 %, poi 1:2, 2:2, 1:3 ... 8:8. |
| ENV | OCT- / OCT+ | La frequenza dell'oscillatore un'ottava sotto / sopra. |
| EDIT | {r:FUNC + knob 2}, {r:FUNC, tocco su EDIT}, EDIT o LFO tenuto + {r:FUNC + ACCENT / FILL / PLAY} | Entra / esce dal gruppo choke; DELETE AUTOMATION?; copia / incolla / azzera questa pagina della traccia. |
| SEQ | Knob, SELECT, ALGORITHM, {o:FILL + SEQ} | Vedi la tabella di SEQ nel capitolo 3. |
| GLO > VEL | {r:FUNC + knob 4} | Un nuovo suono casuale per la traccia selezionata. |
| SEL | {r:FUNC + knob 4} | Un nuovo suono casuale per TUTTE le tracce. |
| GLOBAL (HOME, 3ª pressione) | Knob 4 (oppure HOME tenuto + SELECT) | Il tempo. |

## 2. Il percorso del segnale

![L'engine di FM1 TONIC: ognuna delle 8 tracce ha un modulatore di pitch e un oscillator, un filtro di noise e un envelope di noise, un mix, una distorsione e un EQ, poi cut, level e pan; le 8 tracce passano per il sidechain, il low cut master e il compressore fino al volume master; reverb e delay sono effetti di send](img/fm1-engine.svg "Tutto il percorso del segnale. La piccola parola in ogni riquadro è il pulsante che apre quella pagina.")

Ogni traccia costruisce il suo suono da zero con l'engine di batteria di FM1 TONIC: un modulatore di pitch e un oscillator, un filtro e un envelope di noise, un mix, una distorsione e un EQ, poi i cut, il level e il pan della traccia; le 8 tracce passano per il sidechain, il low cut master e il compressore fino a MASTER, con reverb e delay come effetti di send. Il capitolo 8 spiega ogni impostazione.

**Enhanced e classic.** GLO > settings > Tonic Engine è su **enhanced** di default. In **classic** gli extra di FM1 TONIC (i cut di traccia, i send, il sidechain) sono in grigio e non fanno nulla, così il suono usa solo le impostazioni che può portare un file `.mtpreset`. I loro valori vengono conservati e tornano con enhanced. Level e pan restano disponibili in classic. Il modo è globale, non si salva in un preset.

## 3. Le pagine, pulsante per pulsante

Una pagina con valori di **traccia** (i knob da 1 a 4 delle pagine del suono) appartiene alla traccia selezionata: cambia traccia con un tasto nero da 1 a 8 e i valori cambiano. EDIT o LFO tenuto + {r:FUNC + ACCENT / FILL / PLAY} copia, incolla e azzera **questa pagina** della traccia (solo nelle pagine di traccia).

### FX

Pressione = MIX 1 → PERFORM → MIX 1. Tenuto (mezzo secondo) = REVERB. Da un'altra pagina, FX torna all'ultima pagina FX che hai lasciato.

| Pagina | Knob 1 · 2 · 3 · 4 | Anche in questa pagina |
|---|---|---|
| **MIX 1** (1ª pressione) | mix OSC / NOISE · EQ FREQ (20 Hz-20 kHz) · DIST · EQ GAIN (64 = piatto) | Tasto bianco tenuto + knob: p-lock. SAVE + knob: dimentica quel lock o quel movimento. |
| **PERFORM** (2ª pressione) | REVERB D/W · DELAY D/W (0-100 %, 0 all'accensione) · MASTER LOW CUT (20-663 Hz) · ALL FX (a destra acceso, a sinistra spento) | Il numero grande è il pattern in riproduzione. Quando un pattern è in coda, un riquadro viola con scritto PATTERN n (quello che arriva) lampeggia sopra. SELECT: le pagine FX successive (si ferma agli estremi). ALGORITHM a destra: MUTE / SOLO. Tasti neri da 1 a 8: mute. Tasti bianchi da 1 a 16: pattern successivo (in modo song non fanno nulla: decide la song, e il riquadro viola mostra la battuta successiva della song). {r:FUNC tocco}: modo MUTE, oppure toglie tutti i mute. {o:FILL tocco}: modo SOLO, oppure toglie tutti i solo. Niente p-lock. |
| **MUTE / SOLO** (PERFORM, ALGORITHM a destra) | come PERFORM | Otto riquadri al posto del numero: blu = la traccia suona. I tasti neri da 1 a 8 mettono in mute (modo MUTE) oppure aggiungono / tolgono un solo (modo SOLO, più di uno insieme). ALGORITHM a sinistra: indietro. |
| **REVERB** (FX tenuto) | DECAY · TONE | ALGORITHM a destra: REVERB 2. |
| **REVERB 2** | LOW CUT (di ciò che entra nel reverb) · RETURN (livello nel mix) | ALGORITHM a sinistra: indietro. |
| **DELAY** (tenendo anche HOME) | TIME (1/32 ... 1/2, con valori T e puntati) · FEEDBACK · TONE · TAPE (il primo ottavo = digitale pulito, poi tape con sempre più usura) | ALGORITHM a destra: DELAY 2. |
| **DELAY 2** | LOW CUT · RETURN · PING-PONG (off / on) | ALGORITHM a sinistra: indietro. |

ALL FX spento silenzia i due send D/W di PERFORM e il low cut master; i send propri delle tracce continuano a funzionare. Mute e solo sono condivisi da PERFORM e MUTE / SOLO; i solo non si salvano e finiscono quando si carica un preset. Il modo scelto (MUTE o SOLO) resta quando lasci la pagina e torni. Sui tasti neri una traccia in mute lampeggia e una traccia in solo lampeggia più in fretta; il tasto bianco del pattern in riproduzione lampeggia lentamente; i tasti non lampeggiano a ogni colpo su PERFORM, sulla pagina dei pattern e sul mixer di GLO. Le pagine degli effetti sono globali (tutte le tracce le condividono): SELECT scorre PERFORM, REVERB, DELAY, REVERB 2, DELAY 2.

### HOME

Pressione = MIX 2 → SENDS → GLOBAL → MIX 2. Tenuto = DELAY.

| Pagina | Knob 1 · 2 · 3 · 4 | Anche in questa pagina |
|---|---|---|
| **MIX 2** | LEVEL · PAN · LOW CUT (20-663 Hz, mai spento) · HIGH CUT (da 20 kHz fino a 1 kHz) | I due cut sono in grigio in classic. |
| **SENDS** | REVERB SEND · DELAY SEND · SIDECHAIN THRESHOLD (in dB, come quello di un compressore: tutto a destra = 0.0 dB = niente viene abbassato, verso sinistra scende fino a -59 dB, e l'ultimo passo a sinistra è -INF) · SIDECHAIN INPUT (traccia da 1 a 8, numero grande) | ALGORITHM a destra: la schermata DUCK (sotto). I send funzionano qualunque cosa dicano i D/W di PERFORM e ALL FX. |
| **DUCK** (SENDS, ALGORITHM a destra) | - | Tasti neri da 1 a 8: blu = con duck, grigio = senza, viola = l'input. L'input non può avere il duck. Finché non scegli, non viene abbassato nulla. ALGORITHM a sinistra o HOME: indietro. |
| **GLOBAL** | QUANTIZATION (1/8 ... 1/32) · SWING · FILL RATE (2x, 4x, 8x) · TEMPO | - |

Il sidechain è un compressore con un ingresso laterale: ascolta la traccia di input e abbassa le tracce con il duck finché l'input è più forte del threshold (fissi: ratio 4:1, soft knee, attack 1 ms, release 30 ms). Un kick lungo abbassa a lungo; un input in mute non abbassa; vengono abbassati anche i send delle tracce. Un kick normale comincia ad abbassare a circa -10 dB e abbassa forte da -25 dB in giù, quindi quasi tutto il lavoro si fa nei tre quarti di sinistra del knob; a -INF tutto ciò che suona l'input abbassa il più possibile per il compressore. Un punto rosso indica una traccia con il duck, un punto viola l'input; il punto segue il duck reale (con il knob tutto a destra niente viene abbassato e il punto resta scuro). Un solo sidechain per tutte le tracce, salvato con il preset (un preset fatto con una versione precedente conserva i suoi numeri ma suona diverso: il vecchio valore era una profondità, ora è un threshold).

### SEL, ENV, LFO, EDIT (le pagine della voce)

Tutte e quattro sono pagine di traccia: p-lock e chance funzionano qui.

| Pulsante | Knob 1 · 2 · 3 · 4 | Anche in questa pagina |
|---|---|---|
| **SEL** PITCH MOD | SHAPE (DECAY, SINE, RANDOM) · AMOUNT (64 = nessuno) · RATE · RANDOM ALL | FUNC + knob 4 (il pulsante viola): un nuovo suono casuale per TUTTE le 8 tracce, sicuro come il casuale di una traccia (livello e pan restano, la distorsione non arriva mai al massimo). Il knob 4 da solo mostra solo un promemoria. Nessun annulla. |
| **ENV** OSCILLATOR | WAVE (SINE, TRI, SAW) · FREQUENCY (20 Hz-20 kHz) · ATTACK · DECAY | OCT- / OCT+ spostano la frequenza di un'ottava. |
| **LFO** NOISE FILTER | MODE (LP, BP, HP) · Q · FREQUENCY · - | |
| **EDIT** NOISE ENVELOPE | MODE (EXP, LIN, CLAP) · pulsante CHOKE · ATTACK · DECAY | {r:FUNC + knob 2 a destra}: la traccia entra nel gruppo choke; a sinistra: esce. Il knob 2 da solo mostra solo un messaggio. ALGORITHM a destra: la schermata CHOKE. {r:FUNC, tocco su EDIT}: DELETE AUTOMATION? (OCT+ sì, OCT- no). |
| **CHOKE** (EDIT, ALGORITHM a destra) | - | Tasti neri da 1 a 8: blu = nel gruppo. Un colpo di una traccia del gruppo ferma subito le altre, come un hi-hat chiuso ferma uno aperto. ALGORITHM a sinistra: indietro. Salvato con il preset. |

DELETE AUTOMATION elimina i p-lock e il movimento registrato della traccia selezionata nel pattern in riproduzione (le altre tracce, gli altri pattern e le chance restano; l'undo funziona). Tenere FUNC + EDIT e poi premere ACCENT, FILL o PLAY è la copia della pagina, non una cancellazione.

### GLO

Pressione = VEL → MIXER → COMP → MEMORY → SETTINGS → VEL.

| Pagina | Knob 1 · 2 · 3 · 4 | Anche in questa pagina |
|---|---|---|
| **VEL** | OSC · NOISE · PITCH MOD (quanto la velocity muove ciascuno) · RANDOM | {r:FUNC + knob 4}: la traccia selezionata riceve un nuovo suono casuale (solo le 21 impostazioni del suono). Il knob 4 da solo mostra solo un promemoria. Pagina di traccia: i p-lock funzionano. |
| **MIXER** | LEVEL delle quattro tracce mostrate | Otto slider alti; SELECT passa dalle tracce da 1 a 4 a quelle da 5 a 8 e, come dice il suggerimento blu sullo schermo, alterna level / pan (pan al centro = rosso). Un colpo accende il suo slider. I tasti sono quelli di PERFORM: {r:FUNC tocco}: modo MUTE, o riattiva tutto; {o:FILL tocco}: modo SOLO, o cancella tutti i solo; i tasti neri da 1 a 8 mettono quella traccia in mute (modo MUTE) o in solo (modo SOLO), e una traccia che il solo zittisce è grigia; tasti bianchi da 1 a 16: pattern successivo (non in modo song). Un punto rosso sotto uno slider = con duck, viola = input del sidechain. |
| **COMP** | ATTACK · RELEASE · THRESHOLD · MAKE-UP | Compressore master, ratio 4:1, un ago mostra la riduzione di guadagno. Default: niente compresso (0.0 dB / 0.0 dB). Si conserva solo con SAVE. |
| **MEMORY** | - | Una batteria mostra quanto è piena la memoria del progetto (pattern, song, movimento, note ARP). Viola dal 90 %, FULL al 100 %: a quel punto un salvataggio viene rifiutato. |
| **SETTINGS** | lista (SELECT sposta) | Righe: clock out, notes out, canale MIDI del Tonic, output level, USB audio, key light, 4/4 LED, colour theme, zoom knob (sì / no), Tonic Engine, name projects, save, autosave, clear pattern, factory reset, performance, cores, about, battery. Quelle meno ovvie sono spiegate subito sotto. Le modifiche si conservano da sole circa un secondo e mezzo dopo che smetti di toccare (il sequencer deve essere fermo). |

#### Impostazioni GLO: quelle meno ovvie

Le impostazioni sono nella lista GLO (SELECT sposta su e giù, ALGORITHM cambia il valore). I default sono in grassetto.

| Impostazione | Scelte | Che cosa fa |
|---|---|---|
| **OUTPUT** | NORMAL / **LOUD** | LOUD (l'impostazione di fabbrica) rende le uscite dell'FM-1 più forti di 6 dB (il fondo scala del codec). NORMAL conserva 6 dB di headroom: usalo se il suono è troppo forte e tieni d'occhio il compressore e il limiter. |
| **USB AUDIO** | **FOLLOWS KNOB** / FULL | FOLLOWS KNOB: il computer riceve ciò che sentono le cuffie, dopo il knob MASTER. FULL: il computer riceve sempre il livello pieno, e MASTER cambia solo le uscite dell'FM-1 (speaker, cuffie, line out). Usa FULL per registrare a livello pieno e ascoltare comunque piano. |
| **GLOBAL ACCENT** | **+3**, da +1 a +10 | Quanto è forte l'accento della riga di accento TOTALE (la traccia AC su SEQ): +10 = pieno (ogni traccia che suona su quel passo va a velocity massima), +3 = tre decimi dell'aumento rispetto a un passo normale, +1 = un decimo. L'accento proprio di una traccia resta pieno. Salvato con il progetto. |
| **4/4 LED** | OFF / **ON** | ON (l'impostazione di fabbrica): i tasti bianchi 1, 5, 9 e 13 brillano appena per segnare i quattro movimenti della battuta (non su PERFORM). |
| **ZOOM KNOB** | **YES** / NO | YES: il knob che giri viene ingrandito sullo schermo. NO: viene solo evidenziato, così l'intera pagina resta visibile. La schermata SONG non fa mai zoom. |
| **NAME PROJECTS** | **GENERATE** / MANUAL | GENERATE inventa il nome di un progetto salvato (ALGORITHM = un altro). MANUAL: la schermata del nome è un editor di lettere (vedi il capitolo 5). |
| **TONIC ENGINE** | **ENHANCED** / CLASSIC | CLASSIC mette in grigio e silenzia gli extra di FM1 TONIC (cut di traccia, send, sidechain) così il suono usa solo le impostazioni che può portare un file `.mtpreset` (capitoli 2 e 8). |
| **2ND CORE** | **ON** / OFF | Il chip dell'FM-1 ha due core e Tonic divide tra loro il calcolo del suono. Ha effetto alla prossima accensione. Se l'FM-1 va in crash con due core, si porta da solo su OFF; rimettilo su ON qui. |

### SEQ e ARP

| Pagina | Knob 1 · 2 · 3 · 4 | Anche in questa pagina |
|---|---|---|
| **SEQ** (1ª pressione) | FIXED LEN (8, 16, 32, 64) · AUTOSCROLL · FREE LEN (da 1 a 64) · SWING | Tasti bianchi: gli step. SELECT: le pagine di step (16 alla volta; AUTOSCROLL spento): la pagina (1-16, 17-32 ...) appare grande sul terzo knob per un attimo, poi il knob torna FREE LEN. La pagina non resta mai su step che non esistono: se il loop si accorcia sotto la pagina in cui eri, torna all'ultima pagina che la lunghezza ha. ALGORITHM: la voce (da 1 a 8, poi AC = l'accent globale). Tieni uno step + ACCENT / FILL: flag (accent = blu scuro, fill = giallo, entrambi = verde). {r:FUNC + SEQ}: modo song acceso / spento (conferma con OCT+). {o:FILL + SEQ}: la schermata SONG. |
| **PATTERN** (SEQ, 2ª pressione) | SONG ON / OFF | Tasti neri da 1 a 8 mute, tasti bianchi da 1 a 16 scelgono il pattern successivo (non in modo song). Un tasto scuro = pattern vuoto, luce bassa = ha delle note, lampeggio lento = quello attuale, lampeggio veloce = il prossimo (in coda). {r:FUNC tocco} e {o:FILL tocco} come su PERFORM. SEQ di nuovo: indietro. |
| **ARP** | FIXED LEN · OCTAVE (anche OCT- / OCT+, da -3 a +3) · PAGE (1-16, 17-32 ...) · - | La schermata SEQ, ma ogni tasto (bianco e nero) suona la voce selezionata come una tastiera. I tasti C (fiochi) suonano l'altezza propria della voce all'ottava 0. ALGORITHM: la voce. SELECT: le pagine di step, mostrate sul terzo knob. Non si registra nulla. SEQ: indietro. {r:FUNC + ARP}: cancella la song (DELETE SONG?). Tasto nero 11 tenuto + PLAY: cancella il pattern (CLEAR PATTERN?), PLAY non parte allora. |

Allungare un pattern copia ciò che c'è nella nuova parte (da 8 a 16 copia gli step da 1 a 8 negli step da 9 a 16); accorciarlo lo accorcia soltanto.

### PLAY, REC, SAVE

| Pulsante | Che cosa fa |
|---|---|
| **PLAY** | Avvia e ferma. {r:FUNC + PLAY}: cancella il pattern (chiede conferma). |
| **REC** | REC poi PLAY: registrazione dal vivo. Ogni tasto nero da 1 a 8 scrive un trig della sua traccia sullo step più vicino; ogni knob del suono che giri viene registrato come automazione. Non su PERFORM e sulla pagina dei pattern. {r:FUNC + REC}: QUANTIZED REC (default) / UNQUANTIZED REC (il colpo conserva il suo tempo esatto, dentro lo step); durante la riproduzione accende anche la registrazione. |
| **SAVE** | Vedi il capitolo 5. {r:FUNC + SAVE}: salvataggio rapido. {r:FUNC + HOME}: rinomina. |

### Combinazioni di pulsanti

Tutte le combinazioni di pulsanti e tasti, raggruppate per il pulsante che tieni premuto per primo. Tasto nero 9 = ACCENT, 10 = FILL, 11 = FUNC. {r:Rosso} = FUNC tenuto, {o:arancione} = FILL tenuto. L'ultima colonna dice dove funziona: «Ogni pagina» = tutte le pagine tranne ARP e le due pagine PERFORM.

**FUNC tenuto (tasto nero 11)**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| {r:FUNC + tasto nero da 1 a 8} | Mette la traccia in mute (di nuovo = riattiva). Agisce quando rilasci il tasto. | «Ogni pagina» |
| {r:FUNC + tasto bianco da 1 a 16} | Passa a quel pattern (durante la riproduzione: alla fine della battuta). | «Ogni pagina»; non nella schermata SONG né in modo song (decide la song) |
| {r:FUNC + ACCENT} | Copia il pattern. Lo schermo dice PRESS PATTERN TO PASTE / PRESS SEQ TO EXIT, ogni tasto pattern lampeggia (un pattern vuoto più debole) e SEQ lampeggia: premi un tasto pattern e la copia finisce lì, quel pattern diventa quello attuale. Tocca FUNC da solo prima di scegliere per restare nel modo e incollare più volte. SEQ esce dal modo e apre la pagina del sequencer; anche {r:FUNC + ACCENT} di nuovo o qualsiasi altro pulsante lo chiude. | «Ogni pagina» |
| {r:FUNC + FILL} | Incolla il pattern copiato quando rilasci FILL. Chiede PASTE PATTERN? (OCT+ = sì, OCT- = no). | «Ogni pagina» |
| {r:FUNC + FILL tenuto + tasto bianco} | Incolla subito il pattern copiato in quel posto (senza domanda). | «Ogni pagina» |
| {r:FUNC + PLAY} | Cancella il pattern (chiede CLEAR PATTERN?). Non avvia la musica. | «Ogni pagina», anche ARP (lì tieni il tasto nero 11, un tasto di tastiera su ARP: PLAY non avvia / ferma finché è tenuto) |
| {r:FUNC + tasto nero da 1 a 8 tenuto, poi ACCENT} | Copia il suono di quella traccia (non i suoi step). | «Ogni pagina» |
| {r:FUNC + tasto nero da 1 a 8 tenuto, poi FILL} | Incolla il suono copiato in quella traccia (anche mute, volume e pan; la traccia non viene messa in mute). Nessuna domanda. | «Ogni pagina» |
| {r:FUNC + tasto nero da 1 a 8 tenuto, poi PLAY} | Cancella il pattern di quella traccia (chiede CLEAR TRACK PATTERN?). | «Ogni pagina» |
| {r:FUNC + EDIT o LFO tenuto, poi ACCENT} | Copia questa pagina della traccia selezionata (solo i suoi parametri). | Pagine con parametri di traccia (non GLO, HOME, SEQ) |
| {r:FUNC + EDIT o LFO tenuto, poi FILL} | La incolla nella traccia selezionata, nella stessa pagina. | Pagine con parametri di traccia (non GLO, HOME, SEQ) |
| {r:FUNC + EDIT o LFO tenuto, poi PLAY} | Riporta questa pagina della traccia ai valori all'accensione. Nessuna domanda; l'undo la ripristina. | Pagine con parametri di traccia (non GLO, HOME, SEQ) |
| {r:FUNC + EDIT}, premuto da solo | DELETE AUTOMATION?: elimina i p-lock e il movimento registrato della traccia selezionata nel pattern in riproduzione (chiede conferma). | «Ogni pagina» |
| {r:FUNC + SEQ} | Modo song acceso / spento (chiede conferma: OCT+ = sì). | «Ogni pagina» |
| {r:FUNC + ARP} | DELETE SONG?: cancella l'intera song e esce dal modo song (chiede conferma). | «Ogni pagina» |
| {r:FUNC + HOME} | Rinomina il progetto e lo salva nello stesso posto. | «Ogni pagina» |
| {r:FUNC + SAVE} | Salvataggio rapido nel posto caricato. Nessuna domanda. | «Ogni pagina» |
| {r:FUNC + REC} | Alterna QUANTIZED REC / UNQUANTIZED REC. Durante la riproduzione accende anche la registrazione. | «Ogni pagina» |
| {r:FUNC + OCT- / OCT+} | Niente. | |
| {r:FUNC + knob 2 (gira a destra / a sinistra)} | La traccia selezionata entra nel / esce dal gruppo choke. | Solo EDIT, pagina NOISE ENVELOPE |
| {r:FUNC + knob 4} | Un nuovo suono casuale per la traccia selezionata. | Solo GLO > VEL |
| {r:FUNC + knob 4} | Un nuovo suono casuale per TUTTE le tracce. | Solo SEL (PITCH MOD) |

**FILL tenuto (tasto nero 10)**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| {o:FILL + tasto nero da 1 a 8} | Mette quella traccia in solo (un altro tasto sposta il solo, lo stesso tasto lo termina). | «Ogni pagina» |
| {o:FILL + SEQ} | Torna alla schermata SONG da qualsiasi pagina (in modo song; altrimenti NOT IN SONG MODE). | «Ogni pagina» |
| {o:FILL}, premuto e rilasciato da solo | Cancella la battuta sul cursore (senza domanda). | Solo la schermata SONG |
| {o:FILL} + un tasto bianco tenuto (o prima FILL, poi i tasti bianchi) | Quello step riceve un fill (la traccia suona 2x, 4x o 8x su di esso); di nuovo = spento. | Pagine dove i tasti bianchi sono step |
| {o:FILL} tenuto | I trig della traccia selezionata che hanno un fill lampeggiano sui tasti bianchi (gli altri trig restano accesi). | Pagine dove i tasti bianchi sono step |
| {r:FUNC + FILL} | Incolla. Vedi l'elenco FUNC qui sopra. | |

**ACCENT tenuto (tasto nero 9)**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| Tasto bianco tenuto + ACCENT (o prima ACCENT, poi i tasti bianchi) | Quello step riceve un accent (forte); di nuovo = spento. | Pagine dove i tasti bianchi sono step |
| ACCENT tenuto | I trig della traccia selezionata che hanno un accent lampeggiano sui tasti bianchi (gli altri trig restano accesi). | Pagine dove i tasti bianchi sono step |
| {r:FUNC + ACCENT} | Copia. Vedi l'elenco FUNC qui sopra. | |

**GLO tenuto**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| GLO + ACCENT | Annulla. | «Ogni pagina», anche ARP |
| GLO + FILL | Ripeti. | «Ogni pagina», anche ARP |

**HOME tenuto**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| HOME + SELECT | Il tempo. | «Ogni pagina» |
| HOME tenuto da solo (mezzo secondo) | La pagina DELAY. | «Ogni pagina» |
| HOME + tasto bianco | Niente. | |

**FX tenuto**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| FX tenuto da solo (mezzo secondo) | La pagina REVERB. | «Ogni pagina» |

**ENV tenuto**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| ENV + un knob | Quel knob torna al valore all'accensione. | Pagine con knob |
| ENV + tasto bianco | L'accent di quello step acceso / spento. | Pagine dove i tasti bianchi sono step |
| ENV + tasto nero da 1 a 8 | Suona quella traccia forte. In registrazione: lo step riceve un accent. | «Ogni pagina» |

**SAVE tenuto**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| SAVE + un knob | Il movimento registrato di quel knob viene dimenticato. | Pagine con knob |
| Tasto bianco tenuto + SAVE + lo stesso knob | Toglie il p-lock di quel knob su quello step. | Pagine del suono |

**Un tasto bianco tenuto (uno step)**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| Tasto bianco tenuto + knob da 1 a 4 | **P-lock**: quello step riceve un suo valore del knob (3 knob con lock per traccia e pattern). | Pagine del suono: MIX 1, SEL, ENV, LFO, EDIT, VEL |
| Tasto bianco tenuto + SELECT | La chance di quello step: da 5 a 100 %, poi 1:2, 2:2, 1:3 ... 8:8. | «Ogni pagina» tranne PERFORM, HOME, SONG, COMP e le liste |
| Tasto bianco tenuto + ACCENT / FILL | Flag di accent / fill. | Vedi ACCENT e FILL qui sopra |

**Trasporto e altro**

| Tasti | Che cosa fa | Dove |
|---|---|---|
| REC + PLAY, poi tasti e knob | Registrazione dal vivo: i tasti neri da 1 a 8 scrivono trig, i knob scrivono automazione. | Non su PERFORM e sulla pagina dei pattern |
| OCT- / OCT+ | L'altezza della traccia selezionata un'ottava sotto / sopra. In una domanda: NO / YES. Su ARP: l'ottava della tastiera. Nella schermata SONG: OCT- cancella la riga, OCT+ aggiunge una riga. | «Ogni pagina» |
| Un pulsante tenuto da solo (circa un secondo) | Una scheda dice che cosa fa il pulsante in questa pagina, combinazioni comprese. | Ogni pulsante tranne FX e HOME |

## 4. Suonare

- **Step.** Scegli una traccia, premi un tasto bianco per mettere uno step e premilo di nuovo per toglierlo. **Accent** (forte) e **fill** (la traccia suona quattro volte durante quello step): tieni il tasto bianco e premi ACCENT e / o FILL (di nuovo = flag tolto), oppure tieni prima ACCENT / FILL e premi i tasti bianchi.
- **Pattern.** {r:FUNC + tasto bianco} cambia pattern. {r:FUNC + ACCENT} copia, {r:FUNC + FILL} incolla (tieni FILL e premi un tasto bianco per incollare in quel posto), {r:FUNC + PLAY} cancella. Incollare il pattern chiede **PASTE PATTERN?** (OCT+ = sì, OCT- = no) e cancellare chiede CLEAR PATTERN?. Con in più un tasto nero da 1 a 8 tenuto, copiano, incollano e cancellano solo quella traccia; incollare il suono di una traccia o una pagina, incollare con FILL e un tasto bianco, e il modo copia-incolla al volo incollano subito, senza domanda.
- **Copia-incolla al volo.** Dopo {r:FUNC + ACCENT} tutto lo schermo dice **PRESS PATTERN TO PASTE** e **PRESS SEQ TO EXIT** in grandi lettere blu. Tutti i tasti dei pattern lampeggiano (un pattern con note lampeggia forte, un pattern VUOTO lampeggia più piano, quello copiato resta acceso) e lampeggia anche il pulsante **SEQ**. Premi un tasto pattern e la copia si incolla lì, quel pattern diventa quello attuale e il modo finisce. Per incollare in molti posti, **tocca FUNC da solo** prima di scegliere: il modo resta e il pattern attuale non cambia. Per uscire dal modo premi **SEQ**: apre la **pagina del sequencer**, non la pagina dei pattern (la pressione successiva di SEQ torna normale). Anche qualsiasi altro pulsante esce, e anche {r:FUNC + ACCENT} di nuovo (lo schermo dice COPY MODE OFF). Tieni {r:FUNC + ACCENT} e premi un tasto bianco = quel pattern è la copia. Non disponibile su PERFORM e sulla pagina dei pattern, dove FUNC è il modo MUTE.
- **Mute e solo.** {r:FUNC + tasto nero da 1 a 8} mette in mute (di nuovo = unmute). {o:FILL + tasto nero da 1 a 8} mette in solo quella traccia (un altro tasto sposta il solo, lo stesso tasto lo termina). I solo non si salvano e finiscono quando si carica un preset. Non su ARP.
- **P-lock e chance.** Tieni un tasto bianco e gira un knob in una pagina della voce: quello step riceve un suo valore, tutti gli altri step conservano il suono. Durante la riproduzione lo senti alla prossima volta che lo step arriva. Per toglierne uno: tieni lo step, tieni SAVE, gira lo stesso knob. Una traccia conserva 3 knob con lock per pattern (lo schermo dice MAX 3 PER TRACK, MOTION FULL quando non c'è più posto). Tieni uno step e gira SELECT: la sua chance, che scende dal 100 % a passi del 5 %, poi 1:2, 2:2, 1:3 ... 8:8. Mentre giri, la chance occupa tutto lo schermo (TRIG % e lo step), grande, per leggerla subito.
- **Catene.** Un pattern può avere il flag di passare al successivo quando finisce (dopo il pattern 8 viene l'1). Un flag conta solo su un pattern con note. Un preset con catene si carica suonando attraverso di esse (SONG ON); il knob 1 della pagina dei pattern di SEQ lo spegne e il pattern va in loop.
- **Modo song.** {r:FUNC + SEQ}, conferma con OCT+ (OCT- = indietro). Se non c'è ancora una song, si apre la schermata PATTERNS: i tasti bianchi da 1 a 16 aggiungono pattern, OCT- (oppure FUNC + un tasto bianco) toglie l'ultimo, OCT+ è OK. PLAY suona una battuta per pattern, in loop. Lo stesso gesto esce dal modo song. Nella **schermata SONG** (una riga per battuta) knob 1 = la battuta, knob 2 = il suo pattern, knob 3 = le ripetizioni (da x1 a x8, poi LOOP = finché non fermi); girare ALGORITHM sposta la battuta come il knob 1; i tasti bianchi scrivono un pattern nella battuta sul cursore e avanzano (acceso = il pattern ha note, spento = vuoto, quello della battuta sul cursore lampeggia); su PERFORM un riquadro viola che lampeggia dice PATTERN n = il pattern che la song suona alla fine di questa battuta (i tasti bianchi non fanno nulla in modo song); OCT+ aggiunge una riga dopo il cursore; OCT- oppure FILL (premuto e lasciato) cancella subito la riga (cancellata l'ultima battuta = la song è cancellata e il modo song finisce). {r:FUNC + ARP} cancella l'intera song.

#### Promemoria del modo song

| COSA | PULSANTI |
|---|---|
| **MODO SONG: ENTRA** | {r:FUNC + SEQ} (poi OCT+ = sì) |
| **MODO SONG: ESCI** | {r:FUNC + SEQ} di nuovo (poi OCT+ = sì) |
| **MODO SONG: MODIFICA** | {o:FILL + SEQ} |
| **IN MODIFICA SONG: AGGIUNGI UNA RIGA** | **OCT+** |
| **IN MODIFICA SONG: CANCELLA UNA RIGA** | **OCT-** |
| **MODO SONG: CANCELLA LA SONG** | {r:FUNC + ARP} (poi OCT+ = sì) |

## 5. Preset e salvataggio

**I preset funzionano nei due sensi.** L'FM-1 carica preset `.mtpreset` da **Patternarium** e da **Microtonic**, e i preset che crei sull'FM1 TONIC si possono esportare di nuovo in `.mtpreset` e usare nel **Microtonic VST**. È stato provato: i preset esportati si aprono e suonano in Microtonic. Il suono è molto, molto vicino all'originale, non identico. Importazione ed esportazione sono nella pagina web (la pagina di backup: "Restore my presets" e "Export my presets as .mtpreset"). Gli slot da 1 a 8 non si possono esportare. Ciò che FM1 TONIC ha aggiunto (note ARP, colpi in ritardo, cut e send di traccia, compressore, reverb, delay, mute) non ha posto in un `.mtpreset` e viene lasciato fuori.

Gira **PRESETS** tra i 40 posti: da 01 a 08 sono preset di fabbrica, da 09 a 40 sono tuoi (un posto vuoto si carica con i suoni di un preset di fabbrica e senza pattern: 09 suona come 01, 10 come 02, e così via a rotazione). Prima viene una domanda: OCT+ lo carica, OCT- tiene il tuo suono. Il caricamento viene rifiutato durante la riproduzione.

- **SAVE** chiede SAVE HERE? con il posto caricato. OCT+ salva lì e conserva il nome. ALGORITHM sceglie un altro posto (da 1 a 40; l'ambra mostra chi verrebbe sostituito). OCT- va a CHOOSE SLOT, poi alla schermata dello slot, poi alla schermata del nome (ALGORITHM = un altro nome inventato; OCT+ salva). HOME esce senza salvare. Se dall'accensione non è stato caricato nulla, SAVE parte da CHOOSE SLOT.
- {r:FUNC + SAVE} salva rapidamente nel posto caricato senza chiedere uno slot, anche durante la riproduzione. Un posto vuoto si salva subito come UNTITLED 01, 02 ...; uno pieno chiede OVERWRITE SLOT? (OCT+ sì).
- {r:FUNC + HOME} rinomina il progetto caricato (ALGORITHM = un altro nome, OCT+ salva, OCT- o HOME annulla). Con NAME PROJECTS su MANUAL (impostazioni GLO) la schermata del nome diventa un editor di lettere: il knob 4 scorre i caratteri, OCT+ (lo schermo dice OK) prende la lettera, OCT- cancella l'ultima, poi premi SAVE (8 lettere al massimo; lo schermo mostra PRESS SAVE e TURN KNOB 4 in lettere grandi).
- **Che cosa viene salvato.** Con AUTOSAVE su ON, il tuo progetto di lavoro viene salvato da solo 4 secondi dopo che smetti di toccare, tranne il compressore, il reverb e il delay: questi si conservano solo con SAVE, dentro il preset. PERFORM parte sempre pulito all'accensione (D/W 0 %, master low cut 20 Hz, ALL FX acceso). AUTOSAVE è SPENTO di fabbrica: premi SAVE (o FUNC + SAVE) per tenere il tuo lavoro. I settings si salvano comunque da soli. Metti AUTOSAVE su ON in GLO > SETTINGS se vuoi che il progetto si salvi da solo. Settings di fabbrica: clock out e notes out off, canale MIDI del Tonic 10, canale MIDI remoto off, CC mode off (CC CH BASE 1-8), output LOUD, USB audio follows knob, 4/4 LED on, tema TONIC, zoom knob yes, name projects GENERATE, Tonic Engine ENHANCED, 2nd core ON. Un'unità nuova o un FACTORY RESET li riceve; un'unità che ha già un progetto salvato tiene i propri settings.

## 6. Luci, MIDI e USB

- **Luci.** PLAY si accende a ogni 4º step (rosso durante la registrazione). REC: spento quando non registra; fisso da fermo (quantized) e lampeggiante a tempo durante la riproduzione; due lampeggi veloci e uno lento per unquantized. Il pulsante della pagina attuale è acceso. I tasti bianchi mostrano gli step della traccia selezionata (lo step in riproduzione si inverte); 1, 5, 9 e 13 possono brillare appena (GLO > 4/4 LED). Tasti neri da 1 a 8: la traccia selezionata è luminosa (lampeggia se è in mute), le tracce con note sono fioche e, durante la riproduzione, il tasto di una traccia lampeggia a ogni suo colpo (non su ARP, PERFORM, la pagina dei pattern e il mixer di GLO, dove i tasti neri mostrano mute e solo e restano fissi). Con FUNC tenuto i tasti mostrano quali pattern e quali tracce hanno note; il tasto del pattern in riproduzione lampeggia lentamente, quello di una traccia in mute più in fretta e quello di una traccia in solo il più in fretta (a tempo di orologio, così restano regolari qualunque cosa faccia lo schermo). Nella schermata SONG i tasti bianchi mostrano i pattern (acceso = ha note, spento = vuoto).
- **Batteria.** L'angolo in alto a destra dello schermo mostra la batteria in tre livelli: 3 segmenti verdi = piena, 2 ambra, 1 = un segmento rosso che lampeggia (quasi scarica); 3 segmenti blu = alimentata via USB. Il livello scende subito e risale solo dopo essere rimasto più alto per 3 secondi. Quando scende al livello 1 compare un messaggio a schermo intero **LOW BATTERY (PLUG YOUR DEVICE)**: collega l'alimentazione, oppure premi **HOME** per chiuderlo. La musica continua sotto, e finché non premi HOME tasti, pulsanti e knob non vengono ascoltati. Ritorna solo dopo che la batteria è stata di nuovo più alta (o su USB) e scende ancora al livello 1.
- **MIDI.** Via USB o dal TRS MIDI IN, il canale 10 di default suona le voci con le note di batteria General MIDI nell'ordine delle voci: 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51 (velocity = volume). Un clock in ingresso viene seguito; Start, Stop e Continue comandano il pattern. Impostazioni GLO: TONIC MIDI CH (oppure OFF), REMOTE MIDI CH (oppure OFF, vedi sotto), CC MODE e CC CH BASE (control change, vedi sotto), CLOCK OUT (clock e start / stop), NOTES OUT (il pattern come note).
- **Pannello remoto.** GLO > REMOTE MIDI CH (OFF di default) fa sì che un secondo canale MIDI suoni i tasti propri dell'FM-1. I 27 tasti del pannello sono le 27 note da F3 a G5 (note MIDI da 53 a 79, il do centrale = 60), disposte come una tastiera di pianoforte: il primo tasto, F3, è la nota 53, l'ultimo, G5, è la nota 79. Un note on preme il tasto, un note off lo lascia, quindi una nota lunga tiene il tasto premuto. Tutto ciò che fa un tasto sul pannello funziona allo stesso modo: i tasti neri da 1 a 8 selezionano o mettono in mute le tracce, i tasti bianchi da 1 a 16 sono gli step o i pattern, e i tasti neri 9, 10 e 11 sono ACCENT (nota 73), FILL (nota 75) e FUNC (nota 78). Un accordo va bene in qualunque ordine di note: ACCENT, FILL e FUNC vengono sempre applicati per primi, quindi FUNC + ACCENT + un tasto bianco funziona quando le tre note arrivano insieme. All Notes Off (controller 123) lascia ogni tasto. Le note sul canale remoto non suonano le voci; dagli un canale diverso da TONIC MIDI CH. Le note fuori da 53 a 79 vengono ignorate.

| Tasto | Note |
| --- | --- |
| Tasti bianchi da 1 a 16 | 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 |
| Tasti neri da 1 a 8 (tracce) | 54, 56, 58, 61, 63, 66, 68, 70 |
| Tasto nero 9 = ACCENT, 10 = FILL, 11 = FUNC | 73, 75, 78 |

### Control changes (CC)

Un control change gira un knob dall'esterno: il numero CC è il **parametro** (gli stessi numeri per ogni traccia) e la **traccia** si sceglie in uno di due modi. GLO > **CC MODE** decide (OFF di default: nulla reagisce ai CC finché non lo accendi):

| CC MODE | Che cosa fa |
| --- | --- |
| OFF | Ogni control change viene ignorato. |
| SELECTED | Un CC sul canale del Tonic (TONIC MIDI CH) muove il parametro della traccia **selezionata**. |
| CHANNELS | Un blocco di 8 canali, uno per traccia: il primo canale di GLO > **CC CH BASE** è la traccia 1, il successivo la traccia 2, e così via (base 1 = canali 1-8). Qualsiasi traccia si può muovere in qualsiasi momento, senza doverla selezionare prima. |
| BOTH | Tutti e due insieme. Se il canale del Tonic è dentro il blocco, su quel canale vince il blocco. |

Le note continuano a suonare le voci su TONIC MIDI CH e il pannello remoto tiene il suo canale, quindi dai al blocco canali che quei due non usano (il blocco di default 1-8 lascia liberi 10 e 11). Un CC muove il valore come il knob, quindi lo schermo, SAVE e l'autosave lo seguono; non apre il quadrante grande, non blocca uno step e non viene registrato. Un CC 0-127 è lo 0-127 del knob; un interruttore a tre posizioni (forma d'onda, modo del filtro, modo dell'envelope) divide l'intervallo in tre. Nell'engine CLASSIC gli extra (low cut, high cut, reverb send, delay send) ignorano i CC come i loro knob in grigio.

| CC | Parametro | CC | Parametro |
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

### Collegare un Digitakt o un Digitone

La presa USB dell'FM-1 e quella del Digitakt o del Digitone sono entrambe prese "device", quindi non possono parlarsi. Usa il MIDI OUT (a 5 poli) del Digitakt o del Digitone nel TRS MIDI IN dell'FM-1, con un adattatore MIDI da 5 poli a TRS da 3,5 mm (tipo A o B: se non arriva nulla, prova l'altro tipo).

1. Sull'FM-1: impostazioni GLO, TONIC MIDI CH = 10 (le voci) e REMOTE MIDI CH = 11 (il pannello), oppure due canali qualsiasi diversi.
2. Sul Digitakt o Digitone: usa due tracce MIDI e imposta i loro canali su quei due numeri. La prima suona le voci con le note di batteria (36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51). La seconda è il pannello remoto: disegna le note della tabella qui sopra nella sua griglia di step o nel piano roll.
3. Una traccia MIDI suona fino a 4 note in uno step, con una sola lunghezza per tutte: la lunghezza della nota è per quanto tempo i tasti restano tenuti, quindi una nota lunga per FUNC con altre note su step successivi è FUNC + quei tasti, e un accordo di FUNC + ACCENT + un tasto bianco è un solo step.
4. Per muovere il suono dalle corsie CC del Digitakt o del Digitone: imposta GLO > CC MODE = CHANNELS, fai una traccia MIDI per ogni traccia del Tonic sui canali da 1 a 8 (oppure da CC CH BASE) e assegna le sue corsie CC ai numeri della tabella qui sopra; ogni traccia MIDI ha solo un certo numero di corsie CC (16 sul Digitakt II, meno sul primo Digitakt), quindi scegli i parametri che vuoi automatizzare su ogni traccia.
5. Per seguire il tempo e lo start / stop del Digitakt o del Digitone, accendi il suo MIDI clock e il transport in uscita; l'FM-1 segue un clock in ingresso.

### Collegare una tastiera MIDI

L'FM-1 non può fare da host a una tastiera USB. Usa una tastiera con un MIDI OUT a 5 poli (con un adattatore da 5 poli a TRS nel TRS MIDI IN), oppure manda una tastiera USB attraverso un computer o un box host USB MIDI alla presa USB dell'FM-1.

1. Sull'FM-1: impostazioni GLO, REMOTE MIDI CH = il canale su cui la tastiera trasmette (l'1 è il più comune).
2. Suona le note da F3 a G5 (da 53 a 79). Tieni un tasto per FUNC (nota 78, F#5), ACCENT (nota 73, C#5) o FILL (nota 75, D#5) e premi gli altri: copia, incolla, cambio di pattern, mute e solo funzionano come sul pannello. Una tastiera da 49 tasti copre tutto il pannello; su una più piccola, spostala con i pulsanti dell'ottava.

- **USB audio.** Lo stesso cavo porta ciò che sentono le cuffie, dopo MASTER, come ingresso a 2 canali a 44,1 kHz, senza driver (GLO > USB AUDIO = FULL manda il livello pieno qualunque cosa dica MASTER). Il primo istante dopo che un'app lo apre può fare un click.

## 7. Installare, backup, ripristino

- **Provalo:** l'[emulatore](https://luigirossi-music.github.io/fm1-tonic/emu/) esegue lo stesso codice nel browser.
- **Installare o aggiornare:** apri l'[installer web](https://luigirossi-music.github.io/fm1-tonic/install/) in Chrome o Edge, collega l'FM-1 con un cavo USB dati, premi Install e non scollegarlo mentre scrive. Un altro firmware non conserva il tuo progetto: salvalo prima in un file. "Back to the stock firmware" in fondo alla stessa pagina rimette il firmware di M-VAVE. Installare un altro firmware è a tuo rischio.
- **Backup:** la [pagina di backup](https://luigirossi-music.github.io/fm1-tonic/backup/) salva il progetto in un file `.x0b` (Back up), lo ricarica (Restore: ferma l'FM-1, poi rispondi a RESTORE FROM USB? con SEL), esporta i preset come file `.mtpreset`, e fa il backup / ripristino degli slot da 9 a 40 come file `.tnp`. Da riga di comando: `python3 tools/x0x_backup.py backup my-song.x0b` (anche `restore`, `info`).
- **Se qualcosa va storto:** tieni premuti OCT- e OCT+ per cinque secondi = modo aggiornamento; tienili premuti mentre accendi = calibrazione dell'hardware. Dopo un crash una schermata rossa CRASH riavvia l'FM-1; se va in crash due volte all'avvio entra in SAFE MODE (niente suono, l'USB funziona: reinstalla dall'installer; PLAY prova un avvio normale). Se anche questo fallisce, lo script di rescue per Mac in `tools/fm1_rescue.sh` rimette il firmware di M-VAVE (controlla ogni file che scrive; rieseguirlo è sicuro).
- **Se un aggiornamento viene rifiutato.** Se l'FM-1 non vuole cambiare firmware (per esempio con "Back to the stock firmware"), la sua schermata di aggiornamento mostra FAILED, un numero di errore con un breve motivo (per esempio ERROR -5 HEAD DIFFERS) e il passo in cui si è fermato, per 8 secondi, poi l'FM-1 riavvia FM1 TONIC. ERROR -5 significa che l'area di avvio (l'"head") del tuo FM-1 non è quella del firmware verso cui vai. Per questo errore lo schermo mostra anche il primo byte diverso, quanti byte sono diversi e i due checksum (CRC), per 40 secondi. Annota questi numeri se chiedi aiuto.
- **Un bootloader personalizzato.** Se hai messo sull'FM-1 un bootloader personalizzato (per esempio m-bootloader, con M-Flasher), tornare dalla pagina web al firmware ufficiale di M-VAVE può essere rifiutato con ERROR -5 HEAD DIFFERS. Rimetti prima il bootloader originale di M-VAVE (in M-Flasher: FLASH BOOTLOADER, poi STOCK (FROM .FWSC) con il pacchetto ufficiale), poi torna al firmware originale. Sull'FM-1 dell'autore così ha funzionato di nuovo tutto.
- **FM-1 più recenti.** Alcuni FM-1 recenti hanno un altro chip flash (ID B36014 invece di 856014). FM1 TONIC li accetta entrambi, quindi salvataggio, preset, backup e ripristino funzionano anche su questi (non ancora provato su un'unità con il chip nuovo).
- **Testo completo** dei capitoli su installazione, backup, rescue e parametri: `docs/reference/MANUAL_full_wip381.md`.

## 8. L'engine

Ognuna delle 8 tracce è un sintetizzatore di batteria completo: l'engine di FM1 TONIC, niente è campionato. Le 8 tracce sono identiche e ognuna ha le stesse **26 impostazioni**, tenute per traccia e salvate con il preset. Questo capitolo è il riferimento per tutte. Dove sta un'impostazione è scritto come pulsante · knob.

```
pitch mod ─► OSCILLATOR ─┐
                         ├─► MIX ─► EQ ─► DIST ─► LEVEL ─┐
NOISE ─► filter ─► env ──┘                               │
          ┌──────────────────────────────────────────────┘
          └─► LOW CUT ─► HIGH CUT ─┬─► PAN ─► master
                                   └─► REVERB / DELAY SEND
```

Un colpo suona finché i suoi envelope si spengono. Un nuovo colpo sulla stessa traccia taglia quello che sta ancora suonando (le batterie sono monofoniche), e una traccia del choke group viene tagliata dalle altre (dissolvenza di 3 ms).

**I knob sono curve, non linee rette.** Frequenze, tempi e Q sono esponenziali: la metà del knob non è la metà del valore. Sullo schermo le frequenze compaiono in Hz o kHz, l'AMOUNT del pitch mod in semitoni (sm) e il pan come L / C / R, l'OSC/NOISE MIX come due percentuali (100.00/00.00 = solo oscillator, 00.00/100.00 = solo noise); ogni altro knob mostra un semplice numero da 0 a 127. Usa questa tabella per capire cosa significano quei numeri.

| Knob | 0 | 32 | 64 | 96 | 127 |
|---|---|---|---|---|---|
| **Frequency** (oscillator, noise filter, EQ) | 20 Hz | 114 Hz | 650 Hz | 3,7 kHz | 20 kHz |
| **Decay** (oscillator, noise, discesa del pitch mod) | 10 ms | 57 ms | 0,33 s | 1,8 s | 10 s |
| **Attack** (oscillator, noise) | 0 | 20 ms | 0,16 s | 1 s | 5 s |
| **Filter Q** | 0,1 | 1,8 | 33 | 600 | 10000 |

Un decay è il tempo che il suono impiega per cadere nel silenzio (60 dB più in basso).

### Oscillator: il tono

| Impostazione | Dove | Intervallo | Cosa fa |
|---|---|---|---|
| **WAVE** | ENV · knob 1 | SINE, TRI, SAW | SINE è puro e morbido, TRI cavo, SAW brillante e ronzante. |
| **FREQUENCY** | ENV · knob 2 | da 20 Hz a 20 kHz | L'altezza (pitch). Kick circa da 40 a 70 Hz, tom da 80 a 250 Hz, corpo del rullante da 150 a 250 Hz. OCT- / OCT+ la spostano di un'ottava. |
| **ATTACK** | ENV · knob 3 | da 0 a 5 s | 0 parte subito. Alzalo per un crescendo. Il primo quarto del knob è sotto i 20 ms. |
| **DECAY** | ENV · knob 4 | da 10 ms a 10 s | Quanto suona: corto = click o tic, lungo = boom. |

### Pitch mod: il pitch si muove mentre il suono suona

| Impostazione | Dove | Intervallo | Cosa fa |
|---|---|---|---|
| **SHAPE** | SEL · knob 1 | DECAY, SINE, RANDOM | Il tipo di movimento (qui sotto). |
| **AMOUNT** | SEL · knob 2 | 64 = nessuno; fino a 8 ottave per lato | Sopra 64 il pitch sale, sotto 64 scende. Con DECAY il suono parte così lontano e scivola fino al pitch impostato. |
| **RATE** | SEL · knob 3 | dipende da SHAPE | DECAY: quanto dura lo scivolamento, da 10 ms a 10 s (caduta del kick = corto, laser = lungo). SINE: la velocità dell'oscillazione, da 0 a 2000 Hz; solo i primi passi sono un vibrato, più in alto è FM (campane, zap), e un rate più alto lo fa anche durare di più. RANDOM: quanto in fretta il pitch vaga a caso, da 20 Hz a 20 kHz; lento = derive, veloce = suona metallico. |

### Noise: lo scatto e il sibilo

Il noise è rumore bianco che passa per un filtro e un envelope. L'oscillator e il noise sono separati: il MIX decide quanto senti di ciascuno.

| Impostazione | Dove | Intervallo | Cosa fa |
|---|---|---|---|
| **FILTER MODE** | LFO · knob 1 | LP, BP, HP | LP tiene i bassi (colpo sordo, scuro), HP gli alti (hi-hat, sfrigolio), BP una banda attorno alla frequenza. |
| **FILTER Q** | LFO · knob 2 | da 0,1 a 10000 | Risonanza. Bassa = dolce, intorno al knob 64 = un picco forte, alta = il filtro suona come un tono con altezza: BP con una Q alta e un envelope corto è un ping accordato (tom, cowbell, block). |
| **FILTER FREQUENCY** | LFO · knob 3 | da 20 Hz a 20 kHz | Dove lavora il filtro. |
| **ENVELOPE MODE** | EDIT · knob 1 | EXP, LIN, CLAP | EXP: una salita rapida e una caduta naturale, come l'oscillator. LIN: salita e caduta in linea retta. CLAP: poche raffiche brevi, come delle mani. |
| **ENVELOPE ATTACK** | EDIT · knob 3 | da 0 a 5 s | EXP e LIN: la salita. CLAP: quanto il noise resta a livello pieno prima che partano le raffiche. |
| **ENVELOPE DECAY** | EDIT · knob 4 | da 10 ms a 10 s | La caduta. In CLAP fissa anche la distanza tra le raffiche: corto = un ronzio veloce, lungo = battiti separati. |

### Mix, EQ, distorsione

| Impostazione | Dove | Intervallo | Cosa fa |
|---|---|---|---|
| **OSC/NOISE MIX** | FX · knob 1 | da 100.00/00.00 a 00.00/100.00 | Lo schermo mostra la parte di oscillator e la parte di noise in percentuale. 100.00/00.00 = solo oscillator, 49.61/50.39 (la metà del knob) = entrambi a livello pieno, 00.00/100.00 = solo noise. Una traccia nuova parte da 100.00/00.00: niente noise finché non lo muovi. |
| **EQ FREQUENCY** | FX · knob 2 | da 20 Hz a 20 kHz | Il centro dell'unica banda di EQ. |
| **DIST** | FX · knob 3 | da 0 a 127 | Distorsione, molto ripida: il primo quarto è un calore leggero, oltre la metà è pesante, in cima è un fuzz. È asimmetrica, quindi aggiunge volume e grinta. |
| **EQ GAIN** | FX · knob 4 | 64 = piatto; 40 dB per lato | Enfasi o taglio della banda (larga, una sola campana). L'EQ sta **prima** della distorsione, quindi enfatizzare e poi distorcere dà un suono più pesante dell'inverso. |

### Velocity: accent che cambiano il suono

Il sequencer suona uno step normale a circa un terzo della velocity massima e uno step con accent alla massima. I tre knob VELOCITY (GLO · VEL, knob da 1 a 3) dicono quanto ogni parte la segue.

| Impostazione | Dove | Cosa fa |
|---|---|---|
| **OSC** | GLO · knob 1 | 0 = l'oscillator ignora la velocity. Più alto = sugli step normali suona più piano, gli accent restano al massimo. |
| **NOISE** | GLO · knob 2 | Lo stesso per il noise. |
| **PITCH MOD** | GLO · knob 3 | Lo stesso per la quantità di pitch mod: sugli step normali il movimento si riduce. |

È ripida: un valore piccolo taglia già molto uno step normale, e da circa il knob 95 uno step normale perde del tutto quella parte. Con tutti e tre a 0, un accent suona esattamente come uno step normale: l'accent si sente solo attraverso questi knob. Per esempio, un rullante con VELOCITY OSC alto ha il suo tono solo sugli accent e suona solo noise sugli step normali.

### Output: level, pan, cut, send

| Impostazione | Dove | Intervallo | Cosa fa |
|---|---|---|---|
| **LEVEL** | HOME · knob 1 | dal silenzio a circa +10 dB (default 101 = circa -2,5 dB) | Il volume della traccia. La voce satura a fondo scala: se gracchia, abbassalo. |
| **PAN** | HOME · knob 2 | 0 sinistra, 64 centro, 127 destra | Posizione nel campo stereo. |
| **LOW CUT** | HOME · knob 3 | da 20 a 663 Hz | Toglie i bassi (12 dB per ottava). A 20 Hz non fa nulla di udibile. |
| **HIGH CUT** | HOME · knob 4 | da 20 kHz a 1 kHz | Toglie gli alti (12 dB per ottava). Tutto a destra = aperto. |
| **REVERB SEND** | HOME, 2ª pressione · knob 1 | da 0 a 127 | Quanto della traccia va al reverb. |
| **DELAY SEND** | HOME, 2ª pressione · knob 2 | da 0 a 127 | Lo stesso per il delay. |

I send escono dopo i cut e il sidechain, prima del pan, e funzionano qualunque cosa dicano il D/W di PERFORM e ALL FX.

### Attorno alla voce

Condiviso da tutte le tracce:

- **Choke group** (EDIT, ALGORITHM a destra): un colpo di una traccia del gruppo ferma subito le altre (l'hi-hat chiuso ferma quello aperto).
- **Sidechain** (HOME, 2ª pressione): una traccia di ingresso fa abbassare le tracce che scegli nella schermata DUCK mentre suona; è un compressore con ingresso laterale; il knob di threshold (SENDS, knob 3) va da 0.0 dB (tutto a destra = off) fino a -59 dB, poi -INF.
- **Low cut master** (PERFORM · knob 3, da 20 a 663 Hz): un low cut su tutto il mix, prima del compressore.
- **Compressore** (GLO · COMP): 4:1, soft knee, attack da 0,1 a 100 ms, release da 10 a 1500 ms, threshold da -48 a 0 dB, make-up da 0 a 24 dB. Un limiter dopo di lui tiene l'uscita sotto il fondo scala.
- **Reverb** (tieni FX): decay, tone, low cut, livello di return; una stanza granulosa dei primi tempi del digitale. **Delay** (tieni HOME): tempo sincronizzato (da 1/32 a 1/2), feedback, tone, TAPE (da digitale pulito a nastro consumato), low cut, return, ping-pong.

### Enhanced e classic

22 delle 26 impostazioni sono ciò che porta un preset di Patternarium (le 21 impostazioni di suono, level compreso, più il pan). Quattro sono **extra di FM1 TONIC**: LOW CUT, HIGH CUT, REVERB SEND e DELAY SEND, come il choke group, il sidechain, la sezione master, i p-lock, la chance e il microtiming. In **classic** (GLO > settings > TONIC ENGINE) i quattro extra sono in grigio e silenziosi, e anche il sidechain, così ciò che costruisci usa solo le impostazioni che porta un file `.mtpreset`; i valori tornano con enhanced. Un preset di Patternarium caricato conserva i suoi valori esatti finché non giri quel knob.

### Ricette di suoni

Sono punti di partenza: cambia un valore alla volta e ascolta.

#### Kick

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 20 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +37.04 sm | RATE 25 | - |
| **ENV** | WAVE SINE | FREQUENCY 56 Hz | ATTACK 0 | DECAY 72 |
| **LFO** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **EDIT** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

AMOUNT è la profondità della caduta e RATE la sua velocità. Per un click, metti MIX 40 su FX, poi su LFO: MODE HP, FREQUENCY 2.9 kHz, e su EDIT: DECAY 15. Per un boom più lungo, alza ENV DECAY.

#### Snare

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 15 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +12.85 sm | RATE 13 | - |
| **ENV** | WAVE TRI | FREQUENCY 196 Hz | ATTACK 0 | DECAY 46 |
| **LFO** | MODE HP | Q 22 | FREQUENCY 2.0 kHz | - |
| **EDIT** | MODE EXP | lascialo | ATTACK 0 | DECAY 55 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Più scatto: alza il MIX. Più corpo: abbassa il MIX. Per delle ghost note leggere, metti GLO NOISE a 30: gli step normali suonano il noise più piano e gli accent lo suonano pieno.

#### Hi-hat chiuso

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **ENV** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **LFO** | MODE HP | Q 22 | FREQUENCY 7.1 kHz | - |
| **EDIT** | MODE EXP | lascialo | ATTACK 0 | DECAY 25 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

OSC/NOISE MIX 00.00/100.00 significa solo noise. Abbassa la frequenza di LFO per un hi-hat più scuro.

#### Hi-hat aperto

Come l'hi-hat chiuso, con **EDIT DECAY 68**. Metti i due hi-hat in un **choke group** così quello chiuso ferma quello aperto: su ognuna delle due tracce tieni FUNC e gira il knob 2 di EDIT verso destra.

#### Clap

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **ENV** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.1 kHz | - |
| **EDIT** | MODE CLAP | lascialo | ATTACK 0 | DECAY 60 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Con questi valori ottieni da 3 a 4 raffiche rapide in circa un decimo di secondo. EDIT DECAY più basso = un ronzio più stretto, più alto = battiti più lenti e separati e una coda più lunga (da 75 a 85). Un po' di reverb send (HOME, 2ª pressione) ne fa un clap da stanza.

#### Tom

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +17.39 sm | RATE 20 | - |
| **ENV** | WAVE SINE | FREQUENCY 120 Hz | ATTACK 0 | DECAY 65 |
| **LFO** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **EDIT** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Usa lo stesso suono su altre tracce con una FREQUENCY diversa (80 Hz per un floor tom, 200 Hz per un tom alto).

#### Zap

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +47.62 sm | RATE 62 | - |
| **ENV** | WAVE SAW | FREQUENCY 1.0 kHz | ATTACK 0 | DECAY 75 |
| **LFO** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **EDIT** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Il pitch cade di 4 ottave in circa 0,3 s. Mandalo al delay (HOME, 2ª pressione) per un classico laser.

#### Rimshot

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 30 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +11.34 sm | RATE 10 | - |
| **ENV** | WAVE TRI | FREQUENCY 552 Hz | ATTACK 0 | DECAY 25 |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.8 kHz | - |
| **EDIT** | MODE EXP | lascialo | ATTACK 0 | DECAY 22 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Un click secco e breve (circa 45 ms). Alza DIST per un rim più duro; sposta il MIX a sinistra per più tono.

#### Sub bass

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 35 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +6.80 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 41 Hz | ATTACK 6 | DECAY 80 |
| **LFO** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **EDIT** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Una nota bassa di circa 0,9 s con una piccola caduta di pitch all'inizio e un po' di DIST, così si sente anche su casse piccole. Cambia FREQUENCY (da 30 a 60 Hz) per un'altra nota, oppure accorcia DECAY per un kick lungo.

#### Conga

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 82.68/17.32 | EQ FREQ 20 Hz | DIST 10 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +5.29 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 231 Hz | ATTACK 0 | DECAY 54 |
| **LFO** | MODE BP | Q 30 | FREQUENCY 1.6 kHz | - |
| **EDIT** | MODE EXP | lascialo | ATTACK 0 | DECAY 20 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Un tono rotondo con uno schiocco di noise cortissimo (circa 0,2 s in tutto). Alza FREQUENCY (per esempio 330 Hz) su un'altra traccia per una conga alta.

#### Crash

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **ENV** | non si sente (OSC/NOISE MIX 00.00/100.00), lascialo | | | |
| **LFO** | MODE HP | Q 10 | FREQUENCY 5.1 kHz | - |
| **EDIT** | MODE EXP | lascialo | ATTACK 0 | DECAY 100 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Un piatto di puro noise che dura circa 2,3 s. Abbassa DECAY (85 dà circa 1 s) per uno splash, oppure alza la FREQUENCY di LFO per un suono più sottile. Mandalo un po' al reverb (HOME, 2ª pressione).

#### Clave

| Pulsante | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT 0.00 sm | RATE 0 | - |
| **ENV** | WAVE SINE | FREQUENCY 2.5 kHz | ATTACK 0 | DECAY 28 |
| **LFO** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **EDIT** | non si sente (OSC/NOISE MIX 100.00/00.00), lascialo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | lascialo |

Un tono acuto e secco di circa 50 ms, senza movimento di pitch (AMOUNT 0.00 sm). Cambia FREQUENCY (da 2 a 3 kHz) per un'altra accordatura.

**Consigli.** Per sentire una parte da sola, gira il MIX tutto a sinistra (100.00/00.00, solo oscillator) o tutto a destra (00.00/100.00, solo noise). Perché gli accent cambino il suono e non solo il volume, alza i knob di GLO. Per provare un valore su un solo step, tieni lo step e gira il knob (p-lock, capitolo 4). Un suono che ti piace si conserva con SAVE (capitolo 5).

## 9. Crediti

**FM1 TONIC è fatto da Luigi Rossi**, lo sviluppatore e la mente creativa di questo firmware: la drum machine Tonic, il suo sequencer, le schermate, i preset e gli strumenti web (emulatore, installer, backup). È software libero sotto la GNU General Public License, versione 3; se condividi il firmware devi condividere il suo codice sorgente.

**Luigi Rossi online:** [linktr.ee/luigirossi](https://linktr.ee/luigirossi)

LICENSING.md elenca l'origine e la licenza di ogni file. Si appoggia al lavoro di:

- **libpo32** di Eric Lewis (MIT): il punto di partenza del codice di sintesi di batteria di FM1 TONIC. Niente è campionato.
- **X0X** di Charles Vestal, il firmware da cui è nato FM1 TONIC, e le pagine web (emulatore, installer, backup); X0X è un fork di **Felucca** di Leo Kuroshita (@kurogedelic), Hügelton Instruments: la piattaforma (livello hardware, USB, loader di aggiornamento, installer, memorizzazione) e la scheda di recupero FM-1-transporter.
- Il reverb e il delay sono portati da **9W9** di athousanddetails, basato su **ER-99** di Matthew Cieplak (GPL-3.0). Il compressore master segue Giannoulis, Massberg e Reiss; il filtro master è il filtro a variabile di stato TPT di Vadim Zavalishin.
- **Melodee** di keremimo e **Jangada** di zednaked: ciò che hanno scoperto sul secondo core dell'FM-1.
- Font: **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov), **DotGothic16** (The DotGothic16 Project Authors), tutti sotto SIL Open Font License.

M-VAVE e FM-1 sono marchi dei rispettivi proprietari. FM1 TONIC non è collegato a M-VAVE, Sonic Charge o Luigi Rossi, né da loro approvato.
