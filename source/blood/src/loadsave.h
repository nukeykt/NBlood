#pragma once
#include <stdio.h>
#include "levels.h"

class LoadSave {
public:
    static LoadSave head;
    static FILE *hFile;
    LoadSave *prev;
    LoadSave *next;
    LoadSave() {
        prev = head.prev;
        prev->next = this;
        next = &head;
        next->prev = this;
    }
    LoadSave(int dummy)
    {
        next = prev = this;
    }
    //~LoadSave() { }
    virtual void Save(void);
    virtual void Load(void);
    void Read(void *, int);
    void Write(void *, int);
    static void LoadGame(char *);
    static void SaveGame(char *);
};

extern unsigned int gSavedOffset;
extern GAMEOPTIONS gSaveGameOptions[];
extern char *gSaveGamePic[10];

void UpdateSavedInfo(int nSlot);
void LoadSavedInfo(void);

