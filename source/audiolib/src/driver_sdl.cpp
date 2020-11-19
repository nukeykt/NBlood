/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 Copyright (C) EDuke32 developers and contributors

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

/**
 * libSDL output driver for MultiVoc
 */

#include "driver_sdl.h"

#include "compat.h"
#include "multivoc.h"
#include "mutex.h"
#include "sdl_inc.h"
#include "vfs.h"

#ifdef __ANDROID__
#include "duke3d.h"
#include "android.h"
#endif

enum
{
    SDLErr_Error   = -1,
    SDLErr_Ok      = 0,
    SDLErr_Uninitialised,
    SDLErr_InitSubSystem,
    SDLErr_OpenAudio
};

static int ErrorCode = SDLErr_Ok;
static int Initialised;
static int Playing;
static uint32_t StartedSDL;

static char *MixBuffer;
static int MixBufferSize;
static int MixBufferCount;
static int MixBufferCurrent;
static int MixBufferUsed;
static void (*MixCallBack)(void);

#if (SDL_MAJOR_VERSION >= 2)
static SDL_AudioDeviceID audio_dev;
#endif

#if SDL_MAJOR_VERSION >= 2
char SDLAudioDriverName[16];
char *SDLAudioDriverNameEnv;
#endif

static void fillData(void * userdata, Uint8 * ptr, int remaining)
{
    UNREFERENCED_PARAMETER(userdata);

    if (!MixBuffer || !MixCallBack)
      return;

    while (remaining > 0)
    {
        if (MixBufferUsed == MixBufferSize)
        {
            MixCallBack();
            MixBufferUsed = 0;

            if (++MixBufferCurrent >= MixBufferCount)
                MixBufferCurrent -= MixBufferCount;
        }

        while (remaining > 0 && MixBufferUsed < MixBufferSize)
        {
            auto sptr = MixBuffer + (MixBufferCurrent * MixBufferSize) + MixBufferUsed;
            int  len  = MixBufferSize - MixBufferUsed;

            if (remaining < len)
                len = remaining;

            memcpy(ptr, sptr, len);

            ptr += len;
            MixBufferUsed += len;
            remaining -= len;
        }
    }
}

int SDLDrv_GetError(void) { return ErrorCode; }

const char *SDLDrv_ErrorString(int ErrorNumber)
{
    switch (ErrorNumber)
    {
        case SDLErr_Error:         return SDLDrv_ErrorString(ErrorCode);
        case SDLErr_Ok:            return "SDL Audio ok.";
        case SDLErr_Uninitialised: return "SDL Audio uninitialized.";
        case SDLErr_InitSubSystem: return "SDL Audio: error in Init or InitSubSystem.";
        case SDLErr_OpenAudio:     return "SDL Audio: error in OpenAudio.";
        default:                   return "Unknown SDL Audio error code.";
    }
}

#if SDL_MAJOR_VERSION >= 2
void SDLDrv_PCM_PrintDrivers(void)
{
    MV_Printf("Available audio drivers: ");

    for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
        MV_Printf("%s ", SDL_GetAudioDriver(i));

    MV_Printf("\n");
}

int SDLDrv_PCM_CheckDriverName(char const *dev)
{
    for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
        if (!Bstrcasecmp(dev, SDL_GetAudioDriver(i)))
            return true;

    return false;
}

char const *SDLDrv_PCM_GetDriverName(void) { return SDL_GetCurrentAudioDriver(); }
static void SDLDrv_Cleanup(void) { DO_FREE_AND_NULL(SDLAudioDriverNameEnv); }
#endif

int SDLDrv_PCM_Init(int *mixrate, int *numchannels, void * initdata)
{
    UNREFERENCED_PARAMETER(initdata);

    Uint32 inited;
    int err = 0;

    if (Initialised)
        SDLDrv_PCM_Shutdown();
#if SDL_MAJOR_VERSION >= 2
    else if (SDLAudioDriverNameEnv == nullptr)
    {
        static int done;

        if (!done)
        {
            if (auto s = SDL_getenv("SDL_AUDIODRIVER"))
                SDLAudioDriverNameEnv = Xstrdup(s);

            atexit(SDLDrv_Cleanup);
            done = true;
        }
    }

    if (SDLAudioDriverName[0])
        SDL_setenv("SDL_AUDIODRIVER", SDLAudioDriverName, true);
#endif

    inited = SDL_WasInit(SDL_INIT_AUDIO);

    if (!(inited & SDL_INIT_AUDIO))
    {
        err        = SDL_InitSubSystem(SDL_INIT_AUDIO);
        StartedSDL = SDL_WasInit(SDL_INIT_AUDIO);
    }

    if (err < 0)
    {
        ErrorCode = SDLErr_InitSubSystem;
        return SDLErr_Error;
    }

    int chunksize = 512;
#ifdef __ANDROID__
    chunksize = droidinfo.audio_buffer_size;
#endif

    SDL_AudioSpec spec = {};

    spec.freq = *mixrate;
    spec.format = AUDIO_S16SYS;
    spec.channels = *numchannels;
    spec.samples = chunksize;
    spec.callback = fillData;
    spec.userdata = nullptr;

    SDL_AudioSpec actual = {};

#if (SDL_MAJOR_VERSION >= 2)
    audio_dev = err = SDL_OpenAudioDevice(nullptr, 0, &spec, &actual, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
#else
    err = !SDL_OpenAudio(&spec, &actual);
#endif

#if SDL_MAJOR_VERSION >= 2
    // restore original value
    if (SDLAudioDriverNameEnv)
        SDL_setenv("SDL_AUDIODRIVER", SDLAudioDriverNameEnv, true);
#endif

    if (err == 0)
    {
        ErrorCode = SDLErr_OpenAudio;
        return SDLErr_Error;
    }

#if (SDL_MAJOR_VERSION >= 2)
    char *drivername = Xstrdup(SDL_GetCurrentAudioDriver());

    for (int i=0;drivername[i] != 0;++i)
        drivername[i] = toupperlookup[drivername[i]];

    auto devname = Xstrdup(SDL_GetAudioDeviceName(0, 0));
    auto pdevname = Bstrchr(devname, '(');

    if (pdevname)
    {
        auto rt = Bstrchr(pdevname++, ')');
        if (rt != nullptr) *rt = '\0';
    }
    else
        pdevname = devname;

    MV_Printf("SDL %s driver on %s", drivername, pdevname);

    Xfree(devname);
    Xfree(drivername);
#else
    char drivernamestr[64] = "(error)";
    SDL_AudioDriverName(drivernamestr, sizeof(drivernamestr));
    MV_Printf("SDL %s driver", drivernamestr);

    if (actual.freq == 0 || actual.channels == 0)
    {
        // hack for when SDL said it opened the audio, but clearly didn't
        SDL_CloseAudio();
        ErrorCode = SDLErr_OpenAudio;
        return SDLErr_Error;
    }
#endif
    err = 0;

    *mixrate = actual.freq;
    if (actual.format == AUDIO_U8 || actual.format == AUDIO_S8)
    {
        ErrorCode = SDLErr_OpenAudio;
        err = 1;
    }

    *numchannels = actual.channels;
    if (actual.channels != 1 && actual.channels != 2)
    {
        ErrorCode = SDLErr_OpenAudio;
        err = 1;
    }

    if (err)
    {
        SDL_CloseAudio();
        return SDLErr_Error;
    }

    Initialised = 1;
    return SDLErr_Ok;
}

void SDLDrv_PCM_Shutdown(void)
{
    if (!Initialised)
        return;

    if (StartedSDL)
        SDL_QuitSubSystem(StartedSDL);

    StartedSDL = 0;
    Initialised = 0;
}

int SDLDrv_PCM_BeginPlayback(char *BufferStart, int BufferSize,
                        int NumDivisions, void ( *CallBackFunc )( void ) )
{
    if (!Initialised)
    {
        ErrorCode = SDLErr_Uninitialised;
        return SDLErr_Error;
    }

    if (Playing)
        SDLDrv_PCM_StopPlayback();

    MixBuffer = BufferStart;
    MixBufferSize = BufferSize;
    MixBufferCount = NumDivisions;
    MixBufferCurrent = 0;
    MixBufferUsed = 0;
    MixCallBack = CallBackFunc;

    // prime the buffer
    MixCallBack();

#if (SDL_MAJOR_VERSION >= 2)
    SDL_PauseAudioDevice(audio_dev, 0);
#else
    SDL_PauseAudio(0);
#endif
    Playing = 1;

    return SDLErr_Ok;
}

void SDLDrv_PCM_StopPlayback(void)
{
    if (!Initialised || !Playing)
        return;

#if (SDL_MAJOR_VERSION >= 2)
    SDL_PauseAudioDevice(audio_dev, 1);
#else
    SDL_PauseAudio(1);
#endif

    Playing = 0;
}

void SDLDrv_PCM_Lock(void)
{
#if (SDL_MAJOR_VERSION >= 2)
    SDL_LockAudioDevice(audio_dev);
#else
    SDL_LockAudio();
#endif
}

void SDLDrv_PCM_Unlock(void)
{
#if (SDL_MAJOR_VERSION >= 2)
    SDL_UnlockAudioDevice(audio_dev);
#else
    SDL_UnlockAudio();
#endif
}
