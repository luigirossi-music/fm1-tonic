# FM1 Tonic

FM1 Tonic transforme le M-VAVE FM-1 en un **synthétiseur de batterie à 8 pistes** inspiré de Microtonic, marque de Sonic Charge. Le son vient de l'engine propre à FM1 Tonic : il se comporte de façon similaire, mais ce n'est pas le même engine, et FM1 Tonic n'a aucun lien avec Sonic Charge. Huit voix de batterie, chacune synthétisée de zéro (rien n'est échantillonné), un séquenceur avec accents et fills (patterns de 1 à 64 steps, 16 patterns par projet), une table de mixage, de la reverb, du delay et un compresseur master. **40 project slots** conservent votre travail : 8 presets d'usine et 32 à vous.

## Caractéristiques

**Son**

- Synthétiseur de batterie à 8 voix : chaque track a son propre oscillator, pitch mod, noise, mix, EQ et sa propre distorsion.
- Level et pan par track, et reverb send et delay send par track (plus low cut et high cut dans l'engine Enhanced).
- Engine Enhanced ou Classic (Classic = uniquement les réglages de base du son, sans les extras du FM1 Tonic).
- Randomizer de track : un nouveau son aléatoire pour la track sélectionnée.
- Choke groups (par exemple charleston fermée et ouverte).
- Compresseur sidechain : n'importe quelle track peut faire duck les autres.

**Séquenceur**

- Séquenceur de 64 steps, 16 patterns par projet. Longueur fixe de 8, 16, 32 ou 64 steps, ou longueur libre de 1 à 64.
- Song mode.
- Accent global et accent par track, fill avec retrig (2x, 4x, 8x) et trig % (chance) sur chaque step.
- Parameter lock (limité : 3 knobs par track dans chaque pattern).
- Record automation (limité : 3 knobs par track).
- Enregistrement en direct, quantized ou unquantized. Swing, quantization et tempo.
- Clavier (ARP) : jouez une track comme un clavier et enregistrez les notes dans les steps (un piano roll).
- Copier-coller d'un pattern, d'une track, et d'un pattern à la volée pendant que le morceau joue.
- Mute et solo.
- Undo et redo.

**Mixage et performance**

- Mixer master : level et pan de chaque track (le point rouge marque le centre).
- Compresseur master.
- Page Performance : 2 effets master (reverb et delay), un low cut et ALL FX OFF.
- Page Memory : le taux de remplissage de la mémoire du projet.

**Projets et sauvegarde**

- 40 project slots : 8 presets d'usine et 32 à vous.
- Autosave et quick save.
- Nom du projet : GENERATE ou MANUAL. Renommer le projet.
- Sauvegarde et restauration depuis la page web. Export et import de presets `.mtpreset`.

**Écran, lumières et réglages**

- Indicateur LED 4/4 (touches blanches 1, 5, 9 et 13), lumière des touches allumée ou éteinte.
- Zoom knobs (le grand cadran quand on tourne un knob), thème de couleur.
- Volume de sortie NORMAL ou LOUD. USB audio.

**MIDI**

- Sélecteur de canal MIDI, notes out activé ou désactivé, clock out activé ou désactivé.
- Control changes (CC) : 26 paramètres de son, par track sélectionnée ou par canal MIDI.
- Panneau distant : jouez les touches mêmes du FM-1 depuis un clavier externe ou un Digitakt.

## 1. Le panneau

![Le panneau du FM1 Tonic : knobs MASTER, SELECT, PRESETS, ALGORITHM et KNOB 1 à 4 ; boutons FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- et OCT+ ; touches noires TRACK 1 à 8, ACCENT, FILL, FUNC ; touches blanches 1 à 16](img/fm1-tonic-panel.svg "")

- **Knobs.** MASTER est le volume. SELECT change de page (dans une liste : haut et bas). PRESETS choisit l'un des 40 presets. KNOB 1 à 4 règlent les quatre valeurs de la page affichée à l'écran (celui que l'on tourne s'affiche plus grand). ALGORITHM passe à une seconde page ou choisit une voix, selon la page (les tableaux de pages l'indiquent). Un petit triangle bleu dans un coin inférieur de l'écran signale qu'une seconde page se trouve derrière ALGORITHM : triangle à droite = tournez ALGORITHM vers la droite, triangle à gauche = tournez-le vers la gauche pour revenir.
- **Touches noires.** 1 à 8 sont TRACK 1 à 8, 9 est ACCENT, 10 est FILL, 11 est FUNC.
- **Touches blanches 1 à 16** sont les 16 steps de la track sélectionnée. Un pattern peut avoir jusqu'à 64 steps, affichés 16 par 16 (voir SEQ).
- **OCT- / OCT+** montent ou descendent d'une octave la hauteur de la track sélectionnée (sur ARP : l'octave du clavier). Dans une question, OCT+ = YES, OCT- = NO.

### Sommaire

1. Le panneau {#s1}
2. Chemin du signal {#s2}
3. Les pages, bouton par bouton {#s3}
  - Combinaisons de boutons
4. Jouer {#s4}
5. Presets et sauvegarde {#s5}
6. Lumières, MIDI et USB {#s6}
7. Installer, sauvegarder, récupérer {#s7}
8. L'engine {#s8}
  - Recettes de sons
9. Crédits {#s9}

---

**Comment lire les tableaux de pages.** Chaque bouton a un tableau, avec une ligne par page. Le texte normal est ce que fait la commande seule. {r:Rouge} est ce qu'elle fait avec **FUNC** maintenue, {o:orange} est ce qu'elle fait avec **FILL** maintenue (à la manière du Digitakt).

**Ce qui marche sur toutes les pages** (les tableaux suivants ne le répètent pas) :

| Touches | Ce qu'elles font |
|---|---|
| Touche noire 1 à 8 | Sélectionne la track et la joue (aussi pendant la lecture). |
| Touche blanche 1 à 16 | Les steps : un appui = trig activé, un autre appui = désactivé. Un pattern a jusqu'à 64 steps, affichés 16 par 16. |
| Touche blanche maintenue + ACCENT / FILL | Ce step reçoit un accent / un fill (encore une fois = retiré). |
| ACCENT ou FILL maintenu | Les trigs de la track qui portent cet accent / fill clignotent sur les touches blanches (les autres trigs restent allumés). |
| {r:FUNC + touche noire 1 à 8} / {o:FILL + touche noire 1 à 8} | Coupe la track (mute) / la met seule (solo) (encore une fois = retour). |
| {r:FUNC + touche blanche} | Change de pattern (pendant la lecture : à la fin de la mesure). |
| {r:FUNC + ACCENT} / {r:FUNC + FILL} / {r:FUNC + PLAY} | Copie le pattern / le colle (demande PASTE PATTERN ?) / l'efface (demande CLEAR PATTERN ?). OCT+ = oui, OCT- = non. |
| {r:FUNC + touche noire 1 à 8 maintenue, puis ACCENT / FILL / PLAY} | Copie le son de cette track / le colle / efface le pattern de cette track (demande confirmation). |
| {r:FUNC + SEQ} | Song mode activé / désactivé (demande confirmation). |
| {r:FUNC + ARP} | Efface le morceau (demande confirmation). |
| {r:FUNC + SAVE} / {r:FUNC + HOME} | Quick save dans l'emplacement chargé / renomme le projet. |
| {r:FUNC + REC} | QUANTIZED REC / UNQUANTIZED REC. |
| ENV + un knob | Ce knob revient à sa valeur par défaut. |
| GLO + ACCENT / GLO + FILL | Undo / redo (sur toutes les pages). |
| REC + PLAY, puis touches et knobs | Enregistrement en direct : les touches noires 1 à 8 écrivent des trigs, les knobs écrivent de l'automation (3 knobs par track). |
| PLAY | Lance et arrête le pattern. |
| OCT- / OCT+ | La hauteur de la track une octave plus bas / plus haut. Dans une question : NO / YES. |

Exceptions : sur PERFORM, MUTE / SOLO et la page des patterns, les touches noires sont des boutons de mute ; sur ARP, toutes les touches sont des touches de clavier. Elles sont toutes dans la liste suivante.

**Où les touches font autre chose.** Les touches du panneau (touches noires 1 à 8 = tracks, ACCENT, FILL, FUNC, touches blanches 1 à 16 = steps) et les raccourcis ci-dessus valent sur toutes les pages de ce manuel, sauf sur les pages de cette liste, où les touches ont une autre fonction :

| Page | Ce que les touches y font |
|---|---|
| **ARP** | Toutes les touches, blanches et noires, sont des touches de clavier qui jouent la voix sélectionnée. |
| **PERFORM, MUTE / SOLO, PATTERN** (SEQ, 2e appui) | Les touches noires 1 à 8 sont des boutons de mute (des boutons de solo en mode SOLO), elles ne sélectionnent pas la track. Les touches blanches 1 à 16 choisissent le pattern suivant. FUNC et FILL, d'une simple touche, passent du mode MUTE au mode SOLO. |
| **DUCK** (SENDS, ALGORITHM à droite) | Les touches noires 1 à 8 choisissent quelles tracks reçoivent le duck. |
| **CHOKE** (EDIT, ALGORITHM à droite) | Les touches noires 1 à 8 choisissent quelles tracks sont dans le choke group. |
| **Écrans SONG et PATTERNS** (song mode) | Les touches blanches 1 à 16 écrivent un pattern dans la mesure. FILL efface la mesure. |
| **Mode copier-coller** (après FUNC + ACCENT) | Les touches blanches 1 à 16 sont les endroits où coller. |
| **FUNC maintenue** | Les touches blanches 1 à 16 sont les patterns 1 à 16 ; les touches s'allument là où un pattern ou une track a des notes. |

**Ce qui n'existe que sur certaines pages.**

| Page | Touches | Ce que ça fait |
|---|---|---|
| Pages de son : MIX 1, SEL, ENV, LFO, EDIT, VEL | Touche blanche maintenue + knob 1 à 4 | **P-lock** : ce step reçoit sa propre valeur du knob. Touche blanche maintenue + SAVE + le même knob le retire. Une track garde 3 knobs avec p-lock par pattern. |
| Toutes les pages sauf PERFORM, HOME, SONG, COMP et les listes | Touche blanche maintenue + SELECT | La **chance** de ce step : de 5 à 100 %, puis 1:2, 2:2, 1:3 ... 8:8. |
| ENV | OCT- / OCT+ | La fréquence de l'oscillator une octave plus bas / plus haut. |
| EDIT | {r:FUNC + knob 2}, {r:FUNC, appui sur EDIT}, EDIT ou LFO maintenu + {r:FUNC + ACCENT / FILL / PLAY} | Entre dans / sort du choke group ; DELETE AUTOMATION? ; copie / colle / réinitialise cette page de la track. |
| SEQ | Knobs, SELECT, ALGORITHM, {o:FILL + SEQ} | Voir le tableau SEQ au chapitre 3. |
| GLO > VEL | {r:FUNC + knob 4} | Un nouveau son aléatoire pour la track sélectionnée. |
| GLOBAL (HOME, 3e appui) | Knob 4 (ou HOME maintenu + SELECT) | Le tempo. |

## 2. Chemin du signal

![L'engine du FM1 Tonic : chacune des 8 tracks a un modulateur de pitch et un oscillator, un filtre de noise et un envelope de noise, un mix, une distorsion et un EQ, puis les cuts, le level et le pan ; les 8 tracks passent par le sidechain, le low cut master et le compresseur jusqu'au volume master ; reverb et delay sont des effets en send](img/fm1-engine.svg "Tout le chemin du signal. Le petit mot dans chaque case est le bouton qui ouvre cette page.")

Chaque track fabrique son son de zéro avec l'engine de batterie du FM1 Tonic : un modulateur de pitch et un oscillator, un filtre et un envelope de noise, un mix, une distorsion et un EQ, puis les cuts, le level et le pan de la track ; les 8 tracks passent par le sidechain, le low cut master et le compresseur jusqu'à MASTER, avec reverb et delay en effets de send. Le chapitre 8 explique chaque réglage.

**Enhanced et classic.** GLO > settings > Tonic Engine est sur **enhanced** par défaut. En **classic**, les extras du FM1 Tonic (les cuts de track, les sends, le sidechain) sont grisés et ne font rien, si bien que le son n'utilise que les réglages qu'un fichier `.mtpreset` peut porter. Leurs valeurs sont conservées et reviennent avec enhanced. Level et pan restent disponibles en classic. Le mode est global, il n'est pas enregistré dans un preset.

## 3. Les pages, bouton par bouton

Une page avec des valeurs de **track** (knobs 1 à 4 des pages de son) appartient à la track sélectionnée : changez de track avec une touche noire 1 à 8 et les valeurs changent. EDIT ou LFO maintenu + {r:FUNC + ACCENT / FILL / PLAY} copie, colle et remet à zéro **cette page** de la track (pages de track uniquement).

### FX

Appui = MIX 1 → PERFORM → MIX 1. Maintien (une demi-seconde) = REVERB. Depuis la page d'un autre bouton, FX revient à la dernière page FX que vous avez quittée.

| Page | Knob 1 · 2 · 3 · 4 | Aussi sur cette page |
|---|---|---|
| **MIX 1** (1er appui) | mix OSC / NOISE · EQ FREQ (20 Hz-20 kHz) · DIST · EQ GAIN (64 = plat) | Touche blanche maintenue + knob : p-lock. SAVE + knob : oublie ce lock ou ce motion. |
| **PERFORM** (2e appui) | REVERB D/W · DELAY D/W (0-100 %, 0 à l'allumage) · MASTER LOW CUT (20-663 Hz) · ALL FX (à droite activé, à gauche désactivé) | Le grand nombre est le pattern en cours de lecture. SELECT : pages FX suivantes (s'arrête aux extrémités). ALGORITHM à droite : MUTE / SOLO. Touches noires 1 à 8 : mute. Touches blanches 1 à 16 : pattern suivant. {r:FUNC tap} : mode MUTE, ou unmute de tout. {o:FILL tap} : mode SOLO, ou efface tous les solos. Pas de p-locks. |
| **MUTE / SOLO** (PERFORM, ALGORITHM à droite) | comme PERFORM | Huit cases à la place du nombre : bleu = la track joue. Touches noires 1 à 8 : mute (mode MUTE) ou ajout / retrait d'un solo (mode SOLO, plusieurs à la fois). ALGORITHM à gauche : retour. |
| **REVERB** (maintenir FX) | DECAY · TONE | ALGORITHM à droite : REVERB 2. |
| **REVERB 2** | LOW CUT (de ce qui entre dans le reverb) · RETURN (niveau dans le mix) | ALGORITHM à gauche : retour. |
| **DELAY** (maintenir HOME aussi) | TIME (1/32 ... 1/2, avec triolets T et valeurs pointées) · FEEDBACK · TONE · TAPE (premier huitième = numérique propre, ensuite bande avec plus d'usure) | ALGORITHM à droite : DELAY 2. |
| **DELAY 2** | LOW CUT · RETURN · PING-PONG (off / on) | ALGORITHM à gauche : retour. |

ALL FX désactivé coupe les deux sends D/W de PERFORM et le low cut master ; les sends propres des tracks continuent de fonctionner. Les mutes et les solos sont partagés par PERFORM et MUTE / SOLO ; les solos ne sont pas enregistrés et se terminent au chargement d'un preset. Les pages d'effets sont globales (toutes les tracks les partagent) : SELECT parcourt PERFORM, REVERB, DELAY, REVERB 2, DELAY 2.

### HOME

Appui = MIX 2 → SENDS → GLOBAL → MIX 2. Maintien = DELAY.

| Page | Knob 1 · 2 · 3 · 4 | Aussi sur cette page |
|---|---|---|
| **MIX 2** | LEVEL · PAN · LOW CUT (20-663 Hz, jamais coupé) · HIGH CUT (de 20 kHz jusqu'à 1 kHz) | Les deux cuts sont grisés en classic. |
| **SENDS** | REVERB SEND · DELAY SEND · SIDECHAIN INPUT (track 1 à 8, grand nombre) · SIDECHAIN THRESHOLD (0-100 %, 0 = rien n'est duck) | ALGORITHM à droite : l'écran DUCK (ci-dessous). Les sends fonctionnent quoi que disent le D/W de PERFORM et ALL FX. |
| **DUCK** (SENDS, ALGORITHM à droite) | - | Touches noires 1 à 8 : bleu = duck, gris = non, violet = l'input. L'input ne peut pas être duck. Rien n'est duck tant que vous ne choisissez pas. ALGORITHM à gauche ou HOME : retour. |
| **GLOBAL** | QUANTIZATION (1/8 ... 1/32) · SWING · FILL RATE (2x, 4x, 8x) · TEMPO | - |

Le duck suit le son de la track d'input (un long kick fait duck longtemps ; un input en mute ne fait pas duck) et fait aussi duck les sends des tracks. Un point rouge indique une track duck, un point violet l'input ; le point pulse à chaque fois que l'input fait duck. Un seul sidechain pour toutes les tracks, enregistré avec le preset.

### SEL, ENV, LFO, EDIT (les pages de voix)

Ce sont quatre pages de track : les p-locks et la chance y fonctionnent.

| Bouton | Knob 1 · 2 · 3 · 4 | Aussi sur cette page |
|---|---|---|
| **SEL** PITCH MOD | SHAPE (DECAY, SINE, RANDOM) · AMOUNT (64 = aucun) · RATE · - | |
| **ENV** OSCILLATOR | WAVE (SINE, TRI, SAW) · FREQUENCY (20 Hz-20 kHz) · ATTACK · DECAY | OCT- / OCT+ déplacent la fréquence d'une octave. |
| **LFO** NOISE FILTER | MODE (LP, BP, HP) · Q · FREQUENCY · - | |
| **EDIT** NOISE ENVELOPE | MODE (EXP, LIN, CLAP) · bouton CHOKE · ATTACK · DECAY | {r:FUNC + knob 2 à droite} : la track rejoint le choke group ; à gauche : elle le quitte. Le knob 2 seul n'affiche qu'un message. ALGORITHM à droite : l'écran CHOKE. {r:FUNC, tap sur EDIT} : DELETE AUTOMATION ? (OCT+ oui, OCT- non). |
| **CHOKE** (EDIT, ALGORITHM à droite) | - | Touches noires 1 à 8 : bleu = dans le groupe. Un coup d'une track du groupe arrête aussitôt les autres, comme un hi-hat fermé arrête un hi-hat ouvert. ALGORITHM à gauche : retour. Enregistré avec le preset. |

DELETE AUTOMATION supprime les p-locks et le motion enregistré de la track sélectionnée dans le pattern en cours de lecture (les autres tracks, les autres patterns et les chances restent ; l'undo fonctionne). Maintenir FUNC + EDIT puis appuyer sur ACCENT, FILL ou PLAY, c'est la copie de page, pas une suppression.

### GLO

Appui = VEL → MIXER → COMP → MEMORY → SETTINGS → VEL.

| Page | Knob 1 · 2 · 3 · 4 | Aussi sur cette page |
|---|---|---|
| **VEL** | OSC · NOISE · PITCH MOD (de combien la velocity déplace chacun) · RANDOM | {r:FUNC + knob 4} : la track sélectionnée reçoit un nouveau son aléatoire (les 21 réglages de son seulement). Le knob 4 seul n'affiche qu'un rappel. Page de track : les p-locks fonctionnent. |
| **MIXER** | LEVEL des quatre tracks affichées | Huit grands sliders ; SELECT passe des tracks 1 à 4 aux tracks 5 à 8 et, comme le dit l'indication bleue à l'écran, alterne level / pan (pan au centre = rouge). Les touches noires 1 à 8 choisissent une track et son groupe. Un coup allume son slider. {r:FUNC + touche noire 1 à 8} : mute. Un point rouge sous un slider = duck, violet = input du sidechain. La chance fonctionne ici. |
| **COMP** | ATTACK · RELEASE · THRESHOLD · MAKE-UP | Compresseur master, ratio 4:1, une aiguille montre la réduction de gain. Par défaut : rien n'est compressé (0.0 dB / 0.0 dB). Conservé uniquement par SAVE. |
| **MEMORY** | - | Une batterie montre à quel point la mémoire du projet est pleine (patterns, song, motion, notes ARP). Violet à partir de 90 %, FULL à 100 % : une sauvegarde est alors refusée. |
| **SETTINGS** | liste (SELECT déplace) | Lignes : clock out, notes out, canal MIDI Tonic, niveau de sortie, USB audio, key light, 4/4 LED, thème de couleur, zoom knob (yes / no), Tonic Engine, name projects, save, autosave, clear pattern, factory reset, performance, cores, about, battery. Les moins évidentes sont expliquées juste en dessous. Les changements sont conservés tout seuls environ une seconde et demie après que vous avez cessé de toucher (le sequencer doit être arrêté). |

#### Réglages GLO : les moins évidents

Les réglages sont dans la liste GLO (SELECT monte et descend, ALGORITHM change la valeur). Les valeurs par défaut sont en gras.

| Réglage | Choix | Ce qu'il fait |
|---|---|---|
| **OUTPUT** | NORMAL / **LOUD** | LOUD (le réglage d'usine) rend les sorties du FM-1 plus fortes de 6 dB (la pleine échelle du codec). NORMAL garde 6 dB de marge : utilisez-le si le son est trop fort, et surveillez le compresseur et le limiter. |
| **USB AUDIO** | **FOLLOWS KNOB** / FULL | FOLLOWS KNOB : l'ordinateur reçoit ce qu'entend le casque, après le knob MASTER. FULL : l'ordinateur reçoit toujours le niveau plein, et MASTER ne change que les sorties propres du FM-1 (haut-parleur, casque, line out). Utilisez FULL pour enregistrer à plein niveau tout en écoutant doucement. |
| **4/4 LED** | OFF / **ON** | ON (le réglage d'usine) : les touches blanches 1, 5, 9 et 13 brillent faiblement pour marquer les quatre temps de la mesure (pas sur PERFORM). |
| **ZOOM KNOB** | **YES** / NO | YES : le knob que vous tournez est agrandi sur l'écran. NO : il est seulement mis en évidence, donc toute la page reste visible. L'écran SONG ne zoome jamais. |
| **NAME PROJECTS** | **GENERATE** / MANUAL | GENERATE invente le nom d'un projet enregistré (ALGORITHM = un autre). MANUAL : l'écran de nom est un éditeur de lettres (voir chapitre 5). |
| **TONIC ENGINE** | **ENHANCED** / CLASSIC | CLASSIC grise et rend silencieux les extras du FM1 Tonic (cuts de track, sends, sidechain) pour que le son n'utilise que les réglages qu'un fichier `.mtpreset` peut porter (chapitres 2 et 8). |
| **2ND CORE** | **ON** / OFF | La puce du FM-1 a deux cores et Tonic se partage entre eux le rendu du son. Effet au prochain allumage. Si le FM-1 plante un jour avec deux cores, il se met tout seul sur OFF ; remettez ON ici. |

### SEQ et ARP

| Page | Knob 1 · 2 · 3 · 4 | Aussi sur cette page |
|---|---|---|
| **SEQ** (1er appui) | FIXED LEN (8, 16, 32, 64) · AUTOSCROLL · FREE LEN (1 à 64) · SWING | Touches blanches : les steps. SELECT : les pages de steps (16 à la fois ; AUTOSCROLL off). ALGORITHM : la voix (1 à 8, puis AC = l'accent global). Maintenir un step + ACCENT / FILL : flags (accent = bleu foncé, fill = jaune, les deux = vert). {r:FUNC + SEQ} : mode song on / off (confirmer avec OCT+). {o:FILL + SEQ} : l'écran SONG. |
| **PATTERN** (SEQ, 2e appui) | SONG ON / OFF | Touches noires 1 à 8 : mute, touches blanches 1 à 16 : choisissent le pattern suivant. Touche éteinte = pattern vide, faible lumière = a des notes, clignotement lent = l'actuel, clignotement rapide = le suivant (cued). {r:FUNC tap} et {o:FILL tap} comme sur PERFORM. SEQ encore : retour. |
| **ARP** | FIXED LEN · OCTAVE (aussi OCT- / OCT+, -3 à +3) · - · - | L'écran SEQ, mais chaque touche (blanche et noire) joue la voix sélectionnée comme un clavier. Les touches de do (faibles) jouent le pitch propre de la voix à l'octave 0. ALGORITHM : la voix. Rien n'est enregistré. SEQ : retour. {r:FUNC + ARP} : supprime la song (DELETE SONG ?). Touche noire 11 maintenue + PLAY : efface le pattern (CLEAR PATTERN ?), PLAY ne démarre pas alors. |

Allonger un pattern copie ce qui s'y trouve dans la nouvelle partie (de 8 à 16 copie les steps 1 à 8 dans 9 à 16) ; le raccourcir ne fait que le raccourcir.

### PLAY, REC, SAVE

| Bouton | Ce qu'il fait |
|---|---|
| **PLAY** | Lecture et arrêt. {r:FUNC + PLAY} : efface le pattern (il demande). |
| **REC** | REC puis PLAY : enregistrement en direct. Chaque touche noire 1 à 8 écrit un trig de sa track au step le plus proche ; chaque knob de son que vous tournez est enregistré comme automation. Pas sur PERFORM ni sur la page pattern. {r:FUNC + REC} : QUANTIZED REC (par défaut) / UNQUANTIZED REC (le coup garde son timing exact, à l'intérieur du step) ; pendant la lecture, il active aussi l'enregistrement. |
| **SAVE** | Voir chapitre 5. {r:FUNC + SAVE} : sauvegarde rapide. {r:FUNC + HOME} : renommer. |

### Combinaisons de boutons

Toutes les combinaisons de boutons et de touches, groupées par le bouton que vous maintenez en premier (le tableau est en anglais en attendant la traduction). Touche noire 9 = ACCENT, 10 = FILL, 11 = FUNC. {r:Rouge} = FUNC maintenu, {o:orange} = FILL maintenu. La dernière colonne dit où cela marche : « Every page » = toutes les pages sauf ARP et les deux pages PERFORM.

**FUNC held (black key 11)**

| Keys | What it does | Where |
|---|---|---|
| {r:FUNC + black key 1 to 8} | Mutes the track (again = unmute). It acts when you let the key go. | Every page |
| {r:FUNC + white key 1 to 16} | Changes to that pattern (while playing: at the end of the bar). | Every page; not on the SONG screen or in song mode (the song decides) |
| {r:FUNC + ACCENT} | Copies the pattern. The screen says PRESS PATTERN TO PASTE / PRESS SEQ TO EXIT, every pattern key blinks (an empty pattern dimmer) and SEQ blinks: press a pattern key and the copy lands there, that pattern becomes the current one. Tap FUNC alone before choosing to stay in the mode for many pastes. SEQ leaves the mode and opens the sequencer page; {r:FUNC + ACCENT} again or any other button leaves it too. | Every page |
| {r:FUNC + FILL} | Pastes the copied pattern when you let FILL go. It asks PASTE PATTERN? (OCT+ = yes, OCT- = no). | Every page |
| {r:FUNC + FILL held + white key} | Pastes the copied pattern into that slot at once (no question). | Every page |
| {r:FUNC + PLAY} | Clears the pattern (it asks CLEAR PATTERN?). It does not start the music. | Every page, ARP too (there hold black key 11, a piano key on ARP: PLAY does not start / stop while it is held) |
| {r:FUNC + black key 1 to 8 held, then ACCENT} | Copies that track's sound (not its steps). | Every page |
| {r:FUNC + black key 1 to 8 held, then FILL} | Pastes the copied sound into that track (mute, volume and pan too; the track is not muted by it). No question. | Every page |
| {r:FUNC + black key 1 to 8 held, then PLAY} | Clears that track's pattern (it asks CLEAR TRACK PATTERN?). | Every page |
| {r:FUNC + EDIT or LFO held, then ACCENT} | Copies this page of the selected track (only its parameters). | Pages that have track parameters (not GLO, HOME, SEQ) |
| {r:FUNC + EDIT or LFO held, then FILL} | Pastes it into the selected track, on the same page. | Pages that have track parameters (not GLO, HOME, SEQ) |
| {r:FUNC + EDIT or LFO held, then PLAY} | Resets this page of the track to its power-on values. No question; undo takes it back. | Pages that have track parameters (not GLO, HOME, SEQ) |
| {r:FUNC + EDIT}, tapped alone | DELETE AUTOMATION?: removes the selected track's p-locks and recorded motion in the playing pattern (it asks). | Every page |
| {r:FUNC + SEQ} | Song mode on / off (it asks: OCT+ = yes). | Every page |
| {r:FUNC + ARP} | DELETE SONG?: deletes the whole song and leaves song mode (it asks). | Every page |
| {r:FUNC + HOME} | Renames the project and saves it in the same place. | Every page |
| {r:FUNC + SAVE} | Quick save into the loaded place. No question. | Every page |
| {r:FUNC + REC} | Cycles QUANTIZED REC / UNQUANTIZED REC. While playing it also switches recording on. | Every page |
| {r:FUNC + OCT- / OCT+} | Nothing. | |
| {r:FUNC + knob 2 (turn right / left)} | The selected track joins / leaves the choke group. | Only EDIT, NOISE ENVELOPE page |
| {r:FUNC + knob 4} | A new random sound for the selected track. | Only GLO > VEL |

**FILL held (black key 10)**

| Keys | What it does | Where |
|---|---|---|
| {o:FILL + black key 1 to 8} | Solos that track (another key moves the solo, the same key ends it). | Every page |
| {o:FILL + SEQ} | Back to the SONG screen from any page (in song mode; otherwise NOT IN SONG MODE). | Every page |
| {o:FILL}, pressed and let go alone | Deletes the bar on the cursor (no question). | Only the SONG screen |
| {o:FILL} + a white key held (or FILL first, then white keys) | That step gets a fill (the track plays 2x, 4x or 8x over it); again = off. | Pages where white keys are steps |
| {o:FILL} held | The trigs of the selected track that carry a fill blink on the white keys (the other trigs stay lit). | Pages where white keys are steps |
| {r:FUNC + FILL} | Paste. See the FUNC list above. | |

**ACCENT held (black key 9)**

| Keys | What it does | Where |
|---|---|---|
| White key held + ACCENT (or ACCENT first, then white keys) | That step gets an accent (loud); again = off. | Pages where white keys are steps |
| ACCENT held | The trigs of the selected track that carry an accent blink on the white keys (the other trigs stay lit). | Pages where white keys are steps |
| {r:FUNC + ACCENT} | Copy. See the FUNC list above. | |

**GLO held**

| Keys | What it does | Where |
|---|---|---|
| GLO + ACCENT | Undo. | Every page, ARP too |
| GLO + FILL | Redo. | Every page, ARP too |

**HOME held**

| Keys | What it does | Where |
|---|---|---|
| HOME + SELECT | The tempo. | Every page |
| HOME held alone (half a second) | The DELAY page. | Every page |
| HOME + white key | Nothing. | |

**FX held**

| Keys | What it does | Where |
|---|---|---|
| FX held alone (half a second) | The REVERB page. | Every page |

**ENV held**

| Keys | What it does | Where |
|---|---|---|
| ENV + a knob | That knob goes back to its power-on value. | Pages with knobs |
| ENV + white key | The accent of that step on / off. | Pages where white keys are steps |
| ENV + black key 1 to 8 | Plays that track loud. Recording: the step gets an accent. | Every page |

**SAVE held**

| Keys | What it does | Where |
|---|---|---|
| SAVE + a knob | That knob's recorded motion is forgotten. | Pages with knobs |
| White key held + SAVE + the same knob | Removes that step's p-lock of the knob. | Sound pages |

**A white key held (a step)**

| Keys | What it does | Where |
|---|---|---|
| White key held + knob 1 to 4 | **P-lock**: that step gets its own value of the knob (3 locked knobs per track and pattern). | Sound pages: MIX 1, SEL, ENV, LFO, EDIT, VEL |
| White key held + SELECT | That step's chance: 5 to 100 %, then 1:2, 2:2, 1:3 ... 8:8. | Every page but PERFORM, HOME, SONG, COMP and the lists |
| White key held + ACCENT / FILL | Accent / fill flag. | See ACCENT and FILL above |

**Transport and others**

| Keys | What it does | Where |
|---|---|---|
| REC + PLAY, then keys and knobs | Live recording: black keys 1 to 8 write trigs, the knobs write automation. | Not on PERFORM and the pattern page |
| OCT- / OCT+ | The selected track's pitch one octave down / up. In a question: NO / YES. On ARP: the keyboard's octave. | Every page |
| A button held alone (about a second) | A card says what the button does on this page, its combinations included. | Every button except FX and HOME |

## 4. Jouer

- **Steps.** Choisissez une track, appuyez sur une touche blanche pour poser un step, appuyez encore pour l'enlever. **Accent** (fort) et **fill** (la track joue quatre fois pendant ce step) : maintenez la touche blanche et appuyez sur ACCENT et / ou FILL (encore une fois = flag retiré), ou maintenez d'abord ACCENT / FILL et appuyez sur des touches blanches.
- **Patterns.** {r:FUNC + touche blanche} change de pattern. {r:FUNC + ACCENT} copie, {r:FUNC + FILL} colle (maintenez FILL et appuyez sur une touche blanche pour coller à cet emplacement), {r:FUNC + PLAY} efface. Coller le pattern demande **PASTE PATTERN ?** (OCT+ = oui, OCT- = non) et effacer demande CLEAR PATTERN ?. Avec en plus une touche noire 1 à 8 maintenue, ils copient, collent et effacent cette track seulement ; coller le son d'une track ou une page, coller avec FILL et une touche blanche, et le mode copier-coller à la volée collent aussitôt, sans question.
- **Copier-coller à la volée.** Après {r:FUNC + ACCENT}, tout l'écran dit **PRESS PATTERN TO PASTE** et **PRESS SEQ TO EXIT** en grandes lettres bleues. Toutes les touches de pattern clignotent (un pattern avec des notes clignote fort, un pattern VIDE clignote plus doucement, celui qui est copié reste allumé) et le bouton **SEQ** clignote aussi. Appuyez sur une touche de pattern et la copie s'y colle, ce pattern devient l'actuel et le mode se termine. Pour coller à plusieurs endroits, **touchez FUNC seul** avant de choisir : le mode reste et le pattern actuel ne change pas. Pour quitter le mode, appuyez sur **SEQ** : cela ouvre la **page du séquenceur**, pas la page pattern (l'appui suivant sur SEQ redevient normal). N'importe quel autre bouton quitte aussi, tout comme {r:FUNC + ACCENT} de nouveau (l'écran dit COPY MODE OFF). Maintenir {r:FUNC + ACCENT} et appuyer sur une touche blanche = ce pattern est la copie. Pas disponible sur PERFORM ni sur la page pattern, où FUNC est le mode MUTE.
- **Mute et solo.** {r:FUNC + touche noire 1 à 8} met en mute (encore = unmute). {o:FILL + touche noire 1 à 8} met cette track en solo (une autre touche déplace le solo, la même touche le termine). Les solos ne sont pas enregistrés et se terminent au chargement d'un preset. Pas sur ARP.
- **P-locks et chance.** Maintenez une touche blanche et tournez un knob sur une page de voix : ce step reçoit sa propre valeur, tous les autres steps gardent le son. Pendant la lecture, vous l'entendez la prochaine fois que le step revient. Pour en retirer un : maintenez le step, maintenez SAVE, tournez le même knob. Une track garde 3 knobs verrouillés par pattern (l'écran dit MAX 3 PER TRACK, MOTION FULL quand il n'y a plus de place). Maintenez un step et tournez SELECT : sa chance, en descendant de 100 % par pas de 5 %, puis 1:2, 2:2, 1:3 ... 8:8. Pendant que vous tournez, la chance occupe tout l'écran (TRIG % et le step), en grand, pour la lire d'un coup d'œil.
- **Chaînes.** Un pattern peut avoir le flag de passer au suivant quand il se termine (après le pattern 8 vient le 1). Un flag ne compte que sur un pattern qui a des notes. Un preset avec des chaînes se charge en jouant à travers elles (SONG ON) ; le knob 1 de la page pattern de SEQ l'éteint et le pattern boucle.
- **Mode song.** {r:FUNC + SEQ}, confirmez avec OCT+ (OCT- = retour). S'il n'y a pas encore de song, l'écran PATTERNS s'ouvre : les touches blanches 1 à 16 ajoutent des patterns, OCT- (ou FUNC + une touche blanche) enlève le dernier, OCT+ est OK. PLAY joue une mesure par pattern, en boucle. Le même geste quitte le mode song. Sur l'**écran SONG** (une ligne par mesure) knob 1 = la mesure, knob 2 = son pattern, knob 3 = répétitions (x1 à x8) ; les touches blanches écrivent un pattern dans la mesure sous le curseur et avancent ; FILL, pressé et relâché, supprime aussitôt la mesure (la dernière mesure supprimée = la song est supprimée et le mode song se termine). {r:FUNC + ARP} supprime toute la song.

## 5. Presets et sauvegarde

Tournez **PRESETS** parmi les 40 emplacements : 01 à 08 sont des presets d'usine, 09 à 40 sont les vôtres (un emplacement vide se charge avec les sons d'un preset d'usine et sans pattern : 09 sonne comme 01, 10 comme 02, et ainsi de suite en rotation). Une question est posée d'abord : OCT+ le charge, OCT- garde votre son. Le chargement est refusé pendant la lecture.

- **SAVE** demande SAVE HERE ? avec l'emplacement chargé. OCT+ sauvegarde là et garde le nom. ALGORITHM choisit un autre emplacement (1 à 40 ; l'ambre montre qui serait remplacé). OCT- va à CHOOSE SLOT, puis à l'écran de slot, puis à l'écran de nom (ALGORITHM = un autre nom inventé ; OCT+ sauvegarde). HOME quitte sans sauvegarder. Si rien n'a été chargé depuis l'allumage, SAVE commence à CHOOSE SLOT.
- {r:FUNC + SAVE} sauvegarde rapidement dans l'emplacement chargé sans demander de slot, aussi pendant la lecture. Un emplacement vide se sauvegarde aussitôt comme UNTITLED 01, 02 ... ; un emplacement plein demande OVERWRITE SLOT ? (OCT+ oui).
- {r:FUNC + HOME} renomme le projet chargé (ALGORITHM = un autre nom, OCT+ sauvegarde, OCT- ou HOME annule). Avec NAME PROJECTS sur MANUAL (réglages GLO), l'écran de nom devient un éditeur de lettres : le knob 4 fait défiler les caractères, OCT+ (l'écran dit OK) prend la lettre, OCT- efface la dernière, puis appuyez sur SAVE (8 lettres au maximum ; l'écran montre PRESS SAVE et TURN KNOB 4 en grosses lettres).
- **Ce qui est enregistré.** Avec AUTOSAVE sur ON, votre projet de travail est sauvegardé automatiquement 4 secondes après que vous avez cessé de toucher, sauf le compresseur, le reverb et le delay : ceux-là ne sont conservés que par SAVE, dans le preset. PERFORM repart toujours propre à l'allumage (D/W 0 %, low cut master 20 Hz, ALL FX activé). AUTOSAVE est sur OFF d'usine : appuyez sur SAVE (ou FUNC + SAVE) pour conserver votre travail. Les settings sont tout de même conservés d'eux-mêmes. Mettez AUTOSAVE sur ON dans GLO > SETTINGS si vous voulez que le projet se sauvegarde tout seul. Settings d'usine : clock out et notes out off, canal MIDI Tonic 10, canal MIDI remote off, CC mode off (CC CH BASE 1-8), output LOUD, USB audio follows knob, 4/4 LED on, thème TONIC, zoom knob yes, name projects GENERATE, Tonic Engine ENHANCED, 2nd core ON. Une unité neuve ou un FACTORY RESET les reçoit ; une unité qui contient déjà un projet sauvegardé garde ses propres settings.

## 6. Lumières, MIDI et USB

- **Lumières.** PLAY s'allume tous les 4 steps (rouge pendant l'enregistrement). REC : éteint quand on n'enregistre pas ; fixe à l'arrêt (quantized) et clignotant au rythme du tempo pendant la lecture ; deux clignotements rapides et un lent pour unquantized. Le bouton de la page en cours est allumé. Les touches blanches montrent les steps de la track sélectionnée (le step en cours s'inverse) ; 1, 5, 9 et 13 peuvent briller faiblement (GLO > 4/4 LED). Touches noires 1 à 8 : la track sélectionnée est vive (elle clignote si elle est en mute), les tracks qui ont des notes sont faibles, et pendant la lecture la touche d'une track clignote à chacun de ses coups (pas sur ARP). Avec FUNC maintenu, les touches montrent quels patterns et quelles tracks ont des notes.
- **MIDI.** Par USB ou par le TRS MIDI IN, le canal 10 par défaut joue les voix avec les notes de batterie General MIDI dans l'ordre des voix : 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51 (velocity = volume). Une clock entrante est suivie ; Start, Stop et Continue commandent le pattern. Réglages GLO : TONIC MIDI CH (ou OFF), REMOTE MIDI CH (ou OFF, voir plus bas), CC MODE et CC CH BASE (control changes, voir plus bas), CLOCK OUT (clock et start / stop), NOTES OUT (le pattern en notes).
- **Remote panel.** GLO > REMOTE MIDI CH (OFF par défaut) fait jouer à un second canal MIDI les touches propres du FM-1. Les 27 touches du panneau sont les 27 notes de fa 3 à sol 5 (notes MIDI 53 à 79, do central = 60), disposées comme un clavier de piano : la première touche, F3, est la note 53, la dernière, G5, la note 79. Une note on appuie sur la touche, une note off la relâche, donc une note longue maintient la touche enfoncée. Tout ce que fait une touche sur le panneau fonctionne de la même façon : les touches noires 1 à 8 sélectionnent ou mettent en mute les tracks, les touches blanches 1 à 16 sont les steps ou les patterns, et les touches noires 9, 10 et 11 sont ACCENT (note 73), FILL (note 75) et FUNC (note 78). Un accord peut arriver dans n'importe quel ordre de notes : ACCENT, FILL et FUNC sont toujours appliqués en premier, donc FUNC + ACCENT + une touche blanche fonctionne quand les trois notes arrivent ensemble. All Notes Off (controller 123) relâche toutes les touches. Les notes du canal remote ne jouent pas les voix ; donnez-lui un canal différent de TONIC MIDI CH. Les notes hors de 53 à 79 sont ignorées.

| Touche | Notes |
| --- | --- |
| Touches blanches 1 à 16 | 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 |
| Touches noires 1 à 8 (tracks) | 54, 56, 58, 61, 63, 66, 68, 70 |
| Touche noire 9 = ACCENT, 10 = FILL, 11 = FUNC | 73, 75, 78 |

### Control changes (CC)

Un control change tourne un knob depuis l'extérieur : le numéro de CC est le **paramètre** (les mêmes numéros pour toutes les tracks) et la **track** se choisit de deux façons. GLO > **CC MODE** décide (OFF par défaut : rien ne réagit aux CC tant que vous ne l'activez pas) :

| CC MODE | Ce qu'il fait |
| --- | --- |
| OFF | Tous les control changes sont ignorés. |
| SELECTED | Un CC sur le canal du Tonic (TONIC MIDI CH) déplace le paramètre de la track **sélectionnée**. |
| CHANNELS | Un bloc de 8 canaux, un par track : le premier canal de GLO > **CC CH BASE** est la track 1, le suivant la track 2, et ainsi de suite (base 1 = canaux 1-8). N'importe quelle track peut être déplacée à tout moment, sans avoir à la sélectionner d'abord. |
| BOTH | Les deux ensemble. Si le canal du Tonic est dans le bloc, le bloc l'emporte sur ce canal. |

Les notes jouent toujours les voix sur TONIC MIDI CH et le remote panel garde son propre canal : donnez donc au bloc des canaux que ces deux-là n'utilisent pas (le bloc par défaut 1-8 laisse libres le 10 et le 11). Un CC déplace la valeur comme le fait le knob, donc l'écran, SAVE et l'autosave suivent ; il n'ouvre pas le grand cadran, il ne verrouille pas de step et il n'est pas enregistré. Un CC 0-127 correspond au 0-127 du knob ; un sélecteur à trois positions (forme d'onde, mode du filtre, mode de l'envelope) divise la plage en trois. Dans l'engine CLASSIC, les extras (low cut, high cut, reverb send, delay send) ignorent les CC comme leurs knobs grisés.

| CC | Paramètre | CC | Paramètre |
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

### Connecter un Digitakt ou un Digitone

La prise USB du FM-1 et celle du Digitakt ou du Digitone sont toutes deux des prises « device » : elles ne peuvent pas se parler. Utilisez le MIDI OUT (5 broches) du Digitakt ou du Digitone vers le TRS MIDI IN du FM-1, avec un adaptateur MIDI 5 broches vers TRS 3,5 mm (type A ou B : si rien n'arrive, essayez l'autre type).

1. Sur le FM-1 : réglages GLO, TONIC MIDI CH = 10 (les voix) et REMOTE MIDI CH = 11 (le panneau), ou deux autres canaux différents.
2. Sur le Digitakt ou le Digitone : utilisez deux tracks MIDI et réglez leurs canaux sur ces deux numéros. La première joue les voix avec les notes de batterie (36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51). La seconde est le remote panel : dessinez les notes du tableau ci-dessus dans sa grille de steps ou son piano roll.
3. Une track MIDI joue jusqu'à 4 notes dans un step, avec une seule longueur pour toutes : la longueur de la note est le temps pendant lequel les touches sont maintenues, donc une note longue pour FUNC avec d'autres notes sur des steps suivants donne FUNC + ces touches, et un accord FUNC + ACCENT + une touche blanche tient dans un seul step.
4. Pour piloter le son depuis les lanes de CC du Digitakt ou du Digitone : réglez GLO > CC MODE = CHANNELS, faites une track MIDI par track Tonic sur les canaux 1 à 8 (ou à partir de CC CH BASE) et assignez ses lanes de CC aux numéros du tableau ci-dessus ; chaque track MIDI n'a qu'un certain nombre de lanes de CC (16 sur le Digitakt II, moins sur le premier Digitakt), choisissez donc les paramètres que vous voulez automatiser sur chaque track.
5. Pour suivre le tempo et le start / stop du Digitakt ou du Digitone, activez sa MIDI clock et sa sortie transport ; le FM-1 suit une clock entrante.

### Connecter un clavier MIDI

Le FM-1 ne peut pas être l'hôte d'un clavier USB. Utilisez un clavier avec un MIDI OUT 5 broches (avec un adaptateur 5 broches vers TRS dans le TRS MIDI IN), ou faites passer un clavier USB par un ordinateur ou un boîtier hôte USB MIDI vers la prise USB du FM-1.

1. Sur le FM-1 : réglages GLO, REMOTE MIDI CH = le canal sur lequel le clavier émet (1 est le plus courant).
2. Jouez les notes F3 à G5 (53 à 79). Maintenez une touche pour FUNC (note 78, F#5), ACCENT (note 73, C#5) ou FILL (note 75, D#5) et appuyez sur les autres : copier, coller, changement de pattern, mute et solo fonctionnent comme sur le panneau. Un clavier de 49 touches couvre tout le panneau ; sur un clavier plus petit, déplacez-le avec les boutons d'octave.

- **Audio USB.** Le même câble transporte ce qu'entend le casque, après MASTER, comme une entrée 2 canaux 44,1 kHz, sans driver (GLO > USB AUDIO = FULL envoie le niveau plein quoi que dise MASTER). Le premier instant après l'ouverture par une application peut cliquer.

## 7. Installer, sauvegarder, récupérer

- **L'essayer :** l'[émulateur](https://luigirossi-music.github.io/fm1-tonic/emu/) exécute le même code dans le navigateur.
- **Installer ou mettre à jour :** ouvrez l'[installateur web](https://luigirossi-music.github.io/fm1-tonic/install/) dans Chrome ou Edge, branchez le FM-1 avec un câble USB de données, appuyez sur Install et ne le débranchez pas pendant l'écriture. Un autre firmware ne conserve pas votre projet : enregistrez-le d'abord dans un fichier. « Back to the stock firmware » en bas de la même page remet le firmware de M-VAVE. Installer un autre firmware se fait à vos risques.
- **Sauvegarder :** la [page de backup](https://luigirossi-music.github.io/fm1-tonic/backup/) enregistre le projet dans un fichier `.x0b` (Back up), le recharge (Restore : arrêtez le FM-1, puis répondez à RESTORE FROM USB? avec SEL), exporte les presets en fichiers `.mtpreset`, et sauvegarde / restaure les emplacements 9 à 40 en fichiers `.tnp`. En ligne de commande : `python3 tools/x0x_backup.py backup my-song.x0b` (aussi `restore`, `info`).
- **Si quelque chose tourne mal :** maintenez OCT- et OCT+ cinq secondes = mode mise à jour ; maintenez-les en allumant = calibration du matériel. Après un crash, un écran rouge CRASH redémarre le FM-1 ; s'il plante deux fois au démarrage, il entre en SAFE MODE (pas de son, l'USB fonctionne : réinstallez depuis l'installateur ; PLAY tente un démarrage normal). Si même cela échoue, le script de secours pour Mac `tools/fm1_rescue.sh` remet le firmware de M-VAVE (il vérifie chaque fichier qu'il écrit ; le relancer ne présente aucun risque).
- **Texte complet** des chapitres installation, backup, rescue et paramètres : `docs/reference/MANUAL_full_wip381.md`.

## 8. L'engine

Chacune des 8 tracks est un synthétiseur de batterie complet : l'engine du FM1 Tonic, rien n'est échantillonné. Les 8 tracks sont identiques et chacune a les mêmes **26 réglages**, conservés par track et enregistrés avec le preset. Ce chapitre est la référence pour tous. L'endroit où se trouve un réglage s'écrit bouton · knob.

```
pitch mod ─► OSCILLATOR ─┐
                         ├─► MIX ─► EQ ─► DIST ─► LEVEL ─┐
NOISE ─► filter ─► env ──┘                               │
          ┌──────────────────────────────────────────────┘
          └─► LOW CUT ─► HIGH CUT ─┬─► PAN ─► master
                                   └─► REVERB / DELAY SEND
```

Un coup sonne jusqu'à ce que ses envelopes s'éteignent. Un nouveau coup sur la même track coupe celui qui sonne encore (les batteries sont monophoniques), et une track du choke group est coupée par les autres (fondu de 3 ms).

**Les knobs sont des courbes, pas des lignes droites.** Les fréquences, les durées et le Q sont exponentiels : le milieu du knob n'est pas la moitié de la valeur. À l'écran, les fréquences s'affichent en Hz ou kHz, l'AMOUNT du pitch mod en demi-tons (sm) et le pan en L / C / R, l'OSC/NOISE MIX comme deux pourcentages (100.00/00.00 = oscillator seul, 00.00/100.00 = noise seul) ; tous les autres knobs montrent un simple nombre de 0 à 127. Servez-vous de ce tableau pour savoir ce que ces nombres signifient.

| Knob | 0 | 32 | 64 | 96 | 127 |
|---|---|---|---|---|---|
| **Frequency** (oscillator, noise filter, EQ) | 20 Hz | 114 Hz | 650 Hz | 3,7 kHz | 20 kHz |
| **Decay** (oscillator, noise, descente du pitch mod) | 10 ms | 57 ms | 0,33 s | 1,8 s | 10 s |
| **Attack** (oscillator, noise) | 0 | 20 ms | 0,16 s | 1 s | 5 s |
| **Filter Q** | 0,1 | 1,8 | 33 | 600 | 10000 |

Un decay est le temps que le son met à tomber dans le silence (60 dB plus bas).

### Oscillator : la hauteur

| Réglage | Où | Plage | Ce que ça fait |
|---|---|---|---|
| **WAVE** | ENV · knob 1 | SINE, TRI, SAW | SINE est pur et doux, TRI creux, SAW brillant et bourdonnant. |
| **FREQUENCY** | ENV · knob 2 | 20 Hz à 20 kHz | La hauteur (pitch). Kick environ 40 à 70 Hz, toms 80 à 250 Hz, corps de caisse claire 150 à 250 Hz. OCT- / OCT+ la déplacent d'une octave. |
| **ATTACK** | ENV · knob 3 | 0 à 5 s | 0 démarre tout de suite. Montez-le pour un crescendo. Le premier quart du knob est sous 20 ms. |
| **DECAY** | ENV · knob 4 | 10 ms à 10 s | La durée du son : court = clic ou tic, long = boom. |

### Pitch mod : le pitch bouge pendant que le son joue

| Réglage | Où | Plage | Ce que ça fait |
|---|---|---|---|
| **SHAPE** | SEL · knob 1 | DECAY, SINE, RANDOM | Le type de mouvement (ci-dessous). |
| **AMOUNT** | SEL · knob 2 | 64 = aucun ; jusqu'à 8 octaves de chaque côté | Au-dessus de 64 le pitch monte, en dessous de 64 il descend. Avec DECAY, le son part aussi loin et glisse jusqu'au pitch réglé. |
| **RATE** | SEL · knob 3 | dépend de SHAPE | DECAY : la durée du glissement, de 10 ms à 10 s (chute du kick = court, laser = long). SINE : la vitesse de l'oscillation, de 0 à 2000 Hz ; seuls les premiers crans font un vibrato, plus haut c'est de la FM (cloches, zap), et un rate plus haut le fait aussi durer plus longtemps. RANDOM : la vitesse à laquelle le pitch dérive au hasard, de 20 Hz à 20 kHz ; lent = dérives, rapide = son métallique. |

### Noise : le claquement et le sifflement

Le noise est un bruit blanc qui passe par un filtre et une envelope. L'oscillator et le noise sont séparés : le MIX décide de ce que vous entendez de chacun.

| Réglage | Où | Plage | Ce que ça fait |
|---|---|---|---|
| **FILTER MODE** | LFO · knob 1 | LP, BP, HP | LP garde les graves (coup sourd, sombre), HP les aigus (hi-hat, grésillement), BP une bande autour de la fréquence. |
| **FILTER Q** | LFO · knob 2 | 0,1 à 10000 | Résonance. Basse = douce, vers le knob 64 = un pic net, haute = le filtre sonne comme un ton avec une hauteur : BP avec un Q haut et une envelope courte est un ping accordé (tom, cowbell, block). |
| **FILTER FREQUENCY** | LFO · knob 3 | 20 Hz à 20 kHz | L'endroit où le filtre travaille. |
| **ENVELOPE MODE** | EDIT · knob 1 | EXP, LIN, CLAP | EXP : une montée rapide et une chute naturelle, comme l'oscillator. LIN : montée et chute en ligne droite. CLAP : quelques rafales courtes, comme des mains. |
| **ENVELOPE ATTACK** | EDIT · knob 3 | 0 à 5 s | EXP et LIN : la montée. CLAP : le temps pendant lequel le noise reste à plein niveau avant les rafales. |
| **ENVELOPE DECAY** | EDIT · knob 4 | 10 ms à 10 s | La chute. En CLAP il fixe aussi l'écart entre les rafales : court = un bourdonnement rapide, long = des battements séparés. |

### Mix, EQ, distorsion

| Réglage | Où | Plage | Ce que ça fait |
|---|---|---|---|
| **OSC/NOISE MIX** | FX · knob 1 | 100.00/00.00 à 00.00/100.00 | L'écran montre la part d'oscillator et la part de noise en pourcentage. 100.00/00.00 = oscillator seul, 49.61/50.39 (le milieu du knob) = les deux à plein niveau, 00.00/100.00 = noise seul. Une track neuve part de 100.00/00.00 : pas de noise tant que vous ne le bougez pas. |
| **EQ FREQUENCY** | FX · knob 2 | 20 Hz à 20 kHz | Le centre de l'unique bande d'EQ. |
| **DIST** | FX · knob 3 | 0 à 127 | Distorsion, très raide : le premier quart est une chaleur légère, au-delà du milieu c'est lourd, tout en haut c'est un fuzz. Elle est asymétrique, donc elle ajoute du volume et du mordant. |
| **EQ GAIN** | FX · knob 4 | 64 = plat ; 40 dB de chaque côté | Accentue ou coupe la bande (large, une seule cloche). L'EQ est **avant** la distorsion : accentuer puis distordre donne un son plus lourd que l'inverse. |

### Velocity : des accents qui changent le son

Le sequencer joue un step normal à environ un tiers de la velocity maximale et un step avec accent à la maximale. Les trois knobs VELOCITY (GLO · VEL, knobs 1 à 3) disent à quel point chaque partie la suit.

| Réglage | Où | Ce que ça fait |
|---|---|---|
| **OSC** | GLO · knob 1 | 0 = l'oscillator ignore la velocity. Plus haut = il joue plus doucement sur les steps normaux, les accents restent au maximum. |
| **NOISE** | GLO · knob 2 | Pareil pour le noise. |
| **PITCH MOD** | GLO · knob 3 | Pareil pour la quantité de pitch mod : sur les steps normaux le mouvement se réduit. |

C'est raide : une petite valeur coupe déjà beaucoup un step normal, et à partir du knob 95 environ un step normal perd entièrement cette partie. Avec les trois à 0, un accent sonne exactement comme un step normal : l'accent ne s'entend qu'à travers ces knobs. Par exemple, une caisse claire avec VELOCITY OSC haut n'a son ton que sur les accents et ne joue que du noise sur les steps normaux.

### Output : level, pan, cut, send

| Réglage | Où | Plage | Ce que ça fait |
|---|---|---|---|
| **LEVEL** | HOME · knob 1 | du silence à environ +10 dB (défaut 101 = environ -2,5 dB) | Le volume de la track. La voix sature en pleine échelle : si ça grésille, baissez-le. |
| **PAN** | HOME · knob 2 | 0 gauche, 64 centre, 127 droite | La position dans le champ stéréo. |
| **LOW CUT** | HOME · knob 3 | 20 à 663 Hz | Retire les graves (12 dB par octave). À 20 Hz il ne fait rien d'audible. |
| **HIGH CUT** | HOME · knob 4 | 20 kHz à 1 kHz | Retire les aigus (12 dB par octave). Tout à droite = ouvert. |
| **REVERB SEND** | HOME, 2e appui · knob 1 | 0 à 127 | Quelle part de la track part au reverb. |
| **DELAY SEND** | HOME, 2e appui · knob 2 | 0 à 127 | Pareil pour le delay. |

Les send sortent après les cut et le sidechain, avant le pan, et ils fonctionnent quoi que disent le D/W de PERFORM et ALL FX.

### Autour de la voix

Commun à toutes les tracks :

- **Choke group** (EDIT, ALGORITHM à droite) : un coup d'une track du groupe arrête aussitôt les autres (le hi-hat fermé arrête l'ouvert).
- **Sidechain** (HOME, 2e appui) : une track d'entrée fait baisser les tracks que vous choisissez dans l'écran DUCK pendant qu'elle joue ; le knob de threshold dit de combien (0 = off).
- **Master low cut** (PERFORM · knob 3, 20 à 663 Hz) : un low cut sur tout le mix, avant le compresseur.
- **Compresseur** (GLO · COMP) : 4:1, soft knee, attack de 0,1 à 100 ms, release de 10 à 1500 ms, threshold de -48 à 0 dB, make-up de 0 à 24 dB. Un limiter placé après lui garde la sortie sous la pleine échelle.
- **Reverb** (maintenez FX) : decay, tone, low cut, niveau de return ; une pièce granuleuse des débuts du numérique. **Delay** (maintenez HOME) : temps synchronisé (de 1/32 à 1/2), feedback, tone, TAPE (du numérique propre à la bande usée), low cut, return, ping-pong.

### Enhanced et classic

22 des 26 réglages sont ce que porte un preset Patternarium (les 21 réglages de son, level compris, plus le pan). Quatre sont des **extras de FM1 Tonic** : LOW CUT, HIGH CUT, REVERB SEND et DELAY SEND, comme le choke group, le sidechain, la section master, les p-locks, la chance et le microtiming. En **classic** (GLO > settings > TONIC ENGINE) les quatre extras sont grisés et silencieux, ainsi que le sidechain, si bien que ce que vous construisez n'utilise que les réglages qu'un fichier `.mtpreset` porte ; les valeurs reviennent avec enhanced. Un preset Patternarium chargé garde ses valeurs exactes jusqu'à ce que vous tourniez ce knob.

### Recettes de sons

Ce sont des points de départ : changez une valeur à la fois et écoutez.


#### Kick

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 20 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +37.04 sm | RATE 25 | - |
| **ENV** | WAVE SINE | FREQUENCY 56 Hz | ATTACK 0 | DECAY 72 |
| **LFO** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **EDIT** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

AMOUNT est la profondeur de la chute et RATE sa vitesse. Pour un clic, mettez MIX 40 sur FX, puis sur LFO : MODE HP, FREQUENCY 2.9 kHz, et sur EDIT : DECAY 15. Pour un boom plus long, montez ENV DECAY.

#### Snare

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 15 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +12.85 sm | RATE 13 | - |
| **ENV** | WAVE TRI | FREQUENCY 196 Hz | ATTACK 0 | DECAY 46 |
| **LFO** | MODE HP | Q 22 | FREQUENCY 2.0 kHz | - |
| **EDIT** | MODE EXP | laissez tel quel | ATTACK 0 | DECAY 55 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Plus de claquement : montez le MIX. Plus de corps : baissez le MIX. Pour des ghost notes légères, mettez GLO NOISE à 30 : les steps normaux jouent le noise plus doucement et les accents le jouent à plein.

#### Hi-hat fermé

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **ENV** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **LFO** | MODE HP | Q 22 | FREQUENCY 7.1 kHz | - |
| **EDIT** | MODE EXP | laissez tel quel | ATTACK 0 | DECAY 25 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

OSC/NOISE MIX 00.00/100.00 veut dire noise seul. Baissez la fréquence de LFO pour un hi-hat plus sombre.

#### Hi-hat ouvert

Comme le hi-hat fermé, avec **EDIT DECAY 68**. Mettez les deux hi-hats dans un **choke group** pour que le fermé arrête l'ouvert : sur chacune des deux tracks, maintenez FUNC et tournez le knob 2 de EDIT vers la droite.

#### Clap

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **ENV** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.1 kHz | - |
| **EDIT** | MODE CLAP | laissez tel quel | ATTACK 0 | DECAY 60 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Avec ces valeurs vous obtenez 3 à 4 rafales rapides en un dixième de seconde environ. EDIT DECAY plus bas = un bourdonnement plus serré, plus haut = des battements plus lents et séparés et une queue plus longue (75 à 85). Un peu de reverb send (HOME, 2e appui) en fait un clap de salle.

#### Tom

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +17.39 sm | RATE 20 | - |
| **ENV** | WAVE SINE | FREQUENCY 120 Hz | ATTACK 0 | DECAY 65 |
| **LFO** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **EDIT** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Utilisez le même son sur d'autres tracks avec une FREQUENCY différente (80 Hz pour un floor tom, 200 Hz pour un tom aigu).

#### Zap

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +47.62 sm | RATE 62 | - |
| **ENV** | WAVE SAW | FREQUENCY 1.0 kHz | ATTACK 0 | DECAY 75 |
| **LFO** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **EDIT** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Le pitch tombe de 4 octaves en 0,3 s environ. Envoyez-le au delay (HOME, 2e appui) pour un laser classique.

#### Rimshot

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 30 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +11.34 sm | RATE 10 | - |
| **ENV** | WAVE TRI | FREQUENCY 552 Hz | ATTACK 0 | DECAY 25 |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.8 kHz | - |
| **EDIT** | MODE EXP | laissez tel quel | ATTACK 0 | DECAY 22 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Un clic sec et court (environ 45 ms). Montez DIST pour un rim plus dur ; déplacez le MIX vers la gauche pour plus de ton.

#### Sub bass

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 35 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +6.80 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 41 Hz | ATTACK 6 | DECAY 80 |
| **LFO** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **EDIT** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Une note grave d'environ 0,9 s avec une petite chute de pitch au début et un peu de DIST, pour qu'elle s'entende aussi sur de petites enceintes. Changez FREQUENCY (30 à 60 Hz) pour une autre note, ou raccourcissez DECAY pour un kick long.

#### Conga

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 82.68/17.32 | EQ FREQ 20 Hz | DIST 10 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +5.29 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 231 Hz | ATTACK 0 | DECAY 54 |
| **LFO** | MODE BP | Q 30 | FREQUENCY 1.6 kHz | - |
| **EDIT** | MODE EXP | laissez tel quel | ATTACK 0 | DECAY 20 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Un ton rond avec un claquement de noise très court (environ 0,2 s en tout). Montez FREQUENCY (par exemple 330 Hz) sur une autre track pour une conga aiguë.

#### Crash

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **ENV** | non audible (OSC/NOISE MIX 00.00/100.00), laissez tel quel | | | |
| **LFO** | MODE HP | Q 10 | FREQUENCY 5.1 kHz | - |
| **EDIT** | MODE EXP | laissez tel quel | ATTACK 0 | DECAY 100 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Une cymbale de noise pur qui dure environ 2,3 s. Baissez DECAY (85 donne environ 1 s) pour un splash, ou montez la FREQUENCY de LFO pour un son plus fin. Envoyez-la un peu au reverb (HOME, 2e appui).

#### Clave

| Bouton | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT 0.00 sm | RATE 0 | - |
| **ENV** | WAVE SINE | FREQUENCY 2.5 kHz | ATTACK 0 | DECAY 28 |
| **LFO** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **EDIT** | non audible (OSC/NOISE MIX 100.00/00.00), laissez tel quel | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | laissez tel quel |

Un ton aigu et sec d'environ 50 ms, sans mouvement de pitch (AMOUNT 0.00 sm). Changez FREQUENCY (2 à 3 kHz) pour un autre accordage.

**Conseils.** Pour entendre une partie seule, tournez le MIX tout à gauche (100.00/00.00, oscillator seul) ou tout à droite (00.00/100.00, noise seul). Pour que les accents changent le son et pas seulement le volume, montez les knobs de GLO. Pour essayer une valeur sur un seul step, maintenez le step et tournez le knob (p-lock, chapitre 4). Un son que vous aimez se garde avec SAVE (chapitre 5).

## 9. Crédits

**FM1 Tonic est fait par Luigi Rossi**, le développeur et l'esprit créatif de ce firmware : la drum machine Tonic, son sequencer, les écrans, les presets et les outils web (émulateur, installateur, backup). C'est un logiciel libre sous la GNU General Public License, version 3 ; si vous partagez le firmware, vous devez partager son code source.

**Luigi Rossi en ligne :** [linktr.ee/luigirossi](https://linktr.ee/luigirossi)

LICENSING.md indique l'origine et la licence de chaque fichier. Il s'appuie sur le travail de :

- **libpo32** d'Eric Lewis (MIT) : le point de départ du code de synthèse de batterie du FM1 Tonic. Rien n'est échantillonné.
- **X0X** de Charles Vestal, le firmware dont FM1 Tonic est issu, et les pages web (émulateur, installateur, backup) ; X0X est un fork de **Felucca** de Leo Kuroshita (@kurogedelic), Hügelton Instruments : la plateforme (couche matérielle, USB, chargeur de mise à jour, installateur, stockage) et la carte de récupération FM-1-transporter.
- Le reverb et le delay sont portés depuis **9W9** de athousanddetails, basé sur **ER-99** de Matthew Cieplak (GPL-3.0). Le compresseur master suit Giannoulis, Massberg et Reiss ; le filtre master est le filtre à variables d'état TPT de Vadim Zavalishin.
- **Melodee** de keremimo et **Jangada** de zednaked : ce qu'ils ont découvert sur le second cœur du FM-1.
- Polices : **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov), **DotGothic16** (The DotGothic16 Project Authors), toutes sous SIL Open Font License.

M-VAVE et FM-1 sont des marques de leurs propriétaires respectifs. FM1 Tonic n'est pas affilié à M-VAVE, Sonic Charge ou Luigi Rossi, ni approuvé par eux.
