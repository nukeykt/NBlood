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

#include "SDL_theora.h"
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

static int initialized = 0;

const SDL_version *THR_Linked_Version(void)
{
	static SDL_version linked_version;
	SDL_THEORA_VERSION(&linked_version);
	return(&linked_version);
}

extern int THR_InitVideo();
extern void THR_QuitVideo();
extern int THR_InitAudio();
extern void THR_QuitAudio();

int THR_Init(int flags)
{
	int result = 0;

	/* Passing 0 returns the currently initialized loaders */
	if(!flags)
	{
		return initialized;
	}

	if(flags & THR_INIT_VIDEO)
	{
		if((initialized & THR_INIT_VIDEO) || THR_InitVideo() == 0)
		{
			result |= THR_INIT_VIDEO;
		}
	}
	if(flags & THR_INIT_AUDIO)
	{
		if((initialized & THR_INIT_AUDIO) || THR_InitAudio() == 0)
		{
			result |= THR_INIT_AUDIO;
		}
	}
	
	initialized |= result;

	return result;
}

void THR_Quit()
{
	if(initialized & THR_INIT_VIDEO)
	{
		THR_QuitVideo();
	}
	if(initialized & THR_INIT_AUDIO)
	{
		THR_QuitAudio();
	}

	initialized = 0;
}