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

#include "engine.h"
#include "anims.h"
#include "sequence.h"
#include "runlist.h"
#include "exhumed.h"
#include "sound.h"
#include "random.h"
#include "init.h"
#include "save.h"
#include <assert.h>

#define kMaxAnims	400

short nMagicSeq = -1;
short nPreMagicSeq  = -1;
short nSavePointSeq = -1;
short nAnimsFree = 0;

short AnimRunRec[kMaxAnims];
short AnimsFree[kMaxAnims];
Anim AnimList[kMaxAnims];
uint8_t AnimFlags[kMaxAnims];


void InitAnims()
{
    for (int i = 0; i < kMaxAnims; i++) {
        AnimsFree[i] = i;
    }

    nAnimsFree = kMaxAnims;

    nMagicSeq     = SeqOffsets[kSeqItems] + 21;
    nPreMagicSeq  = SeqOffsets[kSeqMagic2];
    nSavePointSeq = SeqOffsets[kSeqItems] + 12;
}

/*
    Use when deleting an ignited sprite to check if any anims reference it. 
    Will remove the Anim's loop flag and set the source (the ignited sprite's) sprite reference to -1.
    FuncAnim() will then delete the anim on next call for this anim.

    Without this, the anim will hold reference to a sprite which will eventually be reused, but the anim code
    will continue to manipulate its hitag value. This can break runlist records for things like LavaDude
    limbs that store these in the sprite hitag.

    Specifically needed for IgniteSprite() anims which can become orphaned from the source sprite (e.g a bullet)
    when the bullet sprite is deleted.
*/
void UnlinkIgnitedAnim(int nSprite)
{
    // scan the active anims (that aren't in the 'free' section of AnimsFree[])
    for (int i = kMaxAnims - 1; i >= nAnimsFree; i--)
    {
        int nAnim = AnimsFree[i];

        int nAnimSprite = GetAnimSprite(nAnim);
        if (nAnimSprite < 0)
            continue;

        if (sprite[nAnimSprite].statnum == kStatIgnited)
        {
            // .hitag holds the sprite number of the source 'sprite that's on fire' sprite
            if (nSprite == sprite[nAnimSprite].hitag)
            {
                AnimFlags[nAnim] &= ~kAnimLoop; // clear the animation loop flag
                sprite[nAnimSprite].hitag = -1; // set the sprite reference to -1
            }
        }
    }
}

void DestroyAnim(int nAnim)
{
    int nSprite = AnimList[nAnim].nSprite;

    if (nSprite >= 0)
    {
        StopSpriteSound(nSprite);
        runlist_SubRunRec(AnimRunRec[nAnim]);
        runlist_DoSubRunRec(sprite[nSprite].extra);
        runlist_FreeRun(sprite[nSprite].lotag - 1);
    }

    AnimsFree[nAnimsFree] = nAnim;
    nAnimsFree++;
}

int BuildAnim(int nSprite, int val, int val2, int x, int y, int z, int nSector, int nRepeat, int nFlag)
{
	if (!nAnimsFree) {
		return -1;
	}

    nAnimsFree--;

    short nAnim = AnimsFree[nAnimsFree];

    if (nSprite == -1) {
        nSprite = insertsprite(nSector, 500);
    }

    sprite[nSprite].x = x;
    sprite[nSprite].y = y;
    sprite[nSprite].z = z;
    sprite[nSprite].cstat = 0;

    if (nFlag & 4)
    {
        sprite[nSprite].pal = 4;
        sprite[nSprite].shade = -64;
    }
    else
    {
        sprite[nSprite].pal = 0;
        sprite[nSprite].shade = -12;
    }

    sprite[nSprite].clipdist = 10;
    sprite[nSprite].xrepeat = nRepeat;
    sprite[nSprite].yrepeat = nRepeat;
    //sprite[nSprite].picnum = 1;
    sprite[nSprite].ang = 0;
    sprite[nSprite].xoffset = 0;
    sprite[nSprite].yoffset = 0;
    sprite[nSprite].xvel = 0;
    sprite[nSprite].yvel = 0;
    sprite[nSprite].zvel = 0;

    // CHECKME - where is hitag set otherwise?
    if (sprite[nSprite].statnum < 900) {
        sprite[nSprite].hitag = -1;
    }

    sprite[nSprite].lotag = runlist_HeadRun() + 1;
    sprite[nSprite].owner = -1;
    sprite[nSprite].extra = runlist_AddRunRec(sprite[nSprite].lotag - 1, nAnim, kRunAnim);

    AnimRunRec[nAnim] = runlist_AddRunRec(NewRun, nAnim, kRunAnim);

    AnimFlags[nAnim] = nFlag;
    AnimList[nAnim].nSprite = nSprite;
    AnimList[nAnim].nFrame = 0;
    AnimList[nAnim].nSeq = SeqOffsets[val] + val2;

    if (nFlag & 0x80) {
        sprite[nSprite].cstat |= 0x2; // set transluscence
    }

    return nAnim;
}

short GetAnimSprite(short nAnim)
{
    return AnimList[nAnim].nSprite;
}

void FuncAnim(int a, int UNUSED(b), int nRun)
{
    short nAnim = RunData[nRun].nVal;
    assert(nAnim >= 0 && nAnim < kMaxAnims);

    short nSprite = AnimList[nAnim].nSprite;
    short nSeq = AnimList[nAnim].nSeq;

    assert(nSprite != -1);

    int nMessage = a & kMessageMask;

    switch (nMessage)
    {
        case 0x20000:
        {
            short nFrame = AnimList[nAnim].nFrame;

            if (!(sprite[nSprite].cstat & 0x8000))
            {
                seq_MoveSequence(nSprite, nSeq, nFrame);
            }

            if (sprite[nSprite].statnum == kStatIgnited)
            {
                int nIgnitedSprite = sprite[nSprite].hitag;
                if (nIgnitedSprite > -1)
                {
                    sprite[nSprite].x = sprite[nIgnitedSprite].x;
                    sprite[nSprite].y = sprite[nIgnitedSprite].y;
                    sprite[nSprite].z = sprite[nIgnitedSprite].z;

                    if (sprite[nIgnitedSprite].sectnum != sprite[nSprite].sectnum)
                    {
                        if (sprite[nIgnitedSprite].sectnum < 0 || sprite[nIgnitedSprite].sectnum >= kMaxSectors)
                        {
                            DestroyAnim(nAnim);
                            mydeletesprite(nSprite);
                            return;
                        }
                        else
                        {
                            mychangespritesect(nSprite, sprite[nIgnitedSprite].sectnum);
                        }
                    }

                    if (!nFrame)
                    {
                        if (sprite[nIgnitedSprite].cstat != 0x8000)
                        {
                            int hitag2 = sprite[nIgnitedSprite].hitag;
                            sprite[nIgnitedSprite].hitag--;

                            if (hitag2 >= 15)
                            {
                                runlist_DamageEnemy(nIgnitedSprite, -1, (sprite[nIgnitedSprite].hitag - 14) * 2);

                                if (sprite[nIgnitedSprite].shade < 100)
                                {
                                    sprite[nIgnitedSprite].pal = 0;
                                    sprite[nIgnitedSprite].shade++;
                                }

                                if (!(sprite[nIgnitedSprite].cstat & 0x101))
                                {
                                    DestroyAnim(nAnim);
                                    mydeletesprite(nSprite);
                                    return;
                                }
                            }
                            else
                            {
                                sprite[nIgnitedSprite].hitag = 1;
                                DestroyAnim(nAnim);
                                mydeletesprite(nSprite);
                            }
                        }
                        else
                        {
                            sprite[nIgnitedSprite].hitag = 1;
                            DestroyAnim(nAnim);
                            mydeletesprite(nSprite);
                        }
                    }
                }
            }

            AnimList[nAnim].nFrame++;
            if (AnimList[nAnim].nFrame >= SeqSize[nSeq])
            {
                if (AnimFlags[nAnim] & 0x10)
                {
                    AnimList[nAnim].nFrame = 0;
                }
                else if (nSeq == nPreMagicSeq)
                {
                    AnimList[nAnim].nFrame = 0;
                    AnimList[nAnim].nSeq = nMagicSeq;
                    short nAnimSprite = AnimList[nAnim].nSprite;
                    AnimFlags[nAnim] |= 0x10;
                    sprite[nAnimSprite].cstat |= 2;
                }
                else if (nSeq == nSavePointSeq)
                {
                    AnimList[nAnim].nFrame = 0;
                    AnimList[nAnim].nSeq++;
                    AnimFlags[nAnim] |= 0x10;
                }
                else
                {
                    DestroyAnim(nAnim);
                    mydeletesprite(nSprite);
                }
                return;
            }

            return;
        }

        case 0x90000:
        {
            seq_PlotSequence(a & 0xFFFF, nSeq, AnimList[nAnim].nFrame, 0x101);
            tsprite[a & 0xFFFF].owner = -1;
            return;
        }

        case 0xA0000:
        {
            return;
        }

        default:
        {
            DebugOut("unknown msg %x for anim\n", nMessage);
            return;
        }
    }
}

void BuildExplosion(short nSprite)
{
    short nSector = sprite[nSprite].sectnum;

    int edx = 36;

    if (SectFlag[nSector] & kSectUnderwater)
    {
        edx = 75;
    }
    else if (sprite[nSprite].z == sector[nSector].floorz)
    {
        edx = 34;
    }

    BuildAnim(-1, edx, 0, sprite[nSprite].x, sprite[nSprite].y, sprite[nSprite].z, sprite[nSprite].sectnum, sprite[nSprite].xrepeat, 4);
}

int BuildSplash(int nSprite, int nSector)
{
    int nRepeat, nSound;

    if (sprite[nSprite].statnum != 200)
    {
        nRepeat = sprite[nSprite].xrepeat + (RandomWord() % sprite[nSprite].xrepeat);
        nSound = kSoundSplashBig;
    }
    else
    {
        nRepeat = 20;
        nSound = kSoundSplashSmall;
    }

    int bIsLava = SectFlag[nSector] & kSectLava;

    int edx, nFlag;

    if (bIsLava)
    {
        edx = 43;
        nFlag = 4;
    }
    else
    {
        edx = 35;
        nFlag = 0;
    }

    int nAnim = BuildAnim(-1, edx, 0, sprite[nSprite].x, sprite[nSprite].y, sector[nSector].floorz, nSector, nRepeat, nFlag);

    if (!bIsLava)
    {
        D3PlayFX(StaticSound[nSound] | 0xa00, AnimList[nAnim].nSprite);
    }

    return AnimList[nAnim].nSprite;
}

class AnimsLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void AnimsLoadSave::Load()
{
    Read(&nMagicSeq, sizeof(nMagicSeq));
    Read(&nPreMagicSeq, sizeof(nPreMagicSeq));
    Read(&nSavePointSeq, sizeof(nSavePointSeq));
    Read(&nAnimsFree, sizeof(nAnimsFree));

    Read(AnimRunRec, sizeof(AnimRunRec));
    Read(AnimsFree, sizeof(AnimsFree));
    Read(AnimList, sizeof(AnimList));
    Read(AnimFlags, sizeof(AnimFlags));
}

void AnimsLoadSave::Save()
{
    Write(&nMagicSeq, sizeof(nMagicSeq));
    Write(&nPreMagicSeq, sizeof(nPreMagicSeq));
    Write(&nSavePointSeq, sizeof(nSavePointSeq));
    Write(&nAnimsFree, sizeof(nAnimsFree));

    Write(AnimRunRec, sizeof(AnimRunRec));
    Write(AnimsFree, sizeof(AnimsFree));
    Write(AnimList, sizeof(AnimList));
    Write(AnimFlags, sizeof(AnimFlags));
}

static AnimsLoadSave* myLoadSave;

void AnimsLoadSaveConstruct()
{
    myLoadSave = new AnimsLoadSave();
}
