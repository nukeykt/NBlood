#pragma once

#include "controls.h"
#include "levels.h"

#define kInputBufferSize 1024

#pragma pack(push, 1)

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

#pragma pack(pop)

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
    void FlushInput(int nCount);
    void ReadInput(int nCount);
    bool at0; // record
    bool at1; // playback
    char at2;
    int at3;
    FILE *at7;
    int atb;
    DEMOHEADER atf;
    GINPUT at1aa[kInputBufferSize];
    char at59aa[5][13];
    int at59eb;
    int at59ef;
};

extern CDemo gDemo;
