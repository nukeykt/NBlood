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
#include "callback.h"
enum {
kChannelZero                        = 0,
kChannelSetTotalSecrets             = 1,
kChannelSecretFound                 = 2,
kChannelTextOver                    = 3,
kChannelLevelExitNormal             = 4,
kChannelLevelExitSecret             = 5,
kChannelModernEndLevelCustom        = 6, // custom level number end (gModernMap)
kChannelLevelStart                  = 7,
kChannelLevelStartMatch             = 8, // DM and TEAMS
kChannelLevelStartCoop              = 9,
kChannelLevelStartTeamsOnly         = 10,
kChannelPlayerDeathTeamA            = 15,
kChannelPlayerDeathTeamB            = 16,
/////////////////////////////
// level start channels for specific ports
kChannelLevelStartNBLOOD            = 17, // *NBlood only* must trigger it at level start (gModernMap)
kChannelLevelStartRAZE              = 18, // *Raze only* must trigger it at level start (gModernMap)
// channels of players to send commands on
kChannelAllPlayers                  = 29,
kChannelPlayer0                     = 30,
kChannelPlayer1,
kChannelPlayer2,
kChannelPlayer3,
kChannelPlayer4,
kChannelPlayer5,
kChannelPlayer6,
kChannelPlayer7,
// channel of event causer
kChannelEventCauser                 = 50,
// map requires modern features to work properly
kChannelMapModernRev1                = 60,
kChannelMapModernRev2                = 61,
/////////////////////////////
kChannelTeamAFlagCaptured           = 80,
kChannelTeamBFlagCaptured,
kChannelRemoteBomb0                 = 90,
kChannelRemoteBomb1,
kChannelRemoteBomb2,
kChannelRemoteBomb3,
kChannelRemoteBomb4,
kChannelRemoteBomb5,
kChannelRemoteBomb6,
kChannelRemoteBomb7,
kChannelUser                        = 100,
kChannelUserMax                     = 1024,
kChannelMax                         = 4096,
};

struct RXBUCKET
{
    unsigned int index : 14;
    unsigned int type : 3;
};
extern RXBUCKET rxBucket[];
extern unsigned short bucketHead[];

enum COMMAND_ID {
kCmdOff                     = 0,
kCmdOn                      = 1,
kCmdState                   = 2,
kCmdToggle                  = 3,
kCmdNotState                = 4,
kCmdLink                    = 5,
kCmdLock                    = 6,
kCmdUnlock                  = 7,
kCmdToggleLock              = 8,
kCmdStopOff                 = 9,
kCmdStopOn                  = 10,
kCmdStopNext                = 11,
kCmdCounterSector           = 12,
kCmdCallback                = 20,
kCmdRepeat                  = 21,


kCmdSpritePush              = 30,
kCmdSpriteImpact            = 31,
kCmdSpritePickup            = 32,
kCmdSpriteTouch             = 33,
kCmdSpriteSight             = 34,
kCmdSpriteProximity         = 35,
kCmdSpriteExplode           = 36,

kCmdSectorPush              = 40,
kCmdSectorImpact            = 41,
kCmdSectorEnter             = 42,
kCmdSectorExit              = 43,

kCmdWallPush                = 50,
kCmdWallImpact              = 51,
kCmdWallTouch               = 52,
#ifdef NOONE_EXTENSIONS
kCmdSectorMotionPause       = 13,   // stops motion of the sector
kCmdSectorMotionContinue    = 14,   // continues motion of the sector
kCmdDudeFlagsSet            = 15,   // copy dudeFlags from sprite to dude
kCmdEventKillFull           = 16,   // immediately kill the pending object events
kCmdModernUse               = 53,   // used by most of modern types
#endif

kCmdNumberic                = 64, // 64: 0, 65: 1 and so on up to 255
kCmdModernFeaturesEnable    = 100, // must be in object with kChannelMapModernize RX / TX
kCmdNumbericMax             = 255,
};

inline bool playerRXRngIsFine(int rx) {
    return (rx >= kChannelPlayer0 && rx < kChannelPlayer7);
}

inline bool channelRangeIsFine(int channel) {
    return (channel >= kChannelUser && channel < kChannelUserMax);
}

struct EVENT {
    unsigned int index:     14; // index
    unsigned int type:      3; // type
    unsigned int cmd:       8; // cmd
    unsigned int funcID:    8; // callback
    unsigned int causer:    14; // spritenum of object which initiated this event (kCauserGame == initiated by the game)
};

void evInit(void);
char evGetSourceState(int nType, int nIndex);
void evSend(int nIndex, int nType, int rxId, COMMAND_ID command, int causerID);
void evPost(int nIndex, int nType, unsigned int nDelta, COMMAND_ID command, int causerID);
void evPost(int nIndex, int nType, unsigned int nDelta, CALLBACK_ID callback);
void evProcess(unsigned int nTime);
void evKill(int a1, int a2);
void evKill(int idx, int type, int causer);
void evKill(int a1, int a2, CALLBACK_ID a3);