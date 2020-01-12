//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2017-2019 Nuke.YKT

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#ifndef _DPMI_H_
#define _DPMI_H_

typedef struct
{
    unsigned        edi, esi, ebp, reserved, ebx, edx, ecx, eax;
    unsigned short  flags, es, ds, fs, gs, ip, cs, sp, ss;
} dpmiregs_t;

typedef struct
{
    unsigned at0, at4, at8, atC, at10, at14, at18, at1C, at20;
    unsigned char reserved[12];
} meminfo_t;

unsigned int dpmiFindDosMemory(void);
unsigned int dpmiRealMalloc(int size, short *s);
void dpmiRealFree(unsigned short s);
void dpmiRealInt386x(char i, dpmiregs_t *req);
unsigned int dpmiUnlockRegion(unsigned int a, unsigned int b);
unsigned int dpmiLockRegion(unsigned int a, unsigned int b);
int dpmiInit(void);
void dpmiUninit(void);
char *dpmiGetTempLowBuffer(int size);

#endif
