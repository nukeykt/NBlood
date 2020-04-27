
SDL_theora 1.0

The latest version of this library is available from:
https://github.com/d0n3val/SDL_theora

This is a simple library to load an play theora encoded video as SDL surfaces.

API:
#include "SDL_theora.h"

---
Basic use example:

int my_video = THR_Load("data/intro.ogv");

SDL_Texture* video_texture = THR_Update(my_video);
// Use SDL_RenderCopy to blit the texture normally

if(THR_IsPlaying(my_video) == 0)
	THR_DestroyVideo(my_video);

---
This library is under the MIT License, see the file "COPYING.txt" for details.
