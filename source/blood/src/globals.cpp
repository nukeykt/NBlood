#include <stdlib.h>

#include "compat.h"
#include "blood.h"

long gFrameClock;
int gFrameTicks;
int gFrame;
long volatile gGameClock;
int gFrameRate;
long gGamma;
int gSaveGameNum;

bool gQuitGame;
bool gQuitRequest;
bool gPaused;
bool gSaveGameActive;
int gCacheMiss;

char *gVersionString;
char gVersionStringBuf[16];

const char *GetVersionString(void)
{
    if (!gVersionString)
    {
        gVersionString = gVersionStringBuf;
        if (!gVersionString)
            return NULL;
        sprintf(gVersionString, "%d.%02d", BloodVersion >> 8, BloodVersion & 0xff);
    }
    return gVersionString;
}