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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "crc32.h"
#include "compat.h"
#include "cache1d.h"
#ifdef WITHKPLIB
#include "kplib.h"
#endif
#include "common_game.h"

#include "misc.h"
#include "qheap.h"
#include "resource.h"

CACHENODE Resource::purgeHead = { NULL, &purgeHead, &purgeHead, 0 };

QHeap *Resource::heap;

Resource::Resource(void)
{
    dict = NULL;
    indexName = NULL;
    indexId = NULL;
    buffSize = 0;
    count = 0;
    handle = -1;
    crypt = true;
}

Resource::~Resource(void)
{
    if (dict)
    {
        for (unsigned long i = 0; i < count; i++)
        {
            if (dict[i].type)
                Free(dict[i].type);
            if (dict[i].name)
                Free(dict[i].name);
        }
        Free(dict);
        dict = NULL;
        buffSize = 0;
        count = 0;
    }
    if (handle != -1)
    {
        kclose(handle);
    }
}

void Resource::Init(const char *filename)
{
    RFFHeader header;
    dassert(heap != NULL);

    if (filename)
    {
        handle = kopen4load(filename, 0);
        if (handle != -1)
        {
            int nFileLength = kfilelength(handle);
            dassert(nFileLength != -1);
            if (kread(handle, &header, sizeof(RFFHeader)) != sizeof(RFFHeader)
                || memcmp(header.sign, "RFF\x1a", 4))
            {
                ThrowError("RFF header corrupted");
            }
            switch (header.version & 0xff00)
            {
            case 0x200:
                crypt = 0;
                break;
            case 0x300:
                crypt = 1;
                break;
            default:
                ThrowError("Unknown RFF version");
                break;
            }
            count = header.filenum;
            if (count)
            {
                buffSize = 1;
                while (count * 2 >= buffSize)
                {
                    buffSize *= 2;
                }
                dict = (DICTNODE*)Alloc(buffSize * sizeof(DICTNODE));
                memset(dict, 0, buffSize * sizeof(DICTNODE));
                DICTNODE_FILE *tdict = (DICTNODE_FILE*)Alloc(count*sizeof(DICTNODE_FILE));
                int r = klseek(handle, header.offset, SEEK_SET);
                dassert(r != -1);
                if ((uint32_t)kread(handle, tdict, count * sizeof(DICTNODE_FILE)) != count*sizeof(DICTNODE_FILE))
                {
                    ThrowError("RFF dictionary corrupted");
                }
                if (crypt)
                {
                    Crypt(tdict, count * sizeof(DICTNODE_FILE),
                        header.offset + (header.version & 0xff) * header.offset);
                }
                for (unsigned long i = 0; i < count; i++)
                {
                    dict[i].offset = tdict[i].offset;
                    dict[i].size = tdict[i].size;
                    dict[i].flags = tdict[i].flags;
                    int nTypeLength = strnlen(tdict[i].type, 3);
                    int nNameLength = strnlen(tdict[i].name, 8);
                    dict[i].type = (char*)Alloc(nTypeLength+1);
                    dict[i].name = (char*)Alloc(nNameLength+1);
                    strncpy(dict[i].type, tdict[i].type, 3);
                    strncpy(dict[i].name, tdict[i].name, 8);
                    dict[i].type[nTypeLength] = 0;
                    dict[i].name[nNameLength] = 0;
                    dict[i].id = tdict[i].id;
                }
                Free(tdict);
            }
        }
    }
    if (!dict)
    {
        buffSize = 16;
        dict = (DICTNODE*)Alloc(buffSize * sizeof(DICTNODE));
        memset(dict, 0, buffSize * sizeof(DICTNODE));
    }
    Reindex();
#if 0
    if (external)
    {
        char fname[BMAX_PATH];
        char type[BMAX_PATH];
        BDIR *dirr;
        struct Bdirent *dirent;
        dirr = Bopendir("./");
        if (dirr)
        {
            while (dirent = Breaddir(dirr))
            {
                if (!Bwildmatch(dirent->name, external))
                    continue;
                _splitpath(dirent->name, NULL, NULL, fname, type);
                if (type[0] == '.')
                {
                    AddExternalResource(fname, &type[1], dirent->size);
                }
                else
                {
                    AddExternalResource(fname, "", dirent->size);
                }
            }
            Bclosedir(dirr);
        }
#if 0
        _splitpath2(external, out, &dir, &node, NULL, NULL);
        _makepath(ext, dir, node, NULL, NULL);
        int status = _dos_findfirst(external, 0, &info);
        while (!status)
        {
            _splitpath2(info.name, out, NULL, NULL, &fname, &type);
            if (*type == '.')
            {
                AddExternalResource(*fname, (char*)(type + 1), info.size);
            }
            else
            {
                AddExternalResource(*fname, "", info.size);
            }
            status = _dos_findnext(&info);
        }
        _dos_findclose(&info);
#endif
    }
#endif
    for (unsigned long i = 0; i < count; i++)
    {
        if (dict[i].flags & DICT_LOCK)
        {
            Lock(&dict[i]);
        }
    }
    for (unsigned long i = 0; i < count; i++)
    {
        if (dict[i].flags & DICT_LOAD)
        {
            Load(&dict[i]);
        }
    }
}

void Resource::Flush(CACHENODE *h)
{
    if (h->ptr)
    {
        heap->Free(h->ptr);
        h->ptr = NULL;
        if (h->lockCount == 0)
        {
            RemoveMRU((CACHENODE*)h);
            return;
        }
        h->lockCount = 0;
    }
}

void Resource::Purge(void)
{
    for (unsigned long i = 0; i < count; i++)
    {
        if (dict[i].ptr)
        {
            Flush((CACHENODE *)&dict[i]);
        }
    }
}

DICTNODE **Resource::Probe(const char *fname, const char *type)
{
    char name[BMAX_PATH];
    dassert(indexName != NULL);
    memset(name, 0, sizeof(name));
    strcpy(name, type);
    strcat(name, fname);
    dassert(dict != NULL);
    unsigned long hash = Bcrc32(name, strlen(name), 0) & (buffSize - 1);
    unsigned long i = hash;
    do
    {
        if (!indexName[i])
        {
            return &indexName[i];
        }
        if (!strcmp((*indexName[i]).type, type)
            && !strcmp((*indexName[i]).name, fname))
        {
            return &indexName[i];
        }
        if (++i == buffSize)
        {
            i = 0;
        }
    } while (i != hash);
    ThrowError("Linear probe failed to find match or unused node!");
    return NULL;
}

DICTNODE **Resource::Probe(unsigned long id, const char *type)
{
    struct {
        int id;
        char type[BMAX_PATH];
    } name;
    dassert(indexName != NULL);
    memset(&name, 0, sizeof(name));
    strcpy(name.type, type);
    name.id = id;
    dassert(dict != NULL);
    unsigned long hash = Bcrc32(&name, strlen(name.type)+sizeof(name.id), 0) & (buffSize - 1);
    unsigned long i = hash;
    do
    {
        if (!indexId[i])
        {
            return &indexId[i];
        }
        if (!strcmp((*indexId[i]).type, type)
            && (*indexId[i]).id == id)
        {
            return &indexId[i];
        }
        if (++i == buffSize)
        {
            i = 0;
        }
    } while (i != hash);
    ThrowError("Linear probe failed to find match or unused node!");
    return NULL;
}

void Resource::Reindex(void)
{
    if (indexName)
    {
        Free(indexName);
    }
    indexName = (DICTNODE **)Alloc(buffSize * sizeof(DICTNODE*));
    memset(indexName, 0, buffSize * sizeof(DICTNODE*));
    for (unsigned long i = 0; i < count; i++)
    {
        DICTNODE **node = Probe(dict[i].name, dict[i].type);
        *node = &dict[i];
    }

    if (indexId)
    {
        Free(indexId);
    }
    indexId = (DICTNODE **)Alloc(buffSize * sizeof(DICTNODE*));
    memset(indexId, 0, buffSize * sizeof(DICTNODE*));
    for (unsigned long i = 0; i < count; i++)
    {
        if (dict[i].flags & DICT_ID)
        {
            DICTNODE **node = Probe(dict[i].id, dict[i].type);
            *node = &dict[i];
        }
    }
}

void Resource::Grow(void)
{
    buffSize *= 2;
    void *p = Alloc(buffSize * sizeof(DICTNODE));
    memset(p, 0, buffSize * sizeof(DICTNODE));
    memcpy(p, dict, count * sizeof(DICTNODE));
    Free(dict);
    dict = (DICTNODE*)p;
    Reindex();
}

void Resource::AddExternalResource(const char *name, const char *type, int id)
{
    char name2[BMAX_PATH], type2[BMAX_PATH], filename[BMAX_PATH*2];
    //if (strlen(name) > 8 || strlen(type) > 3) return;
    sprintf(filename, "%s.%s", name, type);
    int fhandle = kopen4loadfrommod(filename, 0);
    if (fhandle == -1)
        return;
    int size = kfilelength(fhandle);
    kclose(fhandle);
    strcpy(name2, name);
    strcpy(type2, type);
    Bstrupr(name2);
    Bstrupr(type2);
    dassert(dict != NULL);
    DICTNODE **index = Probe(name2, type2);
    dassert(index != NULL);
    DICTNODE *node = *index;
    if (!node)
    {
        if (2 * count >= buffSize)
        {
            Grow();
        }
        node = &dict[count++];
        index = Probe(name2, type2);
        *index = node;
        if (node->type)
            Free(node->type);
        if (node->name)
            Free(node->name);
        int nTypeLength = strlen(type2);
        int nNameLength = strlen(name2);
        node->type = (char*)Alloc(nTypeLength+1);
        node->name = (char*)Alloc(nNameLength+1);
        strcpy(node->type, type2);
        strcpy(node->name, name2);
    }
    node->size = size;
    node->flags |= DICT_EXTERNAL;
    Flush((CACHENODE*)node);
    if (id != -1)
    {
        index = Probe(id, type2);
        dassert(index != NULL);
        DICTNODE *node = *index;
        if (!node)
        {
            if (2 * count >= buffSize)
            {
                Grow();
            }
            node = &dict[count++];
            index = Probe(id, type2);
            *index = node;
        }
        if (node->type)
            Free(node->type);
        if (node->name)
            Free(node->name);
        int nTypeLength = strlen(type2);
        int nNameLength = strlen(name2);
        node->type = (char*)Alloc(nTypeLength+1);
        node->name = (char*)Alloc(nNameLength+1);
        strcpy(node->type, type2);
        strcpy(node->name, name2);
        node->id = id;
        node->size = size;
        node->flags |= DICT_EXTERNAL;
        Flush((CACHENODE*)node);
    }
}

void *Resource::Alloc(long nSize)
{
    dassert(heap != NULL);
    dassert(nSize != 0);
    void *p = heap->Alloc(nSize);
    if (p)
    {
        return p;
    }
    for (CACHENODE *node = purgeHead.next; node != &purgeHead; node = node->next)
    {
        dassert(node->lockCount == 0);
        dassert(node->ptr != NULL);
        long nFree = heap->Free(node->ptr);
        node->ptr = NULL;
        RemoveMRU(node);
        if (nSize <= nFree)
        {
            p = Alloc(nSize);
            dassert(p != NULL);
            return p;
        }
    }
    ThrowError("Out of memory!");
    return NULL;
}

void Resource::Free(void *p)
{
    dassert(heap != NULL);
    dassert(p != NULL);
    heap->Free(p);
}

DICTNODE *Resource::Lookup(const char *name, const char *type)
{
    char name2[BMAX_PATH], type2[BMAX_PATH];
    dassert(name != NULL);
    dassert(type != NULL);
    //if (strlen(name) > 8 || strlen(type) > 3) return NULL;
    // Try to load external resource first
    AddExternalResource(name, type);
    strcpy(name2, name);
    strcpy(type2, type);
    Bstrupr(type2);
    Bstrupr(name2);
    return *Probe(name2, type2);
}

DICTNODE *Resource::Lookup(unsigned long id, const char *type)
{
    char type2[BMAX_PATH];
    dassert(type != NULL);
    //if (strlen(type) > 3) return NULL;
    strcpy(type2, type);
    Bstrupr(type2);
    return *Probe(id, type2);
}

void Resource::Read(DICTNODE *n)
{
    dassert(n != NULL);
    Read(n, n->ptr);
}

void Resource::Read(DICTNODE *n, void *p)
{
    char filename[BMAX_PATH];
    dassert(n != NULL);
    if (n->flags & DICT_EXTERNAL)
    {
        sprintf(filename, "%s.%s", n->name, n->type);
        int fhandle = kopen4loadfrommod(filename, 0);
        if (fhandle == -1 || (uint32_t)kread(fhandle, p, n->size) != n->size)
        {
            ThrowError("Error reading external resource (%i)", errno);
        }
        kclose(fhandle);
    }
    else
    {
        int r = klseek(handle, n->offset, SEEK_SET);
        if (r == -1)
        {
            ThrowError("Error seeking to resource!");
        }
        if ((uint32_t)kread(handle, p, n->size) != n->size)
        {
            ThrowError("Error loading resource!");
        }
        if (n->flags & DICT_CRYPT)
        {
            int size;
            if (n->size > 0x100)
            {
                size = 0x100;
            }
            else
            {
                size = n->size;
            }
            Crypt(n->ptr, size, 0);
        }
    }
}

void *Resource::Load(DICTNODE *h)
{
    dassert(h != NULL);
    if (h->ptr)
    {
        if (!h->lockCount)
        {
            RemoveMRU((CACHENODE*)h);

            h->prev = purgeHead.prev;
            purgeHead.prev->next = (CACHENODE*)h;
            h->next = &purgeHead;
            purgeHead.prev = (CACHENODE*)h;
        }
    }
    else
    {
        h->ptr = Alloc(h->size);
        Read(h);

        h->prev = purgeHead.prev;
        purgeHead.prev->next = (CACHENODE*)h;
        h->next = &purgeHead;
        purgeHead.prev = (CACHENODE*)h;
    }
    return h->ptr;
}

void *Resource::Load(DICTNODE *h, void *p)
{
    dassert(h != NULL);
    if (p)
    {
        Read(h, p);
    }
    return p;
}

void *Resource::Lock(DICTNODE *h)
{
    dassert(h != NULL);
    if (h->ptr)
    {
        if (h->lockCount == 0)
        {
            RemoveMRU((CACHENODE*)h);
        }
    }
    else
    {
        h->ptr = Alloc(h->size);
        Read(h);
    }

    h->lockCount++;
    return h->ptr;
}

void Resource::Unlock(DICTNODE *h)
{
    dassert(h != NULL);
    dassert(h->ptr != NULL);
    if (h->lockCount > 0)
    {
        h->lockCount--;
        if (h->lockCount == 0)
        {
            h->prev = purgeHead.prev;
            purgeHead.prev->next = (CACHENODE*)h;
            h->next = &purgeHead;
            purgeHead.prev = (CACHENODE*)h;
        }
    }
}

void Resource::Crypt(void *p, long length, unsigned short key)
{
    char *cp = (char*)p;
    for (int i = 0; i < length; i++, key++)
    {
        cp[i] ^= (key >> 1);
    }
}

void Resource::RemoveMRU(CACHENODE *h)
{
    h->prev->next = h->next;
    h->next->prev = h->prev;
}

void Resource::FNAddFiles(fnlist_t * fnlist, const char *pattern)
{
    char filename[BMAX_PATH];
    for (unsigned long i = 0; i < count; i++)
    {
        DICTNODE *pNode = &dict[i];
        if (pNode->flags & DICT_EXTERNAL)
            continue;
        sprintf(filename, "%s.%s", pNode->name, pNode->type);
        if (!Bwildmatch(filename, pattern))
            continue;
        switch (klistaddentry(&fnlist->findfiles, filename, CACHE1D_FIND_FILE, CACHE1D_SOURCE_GRP))
        {
        case -1:
            return;
        case 0:
            fnlist->numfiles++;
            break;
        }
    }
}
