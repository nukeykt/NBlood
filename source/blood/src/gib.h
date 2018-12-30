#pragma once


enum GIBTYPE {
    GIBTYPE_0 = 0,
    GIBTYPE_1,
    GIBTYPE_2,
    GIBTYPE_3,
    GIBTYPE_4,
    GIBTYPE_5,
    GIBTYPE_6,
    GIBTYPE_7,
    GIBTYPE_8,
    GIBTYPE_9,
    GIBTYPE_10,
    GIBTYPE_11,
    GIBTYPE_12,
    GIBTYPE_13,
    GIBTYPE_14,
    GIBTYPE_15,
    GIBTYPE_16,
    GIBTYPE_17,
    GIBTYPE_18,
    GIBTYPE_19,
    GIBTYPE_20,
    GIBTYPE_21,
    GIBTYPE_22,
    GIBTYPE_23,
    GIBTYPE_24,
    GIBTYPE_25,
    GIBTYPE_26,
    GIBTYPE_27,
    GIBTYPE_28,
    GIBTYPE_29,
    GIBTYPE_30,
    kGibMax
};

class CGibPosition {
public:
    int x, y, z;
    CGibPosition(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};

class CGibVelocity {
public:
    int vx, vy, vz;
    CGibVelocity(int _vx, int _vy, int _vz) : vx(_vx), vy(_vy), vz(_vz) {}
};

void GibSprite(SPRITE *pSprite, GIBTYPE nGibType, CGibPosition *pPos, CGibVelocity *pVel);
//void GibFX(int nWall, GIBFX * pGFX, int a3, int a4, int a5, int a6, CGibVelocity * pVel);
void GibWall(int nWall, GIBTYPE nGibType, CGibVelocity *pVel);
