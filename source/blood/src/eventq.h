#pragma once
#include "callback.h"

#define kMaxChannels 4096

enum COMMAND_ID {
    COMMAND_ID_0 = 0,
    COMMAND_ID_1,
    COMMAND_ID_2,
    COMMAND_ID_3,
    COMMAND_ID_4,
    COMMAND_ID_5,
    COMMAND_ID_6,
    COMMAND_ID_7,
    COMMAND_ID_8,
    COMMAND_ID_9,

    kCommandCallback = 20,
    COMMAND_ID_21,

    COMMAND_ID_64 = 64,
};

struct EVENT {
    unsigned int at0_0 : 13; // index
    unsigned int at1_5 : 3; // type
    unsigned int at2_0 : 8; // cmd
    unsigned int funcID : 8; // callback
}; // <= 4 bytes

void evInit(void);
char evGetSourceState(int nType, int nIndex);
void evSend(int nIndex, int nType, int rxId, COMMAND_ID command);
void evPost(int nIndex, int nType, unsigned long nDelta, COMMAND_ID command);
void evPost(int nIndex, int nType, unsigned long nDelta, CALLBACK_ID a4);
void evProcess(unsigned long nTime);
void evKill(int a1, int a2);
void evKill(int a1, int a2, CALLBACK_ID a3);