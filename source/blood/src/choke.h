#pragma once

#include "common_game.h"
#include "player.h"
#include "qav.h"
#include "resource.h"

class CChoke
{
public:
    CChoke()
    {
        at0 = NULL;
        at4 = NULL;
        at8 = NULL;
        at1c = NULL;
        at14 = 0;
        at18 = 0;
    };
    void sub_83F54(char *a1, int _x, int _y, void(*a2)(PLAYER*));
    void sub_83ff0(int a1, void(*a2)(PLAYER*));
    void sub_84080(char *a1, void(*a2)(PLAYER*));
    void sub_84110(int x, int y);
    void sub_84218();
    char *at0;
    DICTNODE *at4;
    QAV *at8;
    int atc;
    int at10;
    int at14;
    int at18;
    void(*at1c)(PLAYER *);
};

void sub_84230(PLAYER*);

extern CChoke gChoke;
