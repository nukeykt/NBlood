#pragma once
#include "db.h"

void sfxInit(void);
void sfxTerm(void);
void sfxPlay3DSound(int x, int y, int z, int soundId, int nSector);
void sfxPlay3DSound(SPRITE *pSprite, int soundId, int a3 = -1, int a4 = 0);
void sfxKill3DSound(SPRITE *pSprite, int a2 = -1, int a3 = -1);
void sfxKillAllSounds(void);
void sfxUpdate3DSounds(void);
void sfxSetReverb(bool toggle);
void sfxSetReverb2(bool toggle);