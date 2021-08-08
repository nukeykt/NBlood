#include <string.h>
#include "fx_man.h"
#include "hmpplay.h"
#include "opl3.h"

opl3_chip fm_chip;

void FMSendData(uint8_t *data);
int FMInit(void);
void FMUnInit(void);
void FMReset(void);
int FMSetBank(void* _bank);

uint32_t SendData(uint32_t device, uint8_t *data, uint32_t size)
{
    FMSendData(data);
    return 0;
}

static const char *hmisignature = "HMIMIDIP013195";
static const char *hmisignatureold = "HMIMIDIP";

static HMIHeader *song_header[MAX_SONGS];
static uint8_t *song_track[MAX_SONGS][MAX_TRACKS];
static uint32_t (*song_branchcallback[MAX_SONGS])(uint32_t, uint8_t, uint8_t);
static uint32_t (*song_loopcallback[MAX_SONGS])(uint32_t, uint8_t, uint8_t, uint8_t);
static uint32_t (*song_triggercallback[MAX_SONGS][127])(uint32_t, uint8_t, uint8_t);
static TrackDevice *song_trackmap[MAX_SONGS];
static uint32_t song_activetracks[MAX_SONGS];
static uint32_t song_totaltracks[MAX_SONGS];
static void (*song_callback[MAX_SONGS])(uint32_t);
static uint32_t song_trackdeltatime[MAX_SONGS][MAX_TRACKS];
static uint32_t song_trackdeltacurrent[MAX_SONGS][MAX_TRACKS];
static HMITrack *song_trackheader[MAX_SONGS][MAX_TRACKS];
static HMIBranch *song_branchdata[MAX_SONGS][MAX_TRACKS];
static uint32_t song_active[MAX_SONGS];
static uint8_t song_mastervolume = 0x7f;
static uint32_t song_volume[MAX_SONGS] = {
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
};
static uint8_t mididata[10], mididata1[10];
static uint8_t song_branchoccured;
static uint32_t song_paused[MAX_SONGS];
static uint32_t song_muted[MAX_SONGS];
static uint32_t channelmuted[5][16];
static uint32_t channelstealing;
static uint8_t song_channelremap[5][MAX_SONGS][16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t devicechannelvolume[5][16] = {
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
};

static uint8_t devicechannelavailable[5][16] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1
};

static uint8_t channelowner[5][16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t ownersong[5][16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t channelpriority[5][16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t channeldataindex[5][MAX_SONGS][16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static ChannelData channeldata[5][16][4] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint32_t song_fadedir[MAX_SONGS];
static uint32_t song_fadefrac[MAX_SONGS];
static uint32_t song_fadevol[MAX_SONGS];
static uint32_t song_fadeticks[MAX_SONGS];
static uint8_t song_handlevolumefade[MAX_SONGS];

static uint8_t midieventsize[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 2, 2, 3, 0
};

static uint8_t midieventsizecontrol[] = {
    0, 1, 2, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

static uint32_t song_timerrate[MAX_SONGS] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

static uint32_t song_timeraccum[MAX_SONGS];

static uint32_t song_oldhmp[MAX_SONGS];

static uint32_t HMIGetDeltaTime(uint8_t *track, uint32_t *delta);
static uint32_t HMIResetChannelStealing(uint32_t handle);
uint32_t HMIResetSong(uint32_t handle, InitSong *song);
uint32_t HMISetSongVolume(uint32_t handle, uint8_t volume);
uint32_t HMIHandleMIDIData(uint32_t handle, uint8_t *data, uint32_t device, uint32_t size);
void HMISetTrackState(uint32_t handle, uint32_t track, uint32_t branch);
uint32_t HMISongDone(uint32_t handle);

uint32_t HMIInitSong(InitSong *song, TrackDevice *trackmap, uint32_t *handleptr)
{
    MV_Lock();
    uint32_t trackoffset = 0;
    uint32_t s, i, j, handle, size, status, dev;
    uint8_t *branchptr, *trackptr, *branchcount;
    uint32_t ver = 0;
    for (s = 0; hmisignature[s]; s++)
    {
        if (hmisignature[s] != song->songptr[s])
        {
            ver = 1;
            break;
        }
    }
    if (ver)
    {
        for (s = 0; hmisignatureold[s]; s++)
        {
            if (hmisignatureold[s] != song->songptr[s])
            {
                MV_Unlock();
                return 14;
            }
        }
    }
    for (j = 0; j < MAX_SONGS; j++)
    {
        if (!song_header[j])
        {
            handle = j;
            break;
        }
    }
    if (j == MAX_SONGS)
    {
        MV_Unlock();
        return 11;
    }
    for (i = 0; i < MAX_TRACKS; i++)
    {
        song_track[handle][i] = NULL;
    }
    song_header[handle] = (HMIHeader *)song->songptr;
    song_oldhmp[handle] = ver;
    for (i = 0; i < 127; i++)
    {
        song_triggercallback[handle][i] = NULL;
    }
    song_branchcallback[handle] = NULL;
    song_loopcallback[handle] = NULL;
    branchptr = song->songptr + song_header[handle]->branchoffset;
    trackptr = song->songptr + sizeof(HMIHeader);
    if (song_oldhmp[handle])
        trackptr = song->songptr + 0x308;
    song_trackmap[handle] = trackmap;
    song_activetracks[handle] = song_header[handle]->numtracks;
    song_totaltracks[handle] = song_activetracks[handle];
    song_callback[handle] = song->callback;
    for (i = 0; i < song_activetracks[handle]; i++)
    {
        song_trackdeltatime[handle][i] = 0;
        song_trackheader[handle][i] = (HMITrack*)(trackptr + trackoffset);
        song_track[handle][i] = trackptr + trackoffset + sizeof(HMITrack);
        size = HMIGetDeltaTime(song_track[handle][i], &song_trackdeltacurrent[handle][i]);
        song_track[handle][i] += size;
        trackoffset += song_trackheader[handle][i]->size;
    }
    branchcount = branchptr;
    branchptr += song_activetracks[handle];
    for (i = 0; i < song_activetracks[handle]; i++)
    {
        if (*branchcount)
        {
            song_branchdata[handle][i] = (HMIBranch*)branchptr;
        }
        branchptr += *branchcount * sizeof(HMIBranch);
        branchcount++;
    }
    for (i = 0; i < song_totaltracks[handle]; i++)
    {
        if (song_trackmap[handle]->trackdevice[i] == 0xff)
        {
            status = 0;
            dev = 0;
            while (dev < 5 && song_header[handle]->trackdevice[i][dev] && !status)
            {
                for (j = 0; j < 5; j++)
                {
                    if (song_header[handle]->trackdevice[i][dev] == 0xa002) // OPL2
                    {
                        song_trackmap[handle]->trackdevice[i] = j;
                        status = 1;
                        break;
                    }
                }
                dev++;
            }
            if (!song_header[handle]->trackdevice[i][0])
            {
                song_trackmap[handle]->trackdevice[i] = 0;
            }
            else if (!status)
            {
                song_track[handle][i] = NULL;
                song_trackmap[handle]->trackdevice[i] = 0xff;
                song_activetracks[handle]--;
            }
        }
    }
    if (!song_oldhmp[j])
    for (j = 0; j < 128; j++)
    {
        song_header[handle]->ccrestore[j] = 1;
    }
    *handleptr = handle;
    MV_Unlock();
    return 0;
}

uint32_t HMIUnInitSong(uint32_t handle)
{
    if (handle < MAX_SONGS)
    {
        MV_Lock();
        song_header[handle] = NULL;
        MV_Unlock();
    }
    else
        return 10;
    return 0;
}

uint32_t HMIStartSong(uint32_t handle)
{
    MV_Lock();
    song_timerrate[handle] = song_header[handle]->tempo;
    song_timeraccum[handle] = 0;
    song_active[handle] = 1;
    MV_Unlock();
    return 0;
}

uint32_t HMIStopSong(uint32_t handle)
{
    InitSong init;
    if (handle < MAX_SONGS)
    {
        MV_Lock();
        song_timerrate[handle] = 0xffffffff;
        if (song_active[handle])
        {
            init.songptr = (uint8_t*)song_header[handle];
            init.callback = song_callback[handle];
            HMIResetChannelStealing(handle);
            song_active[handle] = 0;
            song_header[handle] = NULL;
            HMIResetSong(handle, &init);
        }
        MV_Unlock();
    }
    else
        return 10;
    return 0;
}

uint32_t HMIResetSong(uint32_t handle, InitSong *init)
{
    uint32_t trackoffset = 0;
    uint32_t i, size;
    uint8_t *trackptr;
    MV_Lock();
    song_header[handle] = (HMIHeader*)init->songptr;
    trackptr = (uint8_t*)song_header[handle] + sizeof(HMIHeader);
    if (song_oldhmp[handle])
        trackptr = (uint8_t*)song_header[handle] + 0x308;
    song_activetracks[handle] = song_header[handle]->numtracks;
    song_totaltracks[handle] = song_activetracks[handle];
    song_callback[handle] = init->callback;
    for (i = 0; i < song_activetracks[handle]; i++)
    {
        song_trackdeltatime[handle][i] = 0;
        song_trackheader[handle][i] = (HMITrack*)(trackptr + trackoffset);
        song_track[handle][i] = trackptr + trackoffset + sizeof(HMITrack);
        size = HMIGetDeltaTime(song_track[handle][i], &song_trackdeltacurrent[handle][i]);
        song_track[handle][i] += size;
        trackoffset += song_trackheader[handle][i]->size;
    }
    for (i = 0; i < song_totaltracks[handle]; i++)
    {
        if (song_trackmap[handle]->trackdevice[i] == 0xff)
        {
            song_track[handle][i] = NULL;
            song_activetracks[handle]--;
        }
    }
    MV_Unlock();
    return 0;
}

static uint32_t HMIGetDeltaTime(uint8_t *track, uint32_t *delta)
{
    uint8_t b;
    uint32_t flag = 0, shift = 0, value = 0, offset = 0;
    do
    {
        offset++;
        b = *track++;
        if (b & 0x80)
            flag = 1;
        b &= 0x7f;
        value |= b << shift;
        shift += 7;
    } while (!flag);
    *delta = value;
    return offset;
}

uint32_t HMISetMasterVolume(uint8_t volume)
{
    uint32_t i;
    MV_Lock();
    song_mastervolume = volume;
    for (i = 0; i < MAX_SONGS; i++)
    {
        if (song_active[i])
            HMISetSongVolume(i, song_volume[i]);
    }
    MV_Unlock();
    return 0;
}

uint32_t HMIRegisterLoopFunction(uint32_t handle, uint32_t (*callback)(uint32_t, uint8_t, uint8_t, uint8_t))
{
    song_loopcallback[handle] = callback;
    return 0;
}

uint32_t HMIRegisterBranchFunction(uint32_t handle, uint32_t (*callback)(uint32_t, uint8_t, uint8_t))
{
    song_branchcallback[handle] = callback;
    return 0;
}

uint32_t HMIRegisterTriggerFunction(uint32_t handle, uint32_t track, uint32_t (*callback)(uint32_t, uint8_t, uint8_t))
{
    song_triggercallback[handle][track] = callback;
    return 0;
}

uint32_t HMIBranchToTrackLocation(uint32_t handle, uint8_t track, uint8_t id)
{
    uint32_t branch;
    uint32_t trackoffset, i;
    uint8_t *ccdata;

    branch = 0;
    while (song_branchdata[handle][track][branch].id == id)
    {
        branch++;
    }
    trackoffset = song_branchdata[handle][track][branch].offset + sizeof(HMITrack);
    song_track[handle][track] = (uint8_t*)song_trackheader[handle][track] + trackoffset;
    mididata[0] = 0xb0 | song_trackheader[handle][track]->channel;
    ccdata = (uint8_t*)song_header[handle] + song_branchdata[handle][track][branch].ccoffset;
    for (i = 0; i < song_branchdata[handle][track][branch].cccount; i += 2)
    {
        mididata[1] = ccdata[i];
        mididata[2] = ccdata[i + 1];
        HMIHandleMIDIData(handle, mididata, song_trackmap[handle]->trackdevice[track], 3);
    }
    song_branchoccured = 0;
    return 0;
}

uint32_t HMIBranchToSongLocation(uint32_t handle, uint32_t id)
{
    uint8_t i;
    uint32_t branch, trackoffset, t;
    for (i = 1; i < song_totaltracks[handle]; i++)
    {
        if (song_track[handle][i])
        {
            branch = 0;
            while (song_branchdata[handle][i][branch].id == id)
            {
                branch++;
            }
            trackoffset = song_branchdata[handle][i][branch].offset + sizeof(HMITrack);
            song_track[handle][i] = (uint8_t*)song_trackheader[handle][i] + trackoffset;
            t = HMIGetDeltaTime(song_track[handle][i], &song_trackdeltacurrent[handle][i]);
            song_track[handle][i] += t;
            song_trackdeltatime[handle][i] = 0;
            HMISetTrackState(handle, i, branch);
        }
    }
    song_branchoccured = 0;
    return 0;
}

uint32_t HMISongAlterTempo(uint32_t handle, uint32_t value)
{
    uint32_t value2, rate;
    if (HMISongDone(handle))
        return 0;
    value2 = (value << 16) / 100;
    rate = (song_header[handle]->tempo * value2) >> 16;
    if (rate == 0)
        rate = 1;
    song_timerrate[handle] = rate;
    song_timeraccum[handle] = 0;
    return song_timerrate[handle];
}

uint32_t HMIGetTimeToPlay(uint32_t handle)
{
    return song_header[handle]->timetoplay;
}

uint32_t HMIPauseSong(uint32_t handle, uint32_t mute)
{
    uint8_t channel, channelremap;
    uint32_t i, dev;
    song_paused[handle] = 1;
    if (mute)
    {
        song_muted[handle] = 1;
        for (i = 0; i < MAX_TRACKS; i++)
        {
            if (song_track[handle][i])
            {
                channel = song_trackheader[handle][i]->channel;
                dev = song_trackmap[handle]->trackdevice[i];
                if (channelstealing)
                    channelremap = song_channelremap[dev][handle][channel];
                else
                    channelremap = channel;
                mididata1[0] = 0xb0 | channel;
                mididata1[1] = 7;
                mididata1[2] = devicechannelvolume[dev][channelremap];
                HMIHandleMIDIData(handle, mididata1, dev, 3);
                channelmuted[dev][channelremap] = 1;
            }
        }
    }
    return 0;
}

uint32_t HMIResumeSong(uint32_t handle)
{
    uint8_t channel, channelremap;
    uint32_t i, dev;
    song_paused[handle] = 0;
    if (song_muted[handle])
    {
        song_muted[handle] = 0;
        for (i = 0; i < MAX_TRACKS; i++)
        {
            if (song_track[handle][i])
            {
                dev = song_trackmap[handle]->trackdevice[i];
                channel = song_trackheader[handle][i]->channel;
                if (channelstealing)
                    channelremap = song_channelremap[dev][handle][channel];
                else
                    channelremap = channel;
                mididata1[0] = 0xb0 | channel;
                mididata1[1] = 7;
                mididata1[2] = devicechannelvolume[dev][channelremap];
                HMIHandleMIDIData(handle, mididata1, dev, 3);
                channelmuted[dev][channelremap] = 0;
            }
        }
    }
    return 0;
}

uint32_t HMIMuteSong(uint32_t handle)
{
    uint8_t channel;
    uint32_t i;
    for (i = 0; i < MAX_TRACKS; i++)
    {
        if (song_track[handle][i])
        {
            channel = song_trackheader[handle][i]->channel;
            mididata1[0] = 0xb0 | channel;
            mididata1[1] = 7;
            mididata1[2] = 0;
            HMIHandleMIDIData(handle, mididata1, song_trackmap[handle]->trackdevice[i], 3);
        }
    }
    song_muted[handle] = 1;
    return 0;
}

uint32_t HMIUnMuteSong(uint32_t handle)
{
    uint8_t channel, channelremap;
    uint32_t i, dev;
    song_muted[handle] = 0;
    for (i = 0; i < 32; i++)
    {
        if (song_track[handle][i])
        {
            dev = song_trackmap[handle]->trackdevice[i];
            channel = song_trackheader[handle][i]->channel;
            if (channelstealing)
                channelremap = song_channelremap[dev][handle][channel];
            else
                channelremap = channel;
            mididata1[0] = 0xb0 | channel;
            mididata1[1] = 7;
            mididata1[2] = devicechannelvolume[dev][channelremap];
            HMIHandleMIDIData(handle, mididata1, dev, 3);
        }
    }
    return 0;
}

uint32_t HMISetSongVolume(uint32_t handle, uint8_t volume)
{
    uint8_t channel, channelremap;
    uint32_t i, dev;
    song_volume[handle] = volume;
    for (i = 0; i < 32; i++)
    {
        if (song_track[handle][i])
        {
            channel = song_trackheader[handle][i]->channel;
            dev = song_trackmap[handle]->trackdevice[i];
            if (channelstealing)
                channelremap = song_channelremap[dev][handle][channel];
            else
                channelremap = channel;
            mididata1[0] = 0xb0 | channel;
            mididata1[1] = 7;
            mididata1[2] = devicechannelvolume[dev][channelremap];
            HMIHandleMIDIData(handle, mididata1, song_trackmap[handle]->trackdevice[i], 3);
        }
    }
    return 0;
}

uint32_t HMIFadeSong(uint32_t handle, uint32_t flags, uint32_t speed, uint8_t vol1, uint8_t vol2, uint32_t speed2)
{
    uint32_t diff, rate, div, ticks, frac;
    MV_Lock();
    if (flags & SONG_FADE_IN)
    {
        diff = vol2 - vol1;
    }
    else
    {
        diff = vol1 - vol2;
    }
    rate = song_timerrate[handle];
    div = (100 << 16) / rate;
    ticks = (speed << 16) / div;
    ticks = ticks / speed2;
    if (!ticks)
    {
        if (flags & SONG_FADE_OUT_STOP)
        {
            HMIStopSong(handle);
            return 0;
        }
        HMISetSongVolume(handle, vol2);
        return 0;
    }
    HMISetSongVolume(handle, vol1);
    frac = (diff << 16) / ticks;
    song_fadedir[handle] = flags;
    song_fadefrac[handle] = frac;
    song_fadevol[handle] = vol1 << 16;
    song_fadeticks[handle] = ticks;
    MV_Unlock();
    return 0;
}

uint32_t HMISongDone(uint32_t handle)
{
    uint32_t status;
    MV_Lock();
    if (!song_active[handle])
        status = 1;
    else
        status = 0;
    MV_Unlock();
    return status;
}

uint32_t HMIHandleMIDIData(uint32_t handle, uint8_t *data, uint32_t device, uint32_t size)
{
    uint8_t priority = 0, replace = 0xff;
    uint8_t channel, channelremap, cmd;
    uint32_t i, vol, j, cmdsize, newvol = 0xffffffff;
    cmd = data[0];
    channel = cmd & 0x0f;
    if (!channelstealing)
    {
        switch (data[0] & 0xf0)
        {
        case 0xb0:
            switch (data[1])
            {
            case 7:
                mididata[0] = data[0];
                mididata[1] = 7;
                vol = (data[2] * song_volume[handle]) >> 7;
                mididata[2] = (vol * song_mastervolume) >> 7;
                devicechannelvolume[device][channel] = data[2];
                if (song_muted[handle])
                    mididata[2] = 0;
                break;
            default:
                mididata[0] = data[0];
                mididata[1] = data[1];
                mididata[2] = data[2];
                mididata[3] = data[3];
                break;
            }
            SendData(device, mididata, size);
            break;
        default:
            SendData(device, data, size);
            break;
        }
        return 1;
    }
    channelremap = song_channelremap[device][handle][channel];
RESTART:
    if (channelremap != 0xff)
    {
        data[0] = (cmd & 0xf0) | channelremap;
    }
    else
    {
        if (channel == 9)
        {
            song_channelremap[device][handle][channel] = 9;
            channelremap = 9;
            goto RESTART;
        }
        for (i = 0; i < 16; i++)
        {
            while (i < 16 && !devicechannelavailable[device][i])
            {
                i++;
            }
            if (i < 16 && channelowner[device][i] == 0xff)
            {
                song_channelremap[device][handle][channel] = i;
                channelremap = i;
                channelowner[device][i] = channel;
                ownersong[device][i] = handle;
                channelpriority[device][i] = song_header[handle]->priority[channel];
                j = channeldataindex[device][handle][channel];
                if (j != 0xff)
                {
                    devicechannelvolume[device][channelremap] = 0x7f;
                    mididata[0] = 0xb0 | channelremap;
                    mididata[1] = 0x79;
                    mididata[2] = 0;
                    cmdsize = 3;
                    SendData(device, mididata, cmdsize);
                    if (channeldata[device][channel][j].patch != 0xff)
                    {
                        mididata[0] = 0xc0 | channelremap;
                        mididata[1] = channeldata[device][channel][j].patch;
                        cmdsize = 2;
                        SendData(device, mididata, cmdsize);
                    }
                    if (channeldata[device][channel][j].bend != 0xff)
                    {
                        mididata[0] = 0xe0 | channelremap;
                        mididata[1] = 0;
                        mididata[2] = channeldata[device][channel][j].bend;
                        cmdsize = 2;
                        SendData(device, mididata, cmdsize);
                    }
                    if (channeldata[device][channel][j].volume != 0xff)
                    {
                        mididata[0] = 0xb0 | channelremap;
                        mididata[1] = 7;
                        mididata[2] = channeldata[device][channel][j].volume;
                        cmdsize = 3;
                        SendData(device, mididata, cmdsize);
                    }
                    if (channeldata[device][channel][j].sustain != 0xff)
                    {
                        mididata[0] = 0xb0 | channelremap;
                        mididata[1] = 64;
                        mididata[2] = channeldata[device][channel][j].sustain;
                        cmdsize = 3;
                        SendData(device, mididata, cmdsize);
                    }
                }
                else
                {
                    for (j = 0; j < 4; j++)
                    {
                        if (channeldata[device][channel][j].used == 0xff)
                        {
                            channeldata[device][channel][j].used = 1;
                            channeldataindex[device][handle][channel] = j;
                            break;
                        }
                    }
                }
                goto RESTART;
            }
        }
        for (i = 0; i < 16; i++)
        {
            while (i < 16 && !devicechannelavailable[device][i])
            {
                i++;
            }
            if (i < 16)
            {
                if (channelpriority[device][i] > priority && channelpriority[device][i] != 0xff)
                {
                    priority = channelpriority[device][i];
                    replace = i;
                }
            }
        }
        if (replace != 0xff)
        {
            if (priority > song_header[handle]->priority[channel])
            {
                song_channelremap[device][handle][channel] = replace;
                song_channelremap[device][ownersong[device][replace]][replace] = 0xff;
                channelowner[device][replace] = channel;
                ownersong[device][replace] = handle;
                channelremap = replace;
                channelpriority[device][replace] = song_header[handle]->priority[channel];
                mididata[0] = 0xb0 | channelremap;
                mididata[1] = 0x7b;
                mididata[2] = 0;
                cmdsize = 3;
                SendData(device, mididata, cmdsize);
                mididata[0] = 0xb0 | channelremap;
                mididata[1] = 0x79;
                mididata[2] = 0;
                cmdsize = 3;
                SendData(device, mididata, cmdsize);
                if (channeldataindex[device][handle][channel] == 0xff)
                {
                    for (j = 0; j < 4; j++)
                    {
                        if (channeldata[device][channel][j].used == 0xff)
                        {
                            channeldata[device][channel][j].used = 1;
                            channeldataindex[device][handle][channel] = j;
                            break;
                        }
                    }
                }
                goto RESTART;
            }
            if (channeldataindex[device][handle][channel] == 0xff)
            {
                for (j = 0; j < 4; j++)
                {
                    if (channeldata[device][channel][j].used == 0xff)
                    {
                        channeldata[device][channel][j].used = 1;
                        channeldataindex[device][handle][channel] = j;
                        break;
                    }
                }
            }
        }
    }
    if (channel != 9)
    {
        switch (cmd & 0xf0)
        {
        case 0xb0:
            switch (data[1])
            {
            case 7:
                channeldata[device][channel][channeldataindex[device][handle][channel]].volume = data[2];
                newvol = data[2];
                devicechannelvolume[device][channelremap] = newvol;
                break;
            case 64:
                channeldata[device][channel][channeldataindex[device][handle][channel]].sustain = data[2];
                break;
            }
            break;
        case 0xc0:
            channeldata[device][channel][channeldataindex[device][handle][channel]].patch = data[1];
            break;
        case 0xe0:
            channeldata[device][channel][channeldataindex[device][handle][channel]].bend = data[2];
            break;
        }
    }
    else
    {
        if (cmd == 0xb9 && data[1] == 7)
        {
            newvol = data[2];
            devicechannelvolume[device][channelremap] = newvol;
        }
    }

    if (channelremap != 0xff)
    {
        if (newvol != 0xffffffff)
        {
            if (song_muted[handle])
                data[2] = 0;
            else
            {
                vol = (song_volume[handle] * newvol) >> 7;
                vol = (song_mastervolume * vol) >> 7;
                data[2] = vol;
            }
        }
        SendData(device, data, size);
        data[0] = (cmd & 0xf0) | channel;
        if (newvol != 0xffffffff)
            data[2] = newvol;
        return 0;
    }
    return -1;
}

uint32_t HMIResetChannelStealing(uint32_t handle)
{
    uint8_t channel, channelremap, chandataid;
    uint32_t i, device, size;
    for (i = 1; i < song_totaltracks[handle]; i++)
    {
        device = song_trackmap[handle]->trackdevice[i];
        if (device != 0xffffffff && device != 0xff)
        {
            channel = song_trackheader[handle][i]->channel;
            if (!channelstealing)
            {
                mididata[0] = 0xb0 | channel;
                mididata[1] = 0x7b;
                mididata[2] = 0;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xb0 | channel;
                mididata[1] = 0x79;
                mididata[2] = 0;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xe0 | channel;
                mididata[1] = 0x40;
                mididata[2] = 0x40;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xb0 | channel;
                mididata[1] = 0x7;
                mididata[2] = 0x0;
                size = 3;
                SendData(device, mididata, size);
            }
            else
            {
                channelremap = song_channelremap[device][handle][channel];
                song_channelremap[device][handle][channel] = 0xff;
                chandataid = channeldataindex[device][handle][channel];
                channelowner[device][channelremap] = 0xff;
                ownersong[device][channelremap] = 0xff;
                mididata[0] = 0xb0 | channelremap;
                mididata[2] = 0;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xb0 | channelremap;
                mididata[1] = 0x79;
                mididata[2] = 0;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xe0 | channelremap;
                mididata[1] = 0x40;
                mididata[2] = 0x40;
                size = 3;
                SendData(device, mididata, size);
                mididata[0] = 0xb0 | channelremap;
                mididata[1] = 0x7;
                mididata[2] = 0x0;
                size = 3;
                SendData(device, mididata, size);
                if (chandataid != 0xff)
                {
                    channeldata[device][channel][chandataid].patch = 0xff;
                    channeldata[device][channel][chandataid].bend = 0xff;
                    channeldata[device][channel][chandataid].volume = 0xff;
                    channeldata[device][channel][chandataid].sustain = 0xff;
                    channeldata[device][channel][chandataid].used = 0xff;
                    channeldataindex[device][handle][channel] = 0xff;
                }
            }
        }
    }
    return 1;
}

uint32_t HMISendMIDIData(uint32_t device, uint8_t *data, uint32_t size)
{
    return SendData(device, data, size);
}

uint32_t HMIEnableChannelStealing(uint32_t enable)
{
    uint32_t val;
    val = channelstealing;
    channelstealing = enable;
    return val;
}

void HMISongHandler(uint32_t handle)
{
    void (*callback)(uint32_t);
    InitSong init;
    uint32_t v18 = 1, track, size, id, branch, loop, j, jbranch, trackoffset;
    if (!song_active[handle] || song_paused[handle])
        return;

    if (song_fadeticks[handle])
    {
        if (--song_handlevolumefade[handle] == 0)
        {
            song_handlevolumefade[handle] = 3;
            song_fadeticks[handle]--;
            switch (song_fadedir[handle])
            {
            case SONG_FADE_OUT:
            case SONG_FADE_OUT_STOP:
                song_fadevol[handle] -= song_fadefrac[handle];
                HMISetSongVolume(handle, song_fadevol[handle] >> 16);
                if ((song_fadedir[handle] & SONG_FADE_OUT_STOP) && !song_fadeticks[handle])
                {
                    song_active[handle] = 0;
                    HMIResetChannelStealing(handle);
                    song_timerrate[handle] = 0xffffffff;
                    callback = song_callback[handle];
                    init.songptr = (uint8_t*)song_header[handle];
                    init.callback = callback;
                    song_header[handle] = NULL;
                    HMIResetSong(handle, &init);
                    if (callback)
                        callback(handle);
                    return;
                }
                break;
            case SONG_FADE_IN:
                song_fadevol[handle] += song_fadefrac[handle];
                HMISetSongVolume(handle, song_fadevol[handle] >> 16);
                break;
            }
        }
    }
    for (track = 0; track < song_totaltracks[handle]; track++)
    {
        song_trackdeltatime[handle][track]++;
        if (song_track[handle][track])
        {
            if (song_trackdeltacurrent[handle][track] <= song_trackdeltatime[handle][track])
            {
                do
                {
                    song_branchoccured = 0;
                    v18 = 1;
                    if (song_track[handle][track][0] < 0xf0)
                    {
                        size = midieventsize[song_track[handle][track][0] >> 4];
                    }
                    else
                    {
                        size = midieventsizecontrol[song_track[handle][track][0] & 0x0f];
                    }
                    if (song_track[handle][track][0] == 0xff)
                    {
                        switch (song_track[handle][track][1])
                        {
                        case 0x2f:
                            song_track[handle][track] = NULL;
                            if (song_activetracks[handle] - 1 == 1 && song_track[handle][0] != NULL)
                            {
                                song_activetracks[handle]--;
                                song_track[handle][0] = NULL;
                            }
                            song_activetracks[handle]--;
                            if (song_activetracks[handle] == 0)
                            {
                                song_active[handle] = 0;
                                HMIResetChannelStealing(handle);
                                song_timerrate[handle] = 0xffffffff;
                                callback = song_callback[handle];
                                init.songptr = (uint8_t*)song_header[handle];
                                init.callback = callback;
                                song_header[handle] = NULL;
                                HMIResetSong(handle, &init);
                                if (callback)
                                    callback(handle);
                                return;
                            }
                            size = 3;
                            break;
                        case 0x5f:
                            size = 5;
                            break;
                        }
                    }
                    else
                    {
                        if ((song_track[handle][track][0] & 0xf0) == 0xb0)
                        {
                            switch (song_track[handle][track][1])
                            {
                            case 103:
                                if (!song_oldhmp[handle])
                                song_header[handle]->ccrestore[song_track[handle][track][2]] = 0;
                                break;
                            case 104:
                                if (!song_oldhmp[handle])
                                song_header[handle]->ccrestore[song_track[handle][track][2]] = 1;
                                break;
                            case 108:
                                break;
                            case 110:
                                break;
                            case 109:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                song_branchdata[handle][track][branch].loopcount = song_track[handle][track][6];
                                break;
                            case 111:
                            case 112:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                loop = song_branchdata[handle][track][branch].loopcount;
                                if (loop != 0xff && loop != 0)
                                {
                                    song_branchdata[handle][track][branch].loopcount--;
                                    loop--;
                                }
                                if (song_loopcallback[handle])
                                {
                                    song_branchoccured = 1;
                                    if (!song_loopcallback[handle](handle, track, id, loop))
                                        loop = 0;
                                    if (!song_branchoccured)
                                    {
                                        v18 = 0;
                                        size = 0;
                                    }
                                    else
                                        song_branchoccured = 0;
                                }
                                if (loop)
                                {
                                    for (j = 1; j < song_totaltracks[handle]; j++)
                                    {
                                        if (song_track[handle][j])
                                        {
                                            jbranch = 0;
                                            while (song_branchdata[handle][j][jbranch].id != id)
                                            {
                                                jbranch++;
                                            }
                                            trackoffset = song_branchdata[handle][j][jbranch].offset + sizeof(HMITrack);
                                            song_track[handle][j] = (uint8_t*)song_trackheader[handle][j] + sizeof(HMITrack);
                                            size = HMIGetDeltaTime(song_track[handle][j], &song_trackdeltacurrent[handle][j]);
                                            song_track[handle][j] += size;
                                            song_trackdeltatime[handle][j] = 0;
                                            v18 = 0;
                                            HMISetTrackState(handle, j, jbranch);
                                        }
                                    }
                                    size = 0;
                                }
                                break;
                            case 113:
                                break;
                            case 114:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                loop = 1;
                                if (song_branchcallback[handle])
                                {
                                    song_branchoccured = 1;
                                    if (!song_branchcallback[handle](handle, track, id))
                                        loop = 1;
                                    if (!song_branchoccured)
                                    {
                                        v18 = 0;
                                        size = 0;
                                    }
                                    else
                                        song_branchoccured = 0;
                                }
                                if (loop)
                                {
                                    for (j = 1; j < song_totaltracks[handle]; j++)
                                    {
                                        if (song_track[handle][j])
                                        {
                                            jbranch = 0;
                                            while (song_branchdata[handle][j][jbranch].id != id)
                                            {
                                                jbranch++;
                                            }
                                            trackoffset = song_branchdata[handle][j][jbranch].offset + sizeof(HMITrack);
                                            song_track[handle][j] = (uint8_t*)song_trackheader[handle][j] + sizeof(HMITrack);
                                            size = HMIGetDeltaTime(song_track[handle][j], &song_trackdeltacurrent[handle][j]);
                                            song_track[handle][j] += size;
                                            song_trackdeltatime[handle][j] = 0;
                                            v18 = 0;
                                            HMISetTrackState(handle, j, jbranch);
                                        }
                                    }
                                    size = 0;
                                }
                                break;
                            case 116:
                                break;
                            case 115:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                song_branchdata[handle][track][branch].loopcount = song_track[handle][track][6];
                                break;
                            case 117:
                            case 118:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                loop = song_branchdata[handle][track][branch].loopcount;
                                if (loop != 0xff && loop != 0)
                                {
                                    song_branchdata[handle][track][branch].loopcount--;
                                    loop--;
                                }
                                if (song_loopcallback[handle])
                                {
                                    song_branchoccured = 1;
                                    if (!song_loopcallback[handle](handle, track, id, loop))
                                        loop = 0;
                                    if (!song_branchoccured)
                                        size = 0;
                                    else
                                        song_branchoccured = 0;
                                }
                                if (loop)
                                {
                                    trackoffset = song_branchdata[handle][track][branch].offset + sizeof(HMITrack);
                                    song_track[handle][track] = (uint8_t*)song_trackheader[handle][track] + sizeof(HMITrack);
                                    HMISetTrackState(handle, track, branch);
                                    size = 0;
                                }
                                break;
                            case 119:
                                loop = song_track[handle][track][2];
                                if (song_triggercallback[handle][loop])
                                {
                                    song_branchoccured = 1;
                                    song_triggercallback[handle][loop];
                                    if (!song_branchoccured)
                                    {
                                        v18 = 0;
                                        size = 0;
                                    }
                                    else
                                        song_branchoccured = 0;
                                }
                                break;
                            case 120:
                                break;
                            case 121:
                                id = song_track[handle][track][2];
                                branch = 0;
                                while (song_branchdata[handle][track][branch].id != id)
                                {
                                    branch++;
                                }
                                loop = 1;
                                if (song_branchcallback[handle])
                                {
                                    song_branchoccured = 1;
                                    if (!song_branchcallback[handle](handle, track, id))
                                        loop = 0;
                                    if (!song_branchoccured)
                                        size = 0;
                                    else
                                        song_branchoccured = 0;
                                }
                                if (loop)
                                {
                                    trackoffset = song_branchdata[handle][track][branch].offset + sizeof(HMITrack);
                                    song_track[handle][track] = (uint8_t*)song_trackheader[handle][track] + sizeof(HMITrack);
                                    HMISetTrackState(handle, track, branch);
                                    size = 0;
                                }
                                break;
                            default:
                                goto default_case;
                                break;
                            }
                        }
                        else
                        {
                    default_case:
                            if (track)
                            {
                                HMIHandleMIDIData(handle, song_track[handle][track], song_trackmap[handle]->trackdevice[track], size);
                            }
                        }
                    }
                    if (!song_branchoccured)
                        song_trackdeltatime[handle][track] = 0;
                    if (!song_track[handle][track])
                        break;
                    song_track[handle][track] += size;
                    if (v18)
                    {
                        size = HMIGetDeltaTime(song_track[handle][track], &song_trackdeltacurrent[handle][track]);
                        song_track[handle][track] += size;
                    }
                } while (!song_trackdeltacurrent[handle][track]);
            }
        }
    }
}

void HMISetTrackState(uint32_t handle, uint32_t track, uint32_t branch)
{
    uint8_t mevent[3];
    uint8_t *ccdata;
    uint32_t i;
    // TODO:
    if (song_oldhmp[handle])
        return;
    if (song_header[handle]->ccrestore[108])
    {
        mevent[0] = 0xc0 | (song_track[handle][track][0] & 0xf);
        mevent[1] = song_branchdata[handle][track][branch].patch;
        HMIHandleMIDIData(handle, mevent, song_trackmap[handle]->trackdevice[track], 2);
    }
    mevent[0] = 0xb0 | (song_track[handle][track][0] & 0xf);
    ccdata = (uint8_t*)song_header[handle] + song_branchdata[handle][track][branch].ccoffset;
    for (i = 0; i < song_branchdata[handle][track][branch].cccount; i += 2)
    {
        mevent[1] = ccdata[i];
        mevent[2] = ccdata[i + 1];
        if (song_header[handle]->ccrestore[mevent[1]])
        {
            HMIHandleMIDIData(handle, mevent, song_trackmap[handle]->trackdevice[track], 3);
        }
    }
}


static int mixrate = 49716;
static MV_MusicRoutineBuffer musicbuffer;
static int hmiinit;
extern int MV_Channels;

static void HMIFill(void)
{
    int16_t *stream16 = (int16_t*)musicbuffer.buffer;
    int len = musicbuffer.size / (2 * MV_Channels);
    for (int i = 0; i < len; i++)
    {
        int16_t sampl[2] = {};
        for (int j = 0; j < MAX_SONGS; j++)
        {
            if (song_timerrate[j] == 0xffffffff)
                continue;
            song_timeraccum[j] += song_timerrate[j];
            while (song_timeraccum[j] >= mixrate)
            {
                song_timeraccum[j] -= mixrate;
                HMISongHandler(j);
            }
        }
        OPL3_GenerateResampled(&fm_chip, sampl);
        if (MV_Channels == 2)
        {
            *stream16++ = sampl[0];
            *stream16++ = sampl[1];
        }
        else
        {
            *stream16++ = (sampl[0] + sampl[1]) / 2;
        }
    }
}

void HMIInit(int rate)
{
    if (hmiinit)
        return;
    mixrate = rate;
    musicbuffer = MV_GetMusicRoutineBuffer();
    MV_HookMusicRoutine(HMIFill);
    OPL3_Reset(&fm_chip, rate);
    FMInit();
    FMReset();
    hmiinit = 1;
}

void HMIUnInit(void)
{
    if (!hmiinit)
        return;
    MV_UnhookMusicRoutine();
    for (int i = 0; i < MAX_SONGS; i++)
    {
        if (song_header[i])
        {
            HMIStopSong(i);
            HMIUnInitSong(i);
        }
    }
    hmiinit = 0;
}
