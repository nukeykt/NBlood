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
#include "blood.h"

struct DUDEINFO {
    short seqStartID; // seq
    short at2; // health
    unsigned short at4; // mass
    int at6; // unused?
    char ata; // clipdist
    int atb;
    int atf;
    int at13; // target see range?
    int at17; // target see range?
    int at1b; // target see angle range
    int at1f; // unused?
    int at23; // burn health
    int at27; // recoil damage
    int at2b;
    int at2f;
    int at33;
    char at37;
    int at38; // acceleration
    int at3c; // dodge
    int at40; // unused?
    int at44; // turn speed
    int at48[3];
    int at54[7]; // damage
    int at70[7]; // real damage
    int at8c; // unused ?
    int at90; // unused ?
};

extern DUDEINFO dudeInfo[kDudeMax-kDudeBase];
extern DUDEINFO gPlayerTemplate[2];