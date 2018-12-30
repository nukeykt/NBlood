#pragma once

extern long gFrameClock;
extern int gFrameTicks;
extern int gFrame;
extern long volatile gGameClock;
extern int gFrameRate;
extern long gGamma;
extern int gSaveGameNum;


extern bool gPaused;
extern bool gSaveGameActive;
extern bool gSavingGame;
extern bool gQuitGame;
extern bool gQuitRequest;
extern int gSaveGameNum;
extern int gCacheMiss;

const char *GetVersionString(void);