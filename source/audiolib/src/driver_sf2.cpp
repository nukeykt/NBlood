//-------------------------------------------------------------------------
/*
Copyright (C) 2020 EDuke32 developers and contributors

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

#include "driver_sf2.h"

#include "_multivc.h"
#include "common.h"
#include "midi.h"

int SF2_EffectSampleBlockSize = 16;

#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#define TSF_RENDER_EFFECTSAMPLEBLOCK SF2_EffectSampleBlockSize
#define TSF_MALLOC  Xmalloc
#define TSF_REALLOC Xrealloc
#define TSF_FREE    Xfree
#define TSF_MEMCPY Bmemcpy
#define TSF_MEMSET Bmemset

#include "tsf.h"

static tsf *sf2_synth;
char        SF2_BankFile[BMAX_PATH];
static int  SF2_Volume = MIDI_MaxVolume;
static int  ErrorCode  = SF2_Ok;

static inline int SF2_SetError(int const status) { return (ErrorCode = status); }

int SF2Drv_GetError(void) { return ErrorCode; }

static void SF2_NoteOff(int channel, int key, int velocity)
{
    UNREFERENCED_PARAMETER(velocity);
    tsf_channel_note_off(sf2_synth, channel, key);
}

static void SF2_NoteOn(int channel, int key, int velocity)        { tsf_channel_note_on(sf2_synth, channel, key, velocity * (1.f / 127.f)); }
static void SF2_ControlChange(int channel, int number, int value) { tsf_channel_midi_control(sf2_synth, channel, number, value); }
static void SF2_ProgramChange(int channel, int program)           { tsf_channel_set_presetnumber(sf2_synth, channel, program, channel == 9); }
static void SF2_SetPitchBend(int channel, int lsb, int msb)       { tsf_channel_set_pitchwheel(sf2_synth, channel, (msb << 7) | lsb); }
static void SF2_SetVolume(int volume)                             { SF2_Volume = clamp(volume, 0, MIDI_MaxVolume); }

const char *SF2Drv_ErrorString(int ErrorNumber)
{
    switch (ErrorNumber)
    {
        case SF2_Error:     return SF2Drv_ErrorString(ErrorCode);
        case SF2_Ok:        return "SF2 ok.";
        case SF2_BankError: return "SF2 bank error.";
        default:            return "Unknown SF2 error.";
    }
}

static int sf2_stream_read(void *handle, void *ptr, unsigned int size) { return kread(*(buildvfs_kfd *)handle, ptr, size); };
static int sf2_stream_skip(void *handle, unsigned int size)            { return !klseek(*(buildvfs_kfd *)handle, size, SEEK_CUR); };

static int SF2_LoadBank(char const *const filename)
{
    buildvfs_kfd sf2_kfd    = kopen4loadfrommod(filename, 0);
    tsf_stream   sf2_stream = { &sf2_kfd, &sf2_stream_read, &sf2_stream_skip };

    if (sf2_kfd != buildvfs_kfd_invalid)
    {
        tsf_close(sf2_synth);
        sf2_synth = tsf_load(&sf2_stream);
        kclose(sf2_kfd);

        if (sf2_synth)
        {
            VLOG_F(LOG_ASS, "Loaded \"%s\"", filename);
            return SF2_Ok;
        }
    }

    LOG_F(ERROR, "Unable to load \"%s\"!", filename);
    return SF2_Error;
}

int SF2Drv_MIDI_Init(midifuncs* const funcs)
{
    SF2Drv_MIDI_Shutdown();

    auto filename = SF2_BankFile;

    if (!filename[0])
    {
        fnlist_t fnl = FNLIST_INITIALIZER;

        // default to the first .sf2 we find if cvar mus_sf2_bank is unset
        fnlist_getnames(&fnl, g_modDir, "*.sf2", 0, 0);

        if (!fnl.findfiles)
            fnlist_getnames(&fnl, "/", "*.sf2", 0, 0);

        if (fnl.findfiles)
            filename = Xstrdup(fnl.findfiles->name);

        fnlist_clearnames(&fnl);

        if (!filename[0])
        {
            LOG_F(WARNING, "No .sf2 data found!");
            return SF2_SetError(SF2_BankError);
        }
    }

    int const loaded = SF2_LoadBank(filename);

    if (filename != SF2_BankFile)
        Xfree(filename);

    if (loaded != SF2_Ok || !sf2_synth)
        return SF2_SetError(SF2_BankError);

    Bmemset(funcs, 0, sizeof(midifuncs));

    funcs->NoteOff           = SF2_NoteOff;
    funcs->NoteOn            = SF2_NoteOn;
    funcs->PolyAftertouch    = nullptr;
    funcs->ControlChange     = SF2_ControlChange;
    funcs->ProgramChange     = SF2_ProgramChange;
    funcs->ChannelAftertouch = nullptr;
    funcs->PitchBend         = SF2_SetPitchBend;
    funcs->SetVolume         = SF2_SetVolume;

    SF2_Volume = MIDI_MaxVolume;

    return SF2_Ok;
}

void SF2Drv_MIDI_HaltPlayback(void) { MV_UnhookMusicRoutine(); }

void SF2Drv_MIDI_Shutdown(void)
{
    SF2Drv_MIDI_HaltPlayback();

    tsf_close(sf2_synth);
    sf2_synth = nullptr;
    ErrorCode = SF2_Ok;
}

int SF2Drv_MIDI_StartPlayback(void)
{
    SF2Drv_MIDI_HaltPlayback();

    tsf_set_output(sf2_synth, MV_Channels == 1 ? TSF_MONO : TSF_STEREO_INTERLEAVED, TSF_INTERP_CUBIC, MV_MixRate, 0);
    tsf_channel_set_bank_preset(sf2_synth, 9, 128, 0);
    tsf_reset(sf2_synth);

    for (int channel = 0; channel < 16; channel++)
        SF2_ProgramChange(channel, 0);

    MV_HookMusicRoutine(SF2Drv_MIDI_Service);

    return MIDI_Ok;
}

void SF2Drv_MIDI_SetTempo(int const tempo, int const division)
{
    MV_MIDIRenderTempo = tempo * division / 60;
    MV_MIDIRenderTimer = 0;
}

void SF2Drv_MIDI_Service(void)
{
    int16_t *    buffer16 = (int16_t *)MV_MusicBuffer;
    static float fbuf[MV_MIXBUFFERSIZE * 2];
    float const  fvolume = SF2_Volume * (32768.f / MIDI_MaxVolume);

    for (int i = 0; i < MV_MIXBUFFERSIZE;)
    {
        while (MV_MIDIRenderTimer >= MV_MixRate)
        {
            if (MV_MIDIRenderTempo >= 0)
                MIDI_ServiceRoutine();
            MV_MIDIRenderTimer -= MV_MixRate;
        }

        int samples = MV_MIDIRenderTempo > 0 ? (MV_MixRate - MV_MIDIRenderTimer + MV_MIDIRenderTempo - 1) / MV_MIDIRenderTempo : MV_MIXBUFFERSIZE;
        samples     = min(samples, MV_MIXBUFFERSIZE - i);
        tsf_render_float(sf2_synth, fbuf, samples);

        int const nsamples = samples * MV_Channels;

        for (int j = 0; j < nsamples; j++)
            *buffer16++ = clamp((fbuf[j] * fvolume), INT16_MIN, INT16_MAX);

        if (MV_MIDIRenderTempo >= 0)
            MV_MIDIRenderTimer += MV_MIDIRenderTempo * samples;

        i += samples;
    }
}
