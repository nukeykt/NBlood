//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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

// This object is shared by all Build games with MIDI playback!

#include "music.h"

#include "al_midi.h"
#include "compat.h"
#include "drivers.h"
#include "midi.h"
#include "mpu401.h"
#include "multivoc.h"
#include "sndcards.h"

int MUSIC_ErrorCode = MUSIC_Ok;

static midifuncs MUSIC_MidiFunctions;

int MUSIC_InitMidi(int card, midifuncs *Funcs);

#define MUSIC_SetErrorCode(status) MUSIC_ErrorCode = (status);

const char *MUSIC_ErrorString(int ErrorNumber)
{
    const char *ErrorString;

    switch (ErrorNumber)
    {
        case MUSIC_Warning:
        case MUSIC_Error:       ErrorString = MUSIC_ErrorString(MUSIC_ErrorCode); break;
        case MUSIC_Ok:          ErrorString = "Music ok."; break;
        case MUSIC_MidiError:   ErrorString = "Error playing MIDI file."; break;
        default:                ErrorString = "Unknown Music error code."; break;
    }

    return ErrorString;
}


int MUSIC_Init(int SoundCard)
{
    if (SoundCard == ASS_AutoDetect) {
#if defined _WIN32
        SoundCard = ASS_WinMM;
#elif RENDERTYPESDL
        SoundCard = ASS_SDL;
#else
        SoundCard = ASS_NoSound;
#endif
    }

    if (SoundCard < 0 || SoundCard >= ASS_NumSoundCards) {
        MUSIC_ErrorCode = MUSIC_MidiError;
        return MUSIC_Error;
    }

    if (!SoundDriver_IsMIDISupported(SoundCard))
    {
        MV_Printf("Couldn't init %s, falling back to no sound...\n", SoundDriver_GetName(SoundCard));
        MUSIC_ErrorCode = MUSIC_MidiError;
        return MUSIC_Error;
    }

   ASS_MIDISoundDriver = SoundCard;

   int status = SoundDriver_MIDI_Init(&MUSIC_MidiFunctions);
   if (status != MUSIC_Ok)
   {
       MUSIC_ErrorCode = MUSIC_MidiError;
       return MUSIC_Error;
   }

    return MUSIC_InitMidi(SoundCard, &MUSIC_MidiFunctions);
}


int MUSIC_Shutdown(void)
{
    MIDI_StopSong();

    return MUSIC_Ok;
}


void MUSIC_SetVolume(int volume) { MIDI_SetVolume(min(max(0, volume), 255)); }


int MUSIC_GetVolume(void) { return MIDI_GetVolume(); }
void MUSIC_SetLoopFlag(int loopflag) { MIDI_SetLoopFlag(loopflag); }
void MUSIC_Continue(void) { MIDI_ContinueSong(); }
void MUSIC_Pause(void) { MIDI_PauseSong(); }

int MUSIC_StopSong(void)
{
    MIDI_StopSong();
    MUSIC_SetErrorCode(MUSIC_Ok);
    return MUSIC_Ok;
}


int MUSIC_PlaySong(char *song, int songsize, int loopflag, const char *fn /*= nullptr*/)
{
    UNREFERENCED_PARAMETER(songsize);
    UNREFERENCED_PARAMETER(fn);

    MUSIC_SetErrorCode(MUSIC_Ok)

    if (MIDI_PlaySong(song, loopflag) != MIDI_Ok)
    {
        MUSIC_SetErrorCode(MUSIC_MidiError);
        return MUSIC_Warning;
    }

    return MUSIC_Ok;
}


int MUSIC_InitMidi(int card, midifuncs *Funcs)
{
    switch (card)
    {
    case ASS_MPU401:
        Funcs->NoteOff = MPU_NoteOff;
        Funcs->NoteOn = MPU_NoteOn;
        Funcs->PolyAftertouch = MPU_PolyAftertouch;
        Funcs->ControlChange = MPU_ControlChange;
        Funcs->ProgramChange = MPU_ProgramChange;
        Funcs->ChannelAftertouch = MPU_ChannelAftertouch;
        Funcs->PitchBend = MPU_PitchBend;
        break;

    case ASS_OPL3:
        Funcs->NoteOff = OPLMusic::AL_NoteOff;
        Funcs->NoteOn = OPLMusic::AL_NoteOn;
        Funcs->PolyAftertouch = NULL;
        Funcs->ControlChange = OPLMusic::AL_ControlChange;
        Funcs->ProgramChange = OPLMusic::AL_ProgramChange;
        Funcs->ChannelAftertouch = NULL;
        Funcs->PitchBend = OPLMusic::AL_SetPitchBend;
        break;
    }

    MIDI_SetMidiFuncs(Funcs);

    return MIDI_Ok;
}

void MUSIC_Update(void)
{
    if (ASS_MIDISoundDriver != ASS_MPU401)
        return;

    MIDI_UpdateMusic();
}
