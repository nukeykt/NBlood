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
#include <stdio.h>
#include <dos.h>
#include <i86.h>
//#include "duke3d.h"
#include "dpmi.h"


char cdlotrack;
long cdromtime;
short oldtrack;
short whichtrack;
short cddrives;

typedef struct
{
    unsigned char size;
    unsigned char subsize;
    unsigned char cmd;
    unsigned short status;
    unsigned char reserved[8];
} reqst_t;

typedef struct
{
    unsigned char desc;
    unsigned short infooffset;
    unsigned short infoseg;
    unsigned short length;
    unsigned short startsize;
    int volume;
} ioctl_t;

extern short cdon;
extern short numplayers;

static struct {
    char code;
    char lotrack;
    char hitrack;
    int start;
} AudioInfo = { 10 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} AudioInfoReq = { { 13,0,3 }, { 0 } };

static struct {
    char code;
    char trackn;
    int start;
    char control;
} AudioTrackInfo = { 11,2 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} AudioTrackInfoReq = { { 13,0,3 }, { 0 } };

static struct {
    char code;
    int parm;
} DevStatus = { 6 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} DevStatusReq = { { 13,0,3 }, { 0 } };

static struct {
    char code;
    char byte;
} MediaChg = { 9 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} MediaChgReq = { { 13,0,3 }, { 0 } };

static struct {
    reqst_t req;
    char mode;
    unsigned long start;
    unsigned long len;
    char pad;
} PlayReq = { { 13,0,132 }, 0 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} StopReq = { { 13,0,133 }, { 0 } };

static struct {
    char code;
    int volume;
} VolumeSize = { 8 };

static struct {
    reqst_t req;
    ioctl_t ioctl;
} VolumeSizeReq = { { 13,0,3 }, { 0 } };

char cdhitrack;
static char rbEnabled;
static char cdDrive;

int rbSendRequest(char *req, char *info, int size);

int rbInit(void)
{
    union REGS regs;
    regs.x.eax = 0x1500;
    regs.x.ebx = 0;
    int386(0x2f,&regs,&regs);
    if (!regs.x.ebx)
        return 0;

    cdDrive = regs.h.cl;
    rbEnabled = 1;
    rbSendRequest((char*)&DevStatusReq,(char*)&DevStatus,5);
    if ((DevStatus.parm & 2048) || (DevStatus.parm & 1))
    {
        rbEnabled = 0;
        return 0;
    }
    return 1;
}

static long cdTrackStart[101];
int rbRegisterCD(void)
{
    int i;
    rbSendRequest((char*)&AudioInfoReq,(char*)&AudioInfo,7);
    cdlotrack = AudioInfo.lotrack;
    cdhitrack = AudioInfo.hitrack;
    for (i = AudioInfo.lotrack; i <= AudioInfo.hitrack; i++)
    {
        AudioTrackInfo.trackn = i;
        rbSendRequest((char*)&AudioTrackInfoReq,(char*)&AudioTrackInfo,8);
        cdTrackStart[i] = AudioTrackInfo.start;
    }
    cdTrackStart[i] = AudioInfo.start;
    return 1;
}

int rbGetTotalTracks(void)
{
    if(rbCheckMediaChange())
        rbSendRequest((char*)&AudioInfoReq,(char*)&AudioInfo,7);
    return AudioInfo.hitrack;
}

int rbGetVolumeSize(void)
{
    rbSendRequest((char*)&VolumeSizeReq,(char*)&VolumeSize,5);
    return VolumeSize.volume;
}

unsigned long mtol(unsigned long a);

void rbPlayTrack(long track)
{
    long tm, hi, mi, v4;
    if (!cdon)
        return;
    if (!rbCheckMediaChange())
        return;

    PlayReq.mode = 1;
    PlayReq.start = cdTrackStart[track];
    PlayReq.len = mtol(cdTrackStart[track+1])-mtol(cdTrackStart[track]);
    tm = mtol(cdTrackStart[track+1])-mtol(cdTrackStart[track]);
    tm += 150;
    hi = tm / 4500;
    tm %= 4500;
    mi = tm / 75;
    tm %= 75;
    v4 = tm;
    cdromtime = (hi * 60 + mi) * 30;
    rbSendRequest((char*)&PlayReq,0,0);
}

void rbstop(void)
{
    rbSendRequest((char*)&StopReq,0,0);
}

static int rbCheckMediaChange(void)
{
    rbSendRequest((char*)&MediaChgReq, (char*)&MediaChg, 2);
    if (MediaChg.byte != 1)
    {
        rbEnabled = 0;
        return 0;
    }
    return 1;
}

static int rbSendRequest(char *req, char *info,int size)
{
    dpmiregs_t dregs;
    ioctl_t *ioctl;
    char *buf;
    char *infobuf;
    unsigned short status;

    memset(&dregs, 0, sizeof(dregs));
    buf = (char*)dpmiGetTempLowBuffer(128+size);
    memcpy(buf, req, 26);
    ioctl = (ioctl_t*)(buf+sizeof(reqst_t));
    if (info && size)
    {
        infobuf = buf + 128;
        memcpy(infobuf, info, size);
        ioctl->infooffset = ((unsigned int)infobuf) & 0xf;
        ioctl->infoseg = (((unsigned int)infobuf)>>4)&0xffff;
        ioctl->length = size;
    }
    dregs.eax = 0x1510;
    dregs.ecx = (unsigned short)cdDrive;
    dregs.es = (((unsigned int)buf)>>4)&0xffff;
    dregs.ebx = ((unsigned int)buf)&0xf;
    dpmiRealInt386x(0x2f,&dregs);
    if (info && size)
        memcpy(info, infobuf, size);
    memcpy(req, buf, sizeof(reqst_t)+sizeof(ioctl_t));

    status = ((reqst_t*)req)->status;
    if (status & 0x8000)
        rbEnabled = 0;
    return status & 0x7fff;
}

static long ltom(unsigned long a)
{
    unsigned long hi, mi, lo;
    a += 150;
    hi = a / 4500;
    a %= 4500;
    mi = a / 75;
    a %= 75;
    lo = a;

    return (hi<<16)+(mi<<8)+lo;
}

static unsigned long mtol(unsigned long a)
{
    unsigned long hi = (a>>16)&255;
    unsigned long mi = (a>>8)&255;
    unsigned long lo = a & 255;
    return 4500*hi+75*mi+lo;
}

#ifdef RRRA
extern short word_1D7EF8;
#endif

void initcdrom(void)
{
    int totaltracks;
    if (numplayers > 1)
    {
        cdon = 0;
        cddrives = 0;
        return;
    }
    setbuf(stdout, 0);
    dpmiInit();
    rbInit();
    rbRegisterCD();
    totaltracks = rbGetTotalTracks();
    if (totaltracks)
    {
        cddrives = 1;
        cdlotrack = 1;
        whichtrack = cdlotrack;
        cdhitrack = 9;
        oldtrack = -1;
#ifdef RRRA
        cdon = 0;
        word_1D7EF8 = 0;
#else
        cdon = 1;
#endif
    }
    else
    {
        cddrives = 0;
        rbstop();
        dpmiUninit();
    }
}

void shutdowncdrom(void)
{
    rbstop();
    dpmiUninit();
}
