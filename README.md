# FM1 TONIC for the M-VAVE FM-1

**FM1 Tonic** turns the M-VAVE FM-1 into a drum machine: **eight synthesised drum voices**, a
sequencer with accents, fills and up to 64 steps per pattern, 16 patterns per preset that can be
chained into a song, an 8-track mixer, reverb, delay and a master compressor. Over USB the FM-1 is
both a MIDI device and an audio input, with no driver needed. Version **1.4**, by Luigi Rossi.

FM1 Tonic is inspired by Microtonic (a trademark of Sonic Charge). The sound comes from FM1 Tonic's
own engine: it acts in a similar way, but it is not the same engine, and FM1 Tonic is not connected
with Sonic Charge.

- **Try it in your browser** (no FM-1 needed): <https://luigirossi-music.github.io/fm1-tonic/emu/>
- **Install it** (Chrome or Edge, with the FM-1 plugged in by USB): <https://luigirossi-music.github.io/fm1-tonic/install/>
- **Read the manual** (English, Español, Italiano, Français, Português, Deutsch): <https://luigirossi-music.github.io/fm1-tonic/manual/>
- **Download the firmware file** (`.fwsc`): [releases](https://github.com/luigirossi-music/fm1-tonic/releases)

> **Use it at your own risk.** FM1 Tonic is custom firmware, not made or approved by M-VAVE. The
> author accepts no responsibility if your FM-1 is damaged or misbehaves because of it. It is solid
> for everyday playing, but a very dense pattern can still push the FM-1 to its limit; when that
> happens the firmware trades a little sound quality to keep playing rather than drop out. It is hard to
> get stuck: the web installer can put M-VAVE's firmware back, an FM-1 that keeps crashing starts in a
> safe mode, and the manual's Troubleshooting section covers the rest.

## What's inside

- **Eight drum voices** (TRACK 1 to TRACK 8): oscillator, pitch modulation, noise with its own filter
  and envelope, distortion, EQ, level, pan and velocity. Presets use the `.mtpreset` format, so the
  Patternarium presets load.
- **A sequencer of up to 64 steps** with accents, fills (2x / 4x / 8x), per-step chance, p-locks,
  16 patterns per preset, chains and a song mode.
- **An 8-track mixer** with per-track sends, low / high cut, choke groups and a sidechain, reverb and
  delay, and a master compressor.
- **A performance page**, mute and solo, copy and paste of patterns, tracks and pages, undo and redo.
- **USB MIDI and USB audio**, with no driver needed, and 40 project slots on the FM-1.
- **A browser version** of the same code, with a backup page for your projects.

The manual (`docs/MANUAL.md`, or the page above) explains every button.

## Building and testing

[BUILDING.md](BUILDING.md) explains the toolchain. In short:

```
tests/run_tests.sh      # the maths, the sequencer, the engine, the app in a simulator
./build.sh              # build/fm1-tonic.fwsc
```

`host/x0x_host` runs the whole app on a computer from a script (`tests/scenarios/*.x0x`). Button
presses, keys, knob turns and MIDI go in; audio and screenshots come out. That is how most of
FM1 Tonic is tested before it reaches the FM-1. To rebuild the web page, run `tonic_site.sh` (Linux,
with Emscripten installed) and then `tools/publish_pages.sh`.

## Licence and credits

FM1 Tonic is free software under the **GPL-3.0-only** (see `LICENSE`). If you distribute it, or
firmware derived from it, you must give the recipients its complete source under the same licence.
[LICENSING.md](LICENSING.md) lists where every part of the code comes from; the main credits are:

- **X0X** by Charles Vestal, <https://github.com/charlesvestal/fm1-x0x>: FM1 Tonic grew from this
  project.
- **Felucca** by Leo Kuroshita (Hügelton Instruments), <https://github.com/hugelton/Felucca>, which X0X
  is a fork of: the hardware layer, USB, the update loader, the web installer and the storage.
- **libpo32** by Eric Lewis (MIT licence): the starting point of the FM1 Tonic drum synthesis code.
  Nothing is sampled.
- **Melodee** (keremimo) and **Jangada** (zednaked): what they found about the FM-1's second core.
- Fonts: Barlow Semi Condensed and Terminus (SIL OFL 1.1).

Felucca's own assets that are not under its GPL (the icon set, the panel photo and the Hügelton drum
pack) are not part of this project.

Microtonic is a trademark of Sonic Charge. M-VAVE and FM-1 are trademarks of their owners.
FM1 Tonic is not affiliated with or endorsed by M-VAVE, Sonic Charge or Hügelton Instruments.
