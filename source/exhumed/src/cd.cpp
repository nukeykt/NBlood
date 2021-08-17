//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#include "build.h"
#include "compat.h"
#include "baselayer.h"
#include "cd.h"
#include "fx_man.h"
#include "sound.h"
#include "exhumed.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

static char *pTrack = NULL;
static int trackhandle = -1;
int nLastVolumeSet = 0;

/* TODO

Currently playing music must keep playing on return to map screen or exit from training level

*/

void setCDaudiovolume(int val)
{
    if (trackhandle > 0) {
        FX_SetPan(trackhandle, val, val, val);
    }
}

bool playCDtrack(int nTrack, bool bLoop)
{
//  if (!MusicToggle)
//      return true;

    static const char *tracknameformats[] = { "Track%02d", "exhumed%02d", "track%02d" };

    if (nTrack < 2) {
        return false;
    }

    StopCD();

    char filename[BMAX_PATH];
    int32_t hFile = -1;

    for (auto fmt : tracknameformats)
    {
        Bsnprintf(filename, sizeof(filename), fmt, nTrack);
        hFile = S_OpenAudio(filename, 0, 1);
        if (hFile >= 0)
            break;
    }
  
    if (hFile < 0) {
        OSD_Printf("Error opening music track %02d\n", nTrack);
        return false;
    }

    int32_t nFileLen = kfilelength(hFile);

    pTrack = (char*)Xaligned_alloc(16, nFileLen);
    if (pTrack == NULL)
    {
        OSD_Printf("Error allocating music track data memory for %s\n", filename);
        kclose(hFile);
        return false;
    }

    int nRead = kread(hFile, pTrack, nFileLen);
    if (nRead != nFileLen)
    {
        OSD_Printf("Error reading music track data for %s\n", filename);
        Xaligned_free(pTrack);
        pTrack = NULL;
        kclose(hFile);
        return false;
    }

    kclose(hFile);

    trackhandle = FX_Play(pTrack, nRead, bLoop ? 0 : -1, 0, 0, MusicVolume, MusicVolume, MusicVolume, FX_MUSIC_PRIORITY, fix16_one, MUSIC_ID);
    if (trackhandle <= FX_Ok)
    {
        OSD_Printf("Error playing music track %s\n", filename);
        if (pTrack)
        {
            Xaligned_free(pTrack);
            pTrack = NULL;
        }
        return false;
    }

    return true;
}

void StartfadeCDaudio()
{
    if (CDplaying()) {
        nLastVolumeSet = MusicVolume;
    }
}

int StepFadeCDaudio()
{
    if (!CDplaying()) {
        return 0;
    }

    if (nLastVolumeSet <= 0) {
        return 0;
    }

    nLastVolumeSet -= 8;

    if (nLastVolumeSet <= 0) {
        nLastVolumeSet = 0;
    }

    setCDaudiovolume(nLastVolumeSet);

    if (nLastVolumeSet == 0) {
        StopCD();
    }

    return 1;
}

bool CDplaying()
{
    if (trackhandle <= 0) {
        return false;
    }
    else {
        return FX_SoundActive(trackhandle);
    }
}

void StopCD()
{
    if (trackhandle > 0) {
        FX_StopSound(trackhandle);
        trackhandle = -1;
    }

    if (pTrack)
    {
        Xaligned_free(pTrack);
        pTrack = NULL;
    }
}

int fadecdaudio()
{
    StartfadeCDaudio();

    while (1)
    {
        if (!StepFadeCDaudio())
        {
            return 1;
        }
        else
        {
            WaitTicks(1);
        }
    }

    return 1;
}
