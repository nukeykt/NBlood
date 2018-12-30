#pragma once

#include "resource.h"

extern long costable[2048];

int GetOctant(int x, int y);
void RotateVector(long *dx, long *dy, int nAngle);
void RotatePoint(long *x, long *y, int nAngle, int ox, int oy);
void trigInit(Resource &Res);

inline long Sin(int ang)
{
    return costable[(ang - 512) & 2047];
}

inline long Cos(int ang)
{
    return costable[ang & 2047];
}