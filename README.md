# PCExhumed
A port of the PC version of Exhumed based on EDuke32

## Installing
1. Extract PCExhumed to a new directory
2. Copy the following files from the PC retail version of Exhumed or PowerSlave (Exhumed preferred). Beta, demo or pre-release versions not supported.

   STUFF.DAT
   DEMO.VCR
   BOOK.MOV
   
   The game is unfortunately not currently available for sale. Please provide files from an original release of the game.

3. Recommended (but optional) - Add the games CD audio tracks as OGG files in the format exhumedXX.ogg (where XX is the track number) to the same folder as 
   pcexhumed.exe. The game includes tracks 02 to 19.
   These will provide the game with it's awesome music soundtrack and add storyline narration by the King Ramses NPC.

4. Launch PCExhumed.

## Notes
Demo playback is not yet working.

## Adjusting settings
We are currently working on fancy new menus for the game. In the meantime, you can edit the settings.cfg file in the game directory that's created on first run.

To invert the mouse, add the line 'in_mouseflip 0' to settings.cfg.
To change the FOV, add a new line to settings.cfg, e.g. 'fov "120"' where 120 is the desired FOV value between 60 and 140.

## Building PCExhumed
See: https://wiki.eduke32.com/wiki/Main_Page

## Acknowledgments:
  See AUTHORS.md