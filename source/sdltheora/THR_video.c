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
#include "theoraplay.h"

#include <stdlib.h>
#include <string.h>

static int loaded = 0;

typedef struct AudioQueue
{
	const THEORAPLAY_AudioPacket *audio;
	int offset;
	struct AudioQueue *next;
} AudioQueue;

static volatile AudioQueue *audio_queue = NULL;
static volatile AudioQueue *audio_queue_tail = NULL;

typedef struct
{
	THEORAPLAY_Decoder* decoder;
	const THEORAPLAY_AudioPacket* audio;
	const THEORAPLAY_VideoFrame* video;
	SDL_Texture* texture;
	SDL_AudioSpec audio_spec;
	Uint32 baseticks;
	Uint32 framems;
} THR_video;

static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len)
{
	// !!! FIXME: this should refuse to play if item->playms is in the future.
	//const Uint32 now = SDL_GetTicks() - baseticks;
	Sint16 *dst = (Sint16 *)stream;

	while(audio_queue && (len > 0))
	{
		volatile AudioQueue *item = audio_queue;
		AudioQueue *next = item->next;
		const int channels = item->audio->channels;

		const float *src = item->audio->samples + (item->offset * channels);
		int cpy = (item->audio->frames - item->offset) * channels;
		int i;

		if(cpy > (len / sizeof(Sint16)))
			cpy = len / sizeof(Sint16);

		for(i = 0; i < cpy; i++)
		{
			const float val = *(src++);
			if(val < -1.0f)
				*(dst++) = -32768;
			else if(val > 1.0f)
				*(dst++) = 32767;
			else
				*(dst++) = (Sint16)(val * 32767.0f);
		} // for

		item->offset += (cpy / channels);
		len -= cpy * sizeof(Sint16);

		if(item->offset >= item->audio->frames)
		{
			THEORAPLAY_freeAudio(item->audio);
			free((void *)item);
			audio_queue = next;
		} // if
	} // while

	if(!audio_queue)
		audio_queue_tail = NULL;

	if(len > 0)
		memset(dst, '\0', len);
} // audio_callback

static void queue_audio(const THEORAPLAY_AudioPacket *audio)
{
	AudioQueue *item = (AudioQueue *)malloc(sizeof(AudioQueue));
	if(!item)
	{
		THEORAPLAY_freeAudio(audio);
		return;  // oh well.
	} // if

	item->audio = audio;
	item->offset = 0;
	item->next = NULL;

	SDL_LockAudio();
	if(audio_queue_tail)
		audio_queue_tail->next = item;
	else
		audio_queue = item;
	audio_queue_tail = item;
	SDL_UnlockAudio();
} // queue_audio

int THR_InitVideo()
{
	if(loaded == 0)
	{
		/*
		lib.handle = SDL_LoadObject(LOAD_JPG_DYNAMIC);
		if ( lib.handle == NULL ) {
			return -1;
		}
		*/
	}
	++loaded;

	return 0;
}

void THR_QuitVideo()
{
	if(loaded == 0)
	{
		return;
	}

	if(loaded == 1)
	{
		//SDL_UnloadObject(lib.handle);
	}

	--loaded;
}

intptr_t THR_Load(const char *file, SDL_Renderer* renderer)
{
	const THEORAPLAY_AudioPacket* audio = NULL;
	const THEORAPLAY_VideoFrame* video = NULL;

	THEORAPLAY_Decoder* decoder = THEORAPLAY_startDecodeFile(file, 30, THEORAPLAY_VIDFMT_IYUV);
	if(!decoder)
	{
		THR_SetError("Failed to start decoding file");
		return 0;
	}

	// Wait for decompressing the first frame
	while(!audio || !video)
	{
		if(!audio) audio = THEORAPLAY_getAudio(decoder);
		if(!video) video = THEORAPLAY_getVideo(decoder);
		SDL_Delay(10);
	}

	SDL_Texture* overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, video->width, video->height);
	if(overlay == 0)
	{
		// Error set by SDL_CreateTexture()
		THEORAPLAY_freeAudio(audio);
		THEORAPLAY_freeVideo(video);
		THEORAPLAY_stopDecode(decoder);
		return 0;
	}

	// Create video item
	THR_video* thr_video = (THR_video*)malloc(sizeof(THR_video));
	thr_video->audio = audio;
	thr_video->video = video;
	thr_video->decoder = decoder;
	thr_video->texture = overlay;
	thr_video->baseticks = SDL_GetTicks();
	thr_video->framems = (video->fps == 0.0) ? 0 : ((Uint32)(1000.0 / video->fps));

	// Load audio specs
	memset(&thr_video->audio_spec, '\0', sizeof(SDL_AudioSpec));
	thr_video->audio_spec.freq = audio->freq;
	thr_video->audio_spec.format = AUDIO_S16SYS;
	thr_video->audio_spec.channels = audio->channels;
	thr_video->audio_spec.samples = 2048;
	thr_video->audio_spec.callback = audio_callback;
	int openAudioResults = SDL_OpenAudio(&thr_video->audio_spec, NULL);
	
	if(openAudioResults != 0)
	{
		const char* e = SDL_GetError();
		// Error set by SDL_OpenAudio()
		THEORAPLAY_freeAudio(audio);
		THEORAPLAY_freeVideo(video);
		THEORAPLAY_stopDecode(decoder);
		return 0;
	}

	while(audio)
	{
		queue_audio(audio);
		audio = THEORAPLAY_getAudio(decoder);
	}

	SDL_PauseAudio(0);

	return  thr_video;
}

SDL_Texture* THR_UpdateVideo(THR_video* videoID)
{
	if(videoID != NULL && (videoID->decoder != NULL && THEORAPLAY_isDecoding(videoID->decoder)))
	{
		// load next frame if we have to (if we loaded a frame last time video should be null)
		if(!videoID->video)
			videoID->video = THEORAPLAY_getVideo(videoID->decoder);

		const Uint32 now = SDL_GetTicks() - videoID->baseticks;

		// Play video frames when it's time.
		if(videoID->video && (videoID->video->playms <= now))
		{
			if(videoID->framems && ((now - videoID->video->playms) >= videoID->framems))
			{
				// Skip frames to catch up, but keep track of the last one
				//  in case we catch up to a series of dupe frames, which
				//  means we'd have to draw that final frame and then wait for
				//  more.
				const THEORAPLAY_VideoFrame *last = videoID->video;
				while((videoID->video = THEORAPLAY_getVideo(videoID->decoder)) != NULL)
				{
					THEORAPLAY_freeVideo(last);
					last = videoID->video;
					if((now - videoID->video->playms) < videoID->framems)
						break;
				}

				if(!videoID->video)
					videoID->video = last;
			}

			if(!videoID->video)  // do nothing; we're far behind and out of options.
			{
				THR_SetError("WARNING: Playback can't keep up!");
				return NULL;
			}
			else
			{
				// http://www.fourcc.org/yuv.php#YV12
				int half_w = videoID->video->width / 2;

				const Uint8 *y = (const Uint8 *)videoID->video->pixels;
				const Uint8 *u = y + (videoID->video->width * videoID->video->height);
				const Uint8 *v = u + (half_w * (videoID->video->height / 2));

				SDL_UpdateYUVTexture(videoID->texture, NULL, y, videoID->video->width, u, half_w, v, half_w);
			}
			THEORAPLAY_freeVideo(videoID->video);
			videoID->video = NULL;
		}
	}

	while(videoID!= NULL && (videoID->audio = THEORAPLAY_getAudio(videoID->decoder)) != NULL)
		queue_audio(videoID->audio);

	return (videoID != NULL? videoID->texture : 0);
}

void THR_DestroyVideo(THR_video* video, SDL_Window* win)
{
	while (audio_queue != NULL)
	{
		SDL_LockAudio();
		SDL_UnlockAudio();
		SDL_Delay(10);
	}
	SDL_CloseAudio();
	if (video != NULL)
	{
		SDL_DestroyTexture(video->texture);
		THEORAPLAY_freeVideo(video->video);
		THEORAPLAY_freeAudio(video->audio);
		THEORAPLAY_stopDecode(video->decoder);
		// Close and destroy the window
		SDL_DestroyWindow(win);
	}
	else
		SDL_DestroyWindow(win);
}

int THR_IsPlaying(THR_video* video)
{
	return (video != NULL? THEORAPLAY_isDecoding((video)->decoder): 0);
}