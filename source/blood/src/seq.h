#pragma once
#include "resource.h"
struct SEQFRAME {
    unsigned int tile : 12;
    unsigned int at1_4 : 1; // transparent
    unsigned int at1_5 : 1; // transparent
    unsigned int at1_6 : 1; // blockable
    unsigned int at1_7 : 1; // hittable
    unsigned int at2_0 : 8; // xrepeat
    unsigned int at3_0 : 8; // yrepeat
    signed int at4_0 : 8; // shade
    unsigned int at5_0 : 5; // palette
    unsigned int at5_5 : 1; //
    unsigned int at5_6 : 1; //
    unsigned int at5_7 : 1; //
    unsigned int at6_0 : 1; //
    unsigned int at6_1 : 1; //
    unsigned int at6_2 : 1; // invisible
    unsigned int at6_3 : 1; //
    unsigned int at6_4 : 1; //
    unsigned int pad : 11;
};

struct Seq {
    char signature[4];
    short version;
    short nFrames; // at6
    short at8;
    short ata;
    int atc;
    SEQFRAME frames[1]; // at10
    void sub_53998(void);
    void Preload(void);
};

struct ACTIVE
{
    unsigned char type;
    unsigned short xindex;
};

struct SEQINST
{
    DICTNODE *hSeq;
    Seq *pSequence; // at4
    int at8;
    int atc;
    short at10;
    unsigned char frameIndex; // at12
    char at13;
    void Update(ACTIVE *pActive);
};

int seqRegisterClient(void(*pClient)(int, int));
void seqPreloadId(int id);
SEQINST * GetInstance(int a1, int a2);
void UnlockInstance(SEQINST *pInst);
void seqSpawn(int a1, int a2, int a3, int a4 = -1);
void seqKill(int a1, int a2);
void seqKillAll(void);
int seqGetStatus(int a1, int a2);
int seqGetID(int a1, int a2);
void seqProcess(int a1);