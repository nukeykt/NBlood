#pragma once

#include "controls.h"
#include "levels.h"

struct DEMOHEADER
{
    int signature;
    short nVersion;
    int nBuild;
    int nInputCount;
    int nNetPlayers;
    short nMyConnectIndex;
    short nConnectHead;
    short connectPoints[8];
    GAMEOPTIONS gameOptions;
};

class CDemo {
public:
    CDemo();
    ~CDemo();
    bool Create(const char *);
    void Write(GINPUT *);
    void Close(void);
    bool SetupPlayback(const char *);
    void ProcessKeys(void);
    void Playback(void);
    void StopPlayback(void);
    void LoadDemoInfo(void);
    void NextDemo(void);
    bool at0; // record
    bool at1; // playback
    char at2;
    int at3;
    FILE *at7;
    int atb;
    DEMOHEADER atf;
    GINPUT at1aa[1024];
    char at59aa[5][13];
    int at59eb;
    int at59ef;
};

extern CDemo gDemo;
