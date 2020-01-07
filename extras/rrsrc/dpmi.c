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
#include <string.h>
#include <dos.h>
#include <i86.h>
#include "dpmi.h"

unsigned long dosStack = 0;
unsigned long dosStackTop = 0;
unsigned short stackSelector = 0;

long total_bytes = 0;
static unsigned long dpmiDosBuffer = 0;
static unsigned short dpmiDosSelector = 0;
unsigned long dpmiVirtualMemory = 0;
unsigned long dpmiAvailableMemory = 0;
unsigned long dpmiPhysicalMemory = 0;
unsigned long dpmiDosMemory = 0;


unsigned int dpmiFindDosMemory(void)
{
    union REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x100;
    regs.x.ebx = 0xffff;
    int386(0x31,&regs,&regs);
    if (regs.x.cflag)
    {
        return (regs.x.ebx&0xffff)<<4;
    }
    return 0xa0000;
}

unsigned int dpmiRealMalloc(int size, short *s)
{
    union REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x100;
    regs.x.ebx = (size+15)>>4;
    int386(0x31,&regs,&regs);
    if (regs.x.cflag)
        return 0;
    if (s)
        *s = regs.x.edx;
    return ((regs.x.eax&0xffff)<<4);
}

void dpmiRealFree(unsigned short s)
{
    union REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x101;
    regs.x.ebx = s;
    int386(0x31, &regs, &regs);
}

void dpmiRealInt386x(char i, dpmiregs_t *req)
{
    union REGS regs;
    struct SREGS seg;
    memset(&regs, 0, sizeof(regs));
    memset(&seg, 0, sizeof(seg));

    regs.w.ax = 0x300;
    regs.h.bl = i;
    regs.h.bh = 0;
    regs.w.cx = 0;
    seg.es = FP_SEG(req);
    regs.x.edi = (unsigned int)req;
    req->ss = (dosStackTop >> 4) & 0xffff;
    req->sp = dosStackTop & 15;
    int386x(0x31,&regs,&regs,&seg);
}

unsigned int dpmiUnlockRegion(unsigned int a, unsigned int b)
{
    unsigned int s;
    union REGS regs;
    s = a;
    total_bytes -= b;
    memset(&regs, 0, sizeof(regs));
    regs.w.ax = 0x601;
    regs.w.bx = s>>16;
    regs.w.cx = s;
    regs.w.si = b>>16;
    regs.w.di = b;
    int386(0x31, &regs, &regs);
    return !regs.x.cflag;
}

unsigned int dpmiLockRegion(unsigned int a, unsigned int b)
{
    unsigned int s;
    union REGS regs;
    s = a;
    total_bytes += b;
    memset(&regs, 0, sizeof(regs));
    regs.w.ax = 0x600;
    regs.w.bx = s>>16;
    regs.w.cx = s;
    regs.w.si = b>>16;
    regs.w.di = b;
    int386(0x31, &regs, &regs);
    return !regs.x.cflag;
}

extern void cdecl _GETDS();
extern void cdecl cstart_();

int dpmiInit(void)
{
    meminfo_t meminfo;
    union REGS regs;
    struct SREGS seg;
    dpmiDosMemory = dpmiFindDosMemory();
    dpmiDosBuffer = dpmiRealMalloc(1024, &dpmiDosSelector);
    if (!dpmiDosBuffer)
    {
        dpmiDosSelector = 0;
        return 0;
    }
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x400;
    int386(0x31, &regs, &regs);
    if (regs.x.cflag == 0)
        if (regs.w.bx & 0x04)
        dpmiVirtualMemory = 1;
    memset(&regs, 0, sizeof(regs));
    memset(&seg, 0, sizeof(seg));
    regs.x.eax = 0x500;
    seg.es = FP_SEG(&meminfo);
    regs.x.edi = (unsigned)&meminfo;
    int386x(0x31,&regs,&regs,&seg);
    if (regs.x.cflag == 0)
    {
        dpmiPhysicalMemory = meminfo.at18<<12;
        dpmiAvailableMemory = meminfo.atC<<12;
    }
    else
    {
        dpmiPhysicalMemory = 0x1000000;
        dpmiAvailableMemory = 0x1000000;
    }
    if (!dpmiLockRegion((int)_GETDS, 0x1000))
        return 0;
    if (!dpmiLockRegion((int)cstart_, 0x1000))
        return 0;
    if (!dpmiLockRegion((int)_chain_intr, 0x1000))
        return 0;
    dosStack = dpmiRealMalloc(0x800, &stackSelector);
    if (!dosStack)
        return 0;

    dosStackTop = dosStack + 0x800;
    return 1;
}

void dpmiUninit(void)
{
    unsigned short v4, v8;
    union REGS regs;
    if (dpmiDosSelector)
    {
        dpmiDosBuffer = 0;
        dpmiDosSelector = 0;
    }
    v4 = v8 = stackSelector;
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x101;
    regs.x.ebx = v4;
    int386(0x31, &regs, &regs);
}

void *dpmiGetTempLowBuffer(int size)
{
    if (!dpmiDosBuffer)
        return 0;
    if (size > 0x400)
        return 0;
    return (void*)dpmiDosBuffer;
}
