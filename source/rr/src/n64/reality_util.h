#pragma once


int RT_FakeKRand(void);
int RT_KRand2(void);
int RNCDecompress(const char* inbuf, char* outbuf);
void RT_BuildAngleTable(void);
float RT_GetAngle(float dx, float dy);
float RT_AngleMod(float a);
