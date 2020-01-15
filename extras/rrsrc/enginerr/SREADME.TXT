// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

All of my Build Engine code is designed specifically for the Watcom C/C++ 11.0
compiler. You're on your own if you attempt to use any other compiler.

To best view my source, set your "Tab Stops" to 3!

---------------  Engine files required to compile a Build game  ---------------

ENGINE.C: All the major C code is in here
   Graphics functions:
      Function to draw everything in the 3D view
      Function to draw the 2D texturized map view
      Function to draw status bar pieces
      Function to change screen size (+, -, Field of view (shrunk mode))
      Line drawing for both 2D and 3D modes
      Simple 8*8 and 3*5 font routines (mostly used in BUILD editor)
      Screen capture PCX code
      Functions to load palette
      Functions to set video modes
      Function to flip screen pages
      Function to generate palette lookup tables
      Function to set gamma correction
      Functions to help with mirrors (they weren't fully built in to the engine)
      Stereoscopic modes (red blue glasses, Crystal Eyes, Nuvision)
   Movement functions:
      Function to detect if 2 points can see each other
      Function to find where an instant weapon (pistol, shotgun, chaingun) hits
      Function to determine if you're near a tagged object
      Collision detection for all sprites (with smooth sliding)
      Collision detection to keep you away from moving objects
   Boring functions:
      Functions to load/save MAP files
      Function to load ART file
      Disk caching system
      Functions to manipulate sprite linked lists
   Helper functions:
      A couple of line intersection functions
      A couple of math routines like faster atan2, sqrt, 2D rotation
      Functions to help with animation of certain doors & tiles
      Function to move vertices connected to many walls
      A function that returns what sector something is in
      Some Build editor functions also used in games

CACHE1D.C:
   Contains a unique LRU-style caching system
   Group file code to handle all file loading transparently
   Compression & Decompression routines used in .DMO files

A.ASM: (pure assembly routines)
   8 routines for horizontal texture mapping, special cases for
        masked mapping, translucent masked mapping, and # pixels at a time
   11 routines for vertical texture mapping, special cases for
        masked mapping, translucent masked mapping, # pixels at a time,
        slopes, voxels

   NOTE: This assembly code is designed for WASM (the assembler that ships
      with Watcom C/C++ 11.0) In order to make it work under MASM, you will
      have to make some simple modifications:

      1. At the top of the file, change ".586P" to ".386P"

      2. If the assembler complains that a jump is out of range, go to the
         line of the error and remove the word "short" from the line. For
         example, change: "jnc short mylabel" to just: "jnc mylabel"

      3. If you get an error message like: "Symbol not defined: XE9", you
         need to convert the value to a hex format that the assembler
         understands. For example, change "mov al, 0xe9" to: "mov al, 0e9h"
         (You need an extra 0 in front if the left-most digit is a letter)

BUILD.H:
   Sector, Wall, Sprite structures, defines and other documented shared
   variables.  While I designed the format of the structures, the "game"
   developers were very involved with what went into them.

PRAGMAS.H:
   Tons of Watcom in-line assembly pragmas, such as 64-bit multiply and
   divide routines, memory clear & move, and a lot of cute ones for doing
   math, etc.

VES2.H:
   VESA 2.0 routines used by the Build engine. This code unfortunately is
   somewhat integrated with ENGINE.C

MMULTI.C:
   Contains mid-level network routines.  This file arbitrated between
   COMMIT and the game code.  I had to write error correction in this
   module because COMMIT didn't have it.

   NOTE: By default, my Build test uses MULTI.C instead of MMULTI.C. My game
      is the only Build game that does this; all commercial Build games use
      MMULTI.C.

   You can use the COMMIT driver with my test game. You will probably find
   that there are more problems in general - this is not because MMULTI.C
   or COMMIT are bad; it's because I always used MULTI.C by default with
   my Build game. (I used my 2DRAW engine to test the MMULTI.C code.)
   Follow these instructions exactly if you want to use MMULTI.C:

      1. In the makefile, replace all occurrences of "multi" with "mmulti"

      2. In GAME.C, on line 448, Remove the comment in front of the call to
         waitforeverybody(). This needs to be done because my MMULTI.C code
         doesn't have a "sendlogon" function. You will get severe lag if you
         forget to do this.

      3. To run the game, you must first setup BOTH setup programs (on ALL
         computers)

         A) First edit the parameters inside COMMIT.DAT to match your desired
            multiplayer configuration.

         B) Then go into my Build setup program. Under the communications
            menu, set the NUMBER OF PLAYERS. For a COMMIT game, the specific
            ports, etc. have no effect and don't really matter. Press ENTER
            a few times to get through the rest of the menus. I know this
            seems silly, but at the time, it seemed like the simplest way to
            get my GAME.C code to work with both MULTI.C and MMULTI.C.

--------  Additional source required to compile Ken's Build test game  --------

GAME.C:
   Sample code for a complete game.  Well it's not really complete unless
      you loved Ken's Labyrinth.  This code wasn't actually used in the
      distributed games, but it was an important code base for the developers
      to look at when they had questions.  Some of their routines were lifted
      straight from this code.

NAMES.H: EDITART updates this file every time you type in a name. This .H
   file is just a convenient way to access textures inside the game code.

MULTI.C: Before I added support for COMMIT, I had my own COM/network routines.
   This file is almost identical to MMULTI.C.  You will have to change a few
   minor things like function parameters if you want to update my game to use
   MMULTI.C & COMMIT instead.

KDMENG.C: KDM (Ken's Digital Music) sound playback engine, C code
K.ASM: KDM (Ken's Digital Music) sound playback engine, Assembly code

-----------  Additional source required to compile the Build editor  ----------

BUILD.C:
   Combine this code with the engine and you get the Build editor. You will
   find all of the editing functions from the Build editor in here (2D and 3D)

BSTUB.C:
   Game teams were able to customize certain things in the Build editor with
   this module.

---------------------------------  Make file  ---------------------------------

   You can use my makefile if you wish - it shows you my compile options:
MAKEFILE.: A very simple make file.
   Type: "wmake" or "wmake game.exe" to compile my build test game
   Type: "wmake build.exe" to compile my build editor

-------------------------------  Documentation  -------------------------------

   This is the documentation I wrote for the game programmers. Please keep in
   mind that I never gave out the source to my engine (ENGINE.C, CACHE1D.C,
   VES2.H, A.ASM) I gave them only OBJ's and my GAME.C code to play with.
   These text files mainly tell how to USE my engine, not how to write one.

BUILDINF.TXT: This text file has the most up-to-date function descriptions.
   You should look in here first before searching through the old stuff in
   BUILD.TXT and BUILD2.TXT.

BUILD.TXT: This is my original documentation that I sent along with the
   all my library updates to my Build engine. A lot of the information in
   here is redundant with BUILDINF.TXT, but I decided to include it anyway
   because the history section has a lot of interesting things in it that
   reveal how and why I designed things the way I did.

   PLEASE NOTE: The documentation at the top of BUILD.TXT is NOT up to date.
   Many of the function parameters are missing or out of order. Use
   BUILDINF.TXT (or my website :) for the latest function descriptions.

BUILD2.TXT: When BUILD.TXT didn't fit in the memory of my old text editor,
   I started this new file.

-------------------------------------------------------------------------------
-Ken S. (web page: http://www.advsys.net/ken)
