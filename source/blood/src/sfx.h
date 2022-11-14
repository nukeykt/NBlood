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
#pragma once
#include "db.h"

struct BONKLE
{
    int lChan; // in terms of audio, left channel is main channel when using mono
    int rChan;
    DICTNODE* hSnd;
    int sfxId;
    spritetype* pSndSpr;
    int chanId;
    int pitch;
    int vol;
    POINT3D curPos;
    POINT3D oldPos;
    int sectnum;
    int format;
};

extern BONKLE Bonkle[256];
extern BONKLE* BonkleCache[256];
extern int nBonkles;

void sfxInit(void);
void sfxTerm(void);
void sfxPlay3DSound(int x, int y, int z, int soundId, int nSector);
void sfxPlay3DSound(spritetype *pSprite, int soundId, int chanId = -1, int nFlags = 0);
void sfxPlay3DSoundCP(spritetype* pSprite, int soundId, int chanId = -1, int nFlags = 0, int pitch = 0, int volume = 0);
void sfxKill3DSound(spritetype *pSprite, int chanId = -1, int soundId = -1);
void sfxKillAllSounds(void);
void sfxKillSpriteSounds(spritetype *pSprite);
void sfxUpdate3DSounds(void);
void sfxSetReverb(bool toggle);
void sfxSetReverb2(bool toggle);