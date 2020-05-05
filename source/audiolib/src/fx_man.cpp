//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#include "fx_man.h"

#include "_multivc.h"
#include "compat.h"
#include "drivers.h"
#include "driver_adlib.h"
#include "driver_sf2.h"
#include "midi.h"
#include "multivoc.h"
#include "music.h"
#include "osd.h"

#ifdef _WIN32
# include "driver_winmm.h"
# include <mmsystem.h>
#endif

#ifdef RENDERTYPESDL
# include "driver_sdl.h"
#endif

int FX_ErrorCode = FX_Ok;
int FX_Installed;
int FX_MixRate;

const char *FX_ErrorString(int const ErrorNumber)
{
    switch (ErrorNumber)
    {
        case FX_Warning:
        case FX_Error:          return FX_ErrorString(FX_ErrorCode);
        case FX_Ok:             return "Fx ok.";
        case FX_MultiVocError:  return MV_ErrorString(MV_Error);
        default:                return "Unknown Fx error code.";
    }
}

static int osdcmd_cvar_set_audiolib(osdcmdptr_t parm)
{
    int32_t r = osdcmd_cvar_set(parm);

    if (parm->numparms == 0)
    {
#ifdef _WIN32
        if (!Bstrcasecmp(parm->name, "mus_winmm_device"))
            WinMMDrv_MIDI_PrintDevices();
#endif
#if defined RENDERTYPESDL && SDL_MAJOR_VERSION >= 2
        if (!Bstrcasecmp(parm->name, "snd_sdl_audiodriver"))
            SDLDrv_PCM_PrintDrivers();
#endif
    }

    if (r != OSDCMD_OK || parm->numparms < 1) return r;

    if (!Bstrcasecmp(parm->name, "mus_emidicard"))
        MIDI_Restart();
    else if (!Bstrcasecmp(parm->name, "mus_al_stereo"))
        AL_SetStereo(AL_Stereo);
#ifdef HAVE_XMP
    else if (!Bstrcasecmp(parm->name, "mus_xmp_interpolation"))
        MV_SetXMPInterpolation();
#endif
#if defined RENDERTYPESDL && SDL_MAJOR_VERSION >= 2
    else if (!Bstrcasecmp(parm->name, "snd_sdl_audiodriver"))
    {
        if (!FX_Installed || !Bstrcasecmp(parm->parms[0], SDLDrv_PCM_GetDriverName()))
            return r;

        if (!SDLDrv_PCM_CheckDriverName(parm->parms[0]))
        {
            SDLDrv_PCM_PrintDrivers();
            return r;
        }

        FX_Init(MV_MaxVoices, MV_Channels, MV_MixRate, MV_InitDataPtr);
    }
#endif

    return r;
}

void FX_InitCvars(void)
{
    static osdcvardata_t cvars_audiolib [] ={
        { "mus_emidicard", "force a specific EMIDI instrument set", (void*) &ASS_EMIDICard, CVAR_INT | CVAR_FUNCPTR, -1, 10 },
        { "mus_al_additivemode", "enable/disable alternate additive AdLib timbre mode", (void*) &AL_AdditiveMode, CVAR_BOOL, 0, 1 },
        { "mus_al_postamp", "controls post-synthesization OPL3 volume amplification", (void*) &AL_PostAmp, CVAR_INT, 0, 3 },
        { "mus_al_stereo", "enable/disable OPL3 stereo mode", (void*) &AL_Stereo, CVAR_BOOL | CVAR_FUNCPTR, 0, 1 },
        { "mus_sf2_bank", "SoundFont 2 (.sf2) bank filename",  (void*) SF2_BankFile, CVAR_STRING, 0, sizeof(SF2_BankFile) - 1 },
        { "mus_sf2_sampleblocksize", "number of samples per effect processing block", (void*) &SF2_EffectSampleBlockSize, CVAR_INT, 1, 64 },
#ifdef _WIN32
        { "mus_mme_device", "select Windows MME MIDI output device", (void*) &WinMM_DeviceID, CVAR_INT | CVAR_FUNCPTR, -1, WinMMDrv_MIDI_GetNumDevices()-1 },
#endif
#if defined HAVE_XMP && 0
        { "mus_xmp_interpolation", "XMP output interpolation: 0: none  1: linear  2: spline", (void*) &MV_XMPInterpolation, CVAR_INT | CVAR_FUNCPTR, 0, 2 },
#endif
#if defined RENDERTYPESDL && SDL_MAJOR_VERSION >= 2
        { "snd_sdl_audiodriver", "select SDL audio driver (platform-specific)",
          (void *)SDLAudioDriverName, CVAR_STRING | CVAR_FUNCPTR, 0, sizeof(SDLAudioDriverName) - 1 },
#endif
    };

    for (auto& i : cvars_audiolib)
        OSD_RegisterCvar(&i, (i.flags & CVAR_FUNCPTR) ? osdcmd_cvar_set_audiolib : osdcmd_cvar_set);
}

int FX_Init(int numvoices, int numchannels, int mixrate, void* initdata)
{
    if (FX_Installed)
        FX_Shutdown();
   
#if defined RENDERTYPESDL
    int SoundCard = ASS_SDL;
#elif defined RENDERTYPEWIN
    int SoundCard = ASS_DirectSound;
#endif

    MV_Printf("Initializing sound: ");

    if ((unsigned)SoundCard >= ASS_NumSoundCards)
    {
        FX_SetErrorCode(FX_InvalidCard);
        MV_Printf("failed! %s\n", FX_ErrorString(FX_InvalidCard));
        return FX_Error;
    }

    if (SoundDriver_IsPCMSupported(SoundCard) == 0)
    {
        // unsupported cards fall back to no sound
        FX_SetErrorCode(FX_InvalidCard);
        return FX_Error;
    }

    int status = FX_Ok;

    if (MV_Init(SoundCard, mixrate, numvoices, numchannels, initdata) != MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        MV_Printf("failed! %s\n", MV_ErrorString(MV_DriverError));
        status = FX_Error;
    }

    FX_MixRate = MV_MixRate;

    if (status == FX_Ok)
    {
        MV_Printf(": %.1f KHz %s with %d voices\n", MV_MixRate/1000.f, numchannels == 1 ? "mono" : "stereo", numvoices);
        FX_Installed = TRUE;
    }

    return status;
}

int FX_Shutdown(void)
{
    if (!FX_Installed)
        return FX_Ok;

    int status = MV_Shutdown();

    if (status != MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        status = FX_Error;
    }

    FX_Installed = FALSE;

    return status;
}

int FX_GetDevice(void) { return ASS_PCMSoundDriver; }


#define FMT_MAGIC(i, j, k, l) (i + (j << 8) + (k << 16) + (l << 24))
uint32_t constexpr FMT_CDXA_MAGIC = FMT_MAGIC('C','D','X','A');
uint32_t constexpr FMT_FLAC_MAGIC = FMT_MAGIC('f','L','a','C');
uint32_t constexpr FMT_OGG_MAGIC  = FMT_MAGIC('O','g','g','S');
uint32_t constexpr FMT_RIFF_MAGIC = FMT_MAGIC('R','I','F','F');
uint32_t constexpr FMT_VOC_MAGIC  = FMT_MAGIC('C','r','e','a');
uint32_t constexpr FMT_WAVE_MAGIC = FMT_MAGIC('W','A','V','E');
#undef FMT_MAGIC

static wavefmt_t FX_ReadFmt(char const * const ptr, uint32_t length)
{
    if (length < 12)
        return FMT_UNKNOWN;

    auto const ptr32 = (uint32_t const *)ptr;

    switch (B_LITTLE32(*ptr32))
    {
        case FMT_OGG_MAGIC:  return FMT_VORBIS;
        case FMT_VOC_MAGIC:  return FMT_VOC;
        case FMT_FLAC_MAGIC: return FMT_FLAC;
        case FMT_RIFF_MAGIC:
            if (B_LITTLE32(ptr32[2]) == FMT_WAVE_MAGIC) return FMT_WAV;
            if (B_LITTLE32(ptr32[2]) == FMT_CDXA_MAGIC) return FMT_XA;
            break;
        default:
            if (MV_IdentifyXMP(ptr, length)) return FMT_XMP;
            break;
    }

    return FMT_UNKNOWN;
}

static int FX_BadFmt(char *, uint32_t, int, int, int, int, int, int, int, fix16_t, intptr_t) { return MV_SetErrorCode(MV_InvalidFile); }
static int FX_BadFmt3D(char *, uint32_t, int, int, int, int, int, fix16_t, intptr_t)         { return MV_SetErrorCode(MV_InvalidFile); }

int FX_Play(char *ptr, uint32_t ptrlength, int loopstart, int loopend, int pitchoffset,
            int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    static constexpr decltype(FX_Play) *func[] = { FX_BadFmt, nullptr, MV_PlayVOC, MV_PlayWAV, MV_PlayVorbis, MV_PlayFLAC, MV_PlayXA, MV_PlayXMP };

    EDUKE32_STATIC_ASSERT(FMT_MAX == ARRAY_SIZE(func));

    int handle = func[FX_ReadFmt(ptr, ptrlength)](ptr, ptrlength, loopstart, loopend, pitchoffset, vol, left, right, priority, volume, callbackval);

    if (handle <= MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        handle = FX_Warning;
    }

    return handle;
}

int FX_Play3D(char *ptr, uint32_t ptrlength, int loophow, int pitchoffset, int angle, int distance,
              int priority, fix16_t volume, intptr_t callbackval)
{
    static constexpr decltype(FX_Play3D) *func[] = { FX_BadFmt3D, nullptr, MV_PlayVOC3D, MV_PlayWAV3D, MV_PlayVorbis3D, MV_PlayFLAC3D, MV_PlayXA3D, MV_PlayXMP3D };

    EDUKE32_STATIC_ASSERT(FMT_MAX == ARRAY_SIZE(func));

    int handle = func[FX_ReadFmt(ptr, ptrlength)](ptr, ptrlength, loophow, pitchoffset, angle, distance, priority, volume, callbackval);

    if (handle <= MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        handle = FX_Warning;
    }

    return handle;
}

int FX_PlayRaw(char *ptr, uint32_t ptrlength, int rate, int pitchoffset, int vol,
    int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    int handle = MV_PlayRAW(ptr, ptrlength, rate, NULL, NULL, pitchoffset, vol, left, right, priority, volume, callbackval);

    if (handle <= MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        handle = FX_Warning;
    }

    return handle;
}

int FX_PlayLoopedRaw(char *ptr, uint32_t ptrlength, char *loopstart, char *loopend, int rate,
    int pitchoffset, int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    int handle = MV_PlayRAW(ptr, ptrlength, rate, loopstart, loopend, pitchoffset, vol, left, right, priority, volume, callbackval);

    if (handle <= MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        handle = FX_Warning;
    }

    return handle;
}

int FX_StartDemandFeedPlayback(void (*function)(const char** ptr, uint32_t* length, void* userdata), int bitdepth, int channels, int rate, int pitchoffset,
    int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval, void* userdata)
{
    int handle = MV_StartDemandFeedPlayback(function, bitdepth, channels, rate,
        pitchoffset, vol, left, right, priority, volume, callbackval, userdata);

    if (handle <= MV_Ok)
    {
        FX_SetErrorCode(FX_MultiVocError);
        handle = FX_Warning;
    }

    return handle;
}

int FX_SetPrintf(void (*function)(const char *, ...))
{
    MV_SetPrintf(function);

    return FX_Ok;
}
