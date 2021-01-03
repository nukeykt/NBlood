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

#ifndef __sound_h__
#define __sound_h__

#define kMaxSoundFiles      80
#define kMaxSounds          200
#define kMaxSoundNameLen    8
#define kMaxActiveSounds    8

#define kCreepyCount 150

#define MUSIC_ID    (-65536)

enum {
    kSoundSplashBig = 0,
    kSoundSplashSmall,
    kSoundBubbleLow,
    kSoundGrenadeDrop,
    kSoundPistolClick,
    kSoundGrenadeRoll,
    kSoundCobraSprite,
    kSoundMummyChant0,
    kSoundAnubisICU,
    kSound9,
    kSoundItemSpecial,
    kSound11,
    kSoundTorchOn,
    kSoundJonBubbleNest,
    kSoundJonGasp,
    kSound15,
    kSound16,
    kSoundJonFall,
    kSound18,
    kSoundJonAir1,
    kSound20,
    kSound21,
    kSound22,
    kSoundAmbientStone,
    kSoundCatICU,
    kSoundBubbleHigh,
    kSoundSetLand,
    kSoundJonHLand,
    kSoundJonLaugh2,
    kSoundSpiderJump,
    kSoundJonScuba,
    kSoundItemUse,
    kSoundTrapArrow,
    kSoundSwitchFoot,
    kSound34,
    kSoundSwitchWtr1,
    kSoundTrapFire,
    kSoundSpiderAttack = 38,
    kSoundAnubisHit,
    kSoundFishDies,
    kSoundScorpionICU,
    kSoundJonWade,
    kSoundAmbientWater,
    kSoundDrum4 = 47,
    kSoundRexICU = 48,
    kSoundQTail = 50,
    kSoundJonHit3 = 52,
    kSoundTauntStart = 53,
    kSoundJonFDie = 60,
    kSoundWhyIsJeffAFeeb,
    kSoundShip1,
    kSoundSawOn,
    kSound64,
    kSound65,
    kSound66,
    kSoundMana1,
    kSoundMana2,
    kSoundAmmoPickup,
    kSound70,
    kSoundPotPc1,
    kSoundWeapon,
    kSoundAlarm,
    kSoundTick1,
    kSoundScorpionZap,
    kSoundJonTaunt3,
    kSoundJonLaugh1,
    kSoundBlasted,
    kSoundJonAir2,
};

extern short nAmbientChannel;
extern short nStopSound;
extern short nStoneSound;
extern short nSwitchSound;
extern short nLocalEyeSect;
extern short nElevSound;
extern short nCreepyTimer;

extern short StaticSound[];


void UpdateSounds();
void UpdateCreepySounds();

void InitFX();
void UnInitFX();
void FadeSong();
int LocalSoundPlaying();
void LoadFX();
void StopAllSounds();
void SetLocalChan(int nChannel);
int GetLocalSound();
void UpdateLocalSound();
void StopLocalSound();
void PlayLocalSound(short nSound, short val);
int LoadSound(const char* sound);

void BendAmbientSound();
void CheckAmbience(short nSector);

short PlayFX2(unsigned short nSound, short nSprite);
short PlayFXAtXYZ(unsigned short nSound, int x, int y, int z, int nSector);
short D3PlayFX(unsigned short nSound, short nSprite);
void StopSpriteSound(short nSprite);

void StartSwirlies();
void UpdateSwirlies();

void PlayLogoSound(void);
void PlayTitleSound(void);
void PlayGameOverSound(void);

void SoundBigEntrance(void);

void SetMasterFXVolume(int nVolume);

#endif
