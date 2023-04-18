//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

*********************************************************************
NoOne: Custom Dude system. Includes compatibility with older versions
For full documentation visit: http://cruo.bloodgame.ru/xxsystem/cdud/v2/
*********************************************************************

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


#ifdef NOONE_EXTENSIONS
#pragma once
#include "nnexts.h"
#include "tile.h"
#include "sfx.h"
#include "view.h"
#include "globals.h"
#include "trig.h"
#include "sound.h"

#define kCdudeFileNamePrefix        "CDUD"
#define kCdudeFileNamePrefixWild    "CDUD*"
#define kCdudeFileExt               "CDU"
#define kCdudeVer1                  1
#define kCdudeVer2                  2

#define kCdudeMaxVelocity           2345648
#define kCdudeMaxSounds             3
#define kCdudeMaxWeapons            8
#define kCdudeMaxDmgScale           32767
#define kCdudeDmgCheckDelay         1
#define kCdudeMaxDispersion         3500
#define kCdudeDefaultSeq            11520
#define kCdudeV1MaxAttackDist       20000
#define kCdudeMaxDropItems          5
#define kCdudeDefaultAnimScale      256
#define kCdudeMaxEffectGroups       16
#define kCdudeMaxEffects            3
#define kCdudeFXEffectBase          0
#define kCudeFXEffectCallbackBase   512
#define kCdudeGIBEffectBase         1024

#define kCdudeMinSeeDist            3000
#define kCdudeMinHearDist           (kCdudeMinSeeDist >> 1)
#define kCdudeBurningHealth         (25 << 4)

class CUSTOMDUDE;
extern int nCdudeAppearanceCallback;
extern char gCdudeCustomCallback[];

enum enum_VALUE_TYPE {
kValNone                        = 0,
kValAny,
kValFix,
kValUfix,
kValPerc,
kValArrC,
kValArrA,
kValBool,
kValIdKeywordBase,
kValCdud                        = kValIdKeywordBase,
kValWeapon,
kValFX,
kValGIB,
kValVector,
kValProjectile,
kValThrow,
kValVdud,
kValKamikaze,
kValSpecial,
kValIdKeywordMax,
kValMax                         = kValIdKeywordMax,
};

enum enum_ERROR {
kErrInvalidValType              = 0,
kErrInvalidParam,
kErrInvalidParamPos,
kErrInvalidRange,
kErrInvalidResultC,
kErrInvalidVersion,
kErrReqParamNotFound,
kErrInvalidArrayLen1,
kErrInvalidArrayLen2,
kErrReqGroupNotFound,
kErrInvaliValuePos,
kErrMax,
};

enum enum_PAR_GROUP {
kParGroupGeneral                = 0,
kParGroupVelocity,
kParGroupAnimation,
kParGroupSound,
kParGroupWeapon,
kParGroupDodge,
kParGroupRecoil,
kParGroupKnockout,
kParGroupDamage,
kParGroupFXEffect,
kParGroupMovePat,
kParGroupDropItem,
kParGroupParser,
};

enum enum_PAR_GENERAL {
kParGeneralVersion              = 0,
kParGeneralMass,
kParGeneralMedium,
kParGeneralHealth,
kParGeneralClipdist,
kParGeneralMorphTo,
kParGeneralActiveTime,
};


enum enum_PAR_PARSER {
kParParserWarnings              = 0,
};

enum enum_PAR_EVENT {
kParEventOnDmg                  = 0,
kParEventOnAimTargetWrong,
};

enum enum_PAR_DAMAGE {
kParDmgSource                   = kDmgMax,
};

enum enum_PAR_DAMAGE_SOURCE {
kDmgSourceDude                  = 0x01,
kDmgSourcePlayer                = 0x02,
kDmgSourceSelf                  = 0x04,
kDmgSourceFriend                = 0x08,
kDmgSourceEnemy                 = 0x10,
kDmgSourceSlave                 = 0x20,
kDmgSourceKin                   = 0x40,
};

enum enum_PAR_VELOCITY {
kParVelocityForward             = 0,
kParVelocityTurn,
kParVelocityDodge,
kParVelocityMax,
};

enum enum_PAR_APPEARANCE {
kAppearSeq                      = 0,
kAppearClb,
kAppearSnd,
kAppearScale,
kAppearPic,
kAppearPal,
kAppearShade,
kAppearSize,
kAppearOffs1,
};


enum enum_PAR_MEDIUM {
kParMediumAny                   = 0,
kParMediumLand,
kParMediumWater,
};

enum enum_PAR_WEAPON {
kParWeaponId                    = 0,
kParWeaponAkimbo,
kParWeaponDist,
kParWeaponPosture,
kParWeaponDisp,
kParWeaponAttackAng,
kParWeaponMedium,
kParWeaponAmmo,
kParWeaponPickChance,
kParWeaponAttackSetup,
kParWeaponShotSetup,
kParWeaponShotSnd,
kParWeaponShotAppearance,
kParWeaponAttackAnim,
kParWeaponDudeHealth,
kParWeaponTargetHealth,
kParWeaponSkill,
kParWeaponCooldown,
kParWeaponStyle,
};

enum enum_PAR_ATTACK {
kParAttackTime                  = 0,
kParAttackInterrupt,
kParAttackTurn2Target,
kParAttackNumShots,
kParAttackInertia,
};

enum enum_PAR_SHOT {
kParWeaponShotOffs              = 0,
kParWeaponShotVel,
kParWeaponShotSlope,
kParWeaponShotFollow,
kParWeaponShotClipdist,
kParWeaponShotImpact,
kParWeaponShotRemTimer,
};

enum enum_PAR_WEAP_STYLE {
kParWeaponStyleOffset           = 0,
kParWeaponStyleAngle,
};

enum enum_PAR_EFFECT {
kParEffectId                    = 0,
kParEffectTimer,
kParEffectOffset,
kParEffectAppearance,
kParEffectAiState,
kParEffectAnimID,
kParEffectAnimFrame,
kParEffectAngle,
kParEffectVelocity,
kParEffectSlope,
kParEffectPosture,
kParEffectMedium,
kParEffectRemTime,
kParEffectChance,
kParEffectAllUnique,
kParEffectSrcVel,
kParEffectFx2Gib,
};

enum enum_PAR_MOVE {
kParMoveFallHeight              = 0,
};

enum enum_PAR_DROP_ITEM {
kParDropItem                    = 0,
kParDropItemType,
kParDropItemChance,
kParDropItemSkill,
kParDropItemSprChance,
};

enum enum_PAR_GIB {
kParGibType,
kParGibSoundID,
kParGibForce,
kParGibFlags,
kParGibPhysics,
};

enum enum_PAR_TRIG_FLAGS {
kParTigNone                     = 0x00,
kParTrigVector                  = 0x01,
kParTrigTouch                   = 0x02,
kParTrigImpact                  = 0x04,
kParTrigLocked                  = 0x08,
};

enum enum_CDUD_POSTURE {
kCdudePosture                   = 0,
kCdudePostureL                  = kCdudePosture,
kCdudePostureC,
kCdudePostureW,
kCdudePostureMax,
};

enum enum_CDUD_SOUND {
kCdudeSnd                       = 0,
kCdudeSndTargetSpot             = kCdudeSnd,
kCdudeSndGotHit,
kCdudeSndDeathNormal,
kCdudeSndBurning,
kCdudeSndDeathExplode,
kCdudeSndTargetDead,
kCdudeSndTargetChase,
kCdudeSndTransforming,
kCdudeSndWake,
kCdudeSndMax,
kCdudeSndCompatAttack1,
kCdudeSndCompatAttack2,
kCdudeSndCompatAttack3,
};

enum enum_CDUD_WEAPON_TYPE {
kCdudeWeaponNone                = 0,
kCdudeWeaponHitscan,
kCdudeWeaponMissile,
kCdudeWeaponThrow,
kCdudeWeaponSummon,
kCdudeWeaponSummonCdude,
kCdudeWeaponKamikaze,
kCdudeWeaponSpecialBeastStomp,
kCdudeWeaponMax,
};

enum enum_CDUD_WEAPON_TYPE_SPECIAL {
kCdudeWeaponIdSpecialBase       = 900,
kCdudeWeaponIdSpecialBeastStomp = kCdudeWeaponIdSpecialBase,
kCdudeWeaponIdSpecialMax
};

enum enum_CDUD_AISTATE {
kCdudeStateBase                 = 0,
kCdudeStateIdle                 = kCdudeStateBase,
kCdudeStateMoveBase,
kCdudeStateSearch               = kCdudeStateMoveBase,
kCdudeStateDodge,
kCdudeStateChase,
kCdudeStateFlee,
kCdudeStateMoveMax,
kCdudeStateRecoil               = kCdudeStateMoveMax,
kCdudeStateRecoilT,
kCdudeBurnStateSearch,
kCdudeStateMorph,
kCdudeStateKnockEnter,
kCdudeStateKnock,
kCdudeStateKnockExit,
kCdudeStateSleep,
kCdudeStateWake,
kCdudeStateGenIdle,
kCdudeStateNormalMax,
kCdudeStateDeathBase            = kCdudeStateNormalMax,
kCdudeStateDeathBurn,
kCdudeStateDeathVector,
kCdudeStateDeathExplode,
kCdudeStateDeathChoke,
kCdudeStateDeathSpirit,
kCdudeStateDeathElectric,
kCdudeStateDeathMax,
kCdudeStateAttackBase           = kCdudeStateDeathMax,
kCdudeStateAttackMax            = kCdudeStateAttackBase + kCdudeMaxWeapons,
kCdudeStateMax                  = kCdudeStateAttackMax,
kCdudeStateMove,
kCdudeStateDeath,
kCdudeStateAttack,
kCdudeAnimScale,
};

enum enum_CDUD_STATUS {
kCdudeStatusNormal              = 0x00,
kCdudeStatusAwaked              = 0x01,
kCdudeStatusForceCrouch         = 0x02,
kCdudeStatusSleep               = 0x04,
kCdudeStatusMorph               = 0x08,
kCdudeStatusBurning             = 0x10,
kCdudeStatusDying               = 0x20,
kCdudeStatusRespawn             = 0x40,
};

#pragma pack(push, 1)
struct PARAM
{
    unsigned int id             : 8;
    const char* text;
};
#pragma pack(pop)

class ARG_PICK_WEAPON
{
    public:
        unsigned int angle          : 12;
        unsigned int distance       : 32;
        unsigned int dudeHealth     : 8;
        unsigned int targHealth     : 8;
        ARG_PICK_WEAPON(spritetype* pSpr, XSPRITE* pXSpr, spritetype* pTarg, XSPRITE* pXTarg)
        {
            int dx = pTarg->x - pSpr->x;
            int dy = pTarg->y - pSpr->y;
            distance = approxDist(dx, dy);
            angle = klabs(((getangle(dx, dy) + kAng180 - pSpr->ang) & kAngMask) - kAng180);
            dudeHealth = CountHealthPerc(pSpr, pXSpr);
            targHealth = CountHealthPerc(pTarg, pXTarg);
        }

        ARG_PICK_WEAPON(spritetype* pSpr, XSPRITE* pXSpr, spritetype* pTarg, XSPRITE* pXTarg, int nDist, int nAng)
        {
            distance = nDist;
            angle = nAng;
            dudeHealth = CountHealthPerc(pSpr, pXSpr);
            targHealth = CountHealthPerc(pTarg, pXTarg);
        }

        char CountHealthPerc(spritetype* pSpr, XSPRITE* pXSpr)
        {
            int nHealth = ClipLow(nnExtGetStartHealth(pSpr), 1);
            return ClipHigh((kPercFull * pXSpr->health) / nHealth, 255);
        }
};

class CUSTOMDUDE_SOUND
{
    public:
        unsigned int id[kCdudeMaxSounds];
        unsigned int medium         : 3;
        unsigned int ai             : 1;
        unsigned int interruptable  : 1;
        unsigned int once           : 1;
        signed   int volume         : 11;
        int  Pick()                                         { return id[Random(kCdudeMaxSounds)]; }
        void Play(spritetype* pSpr)                         { Play(pSpr, Pick()); }
        void Play(spritetype* pSpr, int nID)
        {
            if (nID > 0)
            {
                int i, j;
                int nClock = (int)gFrameClock;
                char uwater = spriteIsUnderwater(pSpr, true);
                int nRand = Random2(80);

                if (!medium || ((medium & 0x01) && !uwater) || ((medium & 0x02) && uwater))
                {
                    DUDEEXTRA* pExtra = &gDudeExtra[pSpr->extra];
                    if (once)
                    {
                        for (i = 0; i < nBonkles; i++)
                        {
                            BONKLE* pBonk = BonkleCache[i];
                            for (j = 0; j < kCdudeMaxSounds; j++)
                            {
                                if (pBonk->sfxId == (int)id[j])
                                    return;
                            }
                        }
                    }

                    if (interruptable)
                    {
                        pExtra->clock = nClock;
                        Kill(pSpr);
                    }

                    if (ai)
                    {
                        if (pExtra->clock <= nClock)
                        {
                            sfxKill3DSound(pSpr, AI_SFX_PRIORITY_2, -1);
                            sfxPlay3DSoundCP(pSpr, nID, AI_SFX_PRIORITY_2, 0, 0x0, volume);
                            pExtra->sfx_priority = (AI_SFX_PRIORITY)AI_SFX_PRIORITY_2;
                            pExtra->clock = nClock + 384 + nRand;
                        }
                    }
                    else
                    {
                        sfxPlay3DSoundCP(pSpr, nID, -1, 0, 0x0, volume);
                    }
                }
            }
        }

        void Kill(spritetype* pSpr)
        {
            int i = kCdudeMaxSounds;
            while (--i >= 0)
                sfxKill3DSound(pSpr, -1, id[i]);
        }

        void KillOutside(spritetype* pSpr)
        {
            int i = nBonkles, j;
            while (--i >= 0)
            {
                BONKLE* pBonkle = BonkleCache[i];
                if (pBonkle->pSndSpr == pSpr)
                {
                    j = kCdudeMaxSounds;
                    while (--j >= 0)
                    {
                        if (pBonkle->sfxId == (signed int)id[j])
                            return;
                    }
                    
                    sfxKill3DSound(pSpr, -1, pBonkle->sfxId);
                }
            }
        }
};

class APPEARANCE
{
    public:
        CUSTOMDUDE_SOUND sound;
        unsigned short scl[2];
        unsigned int available      : 1;
        unsigned int soundAvailable : 1;
        unsigned int seq            : 20;
        unsigned int clb            : 4;
        unsigned int pic            : 16;
        unsigned int xrp            : 8;
        unsigned int yrp            : 8;
        signed   int xof            : 8;
        signed   int yof            : 8;
        signed   int pal            : 9;
        signed   int shd            : 9;
        void Clear(void)
        {
            Bmemset(this, 0, sizeof(APPEARANCE));
            pal = -1;   shd = 127;
        }

        void SetScale(spritetype* pSpr, XSPRITE* pXSpr)
        {
            if (available)
            {
                int nScale = 0;
                
                if (scl[1] > 0)
                    nScale = ClipRange(scl[1] + Random2(scl[1] - scl[0]), scl[0], scl[1]);
                else
                    nScale = scl[0];

                nnExtSprScaleSet(pSpr, nScale);

                if (pXSpr)
                    pXSpr->scale = nScale;
            }
        }

        void Set(spritetype* pSpr)
        {
            if (!available)
                return;
            
            if (!xspriRangeIsFine(pSpr->extra))
                dbInsertXSprite(pSpr->index);
            
            bool xs = true, ys = true, plu = true;
            XSPRITE* pXSpr = &xsprite[pSpr->extra];
            SEQINST* pInst; int nCallback = -1;
            int nSeq = seq;

            if (soundAvailable)
            {
                sound.KillOutside(pSpr);
                sound.Play(pSpr);
            }

            if (pic)
            {
                seqKill(OBJ_SPRITE, pSpr->extra);
                pSpr->picnum = pic;
                nSeq = -1;
            }

            if (nSeq >= 0)
            {
                pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                if (pInst && pInst->isPlaying)
                {
                    seqCanOverride(pInst->pSequence, 0, &xs, &ys, &plu);
                    nCallback = pInst->nCallbackID;
                }

                if (nSeq > 0)
                {
                    seqSpawn(nSeq, OBJ_SPRITE, pSpr->extra, clb ? -1 : nCallback);
                    pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                    if (pInst && pInst->isPlaying)
                        seqCanOverride(pInst->pSequence, 0, &xs, &ys, &plu);
                }

                if (clb && pInst)
                {
                    pXSpr->sysData2 = clb - 1;
                    if (pXSpr->sysData2 > 10)
                        pInst->nCallbackID = gCdudeCustomCallback[pXSpr->sysData2];
                    else
                        pInst->nCallbackID  = nCdudeAppearanceCallback;
                }
            }

            if (shd != 127)
            {
                seqKill(OBJ_SPRITE, pSpr->extra);
                pSpr->shade = shd;
            }

            if (pal >= 0)
            {
                if (!plu)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->pal = pal;
            }

            if (xrp)
            {
                if (!xs)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->xrepeat = xrp;
            }

            if (yrp)
            {
                if (!ys)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->yrepeat = yrp;
            }

            if (xof) pSpr->xoffset = xof;
            if (yof) pSpr->yoffset = yof;

            if (scl[0])
                SetScale(pSpr, pXSpr);
        }
};

class CUSTOMDUDE_WEAPON
{
    public:
        unsigned int  type                  : 4;
        unsigned int  numshots              : 6;
        unsigned int  id                    : 10;
        unsigned int  sharedId              : 4;
        unsigned int  angle                 : 12;
        unsigned int  medium                : 3;
        unsigned int  pickChance            : 20;
        unsigned int  available             : 1;
        unsigned int  posture               : 3;
        unsigned int  interruptable         : 1;
        unsigned int  turnToTarget          : 1;
        unsigned int  stateID               : 8;
        unsigned int  nextStateID           : 8;
        unsigned int  clipMask              : 32;
        unsigned int  group                 : 4;
        unsigned int  dispersion[2];
        unsigned int  distRange[2];
        unsigned char targHpRange[2];
        unsigned char dudeHpRange[2];
        CUSTOMDUDE_SOUND sound;
        struct SHOT
        {
            unsigned int velocity           : 32;
            signed   int slope              : 32;
            unsigned int targetFollow       : 12;
            unsigned int clipdist           : 8;
            unsigned int impact             : 2;
            signed   int remTime            : 14;
            APPEARANCE appearance;
            POINT3D offset;
        }
        shot;
        struct AMMO
        {
            unsigned int cur, total         : 16;
            void SetTotal(int nVal) { total = nVal; }
            void SetFull()          { Set(total); }
            void Set(int nVal)      { cur = ClipRange(nVal, 0, total); }
            void Inc(int nBy = 1)   { Set(cur + nBy); }
            void Dec(int nBy = 1)   { Set(cur - nBy); }
        }
        ammo;
        struct COOLDOWN
        {
            unsigned int clock              : 32;
            unsigned int delay              : 16;
            unsigned int useCount           : 16;
            unsigned int totalUseCount      : 16;
            void SetTimer(void)
            {
                clock = ClipLow(delay * kTicsPerFrame, kTicsPerFrame);
                clock += (unsigned int)gFrameClock;
            }

            char Check(void)
            {
                if ((unsigned int)gFrameClock < clock)
                    return 2;
                
                if (totalUseCount)
                {
                    useCount = ClipHigh(useCount + 1, totalUseCount);
                    if (useCount < totalUseCount)
                        return 0;
                }
                
                if (delay)
                {
                    SetTimer();
                    return 1;
                }

                return 0;
            }
        }
        cooldown;
        struct SHOT_STYLE
        {
            unsigned int available  : 1;
            unsigned int angle      : 12;
            POINT3D offset;
        }
        style;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_WEAPON));
            
            angle           = kAng45;
            numshots        = 1;
            pickChance      = 0x10000;
            stateID         = kCdudeStateAttackBase;
            turnToTarget    = true;

            distRange[1]    = 20000;
            dudeHpRange[1]  = 255;
            targHpRange[1]  = 255;

            shot.remTime    = -1;
            shot.velocity = INT32_MAX;
            shot.slope    = INT32_MAX;
        }
        char HaveAmmmo(void)        { return (!ammo.total || ammo.cur); }
        int  GetDistance(void)      { return ClipLow(distRange[1] - distRange[0], 0); }
        int  GetNumshots(void)      { return (ammo.total) ? ClipHigh(ammo.cur, numshots) : numshots; }
        char IsTimeout(void)        { return ((unsigned int)gFrameClock < cooldown.clock); }
        char HaveSlope(void)        { return (shot.slope != INT32_MAX); }
        char HaveVelocity(void)     { return (shot.velocity != INT32_MAX); }

};

class CUSTOMDUDE_GIB
{
    public:
        unsigned int available      : 1;
        unsigned int force          : 1;
        unsigned int trFlags        : 8;
        unsigned int physics        : 4;
        unsigned int thingType      : 8;
        unsigned int data1          : 20;
        unsigned int data2          : 20;
        unsigned int data3          : 20;
        unsigned int data4          : 20;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_GIB));
            physics     = (kPhysMove | kPhysGravity | kPhysFalling);
            thingType   = kThingObjectExplode - kThingBase;
            force       = true;
        }

        void Setup(spritetype* pSpr)
        {
            int nStat = pSpr->statnum;
            if (pSpr->statnum != kStatThing)
            {
                if (!force)
                    return;

                evKill(pSpr->index, OBJ_SPRITE, kCallbackRemove);
                ChangeSpriteStat(pSpr->index, kStatThing);
            }

            if (pSpr->extra <= 0)
                dbInsertXSprite(pSpr->index);

            XSPRITE* pXSpr = &xsprite[pSpr->extra];
            pSpr->type  = kThingBase + thingType;
            pSpr->flags &= ~(kPhysMove | kPhysGravity | kPhysFalling);
            pSpr->flags |= physics;
            
            if (!(pSpr->flags & kPhysMove))
                xvel[pSpr->index] = yvel[pSpr->index] = 0;

            if (nStat == kStatFX)
            {
                // scale velocity a bit
                if (zvel[pSpr->index])
                    zvel[pSpr->index] += perc2val(35, zvel[pSpr->index]);
            }

            pXSpr->health       = thingInfo[pSpr->type - kThingBase].startHealth << 4;
            pXSpr->data1        = data1;
            pXSpr->data2        = data2;
            pXSpr->data3        = data3;
            pXSpr->data4        = data4;

            pXSpr->isTriggered  = false;
            pXSpr->triggerOnce  = true;
            pXSpr->state        = 1;

            if (trFlags & kParTrigVector)
            {
                pSpr->cstat |= CSTAT_SPRITE_BLOCK_HITSCAN;
                pXSpr->Vector = true;
            }

            if (trFlags & kParTrigTouch)
            {
                pSpr->cstat |= CSTAT_SPRITE_BLOCK;
                pXSpr->Touch = true;
            }
            
            if (trFlags & kParTrigImpact)
                pXSpr->Impact = true;

            pXSpr->locked = true;
            if (!(trFlags & kParTrigLocked))
                evPost(pSpr->index, OBJ_SPRITE, 16, (COMMAND_ID)kCmdUnlock, kCauserGame);
        }
};

class CUSTOMDUDE_EFFECT
{
    public:
        unsigned short id[kCdudeMaxEffects];
        unsigned int clock          : 32;
        signed   int liveTime       : 32;
        signed   int velocity       : 32;
        signed   int velocitySlope  : 32;
        signed   int angle          : 16;
        unsigned int posture        : 3;
        unsigned int medium         : 3;
        unsigned int allUnique      : 1;
        unsigned int srcVelocity    : 1;
        unsigned int chance         : 20;
        unsigned short delay[2];
        CUSTOMDUDE_GIB spr2gib;
        APPEARANCE appearance;
        IDLIST* pAnims;
        IDLIST* pFrames;
        IDLIST* pStates;
        POINT3D offset;
        void Clear()
        {
            if (pAnims)     delete(pAnims);
            if (pFrames)    delete(pFrames);
            if (pStates)    delete(pStates);

            Bmemset(this, 0, sizeof(CUSTOMDUDE_EFFECT));
            angle       = kAng360;
            velocity    = -1;
            chance      = 0x10000;
            srcVelocity = 1;

            pAnims  = new IDLIST(true);
            pFrames = new IDLIST(true);
            pStates = new IDLIST(true);
        }

        char CanSpawn(spritetype* pSpr)
        {
            int nFrame = 1;
            int nACount = pAnims->Length();
            int nFCount = pFrames->Length();
            if (nACount || nFCount)
            {
                SEQINST* pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                if (pInst)
                {
                    if (pInst->isPlaying)
                    {
                        if (nACount && !pAnims->Exists(pInst->nSeq))
                            return false;

                        nFrame = pInst->frameIndex + 1;
                    }
                    else
                        return false;
                }
                else if (nACount)
                {
                    return false;
                }

                if (nFCount)
                    return pFrames->Exists(nFrame);
            }

            return true;
        }

        void SetDelay(void)
        {
            if (delay[1] > 0)
            {
                clock = ClipRange(delay[1] + Random2(delay[1] - delay[0]), delay[0], delay[1]);
            }
            else
            {
                clock = delay[0];
            }

            clock = ClipLow(clock * kTicsPerFrame, kTicsPerFrame);
            clock += (unsigned int)gFrameClock;
        }

        void Setup(spritetype* pSrc, spritetype* pEff, char relVel)
        {
            int dx = 0, dy = 0, dz = velocitySlope;
            int nAng = ((angle != kAng360) ? (pSrc->ang + angle) : Random2(kAng360)) & kAngMask;
            int nVel = velocity;
            int rp = Random(15);

            pEff->owner = pSrc->index;

            dz   += Random2(perc2val(rp, dz));
            nAng += Random2(perc2val(rp>>1, nAng)) & kAngMask;
            pEff->ang = nAng;

            appearance.Set(pEff);

            if (nVel >= 0)
            {
                nVel += Random2(perc2val(rp, nVel));
                dx = (Cos(nAng) >> 16);
                dy = (Sin(nAng) >> 16);
                
                if (nVel == 0)
                    relVel = false;
            }
            else
            {
                nVel = klabs(dz);
            }

            if (relVel)
            {
                nnExtScaleVelocityRel(pEff, nVel, dx, dy, dz);
            }
            else
            {
                nnExtScaleVelocity(pEff, nVel, dx, dy, dz);
            }

            if (srcVelocity)
            {
                xvel[pEff->index] += xvel[pSrc->index];
                yvel[pEff->index] += yvel[pSrc->index];
                zvel[pEff->index] += zvel[pSrc->index];
            }

            if (spr2gib.available)
                spr2gib.Setup(pEff);

            if (liveTime)
            {
                evKill(pEff->index, OBJ_SPRITE, kCallbackRemove);
                if (liveTime > 0)
                    evPost(pEff->index, OBJ_SPRITE, liveTime, kCallbackRemove);
            }
        }

        void Spawn(int nID, spritetype* pSpr)
        {
            spritetype* pFX;
            int x = pSpr->x, y = pSpr->y, z = pSpr->z;
            nnExtOffsetPos(&offset, pSpr->ang, &x, &y, &z);

            if (nID >= kCdudeGIBEffectBase)
            {
                nID -= kCdudeGIBEffectBase;
                
                IDLIST fxList(true); CGibPosition fxPos = { x, y, z };
                if (nnExtGibSprite(pSpr, &fxList, (GIBTYPE)nID, &fxPos, NULL))
                {
                    int32_t* pDb = fxList.First();
                    while (*pDb != kListEndDefault)
                    {
                        pFX = &sprite[*pDb++];
                        Setup(pSpr, pFX, true);
                    }
                }
                
                fxList.Free();
            }
            else if (nID >= kCudeFXEffectCallbackBase)
            {
                nID = gCdudeCustomCallback[nID - kCudeFXEffectCallbackBase];
                evKill(pSpr->index, OBJ_SPRITE, (CALLBACK_ID)nID);
                evPost(pSpr->index, OBJ_SPRITE, 0, (CALLBACK_ID)nID);
            }
            else
            {
                nID -= kCdudeFXEffectBase;
                if ((pFX = gFX.fxSpawn((FX_ID)nID, pSpr->sectnum, x, y, z)) != NULL)
                    Setup(pSpr, pFX, false);
            }
        }
        
        void Spawn(spritetype* pSpr)
        {
            if (Chance(chance))
            {
                if (allUnique)
                {
                    for (int i = 0; i < kCdudeMaxEffects; i++)
                        Spawn(id[i], pSpr);
                }
                else
                {
                    Spawn(Pick(), pSpr);
                }
            }
        }

        int Pick() { return id[Random(kCdudeMaxEffects)]; }
};


class  CUSTOMDUDE_DAMAGE
{
    public:
        unsigned short id[kDmgMax];
        unsigned int ignoreSources : 8;
        void Set(int nVal, int nFor) { id[nFor] = ClipRange(nVal, 0, kCdudeMaxDmgScale); }
        void Inc(int nVal, int nFor) { Set(id[nFor] + abs(nVal), nFor); }
        void Dec(int nVal, int nFor) { Set(id[nFor] - abs(nVal), nFor); }
};

class  CUSTOMDUDE_DODGE
{
    public:
        struct
        {
            unsigned int times  : 10;
            unsigned int timer  : 32;
            unsigned int chance : 20;
            unsigned int dmgReq : 17;
            char Allow(int nDamage)
            {
                if (nDamage > dmgReq)
                {
                    unsigned int nClock = (unsigned int)gFrameClock;
                    unsigned int nChance = chance;

                    if (timer > nClock)
                    {
                        times += (5 - gGameOptions.nDifficulty);
                        nChance = ClipHigh(perc2val(times, nChance), nChance);
                        return Chance(nChance);
                    }

                    times = 0;
                    timer = nClock + kCdudeDmgCheckDelay;
                    return Chance(nChance);
                }

                return 0;
            }
        }
        onDamage;
        struct
        {
            unsigned int chance : 20;
            char Allow(void) { return Chance(chance); }
        }
        onAimMiss;
};

class  CUSTOMDUDE_RECOIL
{
    public:
        unsigned int times  : 10;
        unsigned int timer  : 32;
        unsigned int chance : 20;
        unsigned int dmgReq : 17;
        char Allow(int nDamage)
        {
            if (nDamage > dmgReq)
            {
                unsigned int nClock = (unsigned int)gFrameClock;
                unsigned int nChance = chance;

                if (timer > nClock)
                {
                    times += (5 - gGameOptions.nDifficulty);
                    nChance = ClipHigh(perc2val(times, nChance), nChance);
                    return Chance(nChance);
                }

                times = 0;
                timer = nClock + kCdudeDmgCheckDelay;
                return Chance(nChance);
            }

            return 0;
        }
};

class  CUSTOMDUDE_KNOCKOUT
{
    public:
        unsigned int times  : 10;
        unsigned int timer  : 32;
        unsigned int chance : 20;
        unsigned int dmgReq : 17;
        char Allow(int nDamage)
        {
            if (nDamage > dmgReq)
            {
                unsigned int nClock = (unsigned int)gFrameClock;
                unsigned int nChance = chance;

                if (timer > nClock)
                {
                    times += (5 - gGameOptions.nDifficulty);
                    nChance = ClipHigh(perc2val(times, nChance), nChance);
                    return Chance(nChance);
                }

                times = 0;
                timer = nClock + kCdudeDmgCheckDelay;
                return Chance(nChance);
            }

            return 0;
        }
};

class CUSTOMDUDE_VELOCITY
{
    public:
        unsigned int id[kParVelocityMax];
        void Set(int nVal, int nFor) { id[nFor] = ClipRange(nVal, 0, kCdudeMaxVelocity); }
        void Inc(int nVal, int nFor) { Set(id[nFor] + abs(nVal), nFor); }
        void Dec(int nVal, int nFor) { Set(id[nFor] - abs(nVal), nFor); }
};

class CUSTOMDUDE_DROPITEM
{
    public:
        unsigned char items[kCdudeMaxDropItems][2];
        unsigned int  sprDropItemChance : 20;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_DROPITEM));
            sprDropItemChance = 0x10000;
        }

        int Pick(XSPRITE* pXSpr, IDLIST* pOut)
        {
            unsigned char nItem;
            unsigned char nPerc;
            int i;

            // add key
            if (pXSpr->key)
                pOut->AddIfNotExists(kItemKeyBase + (pXSpr->key - 1));

            // add item
            if (pXSpr->dropMsg && Chance(sprDropItemChance))
                pOut->AddIfNotExists(pXSpr->dropMsg);

            // add all items with 100% chance
            for (i = 0; i < kCdudeMaxDropItems; i++)
            {
                nItem = items[i][0];
                nPerc = items[i][1];
                if (nItem && nPerc >= 100)
                    pOut->AddIfNotExists(nItem);
            }

            // add item with < 100% chance
            while(--i >= 0)
            {
                nItem = items[i][0];
                nPerc = items[i][1];
                if (nItem)
                {
                    if (nPerc < 100 && Chance(perc2val(nPerc, 0x10000)))
                    {
                        pOut->AddIfNotExists(nItem);
                        break;
                    }
                }
            }

            return pOut->Length();
        }
};

class CUSTOMDUDE
{
    public:
        unsigned int version                            : 2;
        unsigned int initialized                        : 1;
        unsigned int numEffects                         : 5;
        unsigned int numWeapons                         : 5;
        DUDEEXTRA* pExtra; DUDEINFO* pInfo;
        spritetype* pSpr; XSPRITE*pXSpr, *pXLeech;
        CUSTOMDUDE_WEAPON    weapons[kCdudeMaxWeapons];                             // the weapons it may have
        CUSTOMDUDE_WEAPON*   pWeapon;                                               // pointer to current weapon
        CUSTOMDUDE_DAMAGE    damage;                                                // damage control
        CUSTOMDUDE_VELOCITY  velocity[kCdudePostureMax];                            // velocity control
        CUSTOMDUDE_SOUND     sound[kCdudeSndMax];                                   // ai state sounds
        CUSTOMDUDE_DODGE     dodge;                                                 // dodge control
        CUSTOMDUDE_RECOIL    recoil;                                                // recoil control
        CUSTOMDUDE_KNOCKOUT  knockout;                                              // knock control
        CUSTOMDUDE_DROPITEM  dropItem;                                              // drop item control
        CUSTOMDUDE_EFFECT    effects[kCdudeMaxEffectGroups];                        // fx, gib effect stuff
        AISTATE states[kCdudeStateMax][kCdudePostureMax];                           // includes states for weapons
        IDLIST* pSlaves;                                                            // summoned dudes under control of this dude
        unsigned int medium                             : 3;                        // medium in which it can live
        unsigned int posture                            : 3;                        // current posture
        unsigned int mass                               : 20;                       // mass in KG
        unsigned int largestPic                         : 16;                       // in all states to compare on crouching
        unsigned int prevSector                         : 16;                       // the recent sector dude was in
        unsigned int seeDist                            : 20;                       // dudeInfo duplicate for sleeping
        unsigned int hearDist                           : 20;                       // dudeInfo duplicate for sleeping
        unsigned int periphery                          : 12;                       // dudeInfo duplicate for sleeping
        unsigned int fallHeight                         : 32;                       // in pixels
        signed   int nextDude                           : 32;                       // -1: none, <-1: vdude, >=0: ins, >=kMaxSprites: cdude
        //----------------------------------------------------------------------------------------------------
        FORCE_INLINE void PlaySound(int nState)                     { return (sound[nState].Play(pSpr)); }
        FORCE_INLINE int  GetStateSeq(int nState, int nPosture)     { return states[nState][nPosture].seqId; }
        FORCE_INLINE int  GetVelocity(int nPosture, int nVelType)   { return velocity[nPosture].id[nVelType]; }
        FORCE_INLINE int  GetVelocity(int nVelType)                 { return GetVelocity(posture, nVelType); }
        FORCE_INLINE char IsUnderwater(void)                        { return (pXSpr->medium != kMediumNormal); }
        FORCE_INLINE char IsCrouching(void)                         { return (posture == kCdudePostureC); }
        FORCE_INLINE char SeqPlaying(void)                          { return (seqGetStatus(OBJ_SPRITE, pSpr->extra) >= 0); }
        FORCE_INLINE char IsAttacking(void)                         { return (pXSpr->aiState->stateType == kAiStateAttack); }
        FORCE_INLINE char IsKnockout(void)                          { return (pXSpr->aiState->stateType == kAiStateKnockout); }
        FORCE_INLINE char IsRecoil(void)                            { return (pXSpr->aiState->stateType == kAiStateRecoil); }
        FORCE_INLINE char IsBurning(void)                           { return StatusTest(kCdudeStatusBurning); }
        FORCE_INLINE char IsMorphing(void)                          { return StatusTest(kCdudeStatusMorph); }
        FORCE_INLINE char IsDying(void)                             { return StatusTest(kCdudeStatusDying); }
        FORCE_INLINE char IsSleeping(void)                          { return StatusTest(kCdudeStatusSleep); }
        FORCE_INLINE char IsLeechBroken(void)                       { return (pXLeech && pXLeech->locked); }
        // ---------------------------------------------------------------------------------------------------
        FORCE_INLINE void StatusSet(int nStatus)                    { pXSpr->sysData3 |= nStatus; }
        FORCE_INLINE void StatusRem(int nStatus)                    { pXSpr->sysData3 &= ~nStatus; }
        FORCE_INLINE char StatusTest(int nStatus)                   { return ((pXSpr->sysData3 & nStatus) > 0); }
        //----------------------------------------------------------------------------------------------------
        FORCE_INLINE char CanRecoil(void)                           { return (GetStateSeq(kCdudeStateRecoil, posture) > 0); }
        FORCE_INLINE char CanElectrocute(void)                      { return (GetStateSeq(kCdudeStateRecoilT, posture) > 0); }
        FORCE_INLINE char CanKnockout(void)                         { return (GetStateSeq(kCdudeStateKnock, posture)); }
        FORCE_INLINE char CanBurn(void)                             { return (GetStateSeq(kCdudeBurnStateSearch, posture) > 0); }
        FORCE_INLINE char CanCrouch(void)                           { return (GetStateSeq(kCdudeStateSearch, kCdudePostureC) > 0); }
        FORCE_INLINE char CanSwim(void)                             { return (GetStateSeq(kCdudeStateSearch, kCdudePostureW) > 0); }
        FORCE_INLINE char CanSleep(void)                            { return (!StatusTest(kCdudeStatusAwaked) && GetStateSeq(kCdudeStateSleep, posture) > 0); }
        FORCE_INLINE char CanMove(void)                             { return (GetStateSeq(kCdudeStateSearch, posture) > 0); }
        //----------------------------------------------------------------------------------------------------
        int  GetDamage(int nSource, int nDmgType);
        char IsPostureMatch(int nPosture);
        char IsMediumMatch(int nMedium);
        char IsTooTight(void);
        //----------------------------------------------------------------------------------------------------
        CUSTOMDUDE_WEAPON* PickWeapon(ARG_PICK_WEAPON* pArg);
        int  AdjustSlope(int nTarget, int zOffs);
        char AdjustSlope(int nDist, int* nSlope);
        //----------------------------------------------------------------------------------------------------
        void InitSprite(void);
        void Activate(void);
        void Process(void);
        void ProcessEffects(void);
        void Recoil(void);
        int  Damage(int nFrom, int nDmgType, int nDmg);
        void Kill(int nFrom, int nDmgType, int nDmg);
        //----------------------------------------------------------------------------------------------------
        char CanMove(XSECTOR* pXSect, char Crusher, char Water, char Uwater, char Depth, int bottom, int floorZ);
        char FindState(AISTATE* pState, int* nStateType, int* nPosture);
        void NewState(int nStateType, int nTimeOverride = -1);
        char NewState(AISTATE* pState);
        void NextState(int nStateType, int nTimeOverride = 0);
        void NextState(AISTATE* pState, int nTimeOverride = 0);
        AISTATE* PickDeath(int nDmgType);
        void SyncState(void);
        //----------------------------------------------------------------------------------------------------
        void LeechPickup(void);
        void LeechKill(char delSpr);
        void UpdateSlaves(void);
        void DropItems(void);
        void ClearEffectCallbacks(void);
        //----------------------------------------------------------------------------------------------------
};

class CUSTOMDUDEV1_SETUP;
class CUSTOMDUDEV2_SETUP;
class CUSTOMDUDE_SETUP
{
        friend CUSTOMDUDEV1_SETUP;
        friend CUSTOMDUDEV2_SETUP;
    private:
        static const char* pValue;  static DICTNODE* hIni;
        static CUSTOMDUDE* pDude;   static IniFile* pIni;
        static PARAM* pGroup;       static PARAM* pParam;
        static char key[256];       static char val[256];
        static int nWarnings;       static char showWarnings;
        /*------------------------------------------------------------*/
        static int FindParam(const char* str, PARAM* pDb);
        static PARAM* FindParam(int nParam, PARAM* pDb);
        static int ParamLength(PARAM* pDb);
        /*-------------------------------------------------*/
        static char DescriptLoad(int nID);
        static void DescriptClose(void);
        static char DescriptGroupExist(const char* pGroupName);
        static char DescriptParamExist(const char* pGroupName, const char* pParamName);
        static int  DescriptCheck(void);
        static const char* DescriptGetValue(const char* pGroupName, const char* pParamName);
        /*------------------------------------------------------------*/
        static void Warning(const char* pFormat, ...);
        static const char* GetValType(int nType);
        static const char* GetError(int nErr);
        /*------------------------------------------------------------*/
        static void DamageSetDefault(void);
        static void DamageScaleToSkill(int nSkill);
        /*------------------------------------------------------------*/
        static void VelocitySetDefault(int nMaxVel);
        /*------------------------------------------------------------*/
        static void WeaponSoundSetDefault(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponDispersionSetDefault(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponRangeSet(CUSTOMDUDE_WEAPON* pWeapon, int nMin, int nMax);
        /*------------------------------------------------------------*/
        static void AnimationConvert(int baseID);
        static void AnimationFill(AISTATE* pState, int nAnim);
        static void AnimationFill(void);
        /*------------------------------------------------------------*/
        static void SoundConvert(int baseID);
        static void SoundFill(CUSTOMDUDE_SOUND* pSound, int nSnd);
        static void SoundFill(void);
        /*------------------------------------------------------------*/
        static void FindLargestPic(void);
        static void RandomizeDudeSettings(void);
        static void SetupSlaves(void);
        static void SetupLeech(void);
        /*------------------------------------------------------------*/
        static CUSTOMDUDE* SameDudeExist(CUSTOMDUDE* pCmp);
        static CUSTOMDUDE* GetFirstDude(int nID);
        static char IsFirst(CUSTOMDUDE* pCmp);
    public:
        static char FindAiState(AISTATE stateArr[][kCdudePostureMax], int arrLen, AISTATE* pNeedle, int* nType, int* nPosture);
        static void Setup(spritetype* pSpr, XSPRITE* pXSpr);
        static void Setup(CUSTOMDUDE* pOver = NULL);

};

class CUSTOMDUDEV1_SETUP : CUSTOMDUDE_SETUP
{
    private:
        static void DamageScaleToSurface(int nSurface);
        static void DamageScaleToWeapon(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponMeleeSet(CUSTOMDUDE_WEAPON* pWeap);
        static void WeaponConvert(int nWeaponID);
        static void SetupIncarnation(void);
        static void SetupBasics(void);
        static void SetupDamage(void);
    public:
        static void Setup(void);
};

class CUSTOMDUDEV2_SETUP : CUSTOMDUDE_SETUP
{
    private:
        static char ParseVelocity(const char* str, CUSTOMDUDE_VELOCITY* pVelocity);
        static char ParseAppearance(const char* str, APPEARANCE* pAppear);
        static char ParseSound(const char* str, CUSTOMDUDE_SOUND* pSound);
        static char ParseAnimation(const char* str, AISTATE* pState, char asPosture);
        static char ParseRange(const char* str, int nValType, int out[2], int nBaseVal = 0);
        static int  ParseMedium(const char* str);
        static char ParseOffsets(const char* str, POINT3D* pOut);
        static char ParseShotSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseAttackSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponStyle(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponBasicInfo(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParsePosture(const char* str);
        static char ParseOnEventDmg(const char* str, int* pOut, int nLen);
        static char ParseDropItem(const char* str, unsigned char out[2]);
        static char ParseSkill(const char* str);
        static int  ParseKeywords(const char* str, PARAM* pDb);
        static int  ParseIDs(const char* str, int nValType, IDLIST* pOut, int nMax = 0);
        static int  ParseIDs(const char* str, int nValType, int* pOut, int nMax);
        static int  ParseEffectIDs(const char* str, const char* paramName, unsigned short* pOut, int nLen = 0);
        static int  ParseStatesToList(const char* str, IDLIST* pOut);
        static char ParseGibSetup(const char* str, CUSTOMDUDE_GIB* pOut);
        /*-------------------------------------------------*/
        static int  CheckArray(const char* str, int nMin = 0, int nMax = 0, int nDefault = 1);
        static int  CheckValue(const char* str, int nValType, int nDefault);
        static int  CheckValue(const char* str, int nValType, int nMin, int nMax);
        static int  CheckValue(const char* str, int nValType, int nMin, int nMax, int nDefault);
        static int  CheckRange(const char* str, int nVal, int nMin, int nMax);
        static int  CheckParam(const char* str, PARAM* pDb);
        /*-------------------------------------------------*/
        static void SetupGeneral(void);
        static void SetupVelocity(void);
        static void SetupAnimation(AISTATE* pState, char asPosture);
        static void SetupAnimation(void);
        static void SetupSound(CUSTOMDUDE_SOUND* pSound);
        static void SetupMovePattern(void);
        static void SetupSound(void);
        static void SetupDamage(void);
        static void SetupRecoil(void);
        static void SetupDodge(void);
        static void SetupKnockout(void);
        static void SetupWeapons(void);
        static void SetupEffect(void);
        static void SetupDropItem(void);
    public:
        static void Setup(void);
};

void cdudeFree();
CUSTOMDUDE* cdudeAlloc();
CUSTOMDUDE* cdudeGet(int nIndex);
FORCE_INLINE char IsCustomDude(spritetype* pSpr)        { return (pSpr->type == kDudeModernCustom); }
FORCE_INLINE CUSTOMDUDE* cdudeGet(spritetype* pSpr) { return cdudeGet(pSpr->index); };
spritetype* cdudeSpawn(XSPRITE* pXSource, spritetype* pSprite, int nDist);
void cdudeLeechOperate(spritetype* pSprite, XSPRITE* pXSprite);
#endif