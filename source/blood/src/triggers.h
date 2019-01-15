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
#include "common.h"
#include "common_game.h"

#include "blood.h"
#include "db.h"
#include "eventq.h"

void trTriggerSector(unsigned int nSector, XSECTOR *pXSector, int a3);
void trMessageSector(unsigned int nSector, EVENT a2);
void trTriggerWall(unsigned int nWall, XWALL *pXWall, int a3);
void trMessageWall(unsigned int nWall, EVENT a2);
void trTriggerSprite(unsigned int nSprite, XSPRITE *pXSprite, int a3);
void trMessageSprite(unsigned int nSprite, EVENT a2);
void trProcessBusy(void);
void trInit(void);
void trTextOver(int nId);