//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

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

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

// rts.c

#include "build.h"
#include "cache1d.h"
#include "baselayer.h"

#include "mytypes.h"
#include "_rts.h"
#include "rts.h"
#include "cache.h"

extern char ds[];
char lumplockbyte[11];

//=============
// STATICS
//=============

static int32_t numlumps = 0;
static intptr_t *lumpcache = NULL;
static lumpinfo_t *lumpinfo = NULL;              // location of each lump on disk

/*
============================================================================

                                                LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= RTS_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .rts extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

int32_t RTS_AddFile(char *filename)
{
    wadinfo_t  header;
    lumpinfo_t *lump_p;
    buildvfs_kfd handle;
    int32_t      i;
    int32_t      length;
    int32_t      startlump;
    filelump_t *fileinfo, *fileinfoo;

//
// read the entire file in
//      FIXME: shared opens

    handle = kopen4load(filename, 0);
    if (handle == buildvfs_kfd_invalid)
    {
        LOG_F(ERROR, "RTS file %s not found", filename);
        return -1;
    }

    startlump = numlumps;

    // WAD file
//   printf("    Adding %s.\n",filename);
    kread(handle, &header, sizeof(header));
    if (strncmp(header.identification,"IWAD",4))
    {
        LOG_F(ERROR, "RTS file %s doesn't have IWAD id", filename);
        kclose(handle);
        return -1;
    }
    header.numlumps = B_LITTLE32(header.numlumps);
    header.infotableofs = B_LITTLE32(header.infotableofs);
    length = header.numlumps*sizeof(filelump_t);
    fileinfo = fileinfoo = (filelump_t*)Xmalloc(length);
    if (!fileinfo)
    {
        LOG_F(ERROR, "RTS file could not allocate header info");
        kclose(handle);
        return -1;
    }
    klseek(handle, header.infotableofs, SEEK_SET);
    kread(handle, fileinfo, length);

//
// Fill in lumpinfo
//
    lump_p = (lumpinfo_t*)Xrealloc(lumpinfo, (numlumps + header.numlumps)*sizeof(lumpinfo_t));
    if (!lump_p)
    {
        kclose(handle);
        return -1;
    }
    lumpinfo = lump_p;

    numlumps += header.numlumps;

    lump_p = &lumpinfo[startlump];

    for (i=startlump; i<numlumps; i++,lump_p++, fileinfo++)
    {
        lump_p->handle = handle;
        lump_p->position = B_LITTLE32(fileinfo->filepos);
        lump_p->size = B_LITTLE32(fileinfo->size);
        strncpy(lump_p->name, fileinfo->name, 8);
    }

    Xfree(fileinfoo);

    return 0;
}

/*
====================
=
= RTS_Init
=
= Files with a .rts extension are idlink files with multiple lumps
=
====================
*/

void RTS_Init(char *filename)
{
    int32_t length;
    //
    // open all the files, load headers, and count lumps
    //
    numlumps = 0;
    lumpinfo = NULL;   // will be realloced as lumps are added

    // LOG_F(INFO, "RTS Manager Started");
    if (RTS_AddFile(filename)) return;

    if (!numlumps)
    {
        // LOG_F(ERROR, "RTS_Init: no files found");
        return;
    }

    //
    // set up caching
    //
    length = (numlumps) * sizeof(*lumpcache);
    lumpcache = (intptr_t*)Xmalloc(length);
    memset(lumpcache,0,length);
}

/*
====================
=
= RTS_Shutdown
=
= shutdown the RTS system
=
====================
*/

void RTS_Shutdown(void)
{
    if (lumpcache)
    {
#if 0
        int32_t i;

        for (i=0; i<numlumps; i++)
        {
            if (lumpcache[i])
            {
                Xfree(lumpcache[i]);
            }
        }
#endif
        Xfree(lumpcache);
    }
    if (lumpinfo) Xfree(lumpinfo);

    numlumps = 0;
    lumpinfo = NULL;
    lumpcache = NULL;
}


/*
====================
=
= RTS_NumSounds
=
====================
*/

int32_t RTS_NumSounds(void)
{
    return numlumps-1;
}

/*
====================
=
= RTS_SoundLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int32_t RTS_SoundLength(int32_t lump)
{
    lump++;
    if (lump >= numlumps)
        LOG_F(ERROR, "RTS_SoundLength: %i >= numlumps", lump);
    return lumpinfo[lump].size;
}

/*
====================
=
= RTS_GetSoundName
=
====================
*/

char *RTS_GetSoundName(int32_t i)
{
    i++;
    if (i>=numlumps)
        LOG_F(ERROR, "RTS_GetSoundName: %i >= numlumps", i);
    return &(lumpinfo[i].name[0]);
}

/*
====================
=
= RTS_ReadLump
=
= Loads the lump into the given buffer, which must be >= RTS_SoundLength()
=
====================
*/
void RTS_ReadLump(int32_t lump, intptr_t dest)
{
    lumpinfo_t *l;

    if (lump >= numlumps)
        LOG_F(ERROR, "RTS_ReadLump: %i >= numlumps", lump);
    if (lump < 0)
        LOG_F(ERROR, "RTS_ReadLump: %i < 0", lump);
    l = lumpinfo+lump;
    klseek(l->handle, l->position, SEEK_SET);
    kread(l->handle,(void *)dest,l->size);
}

#if 1
/*
====================
=
= RTS_GetSound
=
====================
*/

// allocates off the cache

void *RTS_GetSound(int32_t lump)
{
    lump++;
    if ((uint16_t)lump >= (uint16_t)numlumps)
        LOG_F(ERROR, "RTS_GetSound: %i >= %i", lump, numlumps);

    if (lumpcache[lump] == (intptr_t)NULL)
    {
        lumplockbyte[lump] = CACHE_LOCK_START;
        g_cache.allocateBlock(&lumpcache[lump],(int)RTS_SoundLength(lump-1),&lumplockbyte[lump]);
        RTS_ReadLump(lump, lumpcache[lump]);
    }
    else
    {
        if (lumplockbyte[lump] < CACHE_LOCK_START)
            lumplockbyte[lump] = CACHE_LOCK_START;
        else
            lumplockbyte[lump]++;
    }
    return (void *)lumpcache[lump];
}
#else
/*
====================
=
= RTS_GetSound
=
====================
*/
void *RTS_GetSound(int32_t lump)
{
    lump++;
    if ((uint16_t)lump >= numlumps)
        LOG_F(ERROR, "RTS_GetSound: %i >= numlumps", lump);
    else if (lump < 0)
        LOG_F(ERROR, "RTS_GetSound: %i < 0", lump);

    if (lumpcache[lump] == NULL)
    {
        // read the lump in
        lumpcache[lump] = Xmalloc(RTS_SoundLength(lump-1));
        RTS_ReadLump(lump, lumpcache[lump]);
    }
    return (void *)lumpcache[lump];
}
#endif
