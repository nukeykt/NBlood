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

#include "build.h"
#include "player.h"
#include "actor.h"
#include "levels.h"

inline bool IsTargetTeammate(PLAYER *pSourcePlayer, spritetype *pTargetSprite)
{
    if (pSourcePlayer == NULL)
        return false;
    if (!IsPlayerSprite(pTargetSprite))
        return false;
    if (gGameOptions.nGameType == 1 || gGameOptions.nGameType == 3)
    {
        PLAYER *pTargetPlayer = &gPlayer[pTargetSprite->type-kDudePlayer1];
        if (pSourcePlayer != pTargetPlayer)
        {
            if (gGameOptions.nGameType == 1)
                return true;
            if (gGameOptions.nGameType == 3 && (pSourcePlayer->at2ea&3) == (pTargetPlayer->at2ea&3))
                return true;
        }
    }

    return false;
}

inline bool IsTargetTeammate(spritetype *pSourceSprite, spritetype *pTargetSprite)
{
    if (!IsPlayerSprite(pSourceSprite))
        return false;
    PLAYER *pSourcePlayer = &gPlayer[pSourceSprite->type-kDudePlayer1];
    return IsTargetTeammate(pSourcePlayer, pTargetSprite);
}