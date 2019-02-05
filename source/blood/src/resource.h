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

#include "qheap.h"

#pragma pack(push, 1)

enum DICTFLAGS {
    DICT_ID = 1,
    DICT_EXTERNAL = 2,
    DICT_LOAD = 4,
    DICT_LOCK = 8,
    DICT_CRYPT = 16,
};

struct RFFHeader
{
    char sign[4];
    short version;
    short pad1;
    unsigned long offset;
    unsigned long filenum;
    int pad2[4];
};

struct DICTNODE_FILE
{
    char unused1[16];
    unsigned long offset;
    unsigned long size;
    char unused2[8];
    char flags;
    char type[3];
    char name[8];
    int id;
};

#pragma pack(pop)

struct CACHENODE
{
    void *ptr;
    CACHENODE *prev;
    CACHENODE *next;
    int lockCount;
};

struct DICTNODE
{
    void *ptr;
    CACHENODE *prev;
    CACHENODE *next;
    int lockCount;
    unsigned long offset;
    unsigned long size;
    char flags;
    //char type[3];
    //char name[8];
    char *type;
    char *name;
    int id;
};

class Resource
{
public:
    Resource(void);
    ~Resource(void);

    void Init(const char *filename);
    static void Flush(CACHENODE *h);
    void Purge(void);
    DICTNODE **Probe(const char *fname, const char *type);
    DICTNODE **Probe(unsigned long id, const char *type);
    void Reindex(void);
    void Grow(void);
    void AddExternalResource(const char *name, const char *type, int id = -1);
    static void *Alloc(long nSize);
    static void Free(void *p);
    DICTNODE *Lookup(const char *name, const char *type);
    DICTNODE *Lookup(unsigned long id, const char *type);
    void Read(DICTNODE *n);
    void Read(DICTNODE *n, void *p);
    void *Load(DICTNODE *h);
    void *Load(DICTNODE *h, void *p);
    void *Lock(DICTNODE *h);
    void Unlock(DICTNODE *h);
    void Crypt(void *p, long length, unsigned short key);
    static void RemoveMRU(CACHENODE *h);
    int Size(DICTNODE*h) { return h->size; }
    void FNAddFiles(fnlist_t *fnlist, const char *pattern);

    DICTNODE *dict;
    DICTNODE **indexName;
    DICTNODE **indexId;
    unsigned long buffSize;
    unsigned long count;
    //FILE *handle;
    int handle;
    bool crypt;

    static QHeap *heap;
    static CACHENODE purgeHead;
};