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

// CONSTANTS

#define TRIGGER_START_CHANNEL_NBLOOD kChannelLevelStartNBLOOD  // uncomment only for Nblood
//#define TRIGGER_START_CHANNEL_RAZE kChannelLevelStartRAZE  // uncomment only for Raze

// additional non-thing proximity, sight and physics sprites 
#define kMaxSuperXSprites 512


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
#define kModernTypeFlag64 0x0040

#define kMaxRandomizeRetries 16
#define kPercFull 100
#define kPatrolStateSize 42
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
kStatModernWindGen                  = 26,
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
kDudeModernCustomBurning            = 255,
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

struct SPRITEMASS { // sprite mass info for getSpriteMassBySize();
    int seqId;
    short picnum; // mainly needs for moving debris
    short xrepeat;
    short yrepeat;
    short clipdist; // mass multiplier
    int mass;
    short airVel; // mainly needs for moving debris
    int fraction; // mainly needs for moving debris
};

struct QAVSCENE { // this one stores qavs anims that can be played by trigger
    short index = -1;  // index of sprite which triggered qav scene
    QAV* qavResrc = NULL;
    short dummy = -1;
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

struct TRPLAYERCTRL { // this one for controlling the player using triggers (movement speed, jumps and other stuff)
    QAVSCENE qavScene;
};

struct PATROL_FOUND_SOUNDS {

    int snd;
    int max;
    int cur;

};
#pragma pack(pop)

// - CLASSES ------------------------------------------------------------------
class IDLIST
{
    typedef void (*IDLIST_PROCESS_FUNC)(int32_t nId);

private:
    int32_t* db;
    int32_t  length;
public:
    IDLIST(bool spawnDb)
    {
        length = 0;
        db = NULL;
        if (spawnDb)
            Init();
    }
    ~IDLIST() { Free(); }

    void Init()
    {
        Free();
        db = (int32_t*)Bmalloc(sizeof(int32_t));
        dassert(db != NULL);
        db[0] = -1;
    }

    void Free()
    {
        length = 0;
        if (db)
            Bfree(db), db = NULL;
    }

    int Add(int nID)
    {
        register int t = length;
        db[length++] = nID;
        db = (int32_t*)Brealloc(db, (length + 1) * sizeof(int32_t));
        dassert(db != NULL);
        db[length] = -1;
        return t;
    }

    int AddIfNot(int nID)
    {
        register int t;
        if ((t = Find(nID)) >= 0)
            return t;

        return Add(nID);
    }

    bool Remove(int nID, bool isListId = false)
    {
        if (!isListId && (nID = Find(nID)) < 0)
            return false;

        if (nID < length)
            memmove(&db[nID], &db[nID + 1], (length - nID) * sizeof(int32_t));

        if (length > 0)
        {
            db = (int32_t*)Brealloc(db, length * sizeof(int32_t));
            dassert(db != NULL);
            db[--length] = -1;
        } else
        {
            Init();
        }

        return true;
    }

    int Find(int nID)
    {
        register int i = length;
        while (--i >= 0 && db[i] != nID);
        return i;
    }

    bool Exists(int nID) { return (Find(nID) >= 0); }
    int32_t* GetPtr() { return (int32_t*)db; }
    int32_t GetLength() { return length; }
    int32_t SizeOf() { return (length + 1) * sizeof(int32_t); }

    void Process(IDLIST_PROCESS_FUNC pFunc)
    {
        if (!length)
            return;

        int32_t* pDb = db;
        while (*pDb >= 0)
            pFunc(*pDb++);
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

    int Add(int nType, int nIndex, BOOL check = FALSE)
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
        register int i;
        for (i = 0; i < externalCount; i++)
        {
            if (db[i].type == nType && db[i].index == nIndex)
                return i;
        }

        return -1;
    }
};

// SPRITES_NEAR_SECTORS
// Intended for move sprites that is close to the outside walls with
// TranslateSector and/or zTranslateSector similar to Powerslave(Exhumed) way
// --------------------------------------------------------------------------
class SPRINSECT
{
#define kMaxSprNear 256
#define kWallDist	16

private:
    struct SPRITES
    {
        unsigned int nSector;
        signed   int sprites[kMaxSprNear + 1];
    };
    SPRITES* db;
    unsigned int length;
    bool Alloc(int nLength); // normally should be used when loading saved game
public:
    void Free();
    void Init(int nDist = kWallDist); // used in trInit to collect the sprites before translation
    void Save(LoadSave* pSave);
    void Load(LoadSave* pLoad);
    int* GetSprPtr(int nSector);
    ~SPRINSECT() { Free(); };

};


// - VARIABLES ------------------------------------------------------------------
extern bool gTeamsSpawnUsed;
extern bool gEventRedirectsUsed;
extern ZONE gStartZoneTeam1[kMaxPlayers];
extern ZONE gStartZoneTeam2[kMaxPlayers];
extern THINGINFO_EXTRA gThingInfoExtra[kThingMax];
extern VECTORINFO_EXTRA gVectorInfoExtra[kVectorMax];
extern MISSILEINFO_EXTRA gMissileInfoExtra[kMissileMax];
extern DUDEINFO_EXTRA gDudeInfoExtra[kDudeMax];
extern TRPLAYERCTRL gPlayerCtrl[kMaxPlayers];
extern SPRITEMASS gSpriteMass[kMaxXSprites];
extern short gProxySpritesList[kMaxSuperXSprites];
extern short gSightSpritesList[kMaxSuperXSprites];
extern short gPhysSpritesList[kMaxSuperXSprites];
extern short gImpactSpritesList[kMaxSuperXSprites];
extern short gProxySpritesCount;
extern short gSightSpritesCount;
extern short gPhysSpritesCount;
extern short gImpactSpritesCount;
extern AISTATE genPatrolStates[kPatrolStateSize];
extern SPRINSECT gSprNSect;
// - FUNCTIONS ------------------------------------------------------------------
inline bool xsprIsFine(spritetype* pSpr);
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
int debrisGetIndex(int nSprite);
int debrisGetFreeIndex(void);
void debrisBubble(int nSprite);
void debrisMove(int listIndex);
void debrisConcuss(int nOwner, int listIndex, int x, int y, int z, int dmg);
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
void useSectorLigthChanger(XSPRITE* pXSource, XSECTOR* pXSector);
void useTargetChanger(XSPRITE* pXSource, spritetype* pSprite);
void usePictureChanger(XSPRITE* pXSource, int objType, int objIndex);
void usePropertiesChanger(XSPRITE* pXSource, short objType, int objIndex);
void useSequentialTx(XSPRITE* pXSource, COMMAND_ID cmd, bool setState, int causerID);
void useRandomTx(XSPRITE* pXSource, COMMAND_ID cmd, bool setState, int causerID);
void useDudeSpawn(XSPRITE* pXSource, spritetype* pSprite);
void useCustomDudeSpawn(XSPRITE* pXSource, spritetype* pSprite);
void useVelocityChanger(XSPRITE* pXSource, int causerID, short objType, int objIndex);
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
void callbackGenDudeUpdate(int nSprite);
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
void aiPatrolSetMarker(spritetype* pSprite, XSPRITE* pXSprite);
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

int getDigitFromValue(int nVal, int nOffs);
void killEffectGenCallbacks(XSPRITE* pXSource);
inline bool rngok(int val, int rngA, int rngB) { return (val >= rngA && val < rngB); }
#endif

////////////////////////////////////////////////////////////////////////
// This file provides modern features for mappers.
// For full documentation please visit http://cruo.bloodgame.ru/xxsystem
////////////////////////////////////////////////////////////////////////////////////