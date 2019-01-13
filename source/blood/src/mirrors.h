#pragma once
#include "fix16.h"
void InitMirrors(void);
void sub_5571C(char mode);
void sub_557C4(int x, int y, int interpolation);
void DrawMirrors(long x, long y, long z, fix16_t a, fix16_t horiz);