/*
The MIT License (MIT)

SDL_theora:  An example theora video loading library for use with SDL
Copyright (C) 2016 Ricardo Pillosu <ricard.pillosu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* A simple library to load and play theora video as SDL surfaces */
#ifndef _SDL_THEORA_H
#define _SDL_THEORA_H
#include "SDL2/SDL.h"
#include "SDL2/SDL_version.h"
#include "SDL2/begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
*/
#define SDL_THEORA_MAJOR_VERSION 0
#define SDL_THEORA_MINOR_VERSION 5
#define SDL_THEORA_PATCHLEVEL    0

/* This macro can be used to fill a version structure with the compile-time
* version of the SDL_theora library.
*/
#define SDL_THEORA_VERSION(X)                        \
{                                                   \
    (X)->major = SDL_THEORA_MAJOR_VERSION;           \
    (X)->minor = SDL_THEORA_MINOR_VERSION;           \
    (X)->patch = SDL_THEORA_PATCHLEVEL;              \
}

/* We'll use SDL for reporting errors */
#define THR_SetError    SDL_SetError
#define THR_GetError    SDL_GetError

typedef enum
{
	THR_INIT_VIDEO = 0x00000001,
	THR_INIT_AUDIO = 0x00000002,
} THR_InitFlags;

/* Loads dynamic libraries and prepares them for use.  Flags should be
one or more flags from THR_InitFlags OR'd together.
It returns the flags successfully initialized, or 0 on failure.
*/
extern DECLSPEC int SDLCALL THR_Init(int flags);

/* Unloads libraries loaded with IMG_Init */
extern DECLSPEC void SDLCALL THR_Quit(void);

/* Loads a video file TODO: support RWops*/
extern DECLSPEC intptr_t SDLCALL THR_Load(const char *file, SDL_Renderer* renderer);
extern DECLSPEC SDL_Texture* SDLCALL THR_UpdateVideo(intptr_t video);
extern DECLSPEC int SDLCALL THR_IsPlaying(intptr_t video);
extern DECLSPEC void SDLCALL THR_DestroyVideo(intptr_t video, SDL_Window* win);

/* We'll use SDL for reporting errors */
#define THR_SetError    SDL_SetError
#define THR_GetError    SDL_GetError

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#include "SDL2/close_code.h"

#endif // __SDL_THEORA_H__