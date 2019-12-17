//-------------------------------------------------------------------------
/*
 Copyright (C) 2007 Jonathon Fowler <jf@jonof.id.au>

 This file is part of JFShadowWarrior

 Shadow Warrior is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
//-------------------------------------------------------------------------

#ifndef grpscan_h__
#define grpscan_h__

// List of internally-known GRP files
#define numgrpfiles 7
struct internalgrpfile
{
    const char *name;
    uint32_t crcval;
    int size;
    unsigned int flags;
    uint32_t dependency;
};

enum
{
    GRP_HAS_DEPENDENCY = 1u<<0u,
};

typedef struct grpfile
{
    char * filename;
    struct internalgrpfile const * type;
    struct grpfile *next;
} grpfile_t;

extern internalgrpfile grpfiles[numgrpfiles];
extern grpfile *foundgrps;

extern grpfile_t * FindGroup(uint32_t crcval);

int ScanGroups(void);
void FreeGroups(void);

void SW_LoadAddon(void);

#endif
