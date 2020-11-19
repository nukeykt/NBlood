#pragma once

#include <stdint.h>

#define MAX_SONGS 8
#define MAX_TRACKS 32

enum {
    SONG_FADE_IN = 1,
    SONG_FADE_OUT = 2,
    SONG_FADE_OUT_STOP = 4
};

#pragma pack(push, 1)

struct HMIHeader {
    char signature[32]; // HMIMIDIP013195
    uint32_t branchoffset;
    uint8_t _pad1[12];
    uint32_t numtracks;
    uint32_t tpqn;
    uint32_t tempo;
    uint32_t timetoplay;
    uint32_t priority[16];
    uint32_t trackdevice[32][5];
    uint8_t ccrestore[128];
    uint8_t _pad2[8];
};

struct HMITrack {
    uint32_t id;
    uint32_t size;
    uint32_t channel;
};

struct HMIBranch {
    uint32_t offset;
    uint8_t id;
    uint8_t patch;
    uint8_t loopcount;
    uint8_t cccount;
    uint8_t _pad1[4];
    uint32_t ccoffset;
    uint32_t reserved1;
    uint32_t reserved2;
};

#pragma pack(pop)

struct ChannelData {
    uint8_t used;
    uint8_t bend;
    uint8_t volume;
    uint8_t patch;
    uint8_t sustain;
};

struct InitSong {
    uint8_t *songptr;
    void (*callback)(uint32_t);
};

struct TrackDevice {
    uint32_t trackdevice[32];
};

int FMSetBank(void *_bank);

void HMIInit(int rate);
void HMIUnInit(void);
uint32_t HMIStopSong(uint32_t handle);
uint32_t HMIUnInitSong(uint32_t handle);
uint32_t HMISetMasterVolume(uint8_t volume);
uint32_t HMISongDone(uint32_t handle);
uint32_t HMIFadeSong(uint32_t handle, uint32_t flags, uint32_t speed, uint8_t vol1, uint8_t vol2, uint32_t speed2);
uint32_t HMIInitSong(InitSong *song, TrackDevice *trackmap, uint32_t *handleptr);
uint32_t HMIResetSong(uint32_t handle, InitSong *song);
uint32_t HMIStartSong(uint32_t handle);
