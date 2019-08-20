# NBlood
Blood port based on EDuke32

## Installing
1. Extract NBlood to a new directory
2. Copy the following files from Blood 1.21 to NBlood folder:

   BLOOD.INI
   BLOOD.RFF
   BLOOD000.DEM-BLOOD003.DEM (optional)
   CP01.MAP-CP09.MAP (optional, Cryptic Passage)
   CPART07.AR_ (optional, Cryptic Passage)
   CPART15.AR_ (optional, Cryptic Passage)
   CPBB01.MAP-CPBB04.MAP (optional, Cryptic Passage)
   CPSL.MAP (optional, Cryptic Passage)
   CRYPTIC.INI (optional, Cryptic Passage)
   CRYPTIC.SMK (optional, Cryptic Passage)
   CRYPTIC.WAV (optional, Cryptic Passage)
   GUI.RFF
   SOUNDS.RFF
   SURFACE.DAT
   TILES000.ART-TILES017.ART
   VOXEL.DAT

3. Optionally, if you want to use CD audio tracks instead of MIDI, provide FLAC/OGG recordings in following format: bloodXX.flac/ogg, where XX is track number. Make sure to enable Redbook audio option in sound menu.
4. Optionally, if you want cutscenes and you have the original CD, copy the `movie` folder into NBlood's folder (the folder itself too). If you have the GOG version of the game, rename `game.gog` to `game.bin` and `game.inst` to `game.cue` and mount the `cue` as a virtual CD (for example with `WinCDEmu`), there you will have the `movie` folder.
5. Launch NBlood (on Linux, to play Cryptic Passage, launch with the `-ini CRYPTIC.INI` parameter)

## Building NBlood
See: https://wiki.eduke32.com/wiki/Main_Page

## Acknowledgments:
### EDuke32 engine & game programming:
  * TerminX
  * Hendricks266
  * pogokeen
  * Plagman
  * Helixhorned
  
### JFDuke3D by:
  * JonoF
  
### Uses BUILD Engine technology by:
  * Ken Silverman
  
### NBlood programming:
  * Nuke.YKT
  
### Additional programming:
  * sirlemonhead
  * NoOne
  
### Widescreen tiles & NBlood logo:
  * Maxi Clouds
  
### Special thanks:
  * NY00123, MetHy, Striker, oasiz, Mblackwell, Zombie, Marphy Black, SAmik37, meleemario and contributors
