//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

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


////////////////////////////////////////////////////////////////////////////////////
// This file provides modern features for mappers.
// For full documentation please visit http://cruo.bloodgame.ru/xxsystem
/////////////////////////////////////////////////////////////////////////


#ifdef NOONE_EXTENSIONS
#pragma once
#include "common_game.h"
#include "eventq.h"
#include "qav.h"
#include "actor.h"
#include "dude.h"
#include "player.h"
#include "warp.h"
#include "triggers.h"
#include "ai.h"
#include "loadsave.h"
#include "seq.h"
#include "nnextstr.h"
#include "gib.h"

// CONSTANTS
#define LENGTH(x) 					        (int)(sizeof(x) / sizeof(x[0]))
#define EVTIME2TICKS(x)                     ((x * 120) / 10)

#define TRIGGER_START_CHANNEL_NBLOOD kChannelLevelStartNBLOOD  // uncomment only for Nblood
//#define TRIGGER_START_CHANNEL_RAZE kChannelLevelStartRAZE  // uncomment only for Raze

#define kMaxSuperXSprites    0  // 0 means no limit

// additional physics attributes for debris sprites
#define kPhysDebrisFloat 0x0008 // *debris* slowly goes up and down from it's position
#define kPhysDebrisFly 0x0010 // *debris* affected by negative gravity (fly instead of falling)
#define kPhysDebrisSwim 0x0020 // *debris* can swim underwater (instead of drowning)
#define kPhysDebrisTouch 0x0040 // *debris* can be moved via touch
//#define kPhysDebrisPush 0x0080 // *debris* can be moved via push
#define kPhysDebrisVector 0x0400 // *debris* can be affected by vector weapons
#define kPhysDebrisExplode 0x0800 // *debris* can be affected by explosions

// *modern types only hitag*
#define kModernTypeFlag0 0x0000
#define kModernTypeFlag1 0x0001
#define kModernTypeFlag2 0x0002
#define kModernTypeFlag3 0x0003
#define kModernTypeFlag4 0x0004
#define kModernTypeFlag8 0x0008
#define kModernTypeFlag16 0x0010
#define kModernTypeFlag32 0x0020
#define kModernTypeFlag64 0x0040
#define kModernTypeFlag128 0x0080
#define kModernTypeFlag256 0x0100

#define kMaxRandomizeRetries 16
#define kPercFull 100
#define kPatrolStateSize 50
#define kPatrolAlarmSeeDist 10000
#define kPatrolAlarmHearDist 10000
#define kMaxPatrolVelocity 500000
#define kMaxPatrolCrouchVelocity kMaxPatrolVelocity >> 1
#define kMaxPatrolSpotValue 500
#define kMinPatrolTurnDelay 8
#define kPatrolTurnDelayRange 20

#define kDudeFlagStealth    0x0001
#define kDudeFlagCrouch     0x0002

#define kSlopeDist 0x20
#define kEffectGenCallbackBase 200
#define kTriggerSpriteScreen 0x0001
#define kTriggerSpriteAim    0x0002

#define kMinAllowedPowerup kPwUpFeatherFall
#define kMaxAllowedPowerup kMaxPowerUps

// modern statnums
enum {
kStatModernBase                     = 20,
kStatModernDudeTargetChanger        = kStatModernBase,
kStatModernCondition                = 21,
kStatModernEventRedirector          = 22,
kStatModernPlayerLinker             = 23,
kStatModernBrokenDudeLeech          = 24,
kStatModernQavScene                 = 25,
kStatModernStealthRegion            = 27,
kStatModernTmp                      = 39,
kStatModernMax                      = 40,
};

// modern sprite types
enum {
kModernStealthRegion                = 16,
kModernCustomDudeSpawn              = 24,
kModernRandomTX                     = 25,
kModernSequentialTX                 = 26,
kModernSeqSpawner                   = 27,
kModernObjPropertiesChanger         = 28,
kModernObjPicnumChanger             = 29,
kModernObjSizeChanger               = 31,
kModernDudeTargetChanger            = 33,
kModernSectorFXChanger              = 34,
kModernObjDataChanger               = 35,
kModernSpriteDamager                = 36,
kModernObjDataAccumulator           = 37,
kModernEffectSpawner                = 38,
kModernWindGenerator                = 39,
kModernRandom                       = 40,
kModernRandom2                      = 80,
kItemShroomGrow                     = 129,
kItemShroomShrink                   = 130,
kItemModernMapLevel                 = 150,  // once picked up, draws whole minimap
kDudeModernCustom                   = kDudeVanillaMax,
kModernThingTNTProx                 = 433, // detects only players
kModernThingThrowableRock           = 434, // does small damage if hits target
kModernThingEnemyLifeLeech          = 435, // the same as normal, except it aims in specified target only
kModernPlayerControl                = 500, /// WIP
kModernCondition                    = 501, /// WIP, sends command only if specified conditions == true
kModernConditionFalse               = 502, /// WIP, sends command only if specified conditions != true
kModernSlopeChanger                 = 504,
kModernVelocityChanger              = 506,
kGenModernMissileUniversal          = 704,
kGenModernSound                     = 708,
};

// type of random
enum {
kRandomizeItem                      = 0,
kRandomizeDude                      = 1,
kRandomizeTX                        = 2,
};

// type of object
enum {
OBJ_WALL                            = 0,
OBJ_SPRITE                          = 3,
OBJ_SECTOR                          = 6,
};


enum {
kPatrolMoveForward                  = 0,
kPatrolMoveBackward                 = 1,
};

// - STRUCTS ------------------------------------------------------------------
#pragma pack(push, 1)
struct OBJECT
{
    unsigned int type : 8;
    unsigned int index : 16;
};

// sprite mass info
struct SPRITEMASS
{
    signed   int seqId      : 32;
    unsigned int picnum     : 16;
    unsigned int xrepeat    : 8;
    unsigned int yrepeat    : 8;
    unsigned int clipdist   : 8; // mass multiplier
    unsigned int mass       : 32;
    unsigned int airVel     : 10;
    unsigned int fraction   : 16;
};

struct EXPLOSION_EXTRA
{
    unsigned int seq : 5;
    unsigned int snd : 9;
    unsigned int ground : 1;
};

struct THINGINFO_EXTRA {
    bool allowThrow; // indicates if kDudeModernCustom can throw it
};

struct VECTORINFO_EXTRA {
    int fireSound[2]; // predefined fire sounds. used by kDudeModernCustom, but can be used for something else.
};

struct MISSILEINFO_EXTRA {
    int fireSound[2]; // predefined fire sounds. used by kDudeModernCustom, but can be used for something else.
    bool dmgType[kDamageMax]; // list of damages types missile can use
    bool allowImpact; // allow to trigger object with Impact flag enabled with this missile
};

struct DUDEINFO_EXTRA {
    bool flying;                    // used by kModernDudeTargetChanger (ai fight)
    bool melee;                     // used by kModernDudeTargetChanger (ai fight)
    int idlgseqofs : 6;             // used for patrol
    int mvegseqofs : 6;             // used for patrol
    int idlwseqofs : 6;             // used for patrol
    int mvewseqofs : 6;             // used for patrol
    int idlcseqofs : 6;             // used for patrol
    int mvecseqofs : 6;             // used for patrol
    
};

struct PATROL_FOUND_SOUNDS {

    int snd;
    int max;
    int cur;

};

struct OBJECT_STATUS1
{
    struct
    {
        unsigned int ok : 1;
    }
#if kMaxSprites >= kMaxSectors
    id[kMaxSprites];
#else
    id[kMaxSectors];
#endif
};

#pragma pack(pop)

struct QAVSCENE { // this one stores qavs anims that can be played by trigger
    QAV* qavResrc = NULL;
    short index = -1;  // index of sprite which triggered qav scene
    short dummy = -1;
};

struct TRPLAYERCTRL { // this one for controlling the player using triggers (movement speed, jumps and other stuff)
    QAVSCENE qavScene;
};

struct EXTERNAL_FILES_LIST
{
    const char* name;
    const char* ext;
};


inline bool rngok(int val, int rngA, int rngB) { return (val >= rngA && val < rngB); }
inline bool irngok(int val, int rngA, int rngB) { return (val >= rngA && val <= rngB); }
inline bool mapRev1() { return (gModernMap == 1); }
inline bool mapRev2() { return (gModernMap == 2); }

// - CLASSES ------------------------------------------------------------------
#define kListEndDefault     -1

enum {
kListSKIP                   = 0,
kListOK                     = 1,
kListREMOVE                 = 2,
};

class IDLIST
{
private:
    int32_t*  db;
    int32_t   length;
    int32_t   limit;
    int32_t   EOL;
public:
    ~IDLIST() { Free(); }
    IDLIST(bool spawnDb, int nEOL = kListEndDefault, int nLimit = 0)
    {
        length = 0; db = NULL;
        limit = nLimit; EOL = nEOL;
        if (spawnDb)
            Init(EOL, nLimit);
    }

    void Init(int nEOL = kListEndDefault, int nLimit = 0)
    {
        Free();
        limit = nLimit; EOL = nEOL;
        db = (int32_t*)Bmalloc(sizeof(int32_t));
        dassert(db != NULL);
        db[0] = EOL;
    }

    void Free()
    {
        length = 0;
        if (db)
            Bfree(db), db = NULL;
    }

    int32_t* Add(int nID)
    {
        if (limit > 0 && length >= limit)
            ThrowError("Limit of %d items in list reached!", limit);

        int t = length; db[length++] = nID;
        db = (int32_t*)Brealloc(db, (length + 1) * sizeof(int32_t));
        dassert(db != NULL);
        
        db[length] = EOL;
        return &db[t];
    }

    int32_t* AddIfNotExists(int nID)
    {
        int t;
        if ((t = Find(nID)) != EOL)
            return &db[t];

        return Add(nID);
    }

    int32_t* Remove(int nID, bool internalIndex = false)
    {
        if (!internalIndex && (nID = Find(nID)) == EOL)
            return First();

        if (nID < length)
            memmove(&db[nID], &db[nID + 1], (length - nID) * sizeof(int32_t));

        if (length > 0)
        {
            // we realloc to length because Add reallocs to length + 1
            db = (int32_t*)Brealloc(db, length * sizeof(int32_t));
            dassert(db != NULL);
            db[--length] = EOL;
        }
        else
        {
            Init(EOL, limit);
        }

        return &db[nID];
    }

    int Find(int nID)
    {
        int i = length;
        while (--i >= 0)
        {
            if (db[i] == nID)
                return i;
        }

        return EOL;
    }

    int32_t* PtrTo(int nID, bool internalIndex)
    {
        if (internalIndex)
            return (rngok(nID, 0, length)) ? &db[nID] : NULL;

        return ((nID = Find(nID)) != EOL) ? &db[nID] : NULL;
    }

    bool Exists(int nID)        { return (Find(nID) != EOL); }
    int32_t* First()            { return &db[0]; }
    int32_t* Last()             { return &db[ClipLow(length - 1, 0)]; }
    int32_t Length()            { return length; }
    int32_t EndSign()           { return EOL; }
    int32_t SizeOf()            { return (length + 1) * sizeof(int32_t); }

    void Process(char(*pFunc)(int32_t), bool reverse)
    {
        int i;
        if (reverse)
        {
            i = length;
            while (--i >= 0 && db[i] != EOL)
            {
                if (pFunc(db[i]) == kListREMOVE)
                    Remove(i, true);
            }
        }
        else
        {
            i = 0;
            while (db[i] != EOL)
            {
                if (pFunc(db[i]) == kListREMOVE)
                    Remove(i, true);
                else
                    i++;
            }
        }
    }

};


class OBJECT_LIST
{
#define kMaxType 255

private:
    OBJECT* db;
    int externalCount;
    int internalCount;
    void MarkEnd()
    {
        db[externalCount].type = kMaxType;
        db[externalCount].index = 0;
    }

    void Init()
    {
        db = (OBJECT*)Bmalloc(sizeof(OBJECT));
        externalCount = 0, internalCount = 1;
        MarkEnd();
    }

    void Free()
    {
        if (db)
            free(db);

        externalCount = 0;
        internalCount = 0;
        db = NULL;
    }
public:
    OBJECT_LIST() { Init(); }
    ~OBJECT_LIST() { Free(); }
    OBJECT* Ptr() { return &db[0]; }
    int Length() { return externalCount; }

    int Add(int nType, int nIndex, bool check = false)
    {
        int retn = -1;
        if (check && (retn = Find(nType, nIndex)) >= 0)
            return retn;


        OBJECT* pDb = (OBJECT*)realloc(db, sizeof(OBJECT) * (internalCount + 1));
        if (pDb)
        {
            db = pDb;
            db[externalCount].type = nType;
            db[externalCount].index = nIndex;
            retn = externalCount;
            externalCount++;
            internalCount++;
        }

        MarkEnd();
        return retn;
    }

    int Find(int nType, int nIndex)
    {
        int i;
        for (i = 0; i < externalCount; i++)
        {
            if (db[i].type == nType && db[i].index == nIndex)
                return i;
        }

        return -1;
    }
};

// - VARIABLES ------------------------------------------------------------------
extern bool gTeamsSpawnUsed;
extern bool gEventRedirectsUsed;
extern ZONE gStartZoneTeam1[kMaxPlayers];
extern ZONE gStartZoneTeam2[kMaxPlayers];
extern THINGINFO_EXTRA gThingInfoExtra[kThingMax];
extern VECTORINFO_EXTRA gVectorInfoExtra[kVectorMax];
extern MISSILEINFO_EXTRA gMissileInfoExtra[kMissileMax];
extern EXPLOSION_EXTRA gExplodeExtra[kExplosionMax];
extern DUDEINFO_EXTRA gDudeInfoExtra[kDudeMax];
extern TRPLAYERCTRL gPlayerCtrl[kMaxPlayers];
extern SPRITEMASS gSpriteMass[kMaxXSprites];
extern AISTATE genPatrolStates[kPatrolStateSize];

extern IDLIST gProxySpritesList;
extern IDLIST gSightSpritesList;
extern IDLIST gImpactSpritesList;
extern IDLIST gPhysSpritesList;
extern IDLIST gFlwSpritesList;
extern OBJECT_STATUS1* gEvCauser;

// - FUNCTIONS ------------------------------------------------------------------
bool xsprIsFine(spritetype* pSpr);
bool nnExtEraseModernStuff(spritetype* pSprite, XSPRITE* pXSprite);
void nnExtInitModernStuff(bool bSaveLoad);
void nnExtProcessSuperSprites(void);
bool nnExtIsImmune(spritetype* pSprite, int dmgType, int minScale = 16);
int nnExtRandom(int a, int b);
void nnExtResetGlobals();
void nnExtTriggerObject(int objType, int objIndex, int command, int causerID);
//  -------------------------------------------------------------------------   //
spritetype* randomDropPickupObject(spritetype* pSprite, short prevItem);
spritetype* randomSpawnDude(XSPRITE* pXSource, spritetype* pSprite, int a3, int a4);
int GetDataVal(spritetype* pSprite, int data);
int randomGetDataValue(XSPRITE* pXSprite, int randType);
void sfxPlayMissileSound(spritetype* pSprite, int missileId);
void sfxPlayVectorSound(spritetype* pSprite, int vectorId);
//  -------------------------------------------------------------------------   //
void debrisBubble(int nSpr);
void debrisMove(int nSpr);
void debrisConcuss(int nOwner, int nSpr, int x, int y, int z, int dmg);
//  -------------------------------------------------------------------------   //
void aiSetGenIdleState(spritetype* pSprite, XSPRITE* pXSprite);

// triggers related
//  -------------------------------------------------------------------------   //
int aiFightGetTargetDist(spritetype* pSprite, DUDEINFO* pDudeInfo, spritetype* pTarget);
int aiFightGetFineTargetDist(spritetype* pSprite, spritetype* pTarget);
bool aiFightDudeCanSeeTarget(XSPRITE* pXDude, DUDEINFO* pDudeInfo, spritetype* pTarget);
bool aiFightUnitCanFly(spritetype* pDude);
bool aiFightIsMeleeUnit(spritetype* pDude);
bool aiFightDudeIsAffected(XSPRITE* pXDude);
bool aiFightMatesHaveSameTarget(XSPRITE* pXLeader, spritetype* pTarget, int allow);
bool aiFightGetDudesForBattle(XSPRITE* pXSprite);
bool aiFightIsMateOf(XSPRITE* pXDude, XSPRITE* pXSprite);
void aiFightAlarmDudesInSight(spritetype* pSprite, int max);
void aiFightActivateDudes(int rx);
void aiFightFreeTargets(int nSprite);
void aiFightFreeAllTargets(XSPRITE* pXSource);
spritetype* aiFightGetTargetInRange(spritetype* pSprite, int minDist, int maxDist, short data, short teamMode);
spritetype* aiFightTargetIsPlayer(XSPRITE* pXSprite);
spritetype* aiFightGetMateTargets(XSPRITE* pXSprite);
//  -------------------------------------------------------------------------   //
void useSlopeChanger(XSPRITE* pXSource, int objType, int objIndex);
void useSectorWindGen(XSPRITE* pXSource, sectortype* pSector);
void useEffectGen(XSPRITE* pXSource, spritetype* pSprite);
void useSeqSpawnerGen(XSPRITE* pXSource, int objType, int index);
void damageSprites(XSPRITE* pXSource, spritetype* pSprite);
void useTeleportTarget(XSPRITE* pXSource, spritetype* pSprite);
void useObjResizer(XSPRITE* pXSource, short objType, int objIndex);
void useRandomItemGen(spritetype* pSource, XSPRITE* pXSource);
void useUniMissileGen(XSPRITE* pXSource, spritetype* pSprite);
void useSoundGen(XSPRITE* pXSource, spritetype* pSprite);
void useIncDecGen(XSPRITE* pXSource, short objType, int objIndex);
void useDataChanger(XSPRITE* pXSource, int objType, int objIndex);
void useSectorLigthChanger(XSPRITE* pXSource, sectortype* pSect);
void useTargetChanger(XSPRITE* pXSource, spritetype* pSprite);
void usePictureChanger(XSPRITE* pXSource, int objType, int objIndex);
void usePropertiesChanger(XSPRITE* pXSource, short objType, int objIndex);
void useSequentialTx(XSPRITE* pXSource, COMMAND_ID cmd, bool setState, int causerID);
void useRandomTx(XSPRITE* pXSource, COMMAND_ID cmd, bool setState, int causerID);
void useDudeSpawn(XSPRITE* pXSource, spritetype* pSprite);
void useCustomDudeSpawn(XSPRITE* pXSource, spritetype* pSprite);
void useVelocityChanger(XSPRITE* pXSource, int causerID, short objType, int objIndex);
void useGibObject(XSPRITE* pXSource, spritetype* pSpr);
void useDripGenerator(XSPRITE* pXSource, spritetype* pSprite);
bool txIsRanged(XSPRITE* pXSource);
void seqTxSendCmdAll(XSPRITE* pXSource, int nIndex, COMMAND_ID cmd, bool modernSend, int causerID);
//  -------------------------------------------------------------------------   //
void trPlayerCtrlLink(XSPRITE* pXSource, PLAYER* pPlayer, bool checkCondition);
void trPlayerCtrlSetRace(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlStartScene(XSPRITE* pXSource, PLAYER* pPlayer, bool force);
void trPlayerCtrlStopScene(PLAYER* pPlayer);
void trPlayerCtrlSetMoveSpeed(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlSetJumpHeight(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlSetScreenEffect(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlSetLookAngle(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlEraseStuff(XSPRITE* pXSource, PLAYER* pPlayer);
void trPlayerCtrlGiveStuff(XSPRITE* pXSource, PLAYER* pPlayer, TRPLAYERCTRL* pCtrl);
void trPlayerCtrlUsePackItem(XSPRITE* pXSource, PLAYER* pPlayer, int evCmd);
//  -------------------------------------------------------------------------   //
void modernTypeTrigger(int type, int nDest, EVENT event);
char modernTypeSetSpriteState(int nSprite, XSPRITE* pXSprite, int nState, int causerID);
bool modernTypeOperateSector(int nSector, sectortype* pSector, XSECTOR* pXSector, EVENT event);
bool modernTypeOperateSprite(int nSprite, spritetype* pSprite, XSPRITE* pXSprite, EVENT event);
bool modernTypeOperateWall(int nWall, walltype* pWall, XWALL* pXWall, EVENT event);
void modernTypeSendCommand(int nSprite, int channel, COMMAND_ID command, int causerID);
//  -------------------------------------------------------------------------   //
bool playerSizeShrink(PLAYER* pPlayer, int divider);
bool playerSizeGrow(PLAYER* pPlayer, int multiplier);
bool playerSizeReset(PLAYER* pPlayer);
void playerDeactivateShrooms(PLAYER* pPlayer);
//  -------------------------------------------------------------------------   //
QAV* playerQavSceneLoad(int qavId);
void playerQavSceneProcess(PLAYER* pPlayer, QAVSCENE* pQavScene);
void playerQavScenePlay(PLAYER* pPlayer);
void playerQavSceneDraw(PLAYER* pPlayer, int a2, int a3, int a4, int a5);
void playerQavSceneReset(PLAYER* pPlayer);
//  -------------------------------------------------------------------------   //
void callbackUniMissileBurst(int nSprite);
void callbackMakeMissileBlocking(int nSprite);
//  -------------------------------------------------------------------------   //
PLAYER* getPlayerById(short id);
bool isGrown(spritetype* pSprite);
bool isShrinked(spritetype* pSprite);
bool valueIsBetween(int val, int min, int max);
bool IsBurningDude(spritetype* pSprite);
bool IsKillableDude(spritetype* pSprite);
bool isActive(int nSprite);
int getDataFieldOfObject(int objType, int objIndex, int dataIndex);
bool setDataValueOfObject(int objType, int objIndex, int dataIndex, int value);
bool incDecGoalValueIsReached(XSPRITE* pXSprite);
void windGenStopWindOnSectors(XSPRITE* pXSource);
int getSpriteMassBySize(spritetype* pSprite);
bool ceilIsTooLow(spritetype* pSprite);
void levelEndLevelCustom(int nLevel);
XSPRITE* evrListRedirectors(int objType, int objXIndex, XSPRITE* pXRedir, int* tx);
XSPRITE* evrIsRedirector(int nSprite);
int listTx(XSPRITE* pXRedir, int tx);
void seqSpawnerOffSameTx(XSPRITE* pXSource);
void triggerTouchSprite(spritetype* pSprite, int nHSprite);
void triggerTouchWall(spritetype* pSprite, int nHWall);
void killEvents(int nRx, int nCmd);
void changeSpriteAngle(spritetype* pSpr, int nAng);
int getVelocityAngle(spritetype* pSpr);
//  -------------------------------------------------------------------------   //
char aiPatrolSetMarker(spritetype* pSprite, XSPRITE* pXSprite);
void aiPatrolThink(spritetype* pSprite, XSPRITE* pXSprite);
void aiPatrolStop(spritetype* pSprite, int target, bool alarm = false);
void aiPatrolAlarmFull(spritetype* pSprite, XSPRITE* pXTarget, bool chain);
void aiPatrolAlarmLite(spritetype* pSprite, XSPRITE* pXTarget);
void aiPatrolState(spritetype* pSprite, int state);
void aiPatrolMove(spritetype* pSprite, XSPRITE* pXSprite);
int aiPatrolMarkerBusy(int nExcept, int nMarker);
bool aiPatrolMarkerReached(spritetype* pSprite, XSPRITE* pXSprite);
bool aiPatrolGetPathDir(XSPRITE* pXSprite, XSPRITE* pXMarker);
void aiPatrolFlagsMgr(spritetype* pSource, XSPRITE* pXSource, spritetype* pDest, XSPRITE* pXDest, bool copy, bool init);
void aiPatrolRandGoalAng(spritetype* pSprite, XSPRITE* pXSprite);
void aiPatrolTurn(spritetype* pSprite, XSPRITE* pXSprite);
inline int aiPatrolGetVelocity(int speed, int value) {
    return (value > 0) ? ClipRange((speed / 3) + (2500 * value), 0, 0x47956) : speed;
}

inline bool aiPatrolWaiting(AISTATE* pAiState) {
    return (pAiState->stateType >= kAiStatePatrolWaitL && pAiState->stateType <= kAiStatePatrolWaitW);
}

inline bool aiPatrolMoving(AISTATE* pAiState) {
    return (pAiState->stateType >= kAiStatePatrolMoveL && pAiState->stateType <= kAiStatePatrolMoveW);
}

inline bool aiPatrolTurning(AISTATE* pAiState) {
    return (pAiState->stateType >= kAiStatePatrolTurnL && pAiState->stateType <= kAiStatePatrolTurnW);
}

inline bool aiInPatrolState(AISTATE* pAiState) {
    return (pAiState->stateType >= kAiStatePatrolBase && pAiState->stateType < kAiStatePatrolMax);
}

inline bool aiInPatrolState(int nAiStateType) {
    return (nAiStateType >= kAiStatePatrolBase && nAiStateType < kAiStatePatrolMax);
}

//  -------------------------------------------------------------------------   //
bool readyForCrit(spritetype* pHunter, spritetype* pVictim);
int sectorInMotion(int nSector);
void clampSprite(spritetype* pSprite, int which = 0x03);
int getSpritesNearWalls(int nSrcSect, int* spriOut, int nMax, int nDist);
bool isMovableSector(int nType);
bool isMovableSector(sectortype* pSect);
bool isUnderwaterSector(XSECTOR* pXSect);
bool isUnderwaterSector(sectortype* pSect);
bool isUnderwaterSector(int nSector);
bool isOnRespawn(spritetype* pSpr);
int getDigitFromValue(int nVal, int nOffs);
void killEffectGenCallbacks(XSPRITE* pXSource);
bool seqCanOverride(Seq* pSeq, int nFrame, bool* xrp, bool* yrp, bool* plu);
void getRxBucket(int nChannel, int* nStart, int* nEnd, RXBUCKET** pRx = NULL);
void nnExtOffsetPos(int oX, int oY, int oZ, int nAng, int* x, int* y, int* z);
char nnExtOffsetSprite(spritetype* pSpr, int oX, int oY, int oZ);
char spriteIsUnderwater(spritetype* pSprite, char oldWay = false);
char dudeIsAlive(spritetype* pSpr);
int nnExtGetStartHealth(spritetype* pSpr);

FORCE_INLINE bool isPartOfCauserScript(int objType, int objIndex) { return gEvCauser[objType].id[objIndex].ok; }
FORCE_INLINE int perc2val(int reqPerc, int val) { return (val * reqPerc) / 100; }
FORCE_INLINE void nnExtOffsetPos(POINT3D* pOffs, int nAng, int* x, int* y, int* z)
{
    nnExtOffsetPos(pOffs->x, pOffs->y, pOffs->z, nAng, x, y, z);
}

int nnExtDudeStartHealth(spritetype* pSpr, int nHealth);
void nnExtScaleVelocity(spritetype* pSpr, int nVel, int dx, int dy, int dz, char which = 0x03);
void nnExtScaleVelocityRel(spritetype* pSpr, int nVel, int dx, int dy, int dz, char which = 0x03);
int nnExtGibSprite(spritetype* pSpr, IDLIST* pOut, GIBTYPE nGibType, CGibPosition* pPos, CGibVelocity* pVel);
spritetype* nnExtFireMissile(spritetype* pSpr, int a2, int a3, int a4, int a5, int a6, int nType);
spritetype* nnExtSpawnDude(spritetype* pSrc, int nType, int x, int y, int z);

void nnExtSprScaleSet(spritetype* pSpr, int nScale);
void nnExtCoSin(int nAng, int* x, int* y, int nShift = 16);
DICTNODE* nnExtResFileSearch(Resource* pIn, const char* pName, const char* pExt, char external = true);
int nnExtResAddExternalFiles(Resource* pIn, const char* pPath, EXTERNAL_FILES_LIST* pList, int nLen);
void getSectorWalls(int nSect, int* swal, int* ewal);


// SPRITES_NEAR_SECTORS
// Intended for move sprites that is close to the outside walls with
// TranslateSector and/or zTranslateSector similar to Powerslave(Exhumed) way
// --------------------------------------------------------------------------
class SPRINSECT
{
#define kMaxSprNear 256
#define kWallDist	16

private:
    //-----------------------------------------------------------------------------------
    struct SPRITES
    {
        unsigned int nSector;
        signed   int sprites[kMaxSprNear + 1];
    };
    SPRITES* db;
    unsigned int length;
    //-----------------------------------------------------------------------------------
    bool Alloc(int nLength) // normally should be used when loading saved game
    {
        Free();
        if (nLength <= 0)
            return false;

        db = (SPRITES*)Bmalloc(nLength * sizeof(SPRITES));
        dassert(db != NULL);

        length = nLength;
        while (nLength--)
        {
            SPRITES* pEntry = &db[nLength];
            Bmemset(pEntry->sprites, -1, sizeof(pEntry->sprites));
        }

        return true;
    }
    //-----------------------------------------------------------------------------------
public:
    void Free()
    {
        length = 0;
        if (db)
            Bfree(db), db = NULL;
    }
    //-----------------------------------------------------------------------------------
    void Init(int nDist = kWallDist) // used in trInit to collect the sprites before translation
    {
        Free();

        int i, j, k, nSprites;
        int* collected = (int*)Bmalloc(sizeof(int) * kMaxSprites);
        for (i = 0; i < numsectors; i++)
        {
            sectortype* pSect = &sector[i];
            if (!isMovableSector(pSect->type))
                continue;

            switch (pSect->type) {
            case kSectorZMotionSprite:
            case kSectorSlideMarked:
            case kSectorRotateMarked:
                continue;
                // only allow non-marked sectors
            default:
                break;
            }

            nSprites = getSpritesNearWalls(i, collected, kMaxSprites, nDist);

            // exclude sprites that is not allowed
            for (j = nSprites - 1; j >= 0; j--)
            {
                spritetype* pSpr = &sprite[collected[j]];
                if ((pSpr->cstat & 0x6000) && pSpr->sectnum >= 0)
                {
                    // if *next* sector is movable, exclude to avoid fighting
                    if (!isMovableSector(sector[pSpr->sectnum].type))
                    {
                        switch (pSpr->statnum) {
                        default:
                            continue;
                        case kStatMarker:
                        case kStatPathMarker:
                            if (pSpr->flags & 0x1) continue;
                            // no break
                        case kStatDude:
                            break;
                        }
                    }
                }

                nSprites--;
                for (k = j; k < nSprites; k++)
                    collected[k] = collected[k + 1];
            }

            if (nSprites > 0)
            {
                db = (SPRITES*)Brealloc(db, ((unsigned int)(length + 1)) * sizeof(SPRITES));
                dassert(db != NULL);

                SPRITES* pEntry = &db[length];
                Bmemset(pEntry->sprites, -1, sizeof(pEntry->sprites));
                Bmemcpy(pEntry->sprites, collected, sizeof(pEntry->sprites[0]) * ClipHigh(nSprites, kMaxSprNear));
                pEntry->nSector = i;
                length++;
            }
        }

        Bfree(collected);
    }
    //-----------------------------------------------------------------------------------
    void Save(LoadSave* pSave)
    {
        unsigned int i, j;
        pSave->Write(&length, sizeof(length));  // total db length
        for (i = 0; i < length; i++)
        {
            // owner sector
            pSave->Write(&db[i].nSector, sizeof(db[i].nSector));

            j = 0;
            while (j < kMaxSprNear)
            {
                pSave->Write(&db[i].sprites[j], sizeof(db[i].sprites[j]));
                if (db[i].sprites[j] == -1) // sprites end reached
                    break;

                j++;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void Load(LoadSave* pLoad)
    {
        unsigned int i, j;

        pLoad->Read(&i, sizeof(length));
        if (!Alloc(i))
            return; // the length is zero

        for (i = 0; i < length; i++)
        {
            // owner sector
            pLoad->Read(&db[i].nSector, sizeof(db[i].nSector));

            j = 0;
            while (j < kMaxSprNear)
            {
                pLoad->Read(&db[i].sprites[j], sizeof(db[i].sprites[j]));
                if (db[i].sprites[j] == -1) // sprites end reached
                    break;

                j++;
            }

        }
    }
    //-----------------------------------------------------------------------------------
    int* GetSprPtr(int nSector)
    {
        unsigned int i;
        for (i = 0; i < length; i++)
        {
            if (db[i].nSector == (unsigned int)nSector && db[i].sprites[0] >= 0)
                return (int*)db[i].sprites;
        }
        return NULL;
    }
    //-----------------------------------------------------------------------------------
    ~SPRINSECT() { Free(); };

};

extern SPRINSECT gSprNSect;
#endif

////////////////////////////////////////////////////////////////////////
// This file provides modern features for mappers.
// For full documentation please visit http://cruo.bloodgame.ru/xxsystem
////////////////////////////////////////////////////////////////////////////////////