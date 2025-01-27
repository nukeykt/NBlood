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
#ifndef USERITEMH
#define USERITEMH
#include "compat.h"
#include "common_game.h"

// Item user flags
#define kFlagItemNoEffect       0x0001
#define kFlagItemNoMessage      0x0002
#define kFlagItemShared         0x0004
#define kFlagItemNoPickupS      0x0008
#define kFlagItemNoPickupB      0x0010
#define kFlagItemNoPickupC      0x0020
#define kFlagItemNoPickupT      0x0040
#define kFlagItemNoPickup       (kFlagItemNoPickupS|kFlagItemNoPickupB|kFlagItemNoPickupC|kFlagItemNoPickupT)
#define kFlagItemExtLimits      0x0080

// Item system flags
#define kFlagItemRespawnSpec1   0x2000
#define kFlagItemRespawnSpec2   0x4000
#define kFlagItemHasReqActions  0x8000

// Action user flags
#define kFlagActionReq          0x01

// Added in hope that item system will fully replace current pickups code one day.
// This is a special flag that simulates original code (both vanilla and modern) behavior
// for certain actions mostly to keep demos in sync. For example, it allows to pickup
// armor items when cur slot amount <= 100 and other slots amount < 100.
#define kFlagActionCompat       0x02

extern char gUserItemsInitialized;

#pragma pack(push, 1)
struct ITEM
{
    char *name, *message;
    uint8_t  group, type;
    uint8_t  droplivetime;
    uint8_t  respawntime;
    uint8_t  numactions;
    uint8_t  numeffects;
    int16_t  flags;

    struct ACTION
    {
        uint8_t  operation, type;
        int32_t  amount[3];
        uint8_t  flags;
        uint8_t  slot;
    }
	*action;

    struct APPEARANCE
    {
        uint16_t picnum;
        uint32_t seq;
        uint8_t  xrepeat;
        uint8_t  yrepeat;
        uint8_t  pal;
        int8_t   shade;
        uint32_t sound;
    }
    appearance;
};

struct ACTIONARG
{
    ITEM::ACTION*   act;
    uint8_t         isShared;
    uint8_t         isCompat;
    uint8_t         isTest;
};
#pragma pack(pop)

class CUSTOMITEM_SETUP
{
    private:
        static IniFile* pDesc;
        static ITEM buffer;
        static int version;
//-------------------------------------------------------------------------------
        static ITEM* Setup(char* pGroup);
        static char  SetupAppearance(const char* str);
        static char  SetupActionLimits(ITEM::ACTION* pAct, char extLimits);
        static char  SetupAction(const char* str, int nOperator, int nAction);
        static char  SetupFlags(const char* str);
        static char  SetupRespawn(const char* str);
        static char  SetupLiveTime(const char* str);
        static char  SetupMessage(const char* str);
        static char  SetupGameMode(const char* str);
        static char  SetupType(const char* str);
        static char  SetupName(const char* str);
//-------------------------------------------------------------------------------
        static int   QsortActions(ITEM::ACTION* ref1, ITEM::ACTION* ref2);
        static int   FindParam(const char* str, const char** pDb);
        static int   SetMaxAmount(int nAmount, int nMax);
        static int   SetMinAmount(int nAmount, int nMin);
        static const char* GetGameItemName(int nType);
        static int   CheckVersion(void);
    public:
        static int numNewItemsAdded;
        static int numUserItemsReplaced;
        static int numGameItemsReplaced;
        static int numFailedItems;
        static char showLog;
//-------------------------------------------------------------------------------
        static int  Setup(IniFile* pFile);
        static void ClearItem(ITEM* pItem);
        static void Message(const char* pFormat, ...);

};

int userItemGetRespawnTime(spritetype* pSpr);
char userItemPickup(PLAYER* pPlayer, spritetype* pISpr, ITEM* pItem);
spritetype* userItemDrop(spritetype* pSpr, int nType);
ITEM* userItemGet(int nType);
char userItemViewUseRespawnMarkers(ITEM* pItem);
void userItemsUninit();
int userItemsInit(char showLog);
void userItemsInitSprites();

inline char IsUserItem(int nType)               { return (userItemGet(nType) != NULL); }
inline char IsUserItemSprite(spritetype* pSpr)  { return IsUserItem(pSpr->type); }
#endif
#endif