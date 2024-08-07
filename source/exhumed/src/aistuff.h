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

#ifndef __aistuff_h__
#define __aistuff_h__

#include "compat.h"
#include "anubis.h"
#include "bubbles.h"
#include "mummy.h"
#include "rex.h"
#include "roach.h"
#include "scorp.h"
#include "spider.h"
#include "lion.h"
#include "set.h"
#include "queen.h"
#include "wasp.h"
#include "rat.h"
#include "gun.h"
#include "grenade.h"
#include "snake.h"
#include "fish.h"
#include "lavadude.h"
#include "bullet.h"

#define KMaxTimeSlots	16

void InitTimeSlot();
int GrabTimeSlot(int nVal);

#endif
