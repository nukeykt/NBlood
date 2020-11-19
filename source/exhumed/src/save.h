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

#ifndef __save_h__
#define __save_h__

#include "engine.h"

class LoadSave
{
public:
    static LoadSave head;
    static buildvfs_fd hSFile;
    static buildvfs_fd hLFile;
    LoadSave* prev;
    LoadSave* next;
    LoadSave()
    {
        prev = head.prev;
        prev->next = this;
        next = &head;
        next->prev = this;
    }
    LoadSave(int dummy)
    {
        UNREFERENCED_PARAMETER(dummy);
        next = prev = this;
    }
    //~LoadSave() { }
    virtual void Save();
    virtual void Load();
    void Read(void*, int);
    void Write(void*, int);
    static void LoadGame(const char*);
    static void SaveGame(const char*);
};

void LoadSaveSetup();

int savegame(int nSlot);
int loadgame(int nSlot);

#endif
