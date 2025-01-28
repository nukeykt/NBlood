//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

*********************************************************************
NoOne: Custom Item system. Allows to create custom user pickups.
Potentially can be used to describe and store all the
vanilla items in text file.

For full documentation visit: http://cruo.bloodgame.ru/xxsystem/citem/
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

#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "build.h"
#include "mmulti.h"
#include "globals.h"
#include "common_game.h"
#include "player.h"
#include "network.h"
#include "weapon.h"
#include "sfx.h"

#include "nnextstr.h"
#include "nnexts.h"


/** DEFINITIONS
********************************************************************************/
#define kItemDescriptorVerMajor         1

#define kItemUserBase                   kItemWeaponBase
#define kItemUserMax                    kDudeBase
#define kMaxUserItems                   kItemUserMax - kItemUserBase

typedef char (*ITEMACTIONPROC)(PLAYER* pPlayer, ACTIONARG* a);

char gUserItemsInitialized = 0;

/** ITEM parameters mostly for parser
********************************************************************************/
enum enum_PAR_MAIN_ITEM_PARAMS
{
    kParItemType        = 0,
    kParItemMessage,
    kParItemAppearance,
    kParItemRespawn,
    kParItemLiveTime,
    kParItemGameMode,
    kParItemFlags,
};
static const char* gParItemMain[] =
{
    "Type",
    "Message",
    "Appearance",
    "RespawnTime",
    "DropLiveTime",
    "GameMode",
    "Flags",
    NULL,
};

enum enum_ITEM_GROUP
{
    kItemGroupItem             = 0,
    kItemGroupWeapon,
    kItemGroupAmmo,
    kItemGroupArmor,
    kItemGroupHealth,
    kItemGroupPowerup,
    kItemGroupKey,
    kItemGroupMax,
};
static const char* gParItemGroup[] =
{
    "Item",
    "Weapon",
    "Ammo",
    "Armor",
    "Health",
    "Powerup",
    "Key",
    NULL,
};

enum enum_PAR_APPEARANCE
{
    kParItemAppearTile              = 0,
    kParItemAppearSeq,
    kParItemAppearSize,
    kParItemAppearPal,
    kParItemAppearShade,
    kParItemAppearSnd,
};
static const char* gParItemAppearEntry[] =
{
    "Tile",
    "Seq",
    "Size",
    "Pal",
    "Shade",
    "Sound",
    NULL,
};

enum enum_PAR_FLAGS
{
    kParItemFlagsNoEff              = 0,
    kParItemFlagsNoMsg,
    kParItemFlagsShared,
    kParItemFlagsExtLimits,
};
static const char* gParItemFlags[] =
{
    "NoEffect",
    "NoMessage",
    "Shared",
    "ExtLimits",
    NULL,
};

enum enum_PAR_RESPAWNTIME
{
    kRespawnTimeSpecial1         = 0,
    kRespawnTimeSpecial2,
};

const char* gParRespawnTime[] =
{
    "SPECIAL1",
    "SPECIAL2",
    NULL,
};

enum enum_PAR_ITEM_GAMETYPE_FLAGS
{
    kParItemGameS              = 0,
    kParItemGameB,
    kParItemGameC,
    kParItemGameT,
};
static const char* gParItemGametype[] =
{
    "S",
    "B",
    "C",
    "T",
    NULL,
};

enum enum_ACTION_DEST
{
    kItemActionHealth				= 0,
    kItemActionArmor,
    kItemActionAmmo,
    kItemActionWeapon,
    kItemActionPowerTime,
    kItemActionKey,
    kItemActionPack,
    kItemActionEffect,
    kItemActionAirTime,
    kItemActionDmgIgnore,
};
static const char* gParItemActType[] =
{
    "Health",
    "Armor",
    "Ammo",
    "Weapon",
    "PowerTime",
    "Key",
    "Inventory",
    "ScreenEffect",
    "AirTime",
    "IgnoreDamage",
    NULL,
};

enum enum_PAR_ACTION_ENTRY
{
    kParActionAmount            = 0,
    kParActionAmountMin,
    kParActionAmountMax,
    kParActionSlot,
    kParActionReq,
    kParActionCompat,
};
static const char* gParItemActEntry[] =
{
    "Amount",
    "MinAmount",
    "MaxAmount",
    "Slot",
    "Required",
    "Compatible",
    NULL,
};


enum enum_PAR_ACTION_TYPE
{
    kParItemActionSet            = 0,
    kParItemActionAdd,
    kParItemActionSub,
};
static const char* gParItemActOperator[] =
{
    "Set",
    "Add",
    "Sub",
    NULL,
};


/** ACTION functions prototypes
************************************************************************************************/
static char userItemDoAction(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeArmor(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeHealth(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeKey(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeAmmo(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeWeapon(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangePackItem(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangePowerupTime(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeEffect(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeAirTime(PLAYER* pPlayer, ACTIONARG* a);
static char ACTION_ChangeIgnoreDmg(PLAYER* pPlayer, ACTIONARG* a);


ITEMACTIONPROC gItemActFunc[] =
{
    ACTION_ChangeHealth,
    ACTION_ChangeArmor,
    ACTION_ChangeAmmo,
    ACTION_ChangeWeapon,
    ACTION_ChangePowerupTime,
    ACTION_ChangeKey,
    ACTION_ChangePackItem,
    ACTION_ChangeEffect,
    ACTION_ChangeAirTime,
    ACTION_ChangeIgnoreDmg,
};


/** VARIOUS helpers
************************************************************************************************/
static char canPickupPermanentWeapon(PLAYER* pPlayer, spritetype* pISpr, ITEM* pItem);
static int helperChangeValue(int nValue, ITEM::ACTION* pAct);


/** ARRAY of pointers where we store all the items
********************************************************************************/
static ITEM* gItems[kMaxUserItems];


/** DEFAULT pickup sounds
********************************************************************************/
static const uint16_t gItemGroupSnd[kItemGroupMax] =
{
    775,    // item
    777,    // weapon
    782,    // ammo
    779,    // armor
    780,    // health
    776,    // powerup
    781,    // key
};

/** REPLACING these may ruin multiplayer?
********************************************************************************/
static const uint8_t gBannedItems[] =
{
    kItemFlagA,
    kItemFlagABase,
    kItemFlagB,
    kItemFlagBBase,
};

/** EXTERNAL game functions to handle new items
********************************************************************************/
ITEM* userItemGet(int nType)
{
    if (rngok(nType, kItemUserBase, kItemUserMax))
        return gItems[nType - kItemUserBase];

    return NULL;
}


char userItemPickup(PLAYER* pPlayer, spritetype* pISpr, ITEM* pItem)
{
    char isShared = ((pItem->flags & kFlagItemShared) != 0);
    XSPRITE* pXISpr = &xsprite[pISpr->extra];
    GAMEOPTIONS* pOpt = &gGameOptions;
    ACTIONARG data;

    int nGame = pOpt->nGameType;
    int i, r = 1, testReq = 0;

    memset(&data, 0, sizeof(data));
    if (pItem->flags & kFlagItemNoPickup)
    {
        if (nGame == kGameTypeSinglePlayer && (pItem->flags & kFlagItemNoPickupS))      return 0;
        else if (nGame == kGameTypeCoop && (pItem->flags & kFlagItemNoPickupC))         return 0;
        else if (nGame == kGameTypeBloodBath && (pItem->flags & kFlagItemNoPickupB))    return 0;
        else if (nGame == kGameTypeTeams && (pItem->flags & kFlagItemNoPickupT))        return 0;
    }

    // Unfortunately, a special check required
    // for weapon items with permanent respawn
    // option...

    if (pItem->group == kItemGroupWeapon)
    {
        if (pOpt->nWeaponSettings == 1 || pXISpr->respawn == 3)
        {
            if (!canPickupPermanentWeapon(pPlayer, pISpr, pItem))
                return 0;
        }
    }

    if (pItem->action != NULL)
    {
        testReq = ((pItem->flags & kFlagItemHasReqActions) != 0);

        // Test required actions (if any) for
        // success on first pass, do actions
        // on second.

        while(testReq >= 0)
        {
            data.act = pItem->action;
            r = 0;

            for (i = 0; i < pItem->numactions; i++, data.act++)
            {
                data.isShared   = ((isShared > 0) | (data.act->type == kItemActionKey && pOpt->bPlayerKeys == PLAYERKEYSMODE::SHARED));
                data.isCompat   = ((data.act->flags & kFlagActionCompat) != 0);
                data.isTest     = (testReq > 0);

                if (!userItemDoAction(pPlayer, &data))
                {
                    if (data.act->flags & kFlagActionReq)
                        return 0;
                }
                else
                {
                    r++;
                }
            }

            if (r)
            {
                for (i = 0; i < pItem->numeffects; i++, data.act++)
                {
                    data.isCompat   = ((data.act->flags & kFlagActionCompat) != 0);
                    data.isShared   = (isShared > 0);
                    data.isTest     = (testReq > 0);

                    if (!userItemDoAction(pPlayer, &data))
                    {
                        if (data.act->flags & kFlagActionReq)
                            return 0;
                    }
                    else
                    {
                        r++;
                    }
                }
            }

            testReq--;
        }
    }

    if (r)
    {
        if (pItem->appearance.sound)
            sfxPlay3DSound(pPlayer->pSprite, pItem->appearance.sound, -1, 0);
    }

    return (r > 0);
}

spritetype* userItemDrop(spritetype* pSpr, int nType)
{
    spritetype* pISpr; ITEM* pItem;
    if (!pSpr || pSpr->statnum >= kMaxStatus
        || (pItem = userItemGet(nType)) == NULL || (pISpr = actSpawnFloor(pSpr)) == NULL)
            return NULL;

    pISpr->type     = nType;
    pISpr->picnum   = pItem->appearance.picnum;
    pISpr->pal      = pItem->appearance.pal;
    pISpr->shade    = pItem->appearance.shade;
    pISpr->xrepeat  = pItem->appearance.xrepeat;
    pISpr->yrepeat  = pItem->appearance.yrepeat;

    if (pItem->appearance.seq)
    {
        if (pISpr->extra <= 0)
            dbInsertXSprite(pISpr->index);

        seqSpawn(pItem->appearance.seq, OBJ_SPRITE, pISpr->extra);
    }

    if (pItem->droplivetime)
        evPost(pISpr->index, OBJ_SPRITE, pItem->droplivetime * 100, kCallbackRemove);

    return pISpr;
}

int userItemGetRespawnTime(spritetype* pSpr)
{
    GAMEOPTIONS* pOpt = &gGameOptions;
    XSPRITE* pXSpr; ITEM* pItem;
    int nTime = -1;

    if (pSpr->extra <= 0
        || (pItem = userItemGet(pSpr->type)) == NULL)
            return -1;

    pXSpr = &xsprite[pSpr->extra];

    switch (pItem->group)
    {
        case kItemGroupAmmo:
            if (pXSpr->respawn == 2 || (pXSpr->respawn != 1 && pOpt->nWeaponSettings != 0))
            {
                nTime = pOpt->nWeaponRespawnTime;
                break;
            }
            return -1;
        case kItemGroupWeapon:
            if (pXSpr->respawn == 3 || pOpt->nWeaponSettings == 1) return 0;
            else if (pXSpr->respawn != 1 && pOpt->nWeaponSettings != 0)
            {
                nTime = pOpt->nWeaponRespawnTime;
                break;
            }
            return -1;
        default:
            if (pXSpr->respawn == 3 && pOpt->nGameType == kGameTypeCoop) return 0;
            if (pXSpr->respawn == 2 || (pXSpr->respawn != 1 && pOpt->nItemSettings != 0))
            {
               if (pItem->flags & kFlagItemRespawnSpec1)        nTime = pOpt->nSpecialRespawnTime;
               else if (pItem->flags & kFlagItemRespawnSpec2)   nTime = pOpt->nSpecialRespawnTime<<1;
               else                                             nTime = pOpt->nItemRespawnTime;

               break;
            }
            return -1;
    }

    if (pItem->respawntime > 0)
        nTime = pItem->respawntime * 100;

    return nTime;
}

char userItemViewUseRespawnMarkers(ITEM* pItem)
{
    if (pItem)
    {
        if (pItem->group == kItemGroupWeapon)
            return gGameOptions.nWeaponSettings == 3;

        return gGameOptions.nItemSettings == 2;
    }

    return 0;
}

void userItemsUninit()
{
    int i = kMaxUserItems;
    while (--i >= 0)
    {
        if (gItems[i])
        {
            CUSTOMITEM_SETUP::ClearItem(gItems[i]);
            free(gItems[i]),  gItems[i] = NULL;
        }
    }
}

int userItemsInit(char showLog)
{
    DICTNODE* hRes = gSysRes.Lookup("ITEMS", "ITM");
    IniFile* pIni; int c = 0;

    if (!hRes || hRes->size <= 0)
        return 0;

    CUSTOMITEM_SETUP::showLog = showLog;

    pIni = new IniFile((unsigned char*)gSysRes.Load(hRes), hRes->size, INI_SKIPCM|INI_SKIPZR);
    c += CUSTOMITEM_SETUP::Setup(pIni);
    delete(pIni);

    CUSTOMITEM_SETUP::Message
    (
        "\n"
        "\n"
        "\tUser items added...........: %d\n"
        "\tUser items replaced........: %d\n"
        "\tGame items replaced........: %d\n"
        "\tFailed to process..........: %d\n"
        "\tTotal......................: %d\n",
        CUSTOMITEM_SETUP::numNewItemsAdded,
        CUSTOMITEM_SETUP::numUserItemsReplaced,
        CUSTOMITEM_SETUP::numGameItemsReplaced,
        CUSTOMITEM_SETUP::numFailedItems,
        c
    );

    return c;
}

void userItemsInitSprites()
{
    ITEM* pItem; spritetype* pISpr;
    int i;

    for (i = headspritestat[kStatItem]; i >= 0; i = nextspritestat[i])
    {
        pISpr = &sprite[i];
        if ((pItem = userItemGet(pISpr->type)) == NULL)
            continue;

        if (pItem->appearance.seq)
        {
            if (pISpr->extra <= 0)
                dbInsertXSprite(pISpr->index);

            seqSpawn(pItem->appearance.seq, OBJ_SPRITE, pISpr->extra);
        }
    }
}

static char canPickupPermanentWeapon(PLAYER* pPlayer, spritetype* pISpr, ITEM* pItem)
{
    ITEM::ACTION* pAct = pItem->action;
    int i, n;

    for (i = 0; i < pItem->numactions; i++, pAct++)
    {
        if (pAct->type == kItemActionWeapon)
        {
            n = pAct->slot+1;
            if (!pPlayer->hasWeapon[n] || userItemGetRespawnTime(pISpr))
                return 1;

            break;
        }
    }

    return 0;
}

/** ACTION functions
********************************************************************************/
static char userItemDoAction(PLAYER* pPlayer, ACTIONARG* a)
{
    char r = 0; int i;

    if (a->isShared)
    {
        for (i = connecthead; i >= 0; i = connectpoint2[i])
            if (gItemActFunc[a->act->type](&gPlayer[i], a))
                r = 1;
    }
    else
    {
        r = gItemActFunc[a->act->type](pPlayer, a);
    }

    return r;
}

static char ACTION_ChangeArmor(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot;
    int nCur = pPlayer->armor[nSlot];
    int i;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pPlayer->armor[nSlot])
    {
        if (!a->isCompat)
            return 0;

        i = LENGTH(PLAYER::armor);
        while (--i >= 0)
        {
            if (pPlayer->armor[i] >> 4 < 100)
                break;
        }

        if (i < 0)
            return 0;
    }

    if (!a->isTest)
        pPlayer->armor[nSlot] = nCur;

    return 1;
}

static char ACTION_ChangeHealth(PLAYER* pPlayer, ACTIONARG* a)
{
    int nCur = pPlayer->pXSprite->health;
    int nOld = nCur;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == nOld)
        return 0;


    if (!a->isTest)
    {
        if (nCur > nOld)
        {
            pPlayer->pXSprite->health = nCur;
        }
        else
        {
            playerDamageSprite(pPlayer->nSprite, pPlayer, kDamageFall, nOld - nCur);
            if (pPlayer->pSprite->extra > 0 && pPlayer->pXSprite->health == nOld)
                return 0;
        }
    }

    return 1;
}

static char ACTION_ChangeKey(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot;
    int nCur = pPlayer->hasKey[nSlot];

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == (int)pPlayer->hasKey[nSlot])
        return 0;

    if (!a->isTest)
        pPlayer->hasKey[nSlot] = nCur;

    return 1;
}

static void helperForceLowerWeapon(PLAYER* pPlayer)
{
    // force lowering
    pPlayer->weaponState = 0;
    pPlayer->weaponAmmo = 0;
    pPlayer->weaponTimer = 0;
    pPlayer->weaponQav = -1;
    pPlayer->throwPower = 0;
    pPlayer->throwTime = 0;
    pPlayer->fuseTime = 0;
    pPlayer->qavLoop = 0;

    pPlayer->curWeapon = kWeaponPitchfork;
}

static char ACTION_ChangeAmmo(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot, nCur = pPlayer->ammoCount[nSlot];
    AMMOITEMDATA* pInfo;
    int wSlot;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pPlayer->ammoCount[nSlot])
        return 0;

    if (nCur < pPlayer->ammoCount[nSlot] && gInfiniteAmmo)
        return 0;

    if (!a->isTest)
    {
        pInfo = gAmmoItemData;
        while (pInfo->type && pInfo->weaponType && pInfo->type != nSlot) pInfo++;
        if ((wSlot = pInfo->weaponType) > 0 && nCur > 0)
            pPlayer->hasWeapon[wSlot] = 1;

        pPlayer->ammoCount[nSlot] = nCur;

        if (wSlot > 0)
        {
            if (nCur <= 0 && pPlayer->curWeapon == wSlot)
                helperForceLowerWeapon(pPlayer);
        }
    }

    return 1;
}

static char ACTION_ChangeWeapon(PLAYER* pPlayer, ACTIONARG* a)
{
    GAMEOPTIONS* pOpt = &gGameOptions;
    int wSlot = a->act->slot+1, aSlot = wSlot - 1;
    int nCur = pPlayer->hasWeapon[wSlot];
    int nOld = nCur;

    nCur = helperChangeValue(nCur, a->act);

    if (nCur)
    {
        if ((wSlot == kWeaponLifeLeech)
            && (pOpt->nGameType >= kGameTypeBloodBath) && findDroppedLeech(pPlayer, NULL))
                return 0;

        if (pPlayer->hasWeapon[wSlot])
        {
            if (pPlayer->ammoCount[aSlot] >= a->act->amount[2])
                return 0;

            if (pOpt->nWeaponSettings != 2
                && pOpt->nWeaponSettings != 3)
                    return 0;
        }
    }
    else if (nCur == nOld)
    {
        return 0;
    }

    if (!a->isTest)
    {
        pPlayer->hasWeapon[wSlot] = nCur;

        if (nCur)
        {
            if (pPlayer->ammoCount[aSlot])
            {
                if ((wSlot = WeaponUpgrade(pPlayer, wSlot)) != pPlayer->curWeapon)
                    pPlayer->weaponState = 0, pPlayer->nextWeapon = wSlot;
            }
        }
        else if (pPlayer->curWeapon == wSlot)
        {
            helperForceLowerWeapon(pPlayer);
        }
    }

    return 1;
}

static char ACTION_ChangePackItem(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot; PACKINFO* pSlot = &pPlayer->packSlots[nSlot];
    int nCur = pSlot->curAmount;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pSlot->curAmount)
        return 0;

    if (!a->isTest)
    {
        pSlot->curAmount = nCur;
        if (pPlayer->packItemId < 0 || !pPlayer->packSlots[pPlayer->packItemId].curAmount)
            pPlayer->packItemId = nSlot;
    }

    return 1;
}


static char ACTION_ChangePowerupTime(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot;
    int nCur = pPlayer->pwUpTime[nSlot];
    POWERUPINFO* pInfo = &gPowerUpInfo[nSlot];
    int t;

    if ((t = powerupCheck(pPlayer, nSlot)) > 0 && a->isCompat)
    {
        if (pInfo->pickupOnce)
            return 0;

        return 1;   // pickup without changing time...
    }

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pPlayer->pwUpTime[nSlot])
        return 0;

    if (!a->isTest)
    {
        if (nCur)
        {
            if (t <= 0)
            {
                powerupActivate(pPlayer, nSlot);
                sfxKill3DSound(pPlayer->pSprite, -1, -1);
            }
        }
        else
        {
            if (t > 0)
                powerupDeactivate(pPlayer, nSlot);
        }

        pPlayer->pwUpTime[nSlot] = nCur;
    }

    return 1;
}

static char ACTION_ChangeEffect(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot;
    int nCur = playerEffectGet(pPlayer, nSlot);
    int nOld = nCur;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == nOld)
        return 0;

    if (!a->isTest)
        playerEffectSet(pPlayer, nSlot, nCur);

    return 1;
}

static char ACTION_ChangeAirTime(PLAYER* pPlayer, ACTIONARG* a)
{
    int nCur = pPlayer->underwaterTime;

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pPlayer->underwaterTime)
        return 0;

    if (!a->isTest)
        pPlayer->underwaterTime = nCur;

    return 1;
}

static char ACTION_ChangeIgnoreDmg(PLAYER* pPlayer, ACTIONARG* a)
{
    int nSlot = a->act->slot;
    int nCur = pPlayer->damageControl[nSlot];

    nCur = helperChangeValue(nCur, a->act);
    if (nCur == pPlayer->damageControl[nSlot])
        return 0;

    if (!a->isTest)
        pPlayer->damageControl[nSlot] = nCur;

    return 1;
}

static int helperChangeValue(int nValue, ITEM::ACTION* pAct)
{
    int32_t n = pAct->amount[0];
    int32_t l = pAct->amount[1];
    int32_t g = pAct->amount[2];

    switch (pAct->operation)
    {
        case kParItemActionSet:
            if (!irngok(nValue, l, g)) break;
            nValue = ClipRange(n, l, g);
            break;
        case kParItemActionAdd:
            if (nValue > g) break;
            nValue = ClipHigh(nValue + n, g);
            break;
        case kParItemActionSub:
            if (nValue < l) break;
            nValue = ClipLow(nValue - n, l);
            break;
    }

    return nValue;
}

/** ITEM parser and setup code part
********************************************************************************/
ITEM CUSTOMITEM_SETUP::buffer;
IniFile* CUSTOMITEM_SETUP::pDesc            = NULL;
int CUSTOMITEM_SETUP::version               = 0;
int CUSTOMITEM_SETUP::numGameItemsReplaced  = 0;
int CUSTOMITEM_SETUP::numUserItemsReplaced  = 0;
int CUSTOMITEM_SETUP::numNewItemsAdded      = 0;
int CUSTOMITEM_SETUP::numFailedItems        = 0;
char CUSTOMITEM_SETUP::showLog              = 0;

void CUSTOMITEM_SETUP::Message(const char* pFormat, ...)
{
    char buffer[512], *pBuf = buffer;

    if (showLog)
    {
        pBuf += Bsprintf(pBuf, "ITEM SETUP:");
        pBuf += Bsprintf(pBuf, " ");

        va_list args;
        va_start(args, pFormat);
        pBuf += vsprintf(pBuf, pFormat, args);
        va_end(args);

        pBuf += Bsprintf(pBuf, "\n");

        OSD_Printf("%s", buffer);
    }
}
int CUSTOMITEM_SETUP::Setup(IniFile* pFile)
{
    const char *p; char* pGroup = NULL;
    ITEM *pItem, **pEntry;

    dassert(pFile != NULL);

    pDesc                   = pFile;
    numNewItemsAdded        = 0;
    numGameItemsReplaced    = 0;
    numUserItemsReplaced    = 0;
    numFailedItems          = 0;

    if ((version = CheckVersion()) != kItemDescriptorVerMajor)
    {
        Message("Invalid item descriptor version, Given: %d, Req: %d", version, kItemDescriptorVerMajor);
        return 0;
    }

    pFile->SectionRemove("General");

    while (pFile->GetNextSection(&pGroup))
    {
        if ((pItem = Setup(pGroup)) == NULL)
        {
            numFailedItems++;
            continue;
        }
        pEntry = &gItems[pItem->type - kItemUserBase];

        if (*pEntry)
        {
            p = gItems[pItem->type - kItemUserBase]->name;
            Message("Custom item #%d (%s) has been replaced with \"%s\"!", pItem->type, p, pItem->name);
            ClearItem(*pEntry); numUserItemsReplaced++;
        }
        else if (rngok(pItem->type, kItemWeaponBase, kItemMax))
        {
            p = GetGameItemName(pItem->type);
            Message("Game item #%d (%s) has been replaced with \"%s\"!",  pItem->type, p, pItem->name);
            numGameItemsReplaced++;
        }
        else
        {
            numNewItemsAdded++;
        }

        *pEntry = (ITEM*)malloc(sizeof(ITEM)); dassert(*pEntry != NULL);
        memcpy(*pEntry, pItem, sizeof(ITEM));
    }

    return numNewItemsAdded + numGameItemsReplaced + numUserItemsReplaced + numFailedItems;
}

ITEM* CUSTOMITEM_SETUP::Setup(char* pGroup)
{
    const char **pActOp, **pActType, *pKey, *pVal;
    int nPar, nType, nPrev = -1, i, j;
    char key[256], gotit = 0;

    memset(&buffer, 0, sizeof(buffer));

    /** ITEM DEFAULTS **/
    buffer.group                = kItemGroupItem;
    buffer.appearance.xrepeat   = 64;
    buffer.appearance.yrepeat   = 64;
    buffer.appearance.shade     = -8;

    /* Get type and group before everything else. */
    /*******************/

    pKey = gParItemMain[kParItemType];
    pVal = pDesc->GetKeyString(pGroup, pKey);
    SetupType(pVal);

    nType = buffer.type;
    if (!rngok(nType, kItemUserBase, kItemUserMax))
    {
        Message("Item type %d is out of a range (%d-%d).", nType, kItemUserBase, kItemUserMax);
        return NULL;
    }

    for (i = 0; i < LENGTH(gBannedItems); i++)
    {
        if (gBannedItems[i] != nType)
            continue;

        Message("Game item #%d (%s) cannot be replaced.", nType, GetGameItemName(nType));
        return NULL;
    }

    /* Start ADDING a new custom item or FULLY REPLACING game one with it! */
    /***************************************/

    SetupName(pGroup);

    pKey = gParItemMain[kParItemFlags];
    pVal = pDesc->GetKeyString(pGroup, pKey);
    SetupFlags(pVal);

    buffer.appearance.sound = gItemGroupSnd[buffer.group];

    while (pDesc->GetNextString(&pKey, &pVal, &nPrev, pGroup))
    {
        if (!pKey || !pVal)
            continue;

        gotit = 0;

        // Searching for normal params.
        if ((nPar = FindParam(pKey, gParItemMain)) >= 0)
        {
            switch (nPar)
            {
                case kParItemRespawn:    SetupRespawn(pVal);        break;
                case kParItemLiveTime:   SetupLiveTime(pVal);       break;
                case kParItemAppearance: SetupAppearance(pVal);     break;
                case kParItemGameMode:   SetupGameMode(pVal);       break;
                case kParItemMessage:    SetupMessage(pVal);        break;
            }

            continue;
        }

        // Searching for prefixed params a.k.a actions.
        for (pActOp = gParItemActOperator, i = 0; *pActOp && !gotit; pActOp++, i++)
        {
            for (pActType = gParItemActType, j = 0; *pActType; pActType++, j++)
            {
                sprintf(key, "%s%s", *pActOp, *pActType);
                if (Bstrcasecmp(pKey, key) != 0)
                    continue;

                SetupAction(pVal, i, j);
                gotit = 1;
                break;
            }
        }
    }

    if (buffer.numactions == 0 && buffer.numeffects > 0)
    {
        buffer.numactions = buffer.numeffects;
        buffer.numeffects = 0;

        // Show no *default* pickup effect,
        // but allow to show user
        // ones.

        buffer.flags |= kFlagItemNoEffect;
    }
    else if (buffer.numactions > 1 && buffer.numeffects > 0)
    {
        // Move effect actions to the bottom of list
        // so they won't appear when player pickups
        // nothing.

        i = buffer.numactions + buffer.numeffects;
        qsort(buffer.action, i, sizeof(ITEM::ACTION),
                (int(*)(const void*, const void*))QsortActions);
    }

    return &buffer;
}

char CUSTOMITEM_SETUP::SetupAppearance(const char* str)
{
    ITEM::APPEARANCE* pAppear = &buffer.appearance;
    int nPar, nVal, i = 0, c, t;
    char key[256], val[256];

    while ((i = enumStr(i, str, key, val)) != 0)
    {
        switch (nPar = FindParam(key, gParItemAppearEntry))
        {
            case kParItemAppearSize:
                if (isarray(val, &t))
                {
                    t = c = 0;
                    while ((t = enumStr(t, val, key)) != 0 && c < 2)
                    {
                        if (isufix(key) && (nVal = atoi(key)) <= 255)
                        {
                            if (c == 0)     pAppear->xrepeat = nVal;
                            else            pAppear->yrepeat = nVal;
                        }

                        c++;
                    }
                }
                else if (isufix(val) && (nVal = atoi(val)) <= 255)
                {
                    pAppear->xrepeat = pAppear->yrepeat = nVal;
                }
                break;
            case kParItemAppearTile:
            case kParItemAppearSeq:
            case kParItemAppearPal:
            case kParItemAppearShade:
            case kParItemAppearSnd:
                if (isfix(val))
                {
                    nVal = atoi(val);
                    switch (nPar)
                    {
                        case kParItemAppearTile:    if (rngok(nVal, 0, kMaxTiles))  pAppear->picnum = nVal;     break;
                        case kParItemAppearSeq:     if (nVal >= 0)                  pAppear->seq = nVal;        break;
                        case kParItemAppearPal:     if (irngok(nVal, 0, 255))       pAppear->pal = nVal;        break;
                        case kParItemAppearShade:   if (irngok(nVal, -128, 127))    pAppear->shade = nVal;      break;
                        case kParItemAppearSnd:     if (nVal >= 0)                  pAppear->sound = nVal;      break;
                    }
                }
                break;
        }
    }

    return 1;
}

char CUSTOMITEM_SETUP::SetupActionLimits(ITEM::ACTION* pAct, char extLimits)
{
    switch (pAct->type)
    {
        case kItemActionHealth:
            pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
            pAct->amount[2] = SetMaxAmount(pAct->amount[2], (extLimits) ? 999 : 200);
            pAct->amount[0] <<= 4;
            pAct->amount[1] <<= 4;
            pAct->amount[2] <<= 4;
            break;
        case kItemActionArmor:
            if (pAct->slot < LENGTH(PLAYER::armor))
            {
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                pAct->amount[2] = SetMaxAmount(pAct->amount[2], (extLimits) ? 250 : 200);
                pAct->amount[0] <<= 4;
                pAct->amount[1] <<= 4;
                pAct->amount[2] <<= 4;
                break;
            }
            return 0;
        case kItemActionKey:
            if (++pAct->slot < LENGTH(PLAYER::hasKey))
            {
                // This is bool
                pAct->amount[0] = (pAct->amount[0]) ? 1 : 0;
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                pAct->amount[2] = SetMaxAmount(pAct->amount[2], 1);
                break;
            }
            return 0;
        case kItemActionWeapon:
            if (pAct->slot + 1 < LENGTH(PLAYER::hasWeapon))
            {
                int n = gAmmoInfo[pAct->slot+1].max;
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                if (extLimits)
                {
                    if (n > 999)        n = 9990;   // spray can and such
                    else if (n > 100)   n = 999;    // normal weapons
                    else                n = 99;     // prox / remote bombs cannot have more than 2 digits in HUD.
                }

                pAct->amount[2] = SetMaxAmount(pAct->amount[2], n);
                break;
            }
            return 0;
        case kItemActionAmmo:
            if (pAct->slot < LENGTH(PLAYER::ammoCount))
            {
                int n = gAmmoInfo[pAct->slot].max;
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                if (extLimits)
                {
                    if (n > 999)        n = 9990;   // spray can and such
                    else if (n > 100)   n = 999;    // normal weapons
                    else                n = 99;     // prox / remote bombs cannot have more than 2 digits in HUD.
                }

                pAct->amount[2] = SetMaxAmount(pAct->amount[2], n);
                break;
            }
            return 0;
        case kItemActionPowerTime:
            if (pAct->slot < LENGTH(PLAYER::pwUpTime))
            {
                if (extLimits)
                {
                    pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                    pAct->amount[2] = SetMaxAmount(pAct->amount[2], 999);
                    pAct->amount[0] *= 100;
                    pAct->amount[1] *= 100;
                    pAct->amount[2] *= 100;
                }
                else
                {
                    pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                    pAct->amount[2] = gPowerUpInfo[pAct->slot].maxTime;
                    pAct->amount[0] *= 100;
                    pAct->amount[1] *= 100;
                }

                break;
            }
            return 0;
        case kItemActionPack:
            if (pAct->slot < LENGTH(PLAYER::packSlots))
            {
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                pAct->amount[2] = SetMaxAmount(pAct->amount[2], (extLimits) ? 999 : 100);
                break;
            }
            return 0;
        case kItemActionEffect:
            if (pAct->slot < kPlayerEffectMax)
            {
                break;
            }
            return 0;
        case kItemActionAirTime:
            pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
            pAct->amount[2] = SetMaxAmount(pAct->amount[2], 999);
            pAct->amount[0] *= 100;
            pAct->amount[1] *= 100;
            pAct->amount[2] *= 100;
            break;
        case kItemActionDmgIgnore:
            if (pAct->slot < kDamageMax)
            {
                // Can only iterate by 1
                pAct->amount[0] = (pAct->amount[0]) ? 1 : 0;
                pAct->amount[1] = SetMinAmount(pAct->amount[1], 0);
                break;
            }
            return 0;
    }

    if (pAct->amount[1] > pAct->amount[2])
    {
        int32_t t = pAct->amount[2];
        pAct->amount[2] = pAct->amount[1];
        pAct->amount[1] = t;
    }

    switch (pAct->operation)
    {
        case kParItemActionAdd:
            if (pAct->amount[0] < 0)
            {
                pAct->amount[0] = klabs(pAct->amount[0]);
                pAct->operation = kParItemActionSub;
            }
            break;
        case kParItemActionSub:
            if (pAct->amount[0] < 0)
            {
                pAct->amount[0] = klabs(pAct->amount[0]);
            }
            break;
    }

    return 1;
}

char CUSTOMITEM_SETUP::SetupAction(const char* str, int nOperator, int nAction)
{
    int nPar, nVal, i = 0; ITEM::ACTION action;
    char isEffect, key[256], val[256];

    memset(&action, 0, sizeof(action));

    action.amount[0] = 1;
    action.amount[1] = INT32_MIN + 1;
    action.amount[2] = INT32_MAX - 1;
    action.operation = nOperator;
    action.type      = nAction;

    while ((i = enumStr(i, str, key, val)) != 0)
    {
        switch (nPar = FindParam(key, gParItemActEntry))
        {
            case kParActionAmount:
            case kParActionAmountMin:
            case kParActionAmountMax:
                if (isfix(val))
                {
                    nVal = atoi(val);

                    switch (nPar)
                    {
                        case kParActionAmount:       action.amount[0] = nVal;     break;
                        case kParActionAmountMin:    action.amount[1] = nVal;     break;
                        case kParActionAmountMax:    action.amount[2] = nVal;     break;
                    }
                }
                break;
            case kParActionSlot:
                if (isufix(val))
                {
                    action.slot = atoi(val) - 1;
                }
                break;
            case kParActionReq:
                if ((nVal = btoi(val)) > 0)
                {
                    action.flags |= kFlagActionReq;
                }
                break;
            case kParActionCompat:
                if ((nVal = btoi(val)) > 0)
                {
                    action.flags |= kFlagActionCompat;
                }
                break;
        }
    }

    if (SetupActionLimits(&action, (buffer.flags & kFlagItemExtLimits) != 0))
    {
        isEffect = (action.type == kItemActionEffect);

        if (isEffect && (buffer.flags & kFlagItemNoEffect))
            return 1;

        if ((i = buffer.numactions + buffer.numeffects) < 255)
        {
            if (action.flags & kFlagActionReq)
                buffer.flags |= kFlagItemHasReqActions;

            buffer.action = (ITEM::ACTION*)realloc(buffer.action, sizeof(action) * (i + 1)); dassert(buffer.action != NULL);
            memcpy(&buffer.action[i], &action, sizeof(action));

            if (isEffect) buffer.numeffects++;
            else buffer.numactions++;

            return 1;
        }
    }

    return 0;
}

char CUSTOMITEM_SETUP::SetupType(const char* str)
{
    int nPar, i = 0, t;
    char val[256];

    if (isarray(str, &t))
    {
        if ((i = enumStr(i, str, val)) != 0)
        {
            if ((nPar = FindParam(val, gParItemGroup)) >= 0)
                buffer.group = nPar, t--;
        }

        if ((i = enumStr(i, str, val)) != 0)
        {
            if (isufix(val))
                buffer.type = atoi(val), t--;
        }

        return (t == 0);
    }
    else if (isufix(str))
    {
        buffer.group    = kItemGroupItem;
        buffer.type     = atoi(str);
        return 1;
    }

    return 0;
}

char CUSTOMITEM_SETUP::SetupRespawn(const char* str)
{
    int nPar;

    if (isufix(str))
    {
        buffer.respawntime = (uint8_t)ClipHigh(atoi(str), 255);
        return 1;
    }
    else if ((nPar = FindParam(str, gParRespawnTime)) >= 0)
    {
        switch (nPar)
        {
            case kRespawnTimeSpecial1:
                buffer.flags |= kFlagItemRespawnSpec1;
                buffer.respawntime = 0;
                return 1;
            case kRespawnTimeSpecial2:
                buffer.flags |= kFlagItemRespawnSpec2;
                buffer.respawntime = 0;
                return 1;
        }
    }

    return 0;
}

char CUSTOMITEM_SETUP::SetupLiveTime(const char* str)
{
    if (isufix(str))
    {
        buffer.droplivetime = (uint8_t)ClipHigh(atoi(str), 255);
        return 1;
    }

    return 0;
}

char CUSTOMITEM_SETUP::SetupName(const char* str)
{
    const char* p; char tmp[32];
    int l;

    l = ((p = strchr(str, '_')) != NULL) ? (p - str) : strlen(str);

    if (l <= 0)
    {
        l = sprintf(tmp, "Unnamed#%d", buffer.type);
        str = tmp;
    }

    buffer.name = (char*)malloc(l + 1); dassert(buffer.name != NULL);
    strncpy(buffer.name, str, l);
    buffer.name[l] = '\0';
    return 1;
}

char CUSTOMITEM_SETUP::SetupMessage(const char* str)
{
    const char* p = str;
    int n = strlen(str);
    int i = 0;


    if ((buffer.flags & kFlagItemNoMessage) == 0)
    {
        while ((p = strchr(p, '%')) != NULL && i < 2)
        {
            p++;
            if (*p == '%')
            {
                p++;
                continue;
            }

            if (*p != 's')
            {
                i = 2;
                break;
            }

            i++;
        }

        if (i <= 1)
        {
            if (i)
                n += strlen(buffer.name);

            buffer.message = (char*)malloc(n + 1); dassert(buffer.message != NULL);

            if (i)  sprintf(buffer.message, str, buffer.name);
            else    strcpy(buffer.message, str);

            return 1;
        }

        return 0;
    }

    return 1;
}

char CUSTOMITEM_SETUP::SetupGameMode(const char* str)
{
    int nPar, i = 0; char val[256];
    int16_t nFlags;

    nFlags = (kFlagItemNoPickupS | kFlagItemNoPickupB | kFlagItemNoPickupC | kFlagItemNoPickupT);

    while ((i = enumStr(i, str, val)) != 0)
    {
        switch (nPar = FindParam(val, gParItemGametype))
        {
            case kParItemGameS:             nFlags &= ~kFlagItemNoPickupS;     break;
            case kParItemGameB:             nFlags &= ~kFlagItemNoPickupB;     break;
            case kParItemGameC:             nFlags &= ~kFlagItemNoPickupC;     break;
            case kParItemGameT:             nFlags &= ~kFlagItemNoPickupT;     break;
        }
    }

    buffer.flags ^= nFlags;
    return 1;
}

char CUSTOMITEM_SETUP::SetupFlags(const char* str)
{
    int nPar, i = 0; char val[256];
    while ((i = enumStr(i, str, val)) != 0)
    {
        switch (nPar = FindParam(val, gParItemFlags))
        {
            case kParItemFlagsNoMsg:        buffer.flags |= kFlagItemNoMessage;     break;
            case kParItemFlagsNoEff:        buffer.flags |= kFlagItemNoEffect;      break;
            case kParItemFlagsShared:       buffer.flags |= kFlagItemShared;        break;
            case kParItemFlagsExtLimits:    buffer.flags |= kFlagItemExtLimits;     break;
        }
    }

    return 1;
}

int CUSTOMITEM_SETUP::CheckVersion(void)
{
    int nRetn = 0; char major[16];
    const char* pValue = NULL;

    if (pDesc)
    {
        if (!pDesc->SectionExists("General"))
        {
            Message("Section \"General\" not found!");
            return nRetn;
        }

        pValue = pDesc->GetKeyString("General", "Version");
        if (pValue && rngok(Bstrlen(pValue), 0, 5))
        {
            major[0] = pValue[0]; major[1] = '\0';
            if (isdigit(major[0]))
                nRetn = atoi(major);
        }
    }

    return nRetn;
}

void CUSTOMITEM_SETUP::ClearItem(ITEM* pItem)
{
    if (pItem->action)
        free(pItem->action), pItem->action = NULL;

    if (pItem->name)
        free(pItem->name), pItem->name = NULL;

    if (pItem->message)
        free(pItem->message), pItem->message = NULL;

    pItem->numactions = 0;
    pItem->numeffects = 0;
}

int CUSTOMITEM_SETUP::QsortActions(ITEM::ACTION* ref1, ITEM::ACTION* ref2)
{
    UNREFERENCED_PARAMETER(ref2);
    if (ref1->type == kItemActionEffect)
        return 1;

    return 0;
}

int CUSTOMITEM_SETUP::FindParam(const char* str, const char** pDb)
{
    const char** p; int n;
    for (p = pDb, n = 0; *p; p++, n++)
        if (Bstrcasecmp(str, *p) == 0)
            return n;

    return -1;
}

int CUSTOMITEM_SETUP::SetMaxAmount(int nAmount, int nMax)
{
    //if (nAmount)
        return ClipHigh(nAmount, nMax);

    //return nMax;
}

int CUSTOMITEM_SETUP::SetMinAmount(int nAmount, int nMin)
{
    //if (nAmount)
        return ClipLow(nAmount, nMin);

    //return nMin;
}

const char* CUSTOMITEM_SETUP::GetGameItemName(int nType)
{
    const char* p = NULL;

    if (rngok(nType, kItemWeaponBase, kItemWeaponMax))          p = gWeaponText[nType - kItemWeaponBase];
    else if (rngok(nType, kItemAmmoBase, kItemAmmoMax))         p = gAmmoText[nType - kItemAmmoBase];
    else if (rngok(nType, kItemBase, kItemMax))                 p = gItemText[nType - kItemBase];

    if (isempty(p))
        p = "Unnamed";

    return p;
}
#endif