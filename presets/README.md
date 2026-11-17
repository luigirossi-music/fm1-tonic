# FM1 TONIC factory presets
Up to 8 `.tnp` files (packed with `tools/tnp_pack`, see `firmware/src/app/tnp.h`), in name order: `01_name.tnp` is factory preset 1,
and also the project a new or reset FM1 TONIC starts with. `tools/build.py` packs this folder into the firmware.
Each preset keeps patterns 1-8; patterns 9-32 are empty (a blank canvas) but keep the preset's sound.
Pack one:  tnp_pack "Name.mtpreset" presets/01_Name.tnp   (build line in tools/tnp_pack.c)

The 8 factory presets (wip96), shown on the PRESETS knob as 01-08; places 09-40 are the user slots and show "EMPTY" until you SAVE into them:
01 Marchiav (the Rostoma preset, renamed), 02 Omm e Night, 03 Stai bell, 04 We Masanie, 05 Tiemp e Uerr, 06 Waie e not, 07 Robba bell, 08 Piccerill po po po.
Names are set on the command line: tnp_pack "in.mtpreset" presets/02_Omm_e_Night.tnp "Omm e Night" (the 3rd argument replaces the name inside the preset).
