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

#ifndef __map_h__
#define __map_h__

#include "compat.h"

extern int bShowTowers;
extern int bFollowMode;
extern int ldMapZoom;
extern int lMapZoom;

extern int mapForward;
extern int mapStrafe;
extern fix16_t mapTurn;

extern int32_t nMapPic;

void InitMap();
void GrabMap();
void UpdateMap();
void DrawMap();
void SetMapPosition(int32_t _x, int32_t _y, int16_t _nAngle);

#endif
