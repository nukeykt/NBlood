#pragma once
#include "build.h"
#include "common_game.h"

class CEndGameMgr {
public:
    char at0;
    char at1;
    CEndGameMgr();
    void Setup(void);
    void ProcessKeys(void);
    void Draw(void);
    void Finish(void);
};

class CKillMgr {
public:
    int at0, at4;
    CKillMgr();
    void SetCount(int);
    void sub_263E0(int);
    void AddKill(SPRITE *pSprite);
    void sub_2641C(void);
    void Clear(void);
    void Draw(void);
};

class CSecretMgr {
public:
    int at0, at4, at8;
    CSecretMgr();
    void SetCount(int);
    void Found(int);
    void Clear(void);
    void Draw(void);
};

extern CEndGameMgr gEndGameMgr;
extern CSecretMgr gSecretMgr;
extern CKillMgr gKillMgr;
