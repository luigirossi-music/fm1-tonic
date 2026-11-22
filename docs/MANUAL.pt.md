# FM1 TONIC

Inspirado no Microtonic VST da Sonic Charge, o FM1 TONIC é um sintetizador de bateria de 8 vozes em que cada som é sintetizado do zero, sem nenhum sample. Ele combina um motor de percussão profundo com um sequencer de 1 a 64 steps (com accents, fills e 16 patterns por projeto), um mixer integrado com level e pan, reverb, delay e um compressor master. Com 40 project slots (8 presets de fábrica + 32 slots de usuário) e um fluxo de trabalho pensado para o palco, ele vai sem esforço de beats rítmicos e marcantes a texturas ambient em evolução.

## Características

**Som**

- Sintetizador de bateria de 8 vozes: cada track tem o seu próprio oscillator, pitch mod, noise, mix, EQ e distorção.
- Level e pan por track, e reverb send e delay send por track (mais low cut e high cut no engine Enhanced).
- Engine Enhanced ou Classic (Classic = só os ajustes básicos de som, sem os extras do FM1 TONIC).
- Randomizer de track: um som novo aleatório para a track selecionada.
- Choke groups (por exemplo hi-hat fechado e aberto).
- Compressor sidechain: qualquer track pode fazer duck nas outras.

**Sequencer**

- Sequencer de 64 steps, 16 patterns por projeto. Comprimento fixo de 8, 16, 32 ou 64 steps, ou comprimento livre de 1 a 64.
- Song mode.
- Accent global e accent por track, fill com retrig (2x, 4x, 8x) e trig % (chance) em cada step.
- Parameter lock (limitado: 3 knobs por track em cada pattern).
- Record automation (limitado: 3 knobs por track).
- Gravação ao vivo, quantized ou unquantized. Swing, quantization e tempo.
- Teclado (ARP): toque uma track como um teclado e grave as notas nos steps (um piano roll).
- Copiar e colar um pattern, uma track, e um pattern na hora enquanto a música toca.
- Mute e solo.
- Undo e redo.

**Mixagem e performance**

- Mixer master: level e pan de cada track (o ponto vermelho marca o centro).
- Compressor master.
- Página Performance: 2 efeitos master (reverb e delay), um low cut e ALL FX OFF.
- Página Memory: quanto da memória do projeto está ocupada.

**Projetos e salvamento**

- 40 project slots: 8 presets de fábrica e 32 seus.
- Autosave e quick save.
- Nome do projeto: GENERATE ou MANUAL. Renomear o projeto.
- Backup e restauração pela página web. Exportação e importação de presets `.mtpreset`.
- Os presets funcionam nos dois sentidos: carregue presets `.mtpreset` do Patternarium e do Microtonic, e exporte os seus de volta para o Microtonic VST (testado; o som é muito parecido, não idêntico).

**Tela, luzes e ajustes**

- Indicador LED 4/4 (teclas brancas 1, 5, 9 e 13), luz das teclas on ou off.
- Zoom knobs (o dial grande quando você gira um knob), tema de cor.
- Volume de saída NORMAL ou LOUD. USB audio.

**MIDI**

- Seletor de canal MIDI, notes out on ou off, clock out on ou off.
- Control changes (CC): 26 parâmetros de som, por track selecionada ou por canal MIDI.
- Painel remoto: toque as teclas do próprio FM-1 a partir de um teclado externo ou de um Digitakt.

## 1. O painel

![O painel do FM1 TONIC: knobs MASTER, SELECT, PRESETS, ALGORITHM e KNOB 1 a 4; botões FX, SEL, ENV, LFO, EDIT, GLO, HOME, SAVE, ARP, SEQ, PLAY, REC, OCT- e OCT+; teclas pretas TRACK 1 a 8, ACCENT, FILL, FUNC; teclas brancas 1 a 16](img/fm1-tonic-panel.svg "")

- **Knobs.** MASTER é o volume. SELECT muda de página (em uma lista: para cima e para baixo). PRESETS escolhe um dos 40 presets. KNOB 1 a 4 ajustam os quatro valores da página que aparece na tela (o que você gira é ampliado). ALGORITHM passa para uma segunda página ou escolhe uma voz, conforme a página (as tabelas de páginas indicam). Um pequeno triângulo azul em um canto inferior da tela significa que existe uma segunda página atrás de ALGORITHM: triângulo direito = gire ALGORITHM para a direita, triângulo esquerdo = gire-o para a esquerda para voltar.
- **Teclas pretas.** De 1 a 8 são TRACK 1 a 8, 9 é ACCENT, 10 é FILL, 11 é FUNC.
- **Teclas brancas de 1 a 16** são os 16 steps da faixa selecionada. Um pattern pode ter até 64 steps, mostrados 16 de cada vez (veja SEQ).
- **OCT- / OCT+** movem o tom da faixa selecionada em uma oitava (em ARP: a oitava do teclado). Em uma pergunta, OCT+ = YES, OCT- = NO.

### Índice

1. O painel {#s1}
2. O caminho do sinal {#s2}
3. Páginas, botão por botão {#s3}
4. Tocar {#s4}
5. Presets e salvamento {#s5}
6. Luzes, MIDI e USB {#s6}
7. Instalar, backup, recuperação {#s7}
8. O engine {#s8}
  - Receitas de som
9. Créditos {#s9}

---

**Como ler as tabelas de páginas.** Cada botão tem uma tabela, com uma linha por página. O texto normal é o que o controle faz sozinho. {r:Vermelho} é o que ele faz com **FUNC** pressionado, {o:laranja} é o que ele faz com **FILL** pressionado (ao estilo do Digitakt).

**O que vale em todas as páginas** (as tabelas seguintes não repetem isso):

| Teclas | O que faz |
|---|---|
| Tecla preta de 1 a 8 | Seleciona a faixa e a toca (também durante a reprodução). |
| Tecla branca de 1 a 16 | Os steps: um toque = trig ligado, outro toque = desligado. Um pattern tem até 64 steps, mostrados 16 de cada vez. |
| Tecla branca mantida + ACCENT / FILL | Esse step recebe um accent / um fill (de novo = retira). |
| {r:FUNC + tecla preta de 1 a 8} / {o:FILL + tecla preta de 1 a 8} | Coloca a faixa em mute / em solo (de novo = volta ao normal). |
| {r:FUNC + tecla branca} | Troca de pattern (durante a reprodução: no fim do compasso). |
| {r:FUNC + ACCENT} / {r:FUNC + FILL} / {r:FUNC + PLAY} | Copia o pattern / cola (pergunta PASTE PATTERN?) / apaga (pergunta CLEAR PATTERN?). OCT+ = sim, OCT- = não. |
| {r:FUNC + tecla preta de 1 a 8 mantida, depois ACCENT / FILL / PLAY} | Copia o som dessa faixa / cola / apaga o pattern dessa faixa (pede confirmação). |
| {r:FUNC + SEQ} | Modo song ligado / desligado (pede confirmação). |
| {r:FUNC + ARP} | Apaga a song (pede confirmação). |
| {r:FUNC + SAVE} / {r:FUNC + HOME} | Salvamento rápido no lugar carregado / renomeia o projeto. |
| {r:FUNC + REC} | QUANTIZED REC / UNQUANTIZED REC. |
| ENV + um knob | Esse knob volta ao valor padrão. |
| GLO + ACCENT / GLO + FILL | Desfazer / refazer (em qualquer página). |
| REC + PLAY, depois teclas e knobs | Gravação ao vivo: as teclas pretas de 1 a 8 escrevem trigs, os knobs escrevem automação (3 knobs por faixa). |
| PLAY | Inicia e para o pattern. |
| OCT- / OCT+ | O tom da faixa uma oitava abaixo / acima. Em uma pergunta: NO / YES. |

Exceções: em PERFORM, MUTE / SOLO e na página de patterns as teclas pretas são botões de mute; em ARP todas as teclas são teclas de teclado. Estão todas na lista seguinte.

**Onde as teclas fazem outra coisa.** As teclas do painel (teclas pretas de 1 a 8 = faixas, ACCENT, FILL, FUNC, teclas brancas de 1 a 16 = steps) e os atalhos acima valem em todas as páginas deste manual, exceto nas páginas desta lista, onde as teclas têm outra função:

| Página | O que as teclas fazem ali |
|---|---|
| **ARP** | Todas as teclas, brancas e pretas, são teclas de teclado que tocam a voz selecionada. |
| **PERFORM, MUTE / SOLO, PATTERN** (SEQ, 2ª pressão) | As teclas pretas de 1 a 8 são botões de mute (botões de solo no modo SOLO), não selecionam a faixa. As teclas brancas de 1 a 16 escolhem o próximo pattern. FUNC e FILL, com um toque, alternam entre o modo MUTE e o modo SOLO. |
| **DUCK** (SENDS, ALGORITHM à direita) | As teclas pretas de 1 a 8 escolhem quais faixas recebem o duck. |
| **CHOKE** (EDIT, ALGORITHM à direita) | As teclas pretas de 1 a 8 escolhem quais faixas estão no grupo choke. |
| **Telas SONG e PATTERNS** (modo song) | As teclas brancas de 1 a 16 escrevem um pattern no compasso. FILL apaga o compasso. |
| **Modo copiar-colar** (depois de FUNC + ACCENT) | As teclas brancas de 1 a 16 são os lugares onde colar. |
| **FUNC mantido** | As teclas brancas de 1 a 16 são os patterns de 1 a 16; as teclas acendem onde um pattern ou uma faixa tem notas. |

**Coisas que existem só em algumas páginas.**

| Página | Teclas | O que faz |
|---|---|---|
| Páginas de som: MIX 1, SEL, ENV, LFO, EDIT, VEL | Tecla branca mantida + knob 1 a 4 | **P-lock**: esse step recebe um valor próprio do knob. Tecla branca mantida + SAVE + o mesmo knob o remove. Uma faixa guarda 3 knobs com p-lock por pattern. |
| Todas as páginas, exceto PERFORM, HOME, SONG, COMP e as listas | Tecla branca mantida + SELECT | A **chance** desse step: de 5 a 100 %, depois 1:2, 2:2, 1:3 ... 8:8. |
| ENV | OCT- / OCT+ | A frequência do oscilador uma oitava abaixo / acima. |
| EDIT | {r:FUNC + knob 2}, {r:FUNC, toque em EDIT}, EDIT ou LFO mantido + {r:FUNC + ACCENT / FILL / PLAY} | Entra no / sai do grupo choke; DELETE AUTOMATION?; copia / cola / zera esta página da faixa. |
| SEQ | Knobs, SELECT, ALGORITHM, {o:FILL + SEQ} | Veja a tabela de SEQ no capítulo 3. |
| GLO > VEL | {r:FUNC + knob 4} | Um novo som aleatório para a faixa selecionada. |
| GLOBAL (HOME, 3ª pressão) | Knob 4 (ou HOME mantido + SELECT) | O tempo (andamento). |

## 2. O caminho do sinal

![O engine do FM1 TONIC: cada uma das 8 faixas tem um modulador de tom e oscillator, um filtro de noise e um envelope de noise, um mix, uma distorção e um EQ, depois cuts, level e pan; as 8 faixas passam pelo sidechain, pelo low cut master e pelo compressor até o volume master; reverb e delay são efeitos de send](img/fm1-engine.svg "O caminho completo do sinal. A palavrinha em cada caixa é o botão que abre essa página.")

Cada faixa faz o seu som do zero com o engine de bateria do FM1 TONIC: um modulador de tom e oscillator, um filtro e um envelope de noise, um mix, uma distorção e um EQ, depois os cuts, o level e o pan da faixa; as 8 faixas passam pelo sidechain, pelo low cut master e pelo compressor até o MASTER, com reverb e delay como efeitos de send. O capítulo 8 explica cada ajuste.

**Enhanced e classic.** GLO > settings > Tonic Engine vem em **enhanced** por padrão. Em **classic** os extras do FM1 TONIC (os cuts da faixa, os sends, o sidechain) ficam acinzentados e não fazem nada, então o som usa só os ajustes que um arquivo `.mtpreset` pode levar. Os valores deles são guardados e voltam com o enhanced. Level e pan continuam disponíveis em classic. O modo é global, não é salvo em um preset.

## 3. Páginas, botão por botão

Uma página com valores de **faixa** (knobs 1 a 4 das páginas de som) pertence à faixa selecionada: troque de faixa com uma tecla preta de 1 a 8 e os valores mudam. EDIT ou LFO mantido + {r:FUNC + ACCENT / FILL / PLAY} copia, cola e zera **esta página** da faixa (só nas páginas de faixa).

### FX

Toque = MIX 1 → PERFORM → MIX 1. Mantido (meio segundo) = REVERB. A partir da página de outro botão, FX volta à última página de FX que você deixou.

| Página | Knob 1 · 2 · 3 · 4 | Também nesta página |
|---|---|---|
| **MIX 1** (1º toque) | Mix OSC / NOISE · EQ FREQ (20 Hz-20 kHz) · DIST · EQ GAIN (64 = flat) | Tecla branca mantida + knob: p-lock. SAVE + knob: esquece esse lock ou movimento. |
| **PERFORM** (2º toque) | REVERB D/W · DELAY D/W (0-100 %, 0 ao ligar) · MASTER LOW CUT (20-663 Hz) · ALL FX (direita ligado, esquerda desligado) | O número grande é o pattern que está tocando. SELECT: as próximas páginas de FX (para nas pontas). ALGORITHM à direita: MUTE / SOLO. Teclas pretas de 1 a 8: mute. Teclas brancas de 1 a 16: próximo pattern. {r:FUNC toque}: modo MUTE, ou tira o mute de todas. {o:FILL toque}: modo SOLO, ou limpa todos os solos. Sem p-locks. |
| **MUTE / SOLO** (PERFORM, ALGORITHM à direita) | igual a PERFORM | Oito caixas no lugar do número: azul = a faixa toca. As teclas pretas de 1 a 8 dão mute (modo MUTE) ou adicionam / retiram um solo (modo SOLO, vários ao mesmo tempo). ALGORITHM à esquerda: volta. |
| **REVERB** (FX mantido) | DECAY · TONE | ALGORITHM à direita: REVERB 2. |
| **REVERB 2** | LOW CUT (do que entra no reverb) · RETURN (level no mix) | ALGORITHM à esquerda: volta. |
| **DELAY** (mantenha HOME também) | TIME (1/32 ... 1/2, com valores T e pontuados) · FEEDBACK · TONE · TAPE (o primeiro oitavo = digital limpo, depois fita com mais desgaste) | ALGORITHM à direita: DELAY 2. |
| **DELAY 2** | LOW CUT · RETURN · PING-PONG (off / on) | ALGORITHM à esquerda: volta. |

ALL FX desligado silencia os dois sends D/W do PERFORM e o low cut master; os sends próprios das faixas continuam funcionando. Mutes e solos são compartilhados pelo PERFORM e pelo MUTE / SOLO; os solos não são salvos e acabam quando um preset é carregado. As páginas de efeitos são globais (todas as faixas as compartilham): SELECT percorre PERFORM, REVERB, DELAY, REVERB 2, DELAY 2.

### HOME

Toque = MIX 2 → SENDS → GLOBAL → MIX 2. Mantido = DELAY.

| Página | Knob 1 · 2 · 3 · 4 | Também nesta página |
|---|---|---|
| **MIX 2** | LEVEL · PAN · LOW CUT (20-663 Hz, nunca desligado) · HIGH CUT (20 kHz até 1 kHz) | Os dois cuts ficam acinzentados em classic. |
| **SENDS** | REVERB SEND · DELAY SEND · SIDECHAIN INPUT (faixa de 1 a 8, número grande) · SIDECHAIN THRESHOLD (funciona como o threshold do compressor: todo à direita = 100 % = nada sofre duck, gire para a esquerda para mais duck) | ALGORITHM à direita: a tela DUCK (abaixo). Os sends funcionam seja o que for que PERFORM D/W e ALL FX digam. |
| **DUCK** (SENDS, ALGORITHM à direita) | - | Teclas pretas de 1 a 8: azul = com duck, cinza = sem, roxo = a entrada. A entrada não pode sofrer duck. Nada sofre duck até você escolher. ALGORITHM à esquerda ou HOME: volta. |
| **GLOBAL** | QUANTIZATION (1/8 ... 1/32) · SWING · FILL RATE (2x, 4x, 8x) · TEMPO | - |

O duck segue o som da faixa de entrada (um kick longo faz duck por mais tempo; uma entrada em mute não faz duck) e também faz duck nos sends das faixas. Um ponto vermelho mostra uma faixa com duck, um ponto roxo a entrada; o ponto acompanha a faixa de entrada a cada batida, seja qual for o threshold (com o knob todo à direita nada sofre duck e o ponto fica escuro). Um sidechain para todas as faixas, salvo com o preset.

### SEL, ENV, LFO, EDIT (as páginas de voz)

Todas as quatro são páginas de faixa: p-locks e chance funcionam nelas.

| Botão | Knob 1 · 2 · 3 · 4 | Também nesta página |
|---|---|---|
| **SEL** PITCH MOD | SHAPE (DECAY, SINE, RANDOM) · AMOUNT (64 = nenhum) · RATE · - | |
| **ENV** OSCILLATOR | WAVE (SINE, TRI, SAW) · FREQUENCY (20 Hz-20 kHz) · ATTACK · DECAY | OCT- / OCT+ movem a frequência em uma oitava. |
| **LFO** NOISE FILTER | MODE (LP, BP, HP) · Q · FREQUENCY · - | |
| **EDIT** NOISE ENVELOPE | MODE (EXP, LIN, CLAP) · botão CHOKE · ATTACK · DECAY | {r:FUNC + knob 2 à direita}: a faixa entra no grupo choke; à esquerda: sai. O knob 2 sozinho só mostra uma mensagem. ALGORITHM à direita: a tela CHOKE. {r:FUNC, toque em EDIT}: DELETE AUTOMATION? (OCT+ sim, OCT- não). |
| **CHOKE** (EDIT, ALGORITHM à direita) | - | Teclas pretas de 1 a 8: azul = no grupo. Um toque de uma faixa do grupo para as outras na hora, como um hat fechado para um aberto. ALGORITHM à esquerda: volta. Salvo com o preset. |

DELETE AUTOMATION remove os p-locks e o movimento gravado da faixa selecionada no pattern que está tocando (as outras faixas, os outros patterns e as chances ficam; o undo funciona). Manter FUNC + EDIT e depois apertar ACCENT, FILL ou PLAY é a cópia da página, não um apagar.

### GLO

Toque = VEL → MIXER → COMP → MEMORY → SETTINGS → VEL.

| Página | Knob 1 · 2 · 3 · 4 | Também nesta página |
|---|---|---|
| **VEL** | OSC · NOISE · PITCH MOD (quanto a velocity move cada um) · RANDOM | {r:FUNC + knob 4}: a faixa selecionada recebe um som aleatório novo (só os 21 ajustes de som). O knob 4 sozinho só mostra um lembrete. Página de faixa: os p-locks funcionam. |
| **MIXER** | LEVEL das quatro faixas mostradas | Oito sliders altos; SELECT alterna as faixas 1 a 4 / 5 a 8 e, como diz a dica azul na tela, alterna level / pan (centro do pan = vermelho). As teclas pretas de 1 a 8 escolhem uma faixa e o seu grupo. Um toque acende o seu slider. {r:FUNC + tecla preta de 1 a 8}: mute. Um ponto vermelho sob um slider = com duck, roxo = entrada do sidechain. A chance funciona aqui. |
| **COMP** | ATTACK · RELEASE · THRESHOLD · MAKE-UP | Compressor master, ratio 4:1, uma agulha mostra a redução de ganho. Padrão: nada comprimido (0.0 dB / 0.0 dB). Guardado só por SAVE. |
| **MEMORY** | - | Uma bateria mostra quão cheia está a memória do projeto (patterns, song, movimento, notas ARP). Roxa a partir de 90 %, FULL a 100 %: um salvamento é então recusado. |
| **SETTINGS** | lista (SELECT move) | Linhas: clock out, notes out, canal MIDI do Tonic, output level, USB audio, luz das teclas, LED 4/4, tema de cor, zoom knob (sim / não), Tonic Engine, nome dos projetos, save, autosave, limpar pattern, factory reset, performance, cores, about, bateria. As menos óbvias são explicadas logo abaixo. As mudanças são guardadas sozinhas cerca de um segundo e meio depois que você para de mexer (o sequencer precisa estar parado). |

#### Ajustes GLO: os que não são óbvios

Os ajustes estão na lista GLO (SELECT move para cima e para baixo, ALGORITHM muda o valor). Os padrões estão em negrito.

| Ajuste | Opções | O que faz |
|---|---|---|
| **OUTPUT** | NORMAL / **LOUD** | LOUD (o ajuste de fábrica) deixa as saídas do FM-1 6 dB mais quentes (o fundo de escala do codec). NORMAL mantém 6 dB de headroom: use se o som estiver alto demais, e fique de olho no compressor e no limiter. |
| **USB AUDIO** | **FOLLOWS KNOB** / FULL | FOLLOWS KNOB: o computador recebe o que os fones ouvem, depois do knob MASTER. FULL: o computador sempre recebe o level completo, e o MASTER só muda as saídas do próprio FM-1 (alto-falante, fones, line out). Use FULL para gravar em level completo e mesmo assim monitorar baixo. |
| **LED 4/4** | OFF / **ON** | ON (o ajuste de fábrica): as teclas brancas 1, 5, 9 e 13 brilham de leve para marcar os quatro tempos do compasso (não no PERFORM). |
| **ZOOM KNOB** | **YES** / NO | YES: o knob que você gira é ampliado sobre a tela. NO: ele só é destacado, e a página inteira continua visível. A tela SONG nunca dá zoom. |
| **NAME PROJECTS** | **GENERATE** / MANUAL | GENERATE inventa o nome de um projeto salvo (ALGORITHM = outro). MANUAL: a tela do nome é um editor de letras (veja o capítulo 5). |
| **TONIC ENGINE** | **ENHANCED** / CLASSIC | CLASSIC acinzenta e silencia os extras do FM1 TONIC (cuts da faixa, sends, sidechain) para que o som use só os ajustes que um arquivo `.mtpreset` pode levar (capítulos 2 e 8). |
| **2ND CORE** | **ON** / OFF | O chip do FM-1 tem dois cores e o Tonic divide entre eles o cálculo do som. Vale a partir da próxima vez que você ligar. Se o FM-1 travar alguma vez com dois cores, ele muda sozinho para OFF; volte a colocar ON aqui. |

### SEQ e ARP

| Página | Knob 1 · 2 · 3 · 4 | Também nesta página |
|---|---|---|
| **SEQ** (1º toque) | FIXED LEN (8, 16, 32, 64) · AUTOSCROLL · FREE LEN (1 a 64) · SWING | Teclas brancas: os steps. SELECT: as páginas de steps (16 de cada vez; AUTOSCROLL desligado): a página (1-16, 17-32 ...) aparece grande no terceiro knob por um instante e depois o knob volta a ser FREE LEN. A página nunca fica em steps que não existem: se o loop ficar mais curto que a página em que você estava, ela volta para a última página que o comprimento tem. ALGORITHM: a voz (1 a 8, depois AC = o accent global). Mantenha um step + ACCENT / FILL: flags (accent = azul escuro, fill = amarelo, os dois = verde). {r:FUNC + SEQ}: modo song ligado / desligado (confirme com OCT+). {o:FILL + SEQ}: a tela SONG. |
| **PATTERN** (SEQ, 2º toque) | SONG ON / OFF | Teclas pretas de 1 a 8 dão mute, teclas brancas de 1 a 16 escolhem o próximo pattern. Uma tecla apagada = pattern vazio, luz fraca = tem notas, piscar lento = o atual, piscar rápido = o próximo (em fila). {r:FUNC toque} e {o:FILL toque} como no PERFORM. SEQ de novo: volta. |
| **ARP** | FIXED LEN · OCTAVE (também OCT- / OCT+, -3 a +3) · PAGE (1-16, 17-32 ...) · - | A tela SEQ, mas todas as teclas (brancas e pretas) tocam a voz selecionada como um teclado. As teclas C (fracas) tocam o tom próprio da voz na oitava 0. ALGORITHM: a voz. SELECT: as páginas de steps, mostradas no terceiro knob. Nada é gravado. SEQ: volta. {r:FUNC + ARP}: apaga a song (DELETE SONG?). |

Aumentar um pattern copia o que já existe para a parte nova (de 8 para 16 copia os steps de 1 a 8 para 9 a 16); diminuir só encurta.

### PLAY, REC, SAVE

| Botão | O que faz |
|---|---|
| **PLAY** | Toca e para. {r:FUNC + PLAY}: limpa o pattern (pergunta). |
| **REC** | REC e depois PLAY: gravação ao vivo. Cada tecla preta de 1 a 8 escreve um trig da sua faixa no step mais próximo; cada knob de som que você gira é gravado como automação. Não no PERFORM e na página de patterns. {r:FUNC + REC}: QUANTIZED REC (padrão) / UNQUANTIZED REC (o toque mantém o timing exato, dentro do step); durante a reprodução também liga a gravação. |
| **SAVE** | Veja o capítulo 5. {r:FUNC + SAVE}: salvamento rápido. {r:FUNC + HOME}: renomear. |

### Combinações de botões

Todas as combinações de botões e teclas, agrupadas pelo botão que você mantém primeiro. A tecla preta 9 é ACCENT, a 10 é FILL, a 11 é FUNC. {r:Vermelho} = FUNC mantido, {o:laranja} = FILL mantido. A última coluna diz onde funciona. **"Todas as páginas" significa todas as páginas exceto ARP e as duas páginas PERFORM**: no ARP, FUNC, ACCENT e FILL são teclas de teclado, e no PERFORM FUNC e FILL são as teclas dos modos MUTE e SOLO, então as suas combinações não funcionam ali. A única exceção é FUNC + PLAY (limpar o pattern), que também funciona no ARP com a tecla preta 11 mantida. GLO + ACCENT / FILL e as combinações feitas só de botões e knobs (HOME + SELECT, ENV + um knob ...) funcionam.

**FUNC mantido (tecla preta 11)**

| Teclas | O que faz | Onde |
|---|---|---|
| {r:FUNC + tecla preta de 1 a 8} | Dá mute na faixa (de novo = tira o mute). Age quando você solta a tecla. | Todas as páginas |
| {r:FUNC + tecla branca de 1 a 16} | Troca para esse pattern (durante a reprodução: no fim do compasso). | Todas as páginas; não na tela SONG nem no modo song (a song decide) |
| {r:FUNC + ACCENT} | Copia o pattern. A tela diz PRESS PATTERN TO PASTE / PRESS SEQ TO EXIT, todas as teclas de pattern piscam (um pattern vazio mais fraco) e SEQ pisca: aperte uma tecla de pattern e a cópia vai parar ali, e esse pattern passa a ser o atual. Toque FUNC sozinho antes de escolher para ficar no modo para vários pastes. SEQ sai do modo e abre a página do sequencer; {r:FUNC + ACCENT} de novo ou qualquer outro botão também sai. | Todas as páginas |
| {r:FUNC + FILL} | Cola o pattern copiado quando você solta FILL. Pergunta PASTE PATTERN? (OCT+ = sim, OCT- = não). | Todas as páginas |
| {r:FUNC + FILL mantido + tecla branca} | Cola o pattern copiado nesse lugar na hora (sem pergunta). | Todas as páginas |
| {r:FUNC + PLAY} | Limpa o pattern (pergunta CLEAR PATTERN?). Não inicia a música. | Todas as páginas, ARP também (ali mantenha a tecla preta 11, uma tecla de piano no ARP: PLAY não inicia / para enquanto ela estiver mantida) |
| {r:FUNC + tecla preta de 1 a 8 mantida, depois ACCENT} | Copia o som dessa faixa (não os seus steps). | Todas as páginas |
| {r:FUNC + tecla preta de 1 a 8 mantida, depois FILL} | Cola o som copiado nessa faixa (mute, volume e pan também; a faixa não fica em mute por isso). Sem pergunta. | Todas as páginas |
| {r:FUNC + tecla preta de 1 a 8 mantida, depois PLAY} | Limpa o pattern dessa faixa (pergunta CLEAR TRACK PATTERN?). | Todas as páginas |
| {r:FUNC + EDIT ou LFO mantido, depois ACCENT} | Copia esta página da faixa selecionada (só os seus parâmetros). | Páginas que têm parâmetros de faixa (não GLO, HOME, SEQ) |
| {r:FUNC + EDIT ou LFO mantido, depois FILL} | Cola na faixa selecionada, na mesma página. | Páginas que têm parâmetros de faixa (não GLO, HOME, SEQ) |
| {r:FUNC + EDIT ou LFO mantido, depois PLAY} | Volta esta página da faixa aos valores de quando liga. Sem pergunta; o undo desfaz. | Páginas que têm parâmetros de faixa (não GLO, HOME, SEQ) |
| {r:FUNC + EDIT}, tocado sozinho | DELETE AUTOMATION?: remove os p-locks e o movimento gravado da faixa selecionada no pattern que está tocando (pergunta). | Todas as páginas |
| {r:FUNC + SEQ} | Modo song ligado / desligado (pergunta: OCT+ = sim). | Todas as páginas |
| {r:FUNC + ARP} | DELETE SONG?: apaga a song inteira e sai do modo song (pergunta). | Todas as páginas |
| {r:FUNC + HOME} | Renomeia o projeto e o salva no mesmo lugar. | Todas as páginas |
| {r:FUNC + SAVE} | Salvamento rápido no lugar carregado. Sem pergunta. | Todas as páginas |
| {r:FUNC + REC} | Alterna QUANTIZED REC / UNQUANTIZED REC. Durante a reprodução também liga a gravação. | Todas as páginas |
| {r:FUNC + OCT- / OCT+} | Nada. | |
| {r:FUNC + knob 2 (gire para a direita / esquerda)} | A faixa selecionada entra / sai do grupo choke. | Só EDIT, página NOISE ENVELOPE |
| {r:FUNC + knob 4} | Um som aleatório novo para a faixa selecionada. | Só GLO > VEL |

**FILL mantido (tecla preta 10)**

| Teclas | O que faz | Onde |
|---|---|---|
| {o:FILL + tecla preta de 1 a 8} | Coloca essa faixa em solo (outra tecla move o solo, a mesma tecla o termina). | Todas as páginas |
| {o:FILL + SEQ} | Volta à tela SONG de qualquer página (no modo song; senão NOT IN SONG MODE). | Todas as páginas |
| {o:FILL}, apertado e solto sozinho | Apaga o compasso sob o cursor (sem pergunta). | Só a tela SONG |
| {o:FILL} + uma tecla branca mantida (ou FILL primeiro, depois teclas brancas) | Esse step recebe um fill (a faixa toca 2x, 4x ou 8x sobre ele); de novo = desligado. | Páginas onde as teclas brancas são steps |
| {o:FILL} mantido | Os trigs da faixa selecionada que levam um fill piscam nas teclas brancas (os outros trigs ficam acesos). | Páginas onde as teclas brancas são steps |
| {r:FUNC + FILL} | Colar. Veja a lista de FUNC acima. | |

**ACCENT mantido (tecla preta 9)**

| Teclas | O que faz | Onde |
|---|---|---|
| Tecla branca mantida + ACCENT (ou ACCENT primeiro, depois teclas brancas) | Esse step recebe um accent (forte); de novo = desligado. | Páginas onde as teclas brancas são steps |
| ACCENT mantido | Os trigs da faixa selecionada que levam um accent piscam nas teclas brancas (os outros trigs ficam acesos). | Páginas onde as teclas brancas são steps |
| {r:FUNC + ACCENT} | Copiar. Veja a lista de FUNC acima. | |

**GLO mantido**

| Teclas | O que faz | Onde |
|---|---|---|
| GLO + ACCENT | Undo. | Todas as páginas, ARP também |
| GLO + FILL | Redo. | Todas as páginas, ARP também |

**HOME mantido**

| Teclas | O que faz | Onde |
|---|---|---|
| HOME + SELECT | O tempo. | Todas as páginas |
| HOME mantido sozinho (meio segundo) | A página DELAY. | Todas as páginas |
| HOME + tecla branca | Nada. | |

**FX mantido**

| Teclas | O que faz | Onde |
|---|---|---|
| FX mantido sozinho (meio segundo) | A página REVERB. | Todas as páginas |

**ENV mantido**

| Teclas | O que faz | Onde |
|---|---|---|
| ENV + um knob | Esse knob volta ao valor de quando liga. | Páginas com knobs |
| ENV + tecla branca | O accent desse step liga / desliga. | Páginas onde as teclas brancas são steps |
| ENV + tecla preta de 1 a 8 | Toca essa faixa forte. Gravando: o step recebe um accent. | Todas as páginas |

**SAVE mantido**

| Teclas | O que faz | Onde |
|---|---|---|
| SAVE + um knob | O movimento gravado desse knob é esquecido. | Páginas com knobs |
| Tecla branca mantida + SAVE + o mesmo knob | Remove o p-lock desse knob nesse step. | Páginas de som |

**Uma tecla branca mantida (um step)**

| Teclas | O que faz | Onde |
|---|---|---|
| Tecla branca mantida + knob de 1 a 4 | **P-lock**: esse step recebe o seu próprio valor do knob (3 knobs com lock por faixa e pattern). | Páginas de som: MIX 1, SEL, ENV, LFO, EDIT, VEL |
| Tecla branca mantida + SELECT | A chance desse step: de 5 a 100 %, depois 1:2, 2:2, 1:3 ... 8:8. | Todas as páginas, menos PERFORM, HOME, SONG, COMP e as listas |
| Tecla branca mantida + ACCENT / FILL | Flag de accent / fill. | Veja ACCENT e FILL acima |

**Transporte e outros**

| Teclas | O que faz | Onde |
|---|---|---|
| REC + PLAY, depois teclas e knobs | Gravação ao vivo: as teclas pretas de 1 a 8 escrevem trigs, os knobs escrevem automação. | Não no PERFORM e na página de patterns |
| OCT- / OCT+ | O tom da faixa selecionada uma oitava abaixo / acima. Em uma pergunta: NO / YES. No ARP: a oitava do teclado. | Todas as páginas |
| Um botão mantido sozinho (cerca de um segundo) | Um cartão diz o que o botão faz nesta página, combinações incluídas. | Todos os botões exceto FX e HOME |

## 4. Tocar

- **Steps.** Selecione uma faixa, aperte uma tecla branca para colocar um step, aperte de novo para tirar. **Accent** (forte) e **fill** (a faixa toca quatro vezes sobre esse step): mantenha a tecla branca e aperte ACCENT e / ou FILL (de novo = flag desligada), ou mantenha ACCENT / FILL primeiro e aperte as teclas brancas.
- **Patterns.** {r:FUNC + tecla branca} troca de pattern. {r:FUNC + ACCENT} copia, {r:FUNC + FILL} cola (mantenha FILL e aperte uma tecla branca para colar nesse lugar), {r:FUNC + PLAY} limpa. Colar o pattern pergunta **PASTE PATTERN?** (OCT+ = sim, OCT- = não) e limpar pergunta CLEAR PATTERN?. Com uma tecla preta de 1 a 8 também mantida, copiam, colam e limpam só essa faixa; colar o som de uma faixa ou uma página, colar com FILL e uma tecla branca, e o modo copiar-colar colam na hora, sem pergunta.
- **Copiar-colar na hora.** Depois de {r:FUNC + ACCENT} a tela inteira diz **PRESS PATTERN TO PASTE** e **PRESS SEQ TO EXIT** em letras azuis grandes. Todas as teclas de pattern piscam (um pattern com notas pisca forte, um pattern VAZIO pisca mais fraco, o copiado fica aceso) e o botão **SEQ** também pisca. Aperte uma tecla de pattern e a cópia vai parar ali, esse pattern passa a ser o atual, e o modo termina. Para colar em vários lugares, **toque FUNC sozinho** antes de escolher: o modo continua e o pattern atual não muda. Para sair do modo aperte **SEQ**: ele abre a **página do sequencer**, não a página de patterns (o próximo toque em SEQ volta ao normal). Qualquer outro botão também sai, e {r:FUNC + ACCENT} de novo também (a tela diz COPY MODE OFF). Mantenha {r:FUNC + ACCENT} e aperte uma tecla branca = esse pattern é a cópia. Não disponível no PERFORM e na página de patterns, onde FUNC é o modo MUTE.
- **Mute e solo.** {r:FUNC + tecla preta de 1 a 8} dá mute (de novo = tira o mute). {o:FILL + tecla preta de 1 a 8} coloca essa faixa em solo (outra tecla move o solo, a mesma tecla o termina). Os solos não são salvos e acabam quando um preset é carregado. Não no ARP.
- **P-locks e chance.** Mantenha uma tecla branca e gire um knob em uma página de voz: esse step recebe o seu próprio valor, todos os outros steps mantêm o som. Durante a reprodução, você o ouve na próxima vez que o step passar. Para remover um: mantenha o step, mantenha SAVE, gire o mesmo knob. Uma faixa guarda 3 knobs com lock por pattern (a tela diz MAX 3 PER TRACK, MOTION FULL quando não sobra espaço). Mantenha um step e gire SELECT: a sua chance, descendo de 100 % em passos de 5 %, depois 1:2, 2:2, 1:3 ... 8:8. Enquanto você gira, a chance ocupa a tela inteira (TRIG % e o step), grande, para ler de relance.
- **Chains.** Um pattern pode ter uma flag para passar ao seguinte quando termina (depois do pattern 8 vem o 1). Uma flag só vale em um pattern com notas. Um preset com chains carrega tocando por elas (SONG ON); o knob 1 da página de patterns do SEQ desliga isso e o pattern fica em loop.
- **Modo song.** {r:FUNC + SEQ}, confirme com OCT+ (OCT- = volta). Sem song ainda, abre a tela PATTERNS: as teclas brancas de 1 a 16 adicionam patterns, OCT- (ou FUNC + uma tecla branca) tira o último, OCT+ é OK. PLAY toca um compasso por pattern, em loop. O mesmo gesto sai do modo song. Na **tela SONG** (uma linha por compasso) o knob 1 = o compasso, o knob 2 = o seu pattern, o knob 3 = repetições (x1 a x8); as teclas brancas escrevem um pattern no compasso sob o cursor e avançam; FILL, apertado e solto, apaga o compasso na hora (o último compasso apagado = a song é apagada e o modo song termina). {r:FUNC + ARP} apaga a song inteira.

## 5. Presets e salvamento

**Os presets funcionam nos dois sentidos.** O FM-1 carrega presets `.mtpreset` do **Patternarium** e do **Microtonic**, e os presets que você faz no FM1 TONIC podem ser exportados de volta para `.mtpreset` e usados no **Microtonic VST**. Foi testado: os presets exportados abrem e tocam no Microtonic. O som é muito, muito parecido com o original, não idêntico. Importar e exportar é feito na página web (a página de backup: "Restore my presets" e "Export my presets as .mtpreset"). Os slots 1 a 8 não podem ser exportados. O que o FM1 TONIC acrescentou (notas ARP, batidas atrasadas, cuts e sends de faixa, compressor, reverb, delay, mute) não tem lugar num `.mtpreset` e fica de fora.

Gire **PRESETS** pelos 40 lugares: de 01 a 08 são presets de fábrica, de 09 a 40 são seus (um lugar vazio carrega com os sons de um preset de fábrica e sem pattern: 09 soa como 01, 10 como 02, e assim em rotação). Uma pergunta vem antes: OCT+ carrega, OCT- mantém o seu som. Carregar é recusado durante a reprodução.

- **SAVE** pergunta SAVE HERE? com o lugar carregado. OCT+ salva ali e mantém o nome. ALGORITHM escolhe outro lugar (1 a 40; o âmbar mostra quem seria substituído). OCT- vai para CHOOSE SLOT, depois a tela do slot, depois a tela do nome (ALGORITHM = outro nome inventado; OCT+ salva). HOME sai sem salvar. Se nada foi carregado desde que ligou, SAVE começa em CHOOSE SLOT.
- {r:FUNC + SAVE} faz o salvamento rápido no lugar carregado sem perguntar o slot, também durante a reprodução. Um lugar vazio salva na hora como UNTITLED 01, 02 ...; um lugar ocupado pergunta OVERWRITE SLOT? (OCT+ sim).
- {r:FUNC + HOME} renomeia o projeto carregado (ALGORITHM = outro nome, OCT+ salva, OCT- ou HOME cancela). Com NAME PROJECTS em MANUAL (ajustes GLO) a tela do nome vira um editor de letras: o knob 4 percorre os caracteres, OCT+ (a tela diz OK) aceita a letra, OCT- apaga a última, depois aperte SAVE (8 letras no máximo; a tela mostra PRESS SAVE e TURN KNOB 4 em letras grandes).
- **O que é salvo.** Com AUTOSAVE ON, o seu projeto de trabalho é salvo 4 segundos depois que você para de mexer, exceto o compressor, o reverb e o delay: esses só são guardados por SAVE, dentro do preset. O PERFORM sempre começa limpo ao ligar (D/W 0 %, low cut master 20 Hz, ALL FX ligado). O AUTOSAVE vem OFF de fábrica: aperte SAVE (ou FUNC + SAVE) para guardar o seu trabalho. Os ajustes continuam sendo guardados sozinhos. Coloque AUTOSAVE em ON em GLO > SETTINGS se quiser que o projeto seja guardado sozinho. Ajustes de fábrica: clock out e notes out desligados, canal MIDI do Tonic 10, canal MIDI remoto desligado, CC mode desligado (CC CH BASE 1-8), output LOUD, USB audio segue o knob, LED 4/4 ligado, tema TONIC, zoom knob sim, nome dos projetos GENERATE, Tonic Engine ENHANCED, 2nd core ON. Uma unidade nova ou um FACTORY RESET os recebe; uma unidade que já guarda um projeto salvo mantém os seus próprios ajustes.

## 6. Luzes, MIDI e USB

- **Luzes.** PLAY acende a cada 4º step (vermelho enquanto grava). REC: apagado quando não grava; fixo com a reprodução parada (quantized) e piscando com o tempo durante a reprodução; dois piscares rápidos e um lento para unquantized. O botão da página atual fica aceso. As teclas brancas mostram os steps da faixa selecionada (o step que toca se inverte); 1, 5, 9 e 13 podem brilhar de leve (GLO > LED 4/4). Teclas pretas de 1 a 8: a faixa selecionada forte (piscando quando em mute), as faixas com notas fracas, e durante a reprodução a tecla de uma faixa pisca a cada toque dela (não no ARP). Com FUNC mantido, as teclas mostram quais patterns e faixas têm notas.
- **MIDI.** Por USB ou pela entrada TRS MIDI IN, o canal 10 por padrão toca as vozes com notas de bateria General MIDI na ordem das vozes: 36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51 (velocity = volume). Um clock recebido é seguido; Start, Stop e Continue controlam o pattern. Ajustes GLO: TONIC MIDI CH (ou OFF), REMOTE MIDI CH (ou OFF, veja abaixo), CC MODE e CC CH BASE (control changes, veja abaixo), CLOCK OUT (clock e start / stop), NOTES OUT (o pattern como notas).
- **Painel remoto.** GLO > REMOTE MIDI CH (OFF por padrão) faz um segundo canal MIDI tocar as teclas do próprio FM-1. As 27 teclas do painel são as 27 notas de F3 a G5 (notas MIDI 53 a 79, dó central = 60), dispostas como um teclado de piano: a primeira tecla, F3, é a nota 53, a última, G5, é a nota 79. Uma nota on aperta a tecla, uma nota off a solta, então uma nota longa mantém a tecla apertada. Tudo o que uma tecla faz no painel funciona igual: as teclas pretas de 1 a 8 selecionam ou dão mute nas faixas, as teclas brancas de 1 a 16 são os steps ou os patterns, e as teclas pretas 9, 10 e 11 são ACCENT (nota 73), FILL (nota 75) e FUNC (nota 78). Um acorde vale em qualquer ordem de notas: ACCENT, FILL e FUNC são sempre aplicados primeiro, então FUNC + ACCENT + uma tecla branca funciona quando as três notas chegam juntas. All Notes Off (controller 123) solta todas as teclas. As notas do canal remoto não tocam as vozes; dê a ele um canal diferente de TONIC MIDI CH. As notas fora de 53 a 79 são ignoradas.

| Tecla | Notas |
| --- | --- |
| Teclas brancas de 1 a 16 | 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 |
| Teclas pretas de 1 a 8 (faixas) | 54, 56, 58, 61, 63, 66, 68, 70 |
| Tecla preta 9 = ACCENT, 10 = FILL, 11 = FUNC | 73, 75, 78 |

### Control changes (CC)

Um control change gira um knob de fora: o número do CC é o **parâmetro** (os mesmos números para todas as faixas) e a **faixa** é escolhida de duas maneiras. GLO > **CC MODE** decide (OFF por padrão: nada reage a CC até você ligar):

| CC MODE | O que faz |
| --- | --- |
| OFF | Todo control change é ignorado. |
| SELECTED | Um CC no canal do Tonic (TONIC MIDI CH) move o parâmetro da faixa **selecionada**. |
| CHANNELS | Um bloco de 8 canais, um por faixa: o primeiro canal de GLO > **CC CH BASE** é a faixa 1, o seguinte a faixa 2, e assim por diante (base 1 = canais 1-8). Qualquer faixa pode ser movida a qualquer momento, sem precisar selecioná-la antes. |
| BOTH | Os dois juntos. Se o canal do Tonic estiver dentro do bloco, o bloco vale nesse canal. |

As notas continuam tocando as vozes em TONIC MIDI CH e o painel remoto mantém o seu próprio canal, então dê ao bloco canais que esses dois não usam (o bloco padrão 1-8 deixa livres o 10 e o 11). Um CC move o valor como o knob faz, então a tela, o SAVE e o autosave acompanham; ele não abre o dial grande, não faz lock em um step e não é gravado. Um CC 0-127 é o 0-127 do knob; uma chave de três posições (forma de onda, modo do filtro, modo do envelope) divide a faixa em três. No engine CLASSIC os extras (low cut, high cut, reverb send, delay send) ignoram o CC como os seus knobs acinzentados.

| CC | Parâmetro | CC | Parâmetro |
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

### Conectar um Digitakt ou Digitone

A tomada USB do FM-1 e a do Digitakt ou Digitone são ambas tomadas "device", então não conseguem conversar entre si. Use o MIDI OUT (5 pinos) do Digitakt ou Digitone no TRS MIDI IN do FM-1, com um adaptador MIDI de 5 pinos para TRS de 3,5 mm (tipo A ou B: se nada chegar, tente o outro tipo).

1. No FM-1: ajustes GLO, TONIC MIDI CH = 10 (as vozes) e REMOTE MIDI CH = 11 (o painel), ou dois canais diferentes quaisquer.
2. No Digitakt ou Digitone: use duas faixas MIDI e coloque os seus canais nesses dois números. A primeira toca as vozes com as notas de bateria (36, 38, 41, 45, 50, 37, 39, 42, 46, 49, 51). A segunda é o painel remoto: desenhe as notas da tabela acima na sua grade de steps ou no piano roll.
3. Uma faixa MIDI toca até 4 notas em um step, com um único comprimento para todas: o comprimento da nota é quanto tempo as teclas ficam mantidas, então uma nota longa para FUNC com outras notas em steps posteriores é FUNC + essas teclas, e um acorde de FUNC + ACCENT + uma tecla branca é um step.
4. Para mover o som pelas faixas de CC do Digitakt ou Digitone: ponha GLO > CC MODE = CHANNELS, faça uma faixa MIDI por faixa do Tonic nos canais 1 a 8 (ou a partir de CC CH BASE) e atribua as suas faixas de CC aos números da tabela acima; cada faixa MIDI tem um número limitado de faixas de CC (16 no Digitakt II, menos no primeiro Digitakt), então escolha os parâmetros que quer automatizar em cada faixa.
5. Para seguir o tempo e o start / stop do Digitakt ou Digitone, ligue o seu MIDI clock e transport out; o FM-1 segue um clock recebido.

### Conectar um teclado MIDI

O FM-1 não pode ser o host de um teclado USB. Use um teclado com MIDI OUT de 5 pinos (com um adaptador de 5 pinos para TRS na TRS MIDI IN), ou envie um teclado USB através de um computador ou de uma caixa host USB MIDI para a tomada USB do FM-1.

1. No FM-1: ajustes GLO, REMOTE MIDI CH = o canal em que o teclado envia (1 é o habitual).
2. Toque as notas F3 a G5 (53 a 79). Mantenha uma tecla para FUNC (nota 78, F#5), ACCENT (nota 73, C#5) ou FILL (nota 75, D#5) e aperte as outras: copiar, colar, troca de pattern, mute e solo funcionam como no painel. Um teclado de 49 teclas cobre o painel inteiro; em um menor, desloque-o com os botões de oitava.

- **USB audio.** O mesmo cabo leva o que os fones ouvem, depois do MASTER, como uma entrada de 2 canais a 44,1 kHz, sem driver (GLO > USB AUDIO = FULL envia o level completo seja qual for o MASTER). O primeiro instante depois que um app a abre pode dar um estalo.

## 7. Instalar, backup, recuperação

- **Experimente:** o [emulator](https://luigirossi-music.github.io/fm1-tonic/emu/) roda o mesmo código no navegador.
- **Instalar ou atualizar:** abra o [instalador web](https://luigirossi-music.github.io/fm1-tonic/install/) no Chrome ou no Edge, conecte o FM-1 com um cabo USB de dados, aperte Install e não o desconecte enquanto escreve. Outro firmware não guarda o seu projeto: salve-o em um arquivo antes. "Back to the stock firmware" no fim da mesma página recoloca o firmware da M-VAVE. Instalar outro firmware é por sua conta e risco.
- **Backup:** a [página de backup](https://luigirossi-music.github.io/fm1-tonic/backup/) salva o projeto em um arquivo `.x0b` (Back up), o carrega de volta (Restore: pare o FM-1, depois responda RESTORE FROM USB? com SEL), exporta presets como arquivos `.mtpreset`, e faz backup / restauração dos slots 9 a 40 como arquivos `.tnp`. Linha de comando: `python3 tools/x0x_backup.py backup my-song.x0b` (também `restore`, `info`).
- **Se algo der errado:** mantenha OCT- e OCT+ por cinco segundos = modo de atualização; mantenha-os ao ligar = calibração do hardware. Depois de um crash uma tela vermelha CRASH reinicia o FM-1; se ele travar duas vezes ao iniciar, entra em SAFE MODE (sem som, o USB funciona: reinstale pelo instalador; PLAY tenta um início normal). Se nem isso funcionar, o script de resgate para Mac em `tools/fm1_rescue.sh` recoloca o firmware da M-VAVE (ele confere cada arquivo que escreve; rodá-lo de novo é seguro).
- **Se uma atualização for recusada.** Se o FM-1 não aceitar trocar de firmware (por exemplo com "Back to the stock firmware"), a sua tela de atualização mostra FAILED, um número de erro com um motivo curto (por exemplo ERROR -5 HEAD DIFFERS) e a etapa em que parou, por 8 segundos, e depois o FM-1 inicia o FM1 TONIC de novo. ERROR -5 significa que a área de início (o "head") do seu FM-1 não é a do firmware para o qual você vai. Nesse erro a tela também mostra o primeiro byte que difere, quantos bytes diferem e as duas somas de verificação (CRC), por 40 segundos. Anote esses números se pedir ajuda.
- **Um bootloader personalizado.** Se você colocou um bootloader personalizado no FM-1 (por exemplo m-bootloader, com o M-Flasher), voltar ao firmware oficial da M-VAVE pela página web pode ser recusado com ERROR -5 HEAD DIFFERS. Recoloque primeiro o bootloader original da M-VAVE (no M-Flasher: FLASH BOOTLOADER, depois STOCK (FROM .FWSC) com o pacote oficial), e então volte ao firmware original. No FM-1 do autor isso fez tudo funcionar de novo.
- **FM-1 mais novos.** Alguns FM-1 mais novos têm outro chip de flash (ID B36014 em vez de 856014). O FM1 TONIC aceita os dois, então salvar, presets, backup e restauração funcionam neles também (ainda não testado em uma unidade com o chip novo).
- **Texto completo** dos capítulos de instalação, backup, resgate e parâmetros: `docs/reference/MANUAL_full_wip381.md`.

## 8. O engine

Cada uma das 8 faixas é um sintetizador de bateria completo: o engine do FM1 TONIC, nada é sampleado. As 8 faixas são idênticas e cada uma tem os mesmos **26 ajustes**, guardados por faixa e salvos com o preset. Este capítulo é a referência de todos eles. Onde um ajuste mora está escrito como botão · knob.

```
pitch mod ─► OSCILLATOR ─┐
                         ├─► MIX ─► EQ ─► DIST ─► LEVEL ─┐
NOISE ─► filter ─► env ──┘                               │
          ┌──────────────────────────────────────────────┘
          └─► LOW CUT ─► HIGH CUT ─┬─► PAN ─► master
                                   └─► REVERB / DELAY SEND
```

Um toque soa até os seus envelopes se apagarem. Um toque novo na mesma faixa corta o que ainda soa (a bateria é monofônica), e uma faixa no grupo choke é cortada pelas outras (fade de 3 ms).

**Os knobs são curvas, não retas.** Frequências, tempos e Q são exponenciais: o meio do knob não é o meio do valor. Na tela as frequências aparecem em Hz ou kHz, o AMOUNT do pitch mod em semitons (sm) e o pan como L / C / R, o OSC/NOISE MIX como duas porcentagens (100.00/00.00 = só oscillator, 00.00/100.00 = só noise); todos os outros knobs mostram um número simples de 0 a 127. Use esta tabela para ver o que esses números significam.

| Knob | 0 | 32 | 64 | 96 | 127 |
|---|---|---|---|---|---|
| **Frequência** (oscillator, filtro de noise, EQ) | 20 Hz | 114 Hz | 650 Hz | 3,7 kHz | 20 kHz |
| **Decay** (oscillator, noise, queda do pitch mod) | 10 ms | 57 ms | 0,33 s | 1,8 s | 10 s |
| **Attack** (oscillator, noise) | 0 | 20 ms | 0,16 s | 1 s | 5 s |
| **Q do filtro** | 0,1 | 1,8 | 33 | 600 | 10000 |

Um decay é o tempo que o som leva para cair ao silêncio (60 dB abaixo).

### Oscillator: o tom

| Ajuste | Onde | Intervalo | O que faz |
|---|---|---|---|
| **WAVE** | ENV · knob 1 | SINE, TRI, SAW | SINE é pura e suave, TRI oca, SAW brilhante e zumbida. |
| **FREQUENCY** | ENV · knob 2 | 20 Hz a 20 kHz | O tom. Kick de uns 40 a 70 Hz, toms de 80 a 250 Hz, corpo da snare de 150 a 250 Hz. OCT- / OCT+ movem em uma oitava. |
| **ATTACK** | ENV · knob 3 | 0 a 5 s | 0 começa na hora. Aumente para um crescendo. O primeiro quarto do knob fica abaixo de 20 ms. |
| **DECAY** | ENV · knob 4 | 10 ms a 10 s | Quanto tempo soa: curto = clique ou tique, longo = estrondo. |

### Pitch mod: o tom se move enquanto o som toca

| Ajuste | Onde | Intervalo | O que faz |
|---|---|---|---|
| **SHAPE** | SEL · knob 1 | DECAY, SINE, RANDOM | O tipo de movimento (abaixo). |
| **AMOUNT** | SEL · knob 2 | 64 = nenhum; até 8 oitavas para cada lado | Acima de 64 o tom sobe, abaixo de 64 desce. Com DECAY o som começa tão longe e desliza até o tom ajustado. |
| **RATE** | SEL · knob 3 | depende do SHAPE | DECAY: quanto tempo leva o deslize, de 10 ms a 10 s (queda do kick = curta, laser = longa). SINE: a velocidade da oscilação, de 0 a 2000 Hz; só os primeiros passos são um vibrato, mais alto é FM (sinos, zaps), e uma rate mais alta também o faz durar mais. RANDOM: com que rapidez o tom vagueia ao acaso, de 20 Hz a 20 kHz; lento = deriva, rápido = fica metálico. |

### Noise: o estalo e o chiado

O noise é ruído branco passando por um filtro e um envelope. O oscillator e o noise são separados: o MIX decide quanto de cada um você ouve.

| Ajuste | Onde | Intervalo | O que faz |
|---|---|---|---|
| **FILTER MODE** | LFO · knob 1 | LP, BP, HP | LP guarda os graves (baque, escuro), HP os agudos (hat, chiado), BP uma banda em volta da frequência. |
| **FILTER Q** | LFO · knob 2 | 0,1 a 10000 | Ressonância. Baixa = suave, por volta do knob 64 = um pico forte, alta = o filtro soa como um tom afinado: BP com Q alto e envelope curto é um ping afinado (tom, cowbell, bloco). |
| **FILTER FREQUENCY** | LFO · knob 3 | 20 Hz a 20 kHz | Onde o filtro trabalha. |
| **ENVELOPE MODE** | EDIT · knob 1 | EXP, LIN, CLAP | EXP: uma subida rápida e uma queda natural, como o oscillator. LIN: subida e queda em linha reta. CLAP: algumas rajadas curtas, como palmas. |
| **ENVELOPE ATTACK** | EDIT · knob 3 | 0 a 5 s | EXP e LIN: a subida. CLAP: quanto tempo o noise fica em level máximo antes de começarem as rajadas. |
| **ENVELOPE DECAY** | EDIT · knob 4 | 10 ms a 10 s | A queda. No CLAP também define o intervalo entre as rajadas: curto = um zumbido rápido, longo = palmas separadas. |

### Mix, EQ, distorção

| Ajuste | Onde | Intervalo | O que faz |
|---|---|---|---|
| **OSC/NOISE MIX** | FX · knob 1 | 100.00/00.00 a 00.00/100.00 | A tela mostra a parte do oscillator e a parte do noise em porcentagem. 100.00/00.00 = só oscillator, 49.61/50.39 (o meio do knob) = os dois em level máximo, 00.00/100.00 = só noise. Uma faixa nova começa em 100.00/00.00: sem noise até você mexer. |
| **EQ FREQUENCY** | FX · knob 2 | 20 Hz a 20 kHz | O centro da única banda de EQ. |
| **DIST** | FX · knob 3 | 0 a 127 | Distorção, muito íngreme: o primeiro quarto é um calor suave, passando do meio é pesada, no topo um fuzz. É assimétrica, então acrescenta volume e aspereza. |
| **EQ GAIN** | FX · knob 4 | 64 = flat; 40 dB para cada lado | Reforço ou corte da banda (larga, um só sino). O EQ vem **antes** da distorção, então reforçar e depois distorcer dá um som mais pesado do que o contrário. |

### Velocity: accents que mudam o som

O sequencer toca um step normal com cerca de um terço da velocity máxima e um step com accent com a máxima. Os três knobs VELOCITY (GLO · VEL, knobs 1 a 3) dizem quanto uma parte a segue.

| Ajuste | Onde | O que faz |
|---|---|---|
| **OSC** | GLO · knob 1 | 0 = o oscillator ignora a velocity. Mais alto = nos steps normais ele fica mais baixo, os accents ficam plenos. |
| **NOISE** | GLO · knob 2 | O mesmo para o noise. |
| **PITCH MOD** | GLO · knob 3 | O mesmo para o amount do pitch mod: nos steps normais o movimento encolhe. |

É íngreme: um valor pequeno já corta bastante um step normal, e a partir de uns 95 no knob um step normal perde essa parte por completo. Com os três em 0, um accent soa exatamente como um step normal: o accent só se ouve por meio desses knobs. Por exemplo, uma snare com VELOCITY OSC alto tem o seu tom só nos accents, e toca só noise nos steps normais.

### Output: level, pan, cuts, sends

| Ajuste | Onde | Intervalo | O que faz |
|---|---|---|---|
| **LEVEL** | HOME · knob 1 | silêncio até cerca de +10 dB (padrão 101 = cerca de -2,5 dB) | O volume da faixa. A voz faz clip no fundo de escala: se estalar, abaixe. |
| **PAN** | HOME · knob 2 | 0 esquerda, 64 centro, 127 direita | A posição no campo estéreo. |
| **LOW CUT** | HOME · knob 3 | 20 a 663 Hz | Tira os graves (12 dB por oitava). Em 20 Hz não faz nada audível. |
| **HIGH CUT** | HOME · knob 4 | 20 kHz descendo até 1 kHz | Tira os agudos (12 dB por oitava). Todo à direita = aberto. |
| **REVERB SEND** | HOME, 2º toque · knob 1 | 0 a 127 | Quanto da faixa vai para o reverb. |
| **DELAY SEND** | HOME, 2º toque · knob 2 | 0 a 127 | O mesmo para o delay. |

Os sends saem depois dos cuts e do sidechain, antes do pan, e funcionam seja o que for que PERFORM D/W e ALL FX digam.

### Ao redor da voz

Compartilhado por todas as faixas:

- **Grupo choke** (EDIT, ALGORITHM à direita): um toque de uma faixa do grupo para as outras na hora (o hat fechado para o hat aberto).
- **Sidechain** (HOME, 2º toque): uma faixa de entrada faz as faixas que você escolher na tela DUCK sofrerem duck enquanto ela soa; o knob de threshold funciona como o do compressor: todo à direita = desligado, para a esquerda faz mais duck.
- **Low cut master** (PERFORM · knob 3, 20 a 663 Hz): um low cut na mix inteira, antes do compressor.
- **Compressor** (GLO · COMP): 4:1, soft knee, attack de 0,1 a 100 ms, release de 10 a 1500 ms, threshold de -48 a 0 dB, make-up de 0 a 24 dB. Um limiter depois dele mantém a saída abaixo do fundo de escala.
- **Reverb** (FX mantido): decay, tone, low cut, nível de retorno; uma sala digital antiga e granulada. **Delay** (HOME mantido): tempo sincronizado (1/32 a 1/2), feedback, tone, TAPE (de digital limpo a fita gasta), low cut, retorno, ping-pong.

### Enhanced e classic

22 dos 26 ajustes são o que um preset do Patternarium leva (os 21 ajustes de som, level incluído, mais o pan). Quatro são **extras do FM1 TONIC**: LOW CUT, HIGH CUT, REVERB SEND e DELAY SEND, como também o grupo choke, o sidechain, a seção master, os p-locks, a chance e o microtiming. Em **classic** (GLO > settings > TONIC ENGINE) os quatro extras ficam acinzentados e mudos, e o sidechain também, então o que você constrói usa só os ajustes que um arquivo `.mtpreset` leva; os valores voltam no enhanced. Um preset do Patternarium carregado mantém os seus valores exatos até você girar esse knob.

### Receitas de som

São pontos de partida: mude um valor de cada vez e ouça.

#### Kick

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 20 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +37.04 sm | RATE 25 | - |
| **ENV** | WAVE SINE | FREQUENCY 56 Hz | ATTACK 0 | DECAY 72 |
| **LFO** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **EDIT** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

AMOUNT é a profundidade da queda e RATE a sua velocidade. Para um clique, ponha MIX 40 no FX, depois LFO: MODE HP, FREQUENCY 2.9 kHz, e EDIT: DECAY 15. Para um estrondo mais longo, aumente ENV DECAY.

#### Snare

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 15 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +12.85 sm | RATE 13 | - |
| **ENV** | WAVE TRI | FREQUENCY 196 Hz | ATTACK 0 | DECAY 46 |
| **LFO** | MODE HP | Q 22 | FREQUENCY 2.0 kHz | - |
| **EDIT** | MODE EXP | deixe | ATTACK 0 | DECAY 55 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Mais estalo: aumente o MIX. Mais corpo: abaixe o MIX. Para ghost notes suaves, ponha GLO NOISE em 30: os steps normais tocam o noise mais baixo e os accents o tocam pleno.

#### Hi-hat fechado

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **ENV** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **LFO** | MODE HP | Q 22 | FREQUENCY 7.1 kHz | - |
| **EDIT** | MODE EXP | deixe | ATTACK 0 | DECAY 25 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

OSC/NOISE MIX 00.00/100.00 significa só noise. Abaixe a frequência do LFO para um hat mais escuro.

#### Hi-hat aberto

Igual ao hi-hat fechado, com **EDIT DECAY 68**. Ponha os dois hats em um **grupo choke** para que o fechado pare o aberto: em cada uma das duas faixas mantenha FUNC e gire o knob 2 do EDIT para a direita.

#### Clap

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **ENV** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.1 kHz | - |
| **EDIT** | MODE CLAP | deixe | ATTACK 0 | DECAY 60 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Com esses valores você obtém 3 a 4 rajadas rápidas em cerca de um décimo de segundo. EDIT DECAY mais baixo = um zumbido mais apertado, mais alto = palmas mais lentas e separadas e uma cauda mais longa (75 a 85). Um pouco de reverb send (HOME, 2º toque) o transforma em uma palma de sala.

#### Tom

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +17.39 sm | RATE 20 | - |
| **ENV** | WAVE SINE | FREQUENCY 120 Hz | ATTACK 0 | DECAY 65 |
| **LFO** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **EDIT** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Use o mesmo som em outras faixas com uma FREQUENCY diferente (80 Hz para um surdo, 200 Hz para um tom agudo).

#### Zap

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +47.62 sm | RATE 62 | - |
| **ENV** | WAVE SAW | FREQUENCY 1.0 kHz | ATTACK 0 | DECAY 75 |
| **LFO** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **EDIT** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

O tom cai 4 oitavas em cerca de 0,3 s. Mande-o para o delay (HOME, 2º toque) para um laser clássico.

#### Rimshot

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 49.61/50.39 | EQ FREQ 20 Hz | DIST 30 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +11.34 sm | RATE 10 | - |
| **ENV** | WAVE TRI | FREQUENCY 552 Hz | ATTACK 0 | DECAY 25 |
| **LFO** | MODE BP | Q 40 | FREQUENCY 1.8 kHz | - |
| **EDIT** | MODE EXP | deixe | ATTACK 0 | DECAY 22 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Um clique curto e seco (cerca de 45 ms). Aumente DIST para um rim mais duro; mova o MIX para a esquerda para mais tom.

#### Sub bass

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 35 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +6.80 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 41 Hz | ATTACK 6 | DECAY 80 |
| **LFO** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **EDIT** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Uma nota grave de cerca de 0,9 s com uma pequena queda de tom no início e um pouco de DIST para que se ouça em caixas pequenas. Mude FREQUENCY (30 a 60 Hz) para outra nota, ou encurte o DECAY para um kick longo.

#### Conga

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 82.68/17.32 | EQ FREQ 20 Hz | DIST 10 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT +5.29 sm | RATE 12 | - |
| **ENV** | WAVE SINE | FREQUENCY 231 Hz | ATTACK 0 | DECAY 54 |
| **LFO** | MODE BP | Q 30 | FREQUENCY 1.6 kHz | - |
| **EDIT** | MODE EXP | deixe | ATTACK 0 | DECAY 20 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Um tom redondo com um tapa de noise bem curto (cerca de 0,2 s no total). Aumente FREQUENCY (por exemplo 330 Hz) em outra faixa para uma conga aguda.

#### Crash

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 00.00/100.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **ENV** | não se ouve (OSC/NOISE MIX 00.00/100.00), deixe | | | |
| **LFO** | MODE HP | Q 10 | FREQUENCY 5.1 kHz | - |
| **EDIT** | MODE EXP | deixe | ATTACK 0 | DECAY 100 |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Um prato de puro noise que dura cerca de 2,3 s. Abaixe o DECAY (85 dá cerca de 1 s) para um splash, ou aumente a FREQUENCY do LFO para um som mais fino. Mande um pouco dele para o reverb (HOME, 2º toque).

#### Clave

| Botão | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
|---|---|---|---|---|
| **FX** | OSC/NOISE MIX 100.00/00.00 | EQ FREQ 20 Hz | DIST 0 | EQ GAIN 64 |
| **SEL** | SHAPE DECAY | AMOUNT 0.00 sm | RATE 0 | - |
| **ENV** | WAVE SINE | FREQUENCY 2.5 kHz | ATTACK 0 | DECAY 28 |
| **LFO** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **EDIT** | não se ouve (OSC/NOISE MIX 100.00/00.00), deixe | | | |
| **GLO** | OSC 0 | NOISE 0 | PITCH MOD 0 | deixe |

Um tom seco e agudo de cerca de 50 ms, sem movimento de tom (AMOUNT 0.00 sm). Mude a FREQUENCY (2 a 3 kHz) para outra afinação.

**Dicas.** Para ouvir uma parte sozinha, gire o MIX todo para a esquerda (100.00/00.00, só oscillator) ou todo para a direita (00.00/100.00, só noise). Para que os accents mudem o som e não só o volume, aumente os knobs GLO. Para testar um valor em um step só, mantenha o step e gire o knob (p-lock, capítulo 4). Um som de que você gosta é guardado pelo SAVE (capítulo 5).

## 9. Créditos

**O FM1 TONIC é feito por Luigi Rossi**, o desenvolvedor e a mente criativa por trás deste firmware: a drum machine Tonic, o seu sequencer, as telas, os presets e as ferramentas web (emulator, instalador, backup). É software livre sob a GNU General Public License, versão 3; se você compartilhar o firmware, deve compartilhar o seu código-fonte.

**Luigi Rossi online:** [linktr.ee/luigirossi](https://linktr.ee/luigirossi)

LICENSING.md lista a origem e a licença de cada arquivo. Ele se apoia no trabalho de:

- **libpo32** de Eric Lewis (MIT): o ponto de partida do código de síntese de bateria do FM1 TONIC. Nada é sampleado.
- **X0X** de Charles Vestal, o firmware de que o FM1 TONIC cresceu, e as páginas web (emulator, instalador, backup); o X0X é um fork do **Felucca** de Leo Kuroshita (@kurogedelic), Hügelton Instruments: a plataforma (camada de hardware, USB, loader de atualização, instalador, armazenamento) e a placa de recuperação FM-1-transporter.
- O reverb e o delay são portados do **9W9** de athousanddetails, baseado no **ER-99** de Matthew Cieplak (GPL-3.0). O compressor master segue Giannoulis, Massberg e Reiss; o filtro master é o filtro de variável de estado TPT de Vadim Zavalishin.
- **Melodee** de keremimo e **Jangada** de zednaked: o que descobriram sobre o segundo core do FM-1.
- Fontes: **Barlow Semi Condensed** (The Barlow Project Authors), **Terminus** (Dimitar Toshkov Zhekov), **DotGothic16** (The DotGothic16 Project Authors), todas sob a SIL Open Font License.

M-VAVE e FM-1 são marcas de seus proprietários. O FM1 TONIC não tem ligação com a M-VAVE, a Sonic Charge nem Luigi Rossi, nem conta com o seu aval.
