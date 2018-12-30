#pragma once
#include "common_game.h"
#include "db.h"
class CViewMap {
public:
    char bActive;
    int x, y, nZoom;
    short angle;
    char bFollowMode;
    int forward, turn, strafe;
    CViewMap();
    void sub_25C38(int, int, int, short, char);
    void sub_25C74(void);
    void sub_25DB0(SPRITE *pSprite);
    void sub_25E84(int *, int*);
    void FollowMode(char);
};

extern CViewMap gViewMap;
