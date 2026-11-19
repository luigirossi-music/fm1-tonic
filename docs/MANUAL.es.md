# FM1 TONIC

Inspirado en Microtonic VST de Sonic Charge, FM1 TONIC es un sintetizador de batería de 8 voces donde cada sonido se sintetiza desde cero, sin ningún sample. Combina un motor de percusión profundo con un secuenciador de 1 a 64 steps (con accents, fills y 16 patterns por proyecto), un mixer integrado con level y pan, reverb, delay y un compresor master. Con 40 project slots (8 presets de fábrica + 32 slots de usuario) y un flujo de trabajo pensado para el directo, pasa sin esfuerzo de beats rítmicos y contundentes a texturas ambient en evolución.

## Características

**Sonido**

- Sintetizador de batería de 8 voces: cada track tiene su propio oscillator, pitch mod, noise, mix, EQ y distorsión.
- Level y pan por track, y reverb send y delay send por track (más low cut y high cut en el engine Enhanced).
- Engine Enhanced o Classic (Classic = solo los ajustes básicos de sonido, sin los extras de FM1 TONIC).
- Randomizer de track: un sonido nuevo al azar para el track seleccionado.
- Choke groups (por ejemplo hi-hat cerrado y abierto).
- Compresor sidechain: cualquier track puede hacer duck a los demás.

**Secuenciador**

- Secuenciador de 64 steps, 16 patterns por proyecto. Longitud fija de 8, 16, 32 o 64 steps, o longitud libre de 1 a 64.
- Song mode.
- Accent global y accent por track, fill con retrig (2x, 4x, 8x) y trig % (chance) en cada step.
- Parameter lock (limitado: 3 knobs por track en cada pattern).
- Record automation (limitado: 3 knobs por track).
- Grabación en vivo, quantized o unquantized. Swing, quantization y tempo.
- Teclado (ARP): toca un track como un teclado y graba las notas en los steps (un piano roll).
- Copiar y pegar un pattern, un track, y un pattern al vuelo mientras suena la canción.
- Mute y solo.
- Undo y redo.

**Mezcla e interpretación**

- Mixer master: level y pan de cada track (el punto rojo marca el centro).
- Compresor master.
- Página Performance: 2 efectos master (reverb y delay), un low cut y ALL FX OFF.
- Página Memory: cuánta memoria del proyecto está ocupada.

**Proyectos y guardado**

- 40 project slots: 8 presets de fábrica y 32 tuyos.
- Autosave y quick save.
- Nombre del proyecto: GENERATE o MANUAL. Renombrar el proyecto.
- Copia de seguridad y restauración desde la página web. Exportar e importar presets `.mtpreset`.
- Los presets funcionan en los dos sentidos: carga presets `.mtpreset` de Patternarium y Microtonic, y exporta los tuyos de vuelta para el Microtonic VST (probado; el sonido es muy parecido, no idéntico).

**Pantalla, luces y ajustes**

- Indicador LED 4/4 (teclas blancas 1, 5, 9 y 13), luz de las teclas on u off.
- Zoom knobs (el dial grande cuando giras un knob), tema de color.
- Volumen de salida NORMAL o LOUD. USB audio.

**MIDI**

- Selector de canal MIDI, notes out on u off, clock out on u off.
- Control changes (CC): 26 parámetros de sonido, por track seleccionado o por canal MIDI.
- Panel remoto: toca las teclas del propio FM-1 desde un teclado externo o un Digitakt.

## 1. El panel

![El panel de FM1 TONIC: knobs MASTER, SELECT, PRESETS, ALGORITHM y KNOB 1 a 4; botones FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- y OCT+; teclas negras TRACK 1 a 8, ACCENT, FILL, FUNC; teclas blancas 1 a 16](img/fm1-tonic-panel.svg "")

- **Knobs.** MASTER es el volumen. SELECT cambia de página (en una lista: arriba y abajo). PRESETS elige uno de los 40 presets. KNOB 1 a 4 ajustan los cuatro valores de la página que se ve en la pantalla (el que giras se muestra más grande). ALGORITHM pasa a una segunda página o elige una voz, según la página (las tablas de páginas lo indican). Un pequeño triángulo azul en una esquina inferior de la pantalla significa que hay una segunda página detrás de ALGORITHM: triángulo derecho = gira ALGORITHM a la derecha, triángulo izquierdo = gíralo a la izquierda para volver.
- **Teclas negras.** 1 a 8 son TRACK 1 a 8, 9 es ACCENT, 10 es FILL, 11 es FUNC.
- **Teclas blancas 1 a 16** son los 16 steps de la pista seleccionada. Un pattern puede tener hasta 64 steps, que se muestran de 16 en 16 (ver SEQ).
- **OCT- / OCT+** suben o bajan una octava el tono de la pista seleccionada (en ARP: la octava del teclado). En una pregunta, OCT+ = YES, OCT- = NO.

### Contenido

1. El panel {#s1}
2. Ruta de la señal {#s2}
3. Páginas, botón por botón {#s3}
  - Combinaciones de botones
4. Tocar {#s4}
5. Presets y guardado {#s5}
6. Luces, MIDI y USB {#s6}
7. Instalar, copia de seguridad, recuperación {#s7}
8. El engine {#s8}
  - Recetas de sonido
9. Créditos {#s9}

---

**Cómo leer las tablas de páginas.** Cada botón tiene una tabla, con una fila por página. El texto normal es lo que hace el control por sí solo. {r:Rojo} es lo que hace con **FUNC** pulsado, {o:naranja} es lo que hace con **FILL** pulsado (a la manera del Digitakt).

**Lo que funciona en todas las páginas** (las tablas siguientes no lo repiten):

| Teclas | Qué hace |
|---|---|
| Tecla negra 1 a 8 | Selecciona la pista y la toca (también durante la reproducción). |
| Tecla blanca 1 a 16 | Los steps: un toque = trig activado, otro toque = desactivado. Un pattern tiene hasta 64 steps, mostrados de 16 en 16. |
| Tecla blanca mantenida + ACCENT / FILL | Ese step recibe un accent / un fill (otra vez = se quita). |
| ACCENT o FILL mantenido | Los trigs de la pista que llevan ese accent / fill parpadean en las teclas blancas (los demás trigs quedan encendidos). |
| {r:FUNC + tecla negra 1 a 8} / {o:FILL + tecla negra 1 a 8} | Silencia la pista (mute) / la deja sola (solo) (otra vez = vuelve). |
| {r:FUNC + tecla blanca} | Cambia de pattern (durante la reproducción: al final del compás). |
| {r:FUNC + ACCENT} / {r:FUNC + FILL} / {r:FUNC + PLAY} | Copia el pattern / lo pega (pregunta ¿PASTE PATTERN?) / lo borra (pregunta ¿CLEAR PATTERN?). OCT+ = sí, OCT- = no. |
| {r:FUNC + tecla negra 1 a 8 mantenida, luego ACCENT / FILL / PLAY} | Copia el sonido de esa pista / lo pega / borra el pattern de esa pista (pide confirmación). |
| {r:FUNC + SEQ} | Modo song activado / desactivado (pide confirmación). |
| {r:FUNC + ARP} | Borra la song (pide confirmación). |
| {r:FUNC + SAVE} / {r:FUNC + HOME} | Guardado rápido en el lugar cargado / cambia el nombre del proyecto. |
| {r:FUNC + REC} | QUANTIZED REC / UNQUANTIZED REC. |
| ENV + un knob | Ese knob vuelve a su valor por defecto. |
| GLO + ACCENT / GLO + FILL | Deshacer / rehacer (en cualquier página). |
| REC + PLAY, luego teclas y knobs | Grabación en vivo: las teclas negras 1 a 8 escriben trigs, los knobs escriben automatización (3 knobs por pista). |
| PLAY | Inicia y detiene el pattern. |
| OCT- / OCT+ | El tono de la pista una octava abajo / arriba. En una pregunta: NO / YES. |

Excepciones: en PERFORM, MUTE / SOLO y en la página de patterns las teclas negras son botones de mute; en ARP todas las teclas son teclas de teclado. Están todas en la lista siguiente.

**Donde las teclas hacen otra cosa.** Las teclas del panel (teclas negras 1 a 8 = pistas, ACCENT, FILL, FUNC, teclas blancas 1 a 16 = steps) y los atajos de arriba valen en todas las páginas de este manual, salvo en las páginas de esta lista, donde las teclas tienen otra función:

| Página | Qué hacen las teclas ahí |
|---|---|
| **ARP** | Todas las teclas, blancas y negras, son teclas de teclado que tocan la voz seleccionada. |
| **PERFORM, MUTE / SOLO, PATTERN** (SEQ, 2ª pulsación) | Las teclas negras 1 a 8 son botones de mute (botones de solo en modo SOLO), no seleccionan la pista. Las teclas blancas 1 a 16 eligen el siguiente pattern. FUNC y FILL, con un toque, cambian entre el modo MUTE y el modo SOLO. |
| **DUCK** (SENDS, ALGORITHM a la derecha) | Las teclas negras 1 a 8 eligen qué pistas reciben el duck. |
| **CHOKE** (EDIT, ALGORITHM a la derecha) | Las teclas negras 1 a 8 eligen qué pistas están en el grupo choke. |
| **Pantallas SONG y PATTERNS** (modo song) | Las teclas blancas 1 a 16 escriben un pattern en el compás. FILL borra el compás. |
| **Modo copiar-pegar** (después de FUNC + ACCENT) | Las teclas blancas 1 a 16 son los lugares donde pegar. |
| **FUNC mantenido** | Las teclas blancas 1 a 16 son los patterns 1 a 16; las teclas se encienden donde un pattern o una pista tiene notas. |

**Cosas que existen solo en algunas páginas.**

| Página | Teclas | Qué hace |
|---|---|---|
| Páginas de sonido: MIX 1, SEL, ENV, LFO, EDIT, VEL | Tecla blanca mantenida + knob 1 a 4 | **P-lock**: ese step recibe su propio valor del knob. Tecla blanca mantenida + SAVE + el mismo knob lo quita. Una pista guarda 3 knobs con p-lock por pattern. |
| Todas las páginas menos PERFORM, HOME, SONG, COMP y las listas | Tecla blanca mantenida + SELECT | La **chance** de ese step: de 5 a 100 %, luego 1:2, 2:2, 1:3 ... 8:8. |
| ENV | OCT- / OCT+ | La frecuencia del oscilador una octava abajo / arriba. |
| EDIT | {r:FUNC + knob 2}, {r:FUNC, toque en EDIT}, EDIT o LFO mantenido + {r:FUNC + ACCENT / FILL / PLAY} | Entra / sale del grupo choke; DELETE AUTOMATION?; copia / pega / reinicia esta página de la pista. |
| SEQ | Knobs, SELECT, ALGORITHM, {o:FILL + SEQ} | Ver la tabla de SEQ en el capítulo 3. |
| GLO > VEL | {r:FUNC + knob 4} | Un sonido aleatorio nuevo para la pista seleccionada. |
| GLOBAL (HOME, 3ª pulsación) | Knob 4 (o HOME mantenido + SELECT) | El tempo. |

## 2. Ruta de la señal

![El engine de FM1 TONIC: cada una de las 8 pistas tiene un modulador de pitch y un oscillator, un filtro de noise y un envelope de noise, un mix, una distorsión y un EQ, luego cuts, level y pan; las 8 pistas pasan por el sidechain, el low cut master y el compresor hasta el volumen master; reverb y delay son efectos de send](img/fm1-engine.svg "Toda la ruta de la señal. La palabra pequeña de cada caja es el botón que abre esa página.")

Cada pista fabrica su sonido desde cero con el engine de batería de FM1 TONIC: un modulador de pitch y un oscillator, un filtro y un envelope de noise, un mix, una distorsión y un EQ, luego los cuts, el level y el pan de la pista; las 8 pistas pasan por el sidechain, el low cut master y el compresor hasta MASTER, con reverb y delay como efectos de send. El capítulo 8 explica cada ajuste.

**Enhanced y classic.** GLO > settings > Tonic Engine está en **enhanced** por defecto. En **classic** los extras de FM1 TONIC (los cuts de pista, los sends, el sidechain) quedan en gris y no hacen nada, así el sonido usa solo los ajustes que puede llevar un archivo `.mtpreset`. Sus valores se conservan y vuelven con enhanced. Level y pan siguen disponibles en classic. El modo es global, no se guarda en un preset.

## 3. Páginas, botón por botón

Una página con valores de **pista** (los knobs 1 a 4 de las páginas de sonido) pertenece a la pista seleccionada: cambia de pista con una tecla negra 1 a 8 y los valores cambian. EDIT o LFO mantenido + {r:FUNC + ACCENT / FILL / PLAY} copia, pega y reinicia **esta página** de la pista (solo en páginas de pista).

### FX

Pulsar = MIX 1 → PERFORM → MIX 1. Mantener (medio segundo) = REVERB. Desde la página de otro botón, FX vuelve a la última página FX que dejaste.

| Página | Knob 1 · 2 · 3 · 4 | También en esta página |
|---|---|---|
| **MIX 1** (1ª pulsación) | mix OSC / NOISE · EQ FREQ (20 Hz-20 kHz) · DIST · EQ GAIN (64 = plano) | Tecla blanca mantenida + knob: p-lock. SAVE + knob: olvida ese lock o motion. |
| **PERFORM** (2ª pulsación) | REVERB D/W · DELAY D/W (0-100 %, 0 al encender) · MASTER LOW CUT (20-663 Hz) · ALL FX (derecha activado, izquierda desactivado) | El número grande es el pattern que suena. SELECT: las páginas FX siguientes (se detiene en los extremos). ALGORITHM a la derecha: MUTE / SOLO. Teclas negras 1 a 8: mute. Teclas blancas 1 a 16: siguiente pattern. {r:FUNC toque}: modo MUTE, o quita todos los mutes. {o:FILL toque}: modo SOLO, o quita todos los solos. Sin p-locks. |
| **MUTE / SOLO** (PERFORM, ALGORITHM a la derecha) | igual que PERFORM | Ocho cajas en lugar del número: azul = la pista suena. Las teclas negras 1 a 8 hacen mute (modo MUTE) o añaden / quitan un solo (modo SOLO, varios a la vez). ALGORITHM a la izquierda: volver. |
| **REVERB** (mantener FX) | DECAY · TONE | ALGORITHM a la derecha: REVERB 2. |
| **REVERB 2** | LOW CUT (de lo que entra en la reverb) · RETURN (nivel en la mezcla) | ALGORITHM a la izquierda: volver. |
| **DELAY** (mantener también HOME) | TIME (1/32 ... 1/2, con valores T y con puntillo) · FEEDBACK · TONE · TAPE (el primer octavo = digital limpio, luego cinta con más desgaste) | ALGORITHM a la derecha: DELAY 2. |
| **DELAY 2** | LOW CUT · RETURN · PING-PONG (off / on) | ALGORITHM a la izquierda: volver. |

ALL FX en off silencia los dos sends D/W de PERFORM y el low cut master; los sends propios de las pistas siguen funcionando. Los mutes y solos los comparten PERFORM y MUTE / SOLO; los solos no se guardan y terminan al cargar un preset. Las páginas de efectos son globales (todas las pistas las comparten): SELECT recorre PERFORM, REVERB, DELAY, REVERB 2, DELAY 2.

### HOME

Pulsar = MIX 2 → SENDS → GLOBAL → MIX 2. Mantener = DELAY.

| Página | Knob 1 · 2 · 3 · 4 | También en esta página |
|---|---|---|
| **MIX 2** | LEVEL · PAN · LOW CUT (20-663 Hz, nunca apagado) · HIGH CUT (de 20 kHz a 1 kHz) | Los dos cuts están en gris en classic. |
| **SENDS** | REVERB SEND · DELAY SEND · SIDECHAIN INPUT (pista 1 a 8, número grande) · SIDECHAIN THRESHOLD (0-100 %, 0 = nada hace duck) | ALGORITHM a la derecha: la pantalla DUCK (abajo). Los sends funcionan diga lo que diga el D/W de PERFORM y ALL FX. |
| **DUCK** (SENDS, ALGORITHM a la derecha) | - | Teclas negras 1 a 8: azul = con duck, gris = sin duck, morado = la entrada. La entrada no puede recibir duck. No se hace duck a nada hasta que eliges. ALGORITHM a la izquierda o HOME: volver. |
| **GLOBAL** | QUANTIZATION (1/8 ... 1/32) · SWING · FILL RATE (2x, 4x, 8x) · TEMPO | - |

El duck sigue el sonido de la pista de entrada (un kick largo hace duck largo; una entrada en mute no hace duck) y también hace duck a los sends de las pistas. Un punto rojo marca una pista con duck, un punto morado la entrada; el punto pulsa cada vez que la entrada hace duck. Un solo sidechain para todas las pistas, guardado con el preset.

### SEL, ENV, LFO, EDIT (las páginas de la voz)

Las cuatro son páginas de pista: en ellas funcionan los p-locks y la chance.

| Botón | Knob 1 · 2 · 3 · 4 | También en esta página |
|---|---|---|
| **SEL** PITCH MOD | SHAPE (DECAY, SINE, RANDOM) · AMOUNT (64 = nada) · RATE · - | |
| **ENV** OSCILLATOR | WAVE (SINE, TRI, SAW) · FREQUENCY (20 Hz-20 kHz) · ATTACK · DECAY | OCT- / OCT+ mueven la frecuencia una octava. |
| **LFO** NOISE FILTER | MODE (LP, BP, HP) · Q · FREQUENCY · - | |
| **EDIT** NOISE ENVELOPE | MODE (EXP, LIN, CLAP) · botón CHOKE · ATTACK · DECAY | {r:FUNC + knob 2 a la derecha}: la pista entra en el choke group; a la izquierda: sale. El knob 2 solo muestra un mensaje. ALGORITHM a la derecha: la pantalla CHOKE. {r:FUNC, toque en EDIT}: ¿DELETE AUTOMATION? (OCT+ sí, OCT- no). |
| **CHOKE** (EDIT, ALGORITHM a la derecha) | - | Teclas negras 1 a 8: azul = en el grupo. Un golpe de una pista del grupo detiene a las demás al instante, como un hi-hat cerrado detiene a uno abierto. ALGORITHM a la izquierda: volver. Se guarda con el preset. |

DELETE AUTOMATION quita los p-locks y el motion grabado de la pista seleccionada en el pattern que suena (las otras pistas, los otros patterns y las chances se quedan; undo funciona). Mantener FUNC + EDIT y luego pulsar ACCENT, FILL o PLAY es la copia de página, no un borrado.

### GLO

Pulsar = VEL → MIXER → COMP → MEMORY → SETTINGS → VEL.

| Página | Knob 1 · 2 · 3 · 4 | También en esta página |
|---|---|---|
| **VEL** | OSC · NOISE · PITCH MOD (cuánto mueve la velocity a cada uno) · RANDOM | {r:FUNC + knob 4}: la pista seleccionada recibe un sonido aleatorio nuevo (solo los 21 ajustes de sonido). El knob 4 solo muestra un recordatorio. Página de pista: los p-locks funcionan. |
| **MIXER** | LEVEL de las cuatro pistas mostradas | Ocho sliders altos; SELECT cambia entre las pistas 1 a 4 / 5 a 8 y, como dice la pista azul en pantalla, alterna level / pan (pan centro = rojo). Las teclas negras 1 a 8 eligen una pista y su grupo. Un golpe ilumina su slider. {r:FUNC + tecla negra 1 a 8}: mute. Un punto rojo bajo un slider = con duck, morado = entrada del sidechain. Aquí funciona la chance. |
| **COMP** | ATTACK · RELEASE · THRESHOLD · MAKE-UP | Compresor master, ratio 4:1, una aguja muestra la reducción de ganancia. Por defecto: nada comprimido (0.0 dB / 0.0 dB). Solo SAVE lo guarda. |
| **MEMORY** | - | Una batería muestra cuánta memoria del proyecto está ocupada (patterns, song, motion, notas de ARP). Morado desde el 90 %, FULL al 100 %: entonces se rechaza el guardado. |
| **SETTINGS** | lista (SELECT mueve) | Filas: clock out, notes out, canal MIDI de Tonic, output level, USB audio, key light, 4/4 LED, tema de color, zoom knob (yes / no), Tonic Engine, name projects, save, autosave, clear pattern, factory reset, performance, cores, about, battery. Las que no son obvias se explican justo debajo. Los cambios se guardan solos más o menos un segundo y medio después de que dejes de tocar (el secuenciador debe estar parado). |

#### Ajustes de GLO: los que no son obvios

Los ajustes están en la lista de GLO (SELECT sube y baja, ALGORITHM cambia el valor). Los valores por defecto van en negrita.

| Ajuste | Opciones | Qué hace |
|---|---|---|
| **OUTPUT** | NORMAL / **LOUD** | LOUD (el ajuste de fábrica) hace que las salidas del FM-1 suenen 6 dB más fuertes (el fondo de escala del códec). NORMAL deja 6 dB de margen: úsalo si el sonido es demasiado fuerte y vigila el compresor y el limiter. |
| **USB AUDIO** | **FOLLOWS KNOB** / FULL | FOLLOWS KNOB: el ordenador recibe lo que oyen los auriculares, después del knob MASTER. FULL: el ordenador recibe siempre el nivel completo y MASTER solo cambia las salidas propias del FM-1 (altavoz, auriculares, line out). Usa FULL para grabar a nivel completo y seguir monitorizando bajito. |
| **4/4 LED** | OFF / **ON** | ON (el ajuste de fábrica): las teclas blancas 1, 5, 9 y 13 brillan débilmente para marcar los cuatro tiempos del compás (no en PERFORM). |
| **ZOOM KNOB** | **YES** / NO | YES: el knob que giras se agranda sobre la pantalla. NO: solo se resalta, así toda la página sigue visible. La pantalla SONG nunca hace zoom. |
| **NAME PROJECTS** | **GENERATE** / MANUAL | GENERATE inventa el nombre de un proyecto guardado (ALGORITHM = otro). MANUAL: la pantalla de nombre es un editor de letras (ver capítulo 5). |
| **TONIC ENGINE** | **ENHANCED** / CLASSIC | CLASSIC pone en gris y silencia los extras de FM1 TONIC (cuts de pista, sends, sidechain) para que el sonido use solo los ajustes que puede llevar un archivo `.mtpreset` (capítulos 2 y 8). |
| **2ND CORE** | **ON** / OFF | El chip del FM-1 tiene dos núcleos y Tonic reparte entre ellos el cálculo del sonido. Surte efecto en el siguiente encendido. Si el FM-1 se cuelga alguna vez con dos núcleos, se pone solo en OFF; vuelve a ponerlo en ON aquí. |

### SEQ y ARP

| Página | Knob 1 · 2 · 3 · 4 | También en esta página |
|---|---|---|
| **SEQ** (1ª pulsación) | FIXED LEN (8, 16, 32, 64) · AUTOSCROLL · FREE LEN (1 a 64) · SWING | Teclas blancas: los steps. SELECT: las páginas de steps (de 16 en 16; AUTOSCROLL off). ALGORITHM: la voz (1 a 8, luego AC = el accent global). Mantén un step + ACCENT / FILL: flags (accent = azul oscuro, fill = amarillo, ambos = verde). {r:FUNC + SEQ}: modo song on / off (confirma con OCT+). {o:FILL + SEQ}: la pantalla SONG. |
| **PATTERN** (SEQ, 2ª pulsación) | SONG ON / OFF | Teclas negras 1 a 8: mute; teclas blancas 1 a 16: eligen el siguiente pattern. Tecla apagada = pattern vacío, luz tenue = tiene notas, parpadeo lento = el actual, parpadeo rápido = el siguiente (en cola). {r:FUNC toque} y {o:FILL toque} como en PERFORM. SEQ otra vez: volver. |
| **ARP** | FIXED LEN · OCTAVE (también OCT- / OCT+, -3 a +3) · - · - | La pantalla SEQ, pero todas las teclas (blancas y negras) tocan la voz seleccionada como un teclado. Las teclas C (tenues) tocan el pitch propio de la voz en la octava 0. ALGORITHM: la voz. No se graba nada. SEQ: volver. {r:FUNC + ARP}: borra la song (¿DELETE SONG?). Tecla negra 11 mantenida + PLAY: borra el pattern (¿CLEAR PATTERN?), PLAY no arranca entonces. |

Alargar un pattern copia lo que hay en la parte nueva (de 8 a 16 copia los steps 1 a 8 en el 9 a 16); acortarlo solo lo acorta.

### PLAY, REC, SAVE

| Botón | Qué hace |
|---|---|
| **PLAY** | Reproducir y parar. {r:FUNC + PLAY}: borra el pattern (pregunta antes). |
| **REC** | REC y luego PLAY: grabación en vivo. Cada tecla negra 1 a 8 escribe un trig de su pista en el step más cercano; cada knob de sonido que giras se graba como automatización. No en PERFORM ni en la página de patterns. {r:FUNC + REC}: QUANTIZED REC (por defecto) / UNQUANTIZED REC (el golpe conserva su tiempo exacto, dentro del step); si está sonando, también activa la grabación. |
| **SAVE** | Ver capítulo 5. {r:FUNC + SAVE}: quick save. {r:FUNC + HOME}: renombrar. |

### Combinaciones de botones

Todas las combinaciones de botones y teclas, agrupadas por el botón que mantienes primero. Tecla negra 9 = ACCENT, 10 = FILL, 11 = FUNC. {r:Rojo} = FUNC mantenido, {o:naranja} = FILL mantenido. La última columna dice dónde funciona: «Todas las páginas» = todas las páginas salvo ARP y las dos páginas PERFORM.

**FUNC mantenido (tecla negra 11)**

| Teclas | Qué hace | Dónde |
|---|---|---|
| {r:FUNC + tecla negra 1 a 8} | Silencia la pista (otra vez = vuelve). Actúa al soltar la tecla. | Todas las páginas |
| {r:FUNC + tecla blanca 1 a 16} | Cambia a ese pattern (durante la reproducción: al final del compás). | Todas las páginas; no en la pantalla SONG ni en modo song (lo decide la song) |
| {r:FUNC + ACCENT} | Copia el pattern. La pantalla dice PRESS PATTERN TO PASTE / PRESS SEQ TO EXIT, todas las teclas de pattern parpadean (un pattern vacío más tenue) y SEQ parpadea: pulsa una tecla de pattern y la copia queda ahí, y ese pattern pasa a ser el actual. Pulsa FUNC solo antes de elegir para quedarte en el modo y pegar varias veces. SEQ sale del modo y abre la página del secuenciador; {r:FUNC + ACCENT} otra vez o cualquier otro botón también lo deja. | Todas las páginas |
| {r:FUNC + FILL} | Pega el pattern copiado al soltar FILL. Pregunta PASTE PATTERN? (OCT+ = sí, OCT- = no). | Todas las páginas |
| {r:FUNC + FILL mantenido + tecla blanca} | Pega el pattern copiado en ese lugar al instante (sin pregunta). | Todas las páginas |
| {r:FUNC + PLAY} | Borra el pattern (pregunta CLEAR PATTERN?). No inicia la música. | Todas las páginas, también ARP (allí mantén la tecla negra 11, una tecla de piano en ARP: PLAY no inicia / detiene mientras está mantenida) |
| {r:FUNC + tecla negra 1 a 8 mantenida, luego ACCENT} | Copia el sonido de esa pista (no sus steps). | Todas las páginas |
| {r:FUNC + tecla negra 1 a 8 mantenida, luego FILL} | Pega el sonido copiado en esa pista (también mute, volumen y pan; la pista no queda en mute por eso). Sin pregunta. | Todas las páginas |
| {r:FUNC + tecla negra 1 a 8 mantenida, luego PLAY} | Borra el pattern de esa pista (pregunta CLEAR TRACK PATTERN?). | Todas las páginas |
| {r:FUNC + EDIT o LFO mantenido, luego ACCENT} | Copia esta página de la pista seleccionada (solo sus parámetros). | Páginas que tienen parámetros de pista (no GLO, HOME, SEQ) |
| {r:FUNC + EDIT o LFO mantenido, luego FILL} | La pega en la pista seleccionada, en la misma página. | Páginas que tienen parámetros de pista (no GLO, HOME, SEQ) |
| {r:FUNC + EDIT o LFO mantenido, luego PLAY} | Devuelve esta página de la pista a sus valores de encendido. Sin pregunta; undo lo deshace. | Páginas que tienen parámetros de pista (no GLO, HOME, SEQ) |
| {r:FUNC + EDIT}, pulsado solo | DELETE AUTOMATION?: quita los p-locks y el motion grabado de la pista seleccionada en el pattern que suena (pregunta). | Todas las páginas |
| {r:FUNC + SEQ} | Modo song activado / desactivado (pregunta: OCT+ = sí). | Todas las páginas |
| {r:FUNC + ARP} | DELETE SONG?: borra toda la song y sale del modo song (pregunta). | Todas las páginas |
| {r:FUNC + HOME} | Cambia el nombre del proyecto y lo guarda en el mismo lugar. | Todas las páginas |
| {r:FUNC + SAVE} | Guardado rápido en el lugar cargado. Sin pregunta. | Todas las páginas |
| {r:FUNC + REC} | Alterna QUANTIZED REC / UNQUANTIZED REC. Durante la reproducción también activa la grabación. | Todas las páginas |
| {r:FUNC + OCT- / OCT+} | Nada. | |
| {r:FUNC + knob 2 (gira a la derecha / izquierda)} | La pista seleccionada entra / sale del grupo choke. | Solo EDIT, página NOISE ENVELOPE |
| {r:FUNC + knob 4} | Un sonido aleatorio nuevo para la pista seleccionada. | Solo GLO > VEL |

**FILL mantenido (tecla negra 10)**

| Teclas | Qué hace | Dónde |
|---|---|---|
| {o:FILL + tecla negra 1 a 8} | Deja sola esa pista (otra tecla mueve el solo, la misma tecla lo termina). | Todas las páginas |
| {o:FILL + SEQ} | Vuelve a la pantalla SONG desde cualquier página (en modo song; si no, NOT IN SONG MODE). | Todas las páginas |
| {o:FILL}, pulsado y soltado solo | Borra el compás bajo el cursor (sin pregunta). | Solo la pantalla SONG |
| {o:FILL} + una tecla blanca mantenida (o primero FILL, luego teclas blancas) | Ese step recibe un fill (la pista suena 2x, 4x u 8x sobre él); otra vez = quitado. | Páginas donde las teclas blancas son steps |
| {o:FILL} mantenido | Los trigs de la pista seleccionada que llevan un fill parpadean en las teclas blancas (los demás trigs quedan encendidos). | Páginas donde las teclas blancas son steps |
| {r:FUNC + FILL} | Pegar. Ver la lista de FUNC arriba. | |

**ACCENT mantenido (tecla negra 9)**

| Teclas | Qué hace | Dónde |
|---|---|---|
| Tecla blanca mantenida + ACCENT (o primero ACCENT, luego teclas blancas) | Ese step recibe un accent (fuerte); otra vez = quitado. | Páginas donde las teclas blancas son steps |
| ACCENT mantenido | Los trigs de la pista seleccionada que llevan un accent parpadean en las teclas blancas (los demás trigs quedan encendidos). | Páginas donde las teclas blancas son steps |
| {r:FUNC + ACCENT} | Copiar. Ver la lista de FUNC arriba. | |

**GLO mantenido**

| Teclas | Qué hace | Dónde |
|---|---|---|
| GLO + ACCENT | Deshacer. | Todas las páginas, también ARP |
| GLO + FILL | Rehacer. | Todas las páginas, también ARP |

**HOME mantenido**

| Teclas | Qué hace | Dónde |
|---|---|---|
| HOME + SELECT | El tempo. | Todas las páginas |
| HOME mantenido solo (medio segundo) | La página DELAY. | Todas las páginas |
| HOME + tecla blanca | Nada. | |

**FX mantenido**

| Teclas | Qué hace | Dónde |
|---|---|---|
| FX mantenido solo (medio segundo) | La página REVERB. | Todas las páginas |

**ENV mantenido**

| Teclas | Qué hace | Dónde |
|---|---|---|
| ENV + un knob | Ese knob vuelve a su valor de encendido. | Páginas con knobs |
| ENV + tecla blanca | El accent de ese step activado / desactivado. | Páginas donde las teclas blancas son steps |
| ENV + tecla negra 1 a 8 | Toca esa pista fuerte. Grabando: el step recibe un accent. | Todas las páginas |

**SAVE mantenido**

| Teclas | Qué hace | Dónde |
|---|---|---|
| SAVE + un knob | Se olvida el motion grabado de ese knob. | Páginas con knobs |
| Tecla blanca mantenida + SAVE + el mismo knob | Quita el p-lock de ese knob en ese step. | Páginas de sonido |

**Una tecla blanca mantenida (un step)**

| Teclas | Qué hace | Dónde |
|---|---|---|
| Tecla blanca mantenida + knob 1 a 4 | **P-lock**: ese step recibe su propio valor del knob (3 knobs con lock por pista y pattern). | Páginas de sonido: MIX 1, SEL, ENV, LFO, EDIT, VEL |
| Tecla blanca mantenida + SELECT | La chance de ese step: de 5 a 100 %, luego 1:2, 2:2, 1:3 ... 8:8. | Todas las páginas menos PERFORM, HOME, SONG, COMP y las listas |
| Tecla blanca mantenida + ACCENT / FILL | Flag de accent / fill. | Ver ACCENT y FILL arriba |

**Transporte y otros**

| Teclas | Qué hace | Dónde |
|---|---|---|
| REC + PLAY, luego teclas y knobs | Grabación en vivo: las teclas negras 1 a 8 escriben trigs, los knobs escriben automatización. | No en PERFORM ni en la página de patterns |
| OCT- / OCT+ | El tono de la pista seleccionada una octava abajo / arriba. En una pregunta: NO / YES. En ARP: la octava del teclado. | Todas las páginas |
| Un botón mantenido solo (cerca de un segundo) | Una tarjeta dice qué hace el botón en esta página, con sus combinaciones. | Todos los botones salvo FX y HOME |

## 4. Tocar

- **Steps.** Elige una pista, pulsa una tecla blanca para poner un step y púlsala otra vez para quitarlo. **Accent** (fuerte) y **fill** (la pista suena cuatro veces durante ese step): mantén la tecla blanca y pulsa ACCENT y / o FILL (otra vez = flag quitado), o mantén primero ACCENT / FILL y pulsa teclas blancas.
- **Patterns.** {r:FUNC + tecla blanca} cambia el pattern. {r:FUNC + ACCENT} copia, {r:FUNC + FILL} pega (mantén FILL y pulsa una tecla blanca para pegar en ese lugar), {r:FUNC + PLAY} borra. Pegar el pattern pregunta **¿PASTE PATTERN?** (OCT+ = sí, OCT- = no) y borrar pregunta ¿CLEAR PATTERN?. Con una tecla negra 1 a 8 mantenida además, copian, pegan y borran solo esa pista; pegar el sonido de una pista o una página, pegar con FILL y una tecla blanca, y el modo de copiar y pegar al vuelo pegan de inmediato, sin pregunta.
- **Copiar y pegar al vuelo.** Después de {r:FUNC + ACCENT} toda la pantalla dice **PRESS PATTERN TO PASTE** y **PRESS SEQ TO EXIT** con letras azules grandes. Todas las teclas de pattern parpadean (un pattern con notas parpadea fuerte, un pattern VACÍO parpadea más suave, el copiado se queda encendido) y el botón **SEQ** también parpadea. Pulsa una tecla de pattern y la copia se pega ahí, ese pattern pasa a ser el actual y el modo termina. Para pegar en muchos sitios, **toca FUNC solo** antes de elegir: el modo se queda y el pattern actual no cambia. Para salir del modo pulsa **SEQ**: abre la **página del secuenciador**, no la página de patterns (la siguiente pulsación de SEQ vuelve a ser normal). Cualquier otro botón también sale, y también {r:FUNC + ACCENT} otra vez (la pantalla dice COPY MODE OFF). Mantén {r:FUNC + ACCENT} y pulsa una tecla blanca = ese pattern es la copia. No disponible en PERFORM ni en la página de patterns, donde FUNC es el modo MUTE.
- **Mute y solo.** {r:FUNC + tecla negra 1 a 8} hace mute (otra vez = unmute). {o:FILL + tecla negra 1 a 8} deja sola esa pista (otra tecla mueve el solo, la misma tecla lo termina). Los solos no se guardan y terminan al cargar un preset. No en ARP.
- **P-locks y chance.** Mantén una tecla blanca y gira un knob en una página de voz: ese step recibe su propio valor, todos los demás steps conservan el sonido. Mientras suena, lo oyes la próxima vez que pasa el step. Para quitar uno: mantén el step, mantén SAVE, gira el mismo knob. Una pista guarda 3 knobs con lock por pattern (la pantalla dice MAX 3 PER TRACK, MOTION FULL cuando no queda sitio). Mantén un step y gira SELECT: su chance, bajando desde 100 % de 5 en 5 %, luego 1:2, 2:2, 1:3 ... 8:8. Mientras giras, la chance ocupa toda la pantalla (TRIG % y el step), grande, para leerla de un vistazo.
- **Chains.** Un pattern puede marcarse para pasar al siguiente cuando termina (después del pattern 8 viene el 1). Un flag solo cuenta en un pattern con notas. Un preset con chains se carga sonando a través de ellas (SONG ON); el knob 1 de la página de patterns de SEQ lo apaga y el pattern se repite en bucle.
- **Modo song.** {r:FUNC + SEQ}, confirma con OCT+ (OCT- = volver). Si aún no hay song, se abre la pantalla PATTERNS: las teclas blancas 1 a 16 añaden patterns, OCT- (o FUNC + una tecla blanca) quita el último, OCT+ es OK. PLAY toca un compás por pattern, en bucle. El mismo gesto sale del modo song. En la **pantalla SONG** (una fila por compás) el knob 1 = el compás, el knob 2 = su pattern, el knob 3 = repeticiones (x1 a x8); las teclas blancas escriben un pattern en el compás del cursor y avanzan; FILL, pulsado y soltado, borra el compás al instante (borrar el último compás = se borra la song y termina el modo song). {r:FUNC + ARP} borra la song entera.

## 5. Presets y guardado

**Los presets funcionan en los dos sentidos.** El FM-1 carga presets `.mtpreset` de **Patternarium** y de **Microtonic**, y los presets que haces en el FM1 TONIC se pueden exportar de vuelta a `.mtpreset` y usar en el **Microtonic VST**. Está probado: los presets exportados se abren y suenan en Microtonic. El sonido es muy, muy parecido al original, no idéntico. Importar y exportar se hace en la página web (la página de copia de seguridad: «Restore my presets» y «Export my presets as .mtpreset»). Los slots 1 a 8 no se pueden exportar. Lo que añadió FM1 TONIC (notas ARP, golpes retrasados, cuts y sends de pista, compresor, reverb, delay, mute) no tiene sitio en un `.mtpreset` y se deja fuera.

Gira **PRESETS** por los 40 lugares: del 01 al 08 son presets de fábrica, del 09 al 40 son tuyos (un lugar vacío se carga con los sonidos de un preset de fábrica y sin pattern: el 09 suena como el 01, el 10 como el 02, y así en rotación). Una pregunta lo confirma antes: OCT+ lo carga, OCT- conserva tu sonido. Cargar se rechaza mientras suena.

- **SAVE** pregunta ¿SAVE HERE? con el lugar cargado. OCT+ guarda ahí y mantiene el nombre. ALGORITHM elige otro lugar (1 a 40; el ámbar muestra a quién reemplazaría). OCT- va a CHOOSE SLOT, luego a la pantalla del slot, luego a la pantalla del nombre (ALGORITHM = otro nombre inventado; OCT+ guarda). HOME sale sin guardar. Si no se ha cargado nada desde el encendido, SAVE empieza en CHOOSE SLOT.
- {r:FUNC + SAVE} hace quick save en el lugar cargado sin preguntar el slot, también mientras suena. Un lugar vacío se guarda al instante como UNTITLED 01, 02 ...; uno ocupado pregunta ¿OVERWRITE SLOT? (OCT+ sí).
- {r:FUNC + HOME} renombra el proyecto cargado (ALGORITHM = otro nombre, OCT+ guarda, OCT- o HOME cancelan). Con NAME PROJECTS en MANUAL (GLO settings) la pantalla del nombre pasa a ser un editor de letras: el knob 4 recorre los caracteres, OCT+ (la pantalla dice OK) toma la letra, OCT- borra la última, y luego pulsa SAVE (8 letras como máximo; la pantalla muestra PRESS SAVE y TURN KNOB 4 en letras grandes).
- **Qué se guarda.** Con AUTOSAVE en ON, tu proyecto de trabajo se guarda solo 4 segundos después de que dejes de tocar, salvo el compresor, la reverb y el delay: esos solo los guarda SAVE, dentro del preset. PERFORM siempre empieza limpio al encender (D/W 0 %, low cut master 20 Hz, ALL FX on). AUTOSAVE viene APAGADO de fábrica: pulsa SAVE (o FUNC + SAVE) para guardar tu trabajo. Los settings se guardan igualmente solos. Pon AUTOSAVE en ON en GLO > SETTINGS si quieres que el proyecto se guarde solo. Settings de fábrica: clock out y notes out off, canal MIDI de Tonic 10, canal MIDI remoto off, CC mode off (CC CH BASE 1-8), output LOUD, USB audio follows knob, 4/4 LED on, tema TONIC, zoom knob yes, name projects GENERATE, Tonic Engine ENHANCED, 2nd core ON. Una unidad nueva o un FACTORY RESET los recibe; una unidad que ya tiene un proyecto guardado conserva sus propios settings.

## 6. Luces, MIDI y USB

- **Luces.** PLAY se enciende en cada 4º step (rojo mientras graba). REC: apagado cuando no graba; fijo cuando está parado (quantized) y parpadeando con el pulso mientras suena; dos parpadeos rápidos y uno lento para unquantized. El botón de la página actual está encendido. Las teclas blancas muestran los steps de la pista seleccionada (el step que suena se invierte); 1, 5, 9 y 13 pueden brillar débilmente (GLO > 4/4 LED). Teclas negras 1 a 8: la pista seleccionada brillante (parpadea si está en mute), las pistas con notas tenues, y mientras suena la tecla de una pista parpadea en cada uno de sus golpes (no en ARP). Con FUNC mantenido las teclas muestran qué patterns y pistas tienen notas.
- **MIDI.** Por USB o por el TRS MIDI IN, el canal 10 por defecto toca las voces con notas de batería General MIDI en el orden de las voces: 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51 (velocity = volumen). Se sigue un clock entrante; Start, Stop y Continue controlan el pattern. Ajustes de GLO: TONIC MIDI CH (u OFF), REMOTE MIDI CH (u OFF, ver abajo), CC MODE y CC CH BASE (control changes, ver abajo), CLOCK OUT (clock y start / stop), NOTES OUT (el pattern como notas).
- **Panel remoto.** GLO > REMOTE MIDI CH (OFF por defecto) hace que un segundo canal MIDI toque las teclas propias del FM-1. Las 27 teclas del panel son las 27 notas de F3 a G5 (notas MIDI 53 a 79, do central = 60), dispuestas como un teclado de piano: la primera tecla, F3, es la nota 53, la última, G5, es la nota 79. Un note on pulsa la tecla, un note off la suelta, así que una nota larga mantiene la tecla pulsada. Todo lo que hace una tecla en el panel funciona igual: las teclas negras 1 a 8 seleccionan o silencian las pistas, las teclas blancas 1 a 16 son los steps o los patterns, y las teclas negras 9, 10 y 11 son ACCENT (nota 73), FILL (nota 75) y FUNC (nota 78). Un acorde vale en cualquier orden de notas: ACCENT, FILL y FUNC se aplican siempre primero, así que FUNC + ACCENT + una tecla blanca funciona cuando las tres notas llegan juntas. All Notes Off (controlador 123) suelta todas las teclas. Las notas del canal remoto no tocan las voces; dale un canal distinto de TONIC MIDI CH. Las notas fuera de 53 a 79 se ignoran.

| Tecla | Notas |
| --- | --- |
| Teclas blancas 1 a 16 | 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 |
| Teclas negras 1 a 8 (pistas) | 54, 56, 58, 61, 63, 66, 68, 70 |
| Tecla negra 9 = ACCENT, 10 = FILL, 11 = FUNC | 73, 75, 78 |

### Control changes (CC)

Un control change gira un knob desde fuera: el número de CC es el **parámetro** (los mismos números para todas las pistas) y la **pista** se elige de dos maneras. GLO > **CC MODE** decide (OFF por defecto: nada reacciona a los CC hasta que lo actives):

| CC MODE | Qué hace |
| --- | --- |
| OFF | Se ignoran todos los control changes. |
| SELECTED | Un CC en el canal de Tonic (TONIC MIDI CH) mueve el parámetro de la pista **seleccionada**. |
| CHANNELS | Un bloque de 8 canales, uno por pista: el primer canal de GLO > **CC CH BASE** es la pista 1, el siguiente la pista 2, y así sucesivamente (base 1 = canales 1-8). Cualquier pista puede moverse en cualquier momento, sin seleccionarla antes. |
| BOTH | Las dos cosas a la vez. Si el canal de Tonic está dentro del bloque, en ese canal gana el bloque. |

Las notas siguen tocando las voces en TONIC MIDI CH y el panel remoto conserva su propio canal, así que da al bloque canales que esos dos no usen (el bloque por defecto 1-8 deja libres el 10 y el 11). Un CC mueve el valor igual que el knob, así que la pantalla, SAVE y el autosave lo siguen; no abre el dial grande, no fija un lock en un step y no se graba. Un CC 0-127 es el 0-127 del knob; un interruptor de tres posiciones (waveform, modo de filtro, modo de envelope) divide el rango en tres. En el engine CLASSIC los extras (low cut, high cut, reverb send, delay send) ignoran los CC igual que sus knobs en gris.

| CC | Parámetro | CC | Parámetro |
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

### Conectar un Digitakt o un Digitone

El socket USB del FM-1 y el del Digitakt o Digitone son ambos sockets de «device», así que no pueden hablar entre sí. Usa el MIDI OUT (5 pines) del Digitakt o Digitone hacia el TRS MIDI IN del FM-1, con un adaptador MIDI de 5 pines a TRS de 3,5 mm (tipo A o B: si no llega nada, prueba el otro tipo).

1. En el FM-1: GLO settings, TONIC MIDI CH = 10 (las voces) y REMOTE MIDI CH = 11 (el panel), o dos canales distintos cualesquiera.
2. En el Digitakt o Digitone: usa dos pistas MIDI y pon sus canales en esos dos números. La primera toca las voces con las notas de batería (36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51). La segunda es el panel remoto: dibuja las notas de la tabla de arriba en su rejilla de steps o en su piano roll.
3. Una pista MIDI toca hasta 4 notas en un step, con una sola duración para todas: la duración de la nota es el tiempo que se mantienen las teclas, así que una nota larga para FUNC con otras notas en steps posteriores es FUNC + esas teclas, y un acorde de FUNC + ACCENT + una tecla blanca es un step.
4. Para mover el sonido desde los CC lanes del Digitakt o Digitone: pon GLO > CC MODE = CHANNELS, crea una pista MIDI por cada pista de Tonic en los canales 1 a 8 (o desde CC CH BASE) y asigna sus CC lanes a los números de la tabla de arriba; cada pista MIDI tiene un número limitado de CC lanes (16 en el Digitakt II, menos en el primer Digitakt), así que elige los parámetros que quieres automatizar en cada pista.
5. Para seguir el tempo y el start / stop del Digitakt o Digitone, activa su salida de MIDI clock y transport; el FM-1 sigue un clock entrante.

### Conectar un teclado MIDI

El FM-1 no puede ser el host de un teclado USB. Usa un teclado con MIDI OUT de 5 pines (con un adaptador de 5 pines a TRS hacia el TRS MIDI IN), o envía un teclado USB a través de un ordenador o de una caja host USB MIDI hacia el socket USB del FM-1.

1. En el FM-1: GLO settings, REMOTE MIDI CH = el canal en el que envía el teclado (1 es el habitual).
2. Toca las notas F3 a G5 (53 a 79). Mantén una tecla para FUNC (nota 78, F#5), ACCENT (nota 73, C#5) o FILL (nota 75, D#5) y pulsa las demás: copiar, pegar, cambiar de pattern, mute y solo funcionan como en el panel. Un teclado de 49 teclas cubre todo el panel; en uno más pequeño, muévelo con los botones de octava.

- **USB audio.** El mismo cable lleva lo que oyen los auriculares, después de MASTER, como una entrada de 2 canales a 44,1 kHz, sin driver (GLO > USB AUDIO = FULL envía el nivel completo diga lo que diga MASTER). El primer instante después de que una app lo abra puede producir un clic.

## 7. Instalar, copia de seguridad, recuperación

- **Pruébalo:** el [emulador](https://luigirossi-music.github.io/fm1-tonic/emu/) ejecuta el mismo código en el navegador.
- **Instalar o actualizar:** abre el [instalador web](https://luigirossi-music.github.io/fm1-tonic/install/) en Chrome o Edge, conecta el FM-1 con un cable USB de datos, pulsa Install y no lo desconectes mientras escribe. Otro firmware no conserva tu proyecto: guárdalo antes en un archivo. «Back to the stock firmware» al final de la misma página vuelve a poner el firmware de M-VAVE. Instalar otro firmware es bajo tu propio riesgo.
- **Copia de seguridad:** la [página de backup](https://luigirossi-music.github.io/fm1-tonic/backup/) guarda el proyecto en un archivo `.x0b` (Back up), lo vuelve a cargar (Restore: para el FM-1 y luego responde ¿RESTORE FROM USB? con SEL), exporta presets como archivos `.mtpreset`, y copia / restaura los slots 9 a 40 como archivos `.tnp`. Línea de comandos: `python3 tools/x0x_backup.py backup my-song.x0b` (también `restore`, `info`).
- **Si algo sale mal:** mantén OCT- y OCT+ cinco segundos = modo de actualización; mantenerlos al encender = calibración del hardware. Tras un cuelgue, una pantalla roja CRASH reinicia el FM-1; si se cuelga dos veces al arrancar entra en SAFE MODE (sin sonido, el USB funciona: reinstala desde el instalador; PLAY intenta un arranque normal). Si ni eso funciona, el script de rescate para Mac `tools/fm1_rescue.sh` vuelve a poner el firmware de M-VAVE (comprueba cada archivo que escribe; ejecutarlo otra vez es seguro).
- **Si se rechaza una actualización.** Si el FM-1 no quiere cambiar de firmware (por ejemplo con «Back to the stock firmware»), su pantalla de actualización muestra FAILED, un número de error con una razón breve (por ejemplo ERROR -5 HEAD DIFFERS) y el paso en que se detuvo, durante 8 segundos, y después el FM-1 arranca FM1 TONIC otra vez. ERROR -5 significa que la zona de arranque (el «head») de tu FM-1 no es la del firmware al que vas. En ese error la pantalla muestra además el primer byte que difiere, cuántos bytes difieren y los dos checksums (CRC), durante 40 segundos. Apunta esos números si pides ayuda.
- **Un bootloader personalizado.** Si pusiste un bootloader personalizado en el FM-1 (por ejemplo m-bootloader, con M-Flasher), volver al firmware oficial de M-VAVE desde la página web puede ser rechazado con ERROR -5 HEAD DIFFERS. Primero vuelve a poner el bootloader original de M-VAVE (en M-Flasher: FLASH BOOTLOADER, luego STOCK (FROM .FWSC) con el paquete oficial) y después vuelve al firmware original. En el FM-1 del autor, así todo volvió a funcionar.
- **FM-1 más nuevos.** Algunos FM-1 nuevos llevan otro chip flash (ID B36014 en lugar de 856014). FM1 TONIC acepta los dos, así que guardar, presets, copia de seguridad y restauración también funcionan en ellos (aún no probado en una unidad con el chip nuevo).
- **Texto completo** de los capítulos de instalación, backup, rescate y parámetros: `docs/reference/MANUAL_full_wip381.md`.

## 8. El engine

Cada una de las 8 pistas es un sintetizador de batería completo: el engine de FM1 TONIC, sin nada sampleado. Las 8 pistas son idénticas y cada una tiene los mismos **26 ajustes**, guardados por pista y con el preset. Este capítulo es la referencia de todos ellos. Dónde vive cada ajuste se escribe como botón · knob.

```
pitch mod ─► OSCILLATOR ─┐
                         ├─► MIX ─► EQ ─► DIST ─► LEVEL ─┐
NOISE ─► filter ─► env ──┘                               │
          ┌──────────────────────────────────────────────┘
          └─► LOW CUT ─► HIGH CUT ─┬─► PAN ─► master
                                   └─► REVERB / DELAY SEND
```

Un golpe suena hasta que sus envelopes se apagan. Un golpe nuevo en la misma pista corta al que aún suena (las baterías son monofónicas), y una pista del choke group es cortada por las demás (fundido de 3 ms).

**Los knobs son curvas, no líneas rectas.** Las frecuencias, los tiempos y la Q son exponenciales: la mitad del knob no es la mitad del valor. En pantalla las frecuencias se muestran en Hz o kHz, el AMOUNT del pitch mod en semitonos (sm) y el pan como L / C / R, el OSC/NOISE MIX como dos porcentajes (100.00/00.00 = solo oscillator, 00.00/100.00 = solo noise); todos los demás knobs muestran un número simple de 0 a 127. Usa esta tabla para ver qué significan esos números.

| Knob | 0 | 32 | 64 | 96 | 127 |
|---|---|---|---|---|---|
| **Frequency** (oscillator, noise filter, EQ) | 20 Hz | 114 Hz | 650 Hz | 3,7 kHz | 20 kHz |
| **Decay** (oscillator, noise, caída del pitch mod) | 10 ms | 57 ms | 0,33 s | 1,8 s | 10 s |
| **Attack** (oscillator, noise) | 0 | 20 ms | 0,16 s | 1 s | 5 s |
| **Filter Q** | 0,1 | 1,8 | 33 | 600 | 10000 |

Un decay es el tiempo que tarda el sonido en caer al silencio (60 dB abajo).

### Oscillator: el tono

| Ajuste | Dónde | Rango | Qué hace |
|---|---|---|---|
| **WAVE** | ENV · knob 1 | SINE, TRI, SAW | SINE es puro y suave, TRI hueco, SAW brillante y zumbón. |
| **FREQUENCY** | ENV · knob 2 | 20 Hz a 20 kHz | El pitch. Kick de unos 40 a 70 Hz, toms de 80 a 250 Hz, cuerpo de snare de 150 a 250 Hz. OCT- / OCT+ lo mueven una octava. |
| **ATTACK** | ENV · knob 3 | 0 a 5 s | 0 empieza al instante. Súbelo para un crescendo. El primer cuarto del knob está por debajo de 20 ms. |
| **DECAY** | ENV · knob 4 | 10 ms a 10 s | Cuánto suena: corto = click o tic, largo = boom. |

### Pitch mod: el pitch se mueve mientras suena

| Ajuste | Dónde | Rango | Qué hace |
|---|---|---|---|
| **SHAPE** | SEL · knob 1 | DECAY, SINE, RANDOM | El tipo de movimiento (abajo). |
| **AMOUNT** | SEL · knob 2 | 64 = nada; hasta 8 octavas a cada lado | Por encima de 64 el pitch sube, por debajo de 64 baja. Con DECAY el sonido empieza tan lejos y se desliza hasta el pitch ajustado. |
| **RATE** | SEL · knob 3 | depende de SHAPE | DECAY: cuánto dura el deslizamiento, de 10 ms a 10 s (caída de kick = corta, láser = larga). SINE: la velocidad del vaivén, de 0 a 2000 Hz; solo los primeros pasos son un vibrato, más alto es FM (campanas, zaps), y un rate más alto también lo hace durar más. RANDOM: lo rápido que el pitch divaga al azar, de 20 Hz a 20 kHz; lento = derivas, rápido = suena metálico. |

### Noise: el chasquido y el siseo

El noise es ruido blanco que pasa por un filtro y un envelope. El oscillator y el noise son independientes: el MIX decide cuánto oyes de cada uno.

| Ajuste | Dónde | Rango | Qué hace |
|---|---|---|---|
| **FILTER MODE** | LFO · knob 1 | LP, BP, HP | LP conserva los graves (golpe sordo, oscuro), HP los agudos (hi-hat, siseo), BP una banda alrededor de la frecuencia. |
| **FILTER Q** | LFO · knob 2 | 0,1 a 10000 | Resonancia. Baja = suave, alrededor del knob 64 = un pico fuerte, alta = el filtro suena como un tono con altura: BP con una Q alta y un envelope corto es un ping afinado (tom, cowbell, block). |
| **FILTER FREQUENCY** | LFO · knob 3 | 20 Hz a 20 kHz | Dónde trabaja el filtro. |
| **ENVELOPE MODE** | EDIT · knob 1 | EXP, LIN, CLAP | EXP: una subida rápida y una caída natural, como el oscillator. LIN: subida y caída en línea recta. CLAP: unas pocas ráfagas cortas, como unas manos. |
| **ENVELOPE ATTACK** | EDIT · knob 3 | 0 a 5 s | EXP y LIN: la subida. CLAP: cuánto se queda el noise a nivel completo antes de que empiecen las ráfagas. |
| **ENVELOPE DECAY** | EDIT · knob 4 | 10 ms a 10 s | La caída. En CLAP también fija el espacio entre ráfagas: corto = un zumbido rápido, largo = palmas separadas. |

### Mix, EQ, distorsión

| Ajuste | Dónde | Rango | Qué hace |
|---|---|---|---|
| **OSC/NOISE MIX** | FX · knob 1 | 100.00/00.00 a 00.00/100.00 | La pantalla muestra la parte de oscillator y la parte de noise en porcentaje. 100.00/00.00 = solo oscillator, 49.61/50.39 (el medio del knob) = ambos a nivel completo, 00.00/100.00 = solo noise. Una pista nueva empieza en 100.00/00.00: no hay noise hasta que lo mueves. |
| **EQ FREQUENCY** | FX · knob 2 | 20 Hz a 20 kHz | El centro de la única banda de EQ. |
| **DIST** | FX · knob 3 | 0 a 127 | Distorsión, muy pronunciada: el primer cuarto es un calor suave, pasado el medio es fuerte, arriba del todo es un fuzz. Es asimétrica, así que añade volumen y aspereza. |
| **EQ GAIN** | FX · knob 4 | 64 = plano; 40 dB a cada lado | Realce o corte de la banda (ancha, una sola campana). El EQ va **antes** de la distorsión, así que realzar y luego distorsionar da un sonido más pesado que al revés. |

### Velocity: accents que cambian el sonido

El secuenciador toca un step normal a un tercio aproximado de la velocity máxima y un step con accent a la máxima. Los tres knobs VELOCITY (GLO · VEL, knobs 1 a 3) dicen cuánto la sigue cada parte.

| Ajuste | Dónde | Qué hace |
|---|---|---|
| **OSC** | GLO · knob 1 | 0 = el oscillator ignora la velocity. Más alto = en los steps normales suena más bajo, los accents siguen a tope. |
| **NOISE** | GLO · knob 2 | Lo mismo para el noise. |
| **PITCH MOD** | GLO · knob 3 | Lo mismo para la cantidad de pitch mod: en los steps normales el movimiento se encoge. |

Es muy pronunciado: un valor pequeño ya recorta mucho un step normal, y desde más o menos el knob 95 un step normal pierde esa parte por completo. Con los tres a 0, un accent suena exactamente como un step normal: el accent solo se oye a través de estos knobs. Por ejemplo, una snare con VELOCITY OSC alto tiene su tono solo en los accents y toca solo noise en los steps normales.

### Output: level, pan, cuts, sends

| Ajuste | Dónde | Rango | Qué hace |
|---|---|---|---|
| **LEVEL** | HOME · knob 1 | silencio hasta unos +10 dB (por defecto 101 = unos -2,5 dB) | El volumen de la pista. La voz satura en el fondo de escala: si crepita, bájalo. |
| **PAN** | HOME · knob 2 | 0 izquierda, 64 centro, 127 derecha | Posición en el campo estéreo. |
| **LOW CUT** | HOME · knob 3 | 20 a 663 Hz | Quita los graves (12 dB por octava). A 20 Hz no hace nada audible. |
| **HIGH CUT** | HOME · knob 4 | de 20 kHz a 1 kHz | Quita los agudos (12 dB por octava). Todo a la derecha = abierto. |
| **REVERB SEND** | HOME, 2ª pulsación · knob 1 | 0 a 127 | Cuánto de la pista va a la reverb. |
| **DELAY SEND** | HOME, 2ª pulsación · knob 2 | 0 a 127 | Lo mismo para el delay. |

Los sends salen después de los cuts y del sidechain, antes del pan, y funcionan diga lo que diga el D/W de PERFORM y ALL FX.

### Alrededor de la voz

Compartido por todas las pistas:

- **Choke group** (EDIT, ALGORITHM a la derecha): un golpe de una pista del grupo detiene a las demás al instante (el hi-hat cerrado detiene al abierto).
- **Sidechain** (HOME, 2ª pulsación): una pista de entrada hace que las pistas que elijas en la pantalla DUCK bajen mientras ella suena; el knob de threshold fija cuánto (0 = off).
- **Low cut master** (PERFORM · knob 3, 20 a 663 Hz): un low cut sobre toda la mezcla, antes del compresor.
- **Compresor** (GLO · COMP): 4:1, soft knee, attack de 0,1 a 100 ms, release de 10 a 1500 ms, threshold de -48 a 0 dB, make-up de 0 a 24 dB. Un limiter después mantiene la salida por debajo del fondo de escala.
- **Reverb** (mantener FX): decay, tone, low cut, nivel de return; una sala granulada de los primeros tiempos del digital. **Delay** (mantener HOME): tiempo sincronizado con el tempo (1/32 a 1/2), feedback, tone, TAPE (digital limpio a cinta gastada), low cut, return, ping-pong.

### Enhanced y classic

22 de los 26 ajustes son lo que lleva un preset de Patternarium (los 21 ajustes de sonido, level incluido, más el pan). Cuatro son **extras de FM1 TONIC**: LOW CUT, HIGH CUT, REVERB SEND y DELAY SEND, igual que el choke group, el sidechain, la sección master, los p-locks, la chance y el microtiming. En **classic** (GLO > settings > TONIC ENGINE) los cuatro extras quedan en gris y en silencio, y el sidechain también, así que lo que construyes usa solo los ajustes que lleva un archivo `.mtpreset`; los valores vuelven en enhanced. Un preset de Patternarium cargado conserva sus valores exactos hasta que giras ese knob.

### Recetas de sonido

Son puntos de partida: cambia un valor cada vez y escucha.

#### Kick

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 20 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +37.04 sm | RATE 25 | - |
| **ENV** | WAVE SINE | FREQUENCY 56 Hz | ATTACK 0 | DECAY 72 |
| **LFO** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **EDIT** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

AMOUNT es la profundidad de la caída y RATE su velocidad. Para un click, pon MIX 40 en FX, luego en LFO: MODE HP, FREQUENCY 2.9 kHz, y en EDIT: DECAY 15. Para un boom más largo, sube ENV DECAY.

#### Snare

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 15 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +12.85 sm | RATE 13 | - |
| **ENV** | WAVE TRI | FREQUENCY 196 Hz | ATTACK 0 | DECAY 46 |
| **LFO** | MODE HP | Q 22 | FREQUENCY 2.0 kHz | - |
| **EDIT** | MODE EXP | déjalo | ATTACK 0 | DECAY 55 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Más chasquido: sube MIX. Más cuerpo: bájalo. Para ghost notes suaves, pon GLO NOISE en 30: los steps normales tocan el noise más bajo y los accents lo tocan a tope.

#### Hi-hat cerrado

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **ENV** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **LFO** | MODE HP | Q 22 | FREQUENCY 7.1 kHz | - |
| **EDIT** | MODE EXP | déjalo | ATTACK 0 | DECAY 25 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

OSC/NOISE MIX 00.00/100.00 significa solo noise. Baja la frecuencia de LFO para un hi-hat más oscuro.

#### Hi-hat abierto

Igual que el hi-hat cerrado, con **EDIT DECAY 68**. Pon los dos hi-hats en un **choke group** para que el cerrado detenga al abierto: en cada una de las dos pistas mantén FUNC y gira el knob 2 de EDIT a la derecha.

#### Clap

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **ENV** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.1 kHz | - |
| **EDIT** | MODE CLAP | déjalo | ATTACK 0 | DECAY 60 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Con estos valores obtienes 3 o 4 ráfagas rápidas en una décima de segundo. EDIT DECAY más bajo = un zumbido más apretado, más alto = palmas más lentas y separadas y una cola más larga (75 a 85). Un poco de reverb send (HOME, 2ª pulsación) lo convierte en un clap de sala.

#### Tom

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +17.39 sm | RATE 20 | - |
| **ENV** | WAVE SINE | FREQUENCY 120 Hz | ATTACK 0 | DECAY 65 |
| **LFO** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **EDIT** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Usa el mismo sonido en otras pistas con otra FREQUENCY (80 Hz para un tom de suelo, 200 Hz para un tom agudo).

#### Zap

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +47.62 sm | RATE 62 | - |
| **ENV** | WAVE SAW | FREQUENCY 1.0 kHz | ATTACK 0 | DECAY 75 |
| **LFO** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **EDIT** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

El pitch cae 4 octavas en unos 0,3 s. Mándalo al delay (HOME, 2ª pulsación) para un láser clásico.

#### Rimshot

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 30 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +11.34 sm | RATE 10 | - |
| **ENV** | WAVE TRI | FREQUENCY 552 Hz | ATTACK 0 | DECAY 25 |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.8 kHz | - |
| **EDIT** | MODE EXP | déjalo | ATTACK 0 | DECAY 22 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Un clic seco y corto (unos 45 ms). Sube DIST para un rim más agresivo; baja el MIX hacia la izquierda para más cuerpo de tono.

#### Sub bass

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 35 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +6.80 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 41 Hz | ATTACK 6 | DECAY 80 |
| **LFO** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **EDIT** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Una nota grave de unos 0,9 s con una pequeña caída de pitch al inicio y algo de DIST para que se oiga en altavoces pequeños. Cambia FREQUENCY (de 30 a 60 Hz) para otra nota, o recorta DECAY para un kick largo.

#### Conga

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 82.68/17.32 | EQ FREQ 20 Hz | DIST 10 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +5.29 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 231 Hz | ATTACK 0 | DECAY 54 |
| **LFO** | MODE BP | Q 30 | FREQUENCY 1.6 kHz | - |
| **EDIT** | MODE EXP | déjalo | ATTACK 0 | DECAY 20 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Un tono redondo con un golpe de noise muy corto (unos 0,2 s en total). Sube FREQUENCY (por ejemplo 330 Hz) en otra pista para una conga aguda.

#### Crash

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **ENV** | no se oye (OSC/NOISE MIX 00.00/100.00), déjalo | | | |
| **LFO** | MODE HP | Q 10 | FREQUENCY 5.1 kHz | - |
| **EDIT** | MODE EXP | déjalo | ATTACK 0 | DECAY 100 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Un plato de noise puro que dura unos 2,3 s. Baja DECAY (a 85 cerca de 1 s) para un splash, o sube FREQUENCY del LFO para un sonido más fino. Mándalo un poco a la reverb (HOME, 2ª pulsación).

#### Clave

| Botón | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT 0.00 sm | RATE 0 | - |
| **ENV** | WAVE SINE | FREQUENCY 2.5 kHz | ATTACK 0 | DECAY 28 |
| **LFO** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **EDIT** | no se oye (OSC/NOISE MIX 100.00/00.00), déjalo | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | déjalo |

Un tono agudo y seco de unos 50 ms, sin movimiento de pitch (AMOUNT 0.00 sm). Cambia FREQUENCY (2 a 3 kHz) para otra afinación.

**Consejos.** Para oír una parte sola, gira el MIX del todo a la izquierda (100.00/00.00, solo oscillator) o del todo a la derecha (00.00/100.00, solo noise). Para que los accents cambien el sonido y no solo el volumen, sube los knobs de GLO. Para probar un valor en un solo step, mantén el step y gira el knob (p-lock, capítulo 4). Un sonido que te guste se conserva con SAVE (capítulo 5).

## 9. Créditos

**FM1 TONIC está hecho por Luigi Rossi**, el desarrollador y la mente creativa de este firmware: la caja de ritmos Tonic, su secuenciador, sus pantallas, sus presets y las herramientas web (emulador, instalador, backup). Es software libre bajo la GNU General Public License, versión 3; si compartes el firmware debes compartir su código fuente.

**Luigi Rossi en línea:** [linktr.ee/luigirossi](https://linktr.ee/luigirossi)

LICENSING.md lista el origen y la licencia de cada archivo. Se apoya en el trabajo de:

- **libpo32** de Eric Lewis (MIT): el punto de partida del código de síntesis de batería de FM1 TONIC. Nada está sampleado.
- **X0X** de Charles Vestal, el firmware del que creció FM1 TONIC, y las páginas web (emulador, instalador, backup); X0X es un fork de **Felucca** de Leo Kuroshita (@kurogedelic), Hügelton Instruments: la plataforma (capa de hardware, USB, cargador de actualización, instalador, almacenamiento) y la placa de recuperación FM-1-transporter.
- La reverb y el delay están portados de **9W9** de athousanddetails, basado en **ER-99** de Matthew Cieplak (GPL-3.0). El compresor master sigue a Giannoulis, Massberg y Reiss; el filtro master es el filtro de variable de estado TPT de Vadim Zavalishin.
- **Melodee** de keremimo y **Jangada** de zednaked: lo que descubrieron sobre el segundo núcleo del FM-1.
- Fuentes: **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov), **DotGothic16** (The DotGothic16 Project Authors), todas bajo SIL Open Font License.

M-VAVE y FM-1 son marcas de sus propietarios. FM1 TONIC no está conectado con M-VAVE, Sonic Charge ni Luigi Rossi, ni cuenta con su aval.
