#pragma once

#include "resource.h"
struct SAMPLE2D
{
    int at0;
    char at4;
    DICTNODE *at5;
}; // 9 bytes

struct SFX
{
    int relVol;
    int pitch;
    int pitchRange;
    int format;
    int loopStart;
    char rawName[9];
};

int sndGetRate(int format);
void sndPlaySong(const char *songName, bool bLoop);
bool sndIsSongPlaying(void);
void sndFadeSong(int nTime);
void sndSetMusicVolume(int nVolume);
void sndSetFXVolume(int nVolume);
void sndStopSong(void);
void sndStartSample(const char *pzSound, int nVolume, int nChannel = -1);
void sndStartSample(unsigned long nSound, int nVolume, int nChannel = -1, bool bLoop = false);
void sndStartWavID(unsigned long nSound, int nVolume, int nChannel = -1);
void sndKillAllSounds(void);
void sndProcess(void);
void sndTerm(void);
void sndInit(void);

extern Resource gSoundRes;
