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
  - Receitas de som {#sound-recipes}
9. Créditos {#s9}

---

> Os capítulos 2 a 9 estão em preparação em português: por enquanto aparecem aqui embaixo em inglês.

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
