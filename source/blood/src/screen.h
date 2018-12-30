#pragma once


struct LOADITEM {
    int id;
    const char *name;
};

struct RGB {
    char red, green, blue;
};


extern bool DacInvalid;
extern RGB curDAC[256];
extern RGB baseDAC[256];
extern int gGammaLevels;
extern bool gFogMode;
void gSetDacRange(int start, int end, RGB *pPal);
void scrLoadPLUs(void);
void scrLoadPalette(void);
void scrSetPalette(int palId);
void scrSetGamma(int nGamma);
void scrSetupFade(char red, char green, char blue);
void scrSetupUnfade(void);
void scrFadeAmount(int amount);
void scrSetDac(void);
void scrInit(void);
void scrUnInit(void);
void scrSetGameMode(int vidMode, int XRes, int YRes, int nBits);
void scrNextPage(void);