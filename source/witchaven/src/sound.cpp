
#include "witchaven.h"
#include "sound.h"
#include "compat.h"
#include "baselayer.h"
#include "renderlayer.h" // for win_gethwnd()
#include "build.h"
#include "cache1d.h"
#include "music.h"
#include "fx_man.h"
#include "hmpplay.h"
#include "keyboard.h"
#include "control.h"
#include "config.h"
#include "player.h"

void SND_MIDIFlush(void);
char* SND_LoadMIDISong(int nSong);
int SND_PrepareMIDISong(int SongIndex);
int SND_StartMIDISong(int wSongHandle);

int lavasnd = -1;
int batsnd = -1;
int cartsnd = -1;

int SoundMode, wDIGIVol;
int MusicMode, wMIDIVol;
int use_rec_driver, voicecom_enabled;
int SD_Started;
int wMIDIDeviceID;
int SongPending;

#define  _MIDI_MPU_401              0xa001
#define  _MIDI_FM                   0xa002
#define  _MIDI_OPL2                 0xa002
#define  _MIDI_OPL3                 0xa009
char *BaseSongPtr;
char *EmbSongPtr;
char *SpiceSongPtr;

char *m_bnkptr, *d_bnkptr;

#define MAX_ACTIVE_SONGS 4

int hSOSSongHandles[MAX_ACTIVE_SONGS];
InitSong sSOSInitSongs[MAX_ACTIVE_SONGS];
TrackDevice sSOSTrackMap[MAX_ACTIVE_SONGS] = {
    {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff
    },
    {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff
    },
    {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff
    },
    {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff
    },
};

static uint16_t songelements = 3;
static uint16_t arrangements = 3;
static uint16_t songsperlevel;
static uint16_t totallevels = 6;    // really there use two to test menusong

uint32_t DigiList[4096];
uint32_t LoopList[4096];
uint32_t SongList[4096];

uint32_t PanArray[] = {
    // REAR to HARD LEFT (angle = 0->512)
    0x8000,0x7000,0x6000,0x5000,0x4000,0x3000,0x2000,0x1000,0x0000,
    // HARD LEFT to CENTER (angle = 513-1024)
    0x1000,0x20F0,0x2000,0x3000,0x4000,0x5000,0x6000,0x7000,0x8000,
    // CENTER to HARD RIGHT (angle = 1025-1536)
    0x70F0,0x8000,0x9000,0xA000,0xB000,0xC000,0xD000,0xE000,0xF000,
    // HARD RIGHT to REAR (angle = 1537-2047)
    0xFFFF,0xF000,0xE000,0xD000,0xC000,0xB000,0xA000,0x9000,0x8000
};

struct SampleType
{
//  int     playing;
    int32_t nFXHandle;
    int32_t number;
    int32_t priority;
    int32_t loopcount;
    int32_t x, y;

    int nSize;
    char* pData;
};

SampleType SampleRay[MAX_ACTIVE_SAMPLES];


ambsounds ambsoundarray[] =
{
    0,0,
    S_WINDLOOP1,-1,
    S_WINDLOOP2,-1,
    S_WAVELOOP1,-1,
    S_LAVALOOP1,-1,
    S_WATERY,-1,
    S_STONELOOP1,-1,
    S_BATSLOOP,-1
};

buildvfs_kfd hSoundFile = buildvfs_kfd_invalid;
buildvfs_kfd hLoopsFile = buildvfs_kfd_invalid;
buildvfs_kfd hSongsFile = buildvfs_kfd_invalid;


void SND_SetupTables()
{
    if (SoundMode)
    {
        hSoundFile = kopen4loadfrommod("joesnd", 0);
        if (hSoundFile == buildvfs_kfd_invalid) {
            crash("Couldn't open joesnd");
        }

        int nSize = kfilelength(hSoundFile);
        if (nSize < 4096) {
            crash("Invalid size for joesdn");
        }

        klseek(hSoundFile, nSize - 4096, SEEK_SET);

        if (kread(hSoundFile, DigiList, 4096) != 4096) {
            crash("Couldn't read from joesnd");
        }
    }

    if (MusicMode == _LOOP_MUSIC)
    {
        hLoopsFile = kopen4loadfrommod("loops", 0);
        if (hLoopsFile == buildvfs_kfd_invalid) {
            crash("Couldn't open loops");
        }

        int nSize = kfilelength(hLoopsFile);
        if (nSize < 4096) {
            crash("Invalid size for loops");
        }

        klseek(hLoopsFile, nSize - 4096, SEEK_SET);

        if (kread(hLoopsFile, LoopList, 4096) != 4096) {
            crash("Couldn't read from loops");
        }
    }
    else if (MusicMode)
    {
        hSongsFile = kopen4loadfrommod("songs", 0);
        if (hSongsFile == buildvfs_kfd_invalid) {
            crash("Couldn't open songs");
        }

        int nSize = kfilelength(hSongsFile);
        if (nSize < 4096) {
            crash("Invalid size for songs");
        }

        klseek(hSongsFile, nSize - 4096, SEEK_SET);

        if (kread(hSongsFile, SongList, 4096) != 4096) {
            crash("Couldn't read from songs");
        }
    }

    return;
}

void SND_LoadMidiIns(void)
{
    static int wLength;

    //JSA_DEMO check port address to verify FM device
    buildvfs_kfd handle = kopen4loadfrommod("melodic.bnk", 0);
    if (handle == buildvfs_kfd_invalid)
        crash("MELODIC BANK FILE FAILED!");
    m_bnkptr = (char*)malloc(0x152c);
    kread(handle, m_bnkptr, 0x152c);
    kclose(handle);
    if (FMSetBank(m_bnkptr))
        crash("BAD SetInsData MEL!");

    handle = kopen4loadfrommod("drum.bnk", 0);
    if (handle == buildvfs_kfd_invalid)
        crash("PERCUSSIVE BANK FILE FAILED!");
    d_bnkptr = (char*)malloc(0x152c);
    kread(handle, d_bnkptr, 0x152c);
    kclose(handle);
    if (FMSetBank(d_bnkptr))
        printf("BAD SetInsData DRUM!");
}

void ASSCallback(intptr_t pSample)
{
    SampleType *sample = (SampleType*)pSample;
    sample->loopcount = 0;
    sample->nSize = 0;
    sample->nFXHandle = 0;
    sample->priority = 0;
    sample->number = -1;
}

extern int MV_MixRate;
extern int musiclevel;

void SND_Startup()
{
    #ifdef MIXERTYPEWIN
    void* initdata = (void*)win_gethwnd(); // used for DirectSound
    #else
    void* initdata = NULL;
    #endif

    //  if (bNoSound)
    //      return;

    if (FX_Init(NumVoices, NumChannels, MixRate, initdata) != FX_Ok)
    {
        //        crash("Error initializing sound card!\n");
        initprintf("Error initializing sound card!\n");
        //        DebugOut("ERROR: %s\n", FX_ErrorString(FX_Error));
        return;
    }

    FX_SetCallBack(ASSCallback);

    memset(SampleRay, 0, sizeof(SampleRay));
    for (int i = 0; i < MAX_ACTIVE_SAMPLES; i++)
    {
        SampleRay[i].number = -1;
 //       SampleRay[i].loopcount = -1;
    }

    // init buffer space
    for (int i = 0; i < MAX_ACTIVE_SAMPLES; i++) {
        SampleRay[i].pData = new char[55000]; // just alloc a big block, like the original code
    }

    // Init music
    #if 1
            // if they chose None lets return
    #if 0
    if (!MusicToggle)
    {
        gs.MusicOn = FALSE;
        return;
    }
    #endif

    wMIDIVol = (musiclevel<<3);
    wMIDIDeviceID = _MIDI_FM;
    HMIInit(MV_MixRate);

    SND_LoadMidiIns();
    for (int i = 0; i < MAX_ACTIVE_SONGS; i++)
        hSOSSongHandles[i] = 0x7fff;

    songsperlevel = songelements * arrangements;

    HMISetMasterVolume(wMIDIVol);
        /*
    auto const fil = kopen4load("swtimbr.tmb", 0);

    if (fil != buildvfs_kfd_invalid)
    {
        int l = kfilelength(fil);
        auto tmb = (uint8_t*)Xmalloc(l);
        kread(fil, tmb, l);
        AL_RegisterTimbreBank(tmb);
        Xfree(tmb);
        kclose(fil);
    }
    */
    #endif

    // read in offset page lists
    SND_SetupTables();

    SD_Started = 1;
}

void SND_Shutdown()
{
    FX_Shutdown();

    for (int i = 0; i < MAX_ACTIVE_SAMPLES; i++)
    {
        if (SampleRay[i].pData) {
            delete[] SampleRay[i].pData;
        }
    }


    SND_MIDIFlush();
    HMIUnInit();
    if (m_bnkptr != NULL)
        free(m_bnkptr);
    if (d_bnkptr != NULL)
        free(d_bnkptr);

    if (hSoundFile != buildvfs_kfd_invalid) {
        kclose(hSoundFile);
    }

    if (hLoopsFile != buildvfs_kfd_invalid) {
        kclose(hLoopsFile);
    }

    if (hSongsFile != buildvfs_kfd_invalid) {
        kclose(hSongsFile);
    }

    SD_Started = 0;
}

int SND_Sound(int nSound)
{
    if (!SoundMode)
        return -1;

    return(SND_PlaySound(nSound, 0, 0, 0, 0));
}

int SND_PlaySound(int nSound, int32_t x, int32_t y, int16_t Pan, int16_t loopcount)
{
    //return 0; // TODO

    short wVol, flag = 0;
    int32_t sqrdist;
    int32_t prioritize;
    int i;

    if (!SoundMode)
        return 0;

    prioritize = DigiList[(nSound * 3) + 2];

    if (((x == 0) && (y == 0)) || ((player[pyrn].x == x) && (player[pyrn].y == y)))
    {
        wVol = 0x7fff;
        Pan = 0;
    }
    else
    {
        sqrdist = labs(player[pyrn].x - x) + labs(player[pyrn].y - y);
        if (sqrdist < 1500)
            wVol = 0x7fff;
        else if (sqrdist > 8500)
            wVol = 0x1f00;
        else
            wVol = 39000 - (sqrdist << 2);
    }

    if (nSound == S_STONELOOP1)
    {
        for (i = 0, flag = 0; i < MAX_ACTIVE_SAMPLES; i++)
        {
            if (nSound == SampleRay[i].number)
                return 0;
        }
    }

    for (i = 0, flag = 0; i < MAX_ACTIVE_SAMPLES; i++)
    {
        if (SampleRay[i].nFXHandle <= 0)
        {
            flag = 1;
            break;
        }
    }

    if (!flag && prioritize < 9)           // none available low priority
        return 0;
    else if (!flag)                        // none available but high priority
    {
        for (i = 0; i < MAX_ACTIVE_SAMPLES; i++)
        {
            if (SampleRay[i].priority < 9 && SampleRay[i].loopcount != -1)//sSOSSampleData[i].wLoopCount != -1)
            {
                //if (!sosDIGISampleDone(hSOSDriverHandles[DIGI], SampleRay[i].SOSHandle) && (sSOSSampleData[i].dwSampleSize != 0))
                if (SampleRay[i].nFXHandle > 0)
                {
                    FX_StopSound(SampleRay[i].nFXHandle);
                    SampleRay[i].loopcount = 0;
                    SampleRay[i].nSize = 0;
                    SampleRay[i].nFXHandle = 0;
//                    SampleRay[i].playing = 0;
                    SampleRay[i].priority = 0;
                    SampleRay[i].number = -1;
                    break;
                }
            }
        }
    }

    if (i >= MAX_ACTIVE_SAMPLES)
    {
        return 0; // FIXME - this shouldn't trigger?
    }

    assert(i >= 0 && i < 10);

    if (Pan) {
        Pan = ((getangle(player[pyrn].x - x, player[pyrn].y - y) + (2047 - player[pyrn].ang)) % kAngleMask) >> 6;
    }

    int SeekIndex = (DigiList[(nSound * 3) + 0] * 4096);

    if (klseek(hSoundFile, SeekIndex, SEEK_SET) < 0)
    {
        assert(1 == 2);
    }

    SampleRay[i].nSize = (uint16_t)DigiList[(nSound * 3) + 1];

//    assert(hSoundFileSize - SeekIndex >= SampleRay[i].nSize);


    int nRead = kread(hSoundFile, SampleRay[i].pData, SampleRay[i].nSize);
    if (nRead != SampleRay[i].nSize)
    {
        assert(1 == 2);
    }

    SampleRay[i].nFXHandle = FX_PlayRaw(SampleRay[i].pData, SampleRay[i].nSize, 11025, 0, 32, 32, 32, 0, fix16_one, (intptr_t)&SampleRay[i]);
    assert(SampleRay[i].nFXHandle > 0);

    #if 0 // TODO
    if (loopcount)
        sSOSSampleData[i].wLoopCount = loopcount;

    if (Pan)
        Pan = ((getangle(player[pyrn].x - x, player[pyrn].y - y) + (2047 - player[pyrn].ang)) % 2047) >> 6;

    sSOSSampleData[wIndex].wSamplePanLocation = PanArray[Pan];
    sSOSSampleData[wIndex].wVolume = wVol;
    SampleRay[wIndex].SOSHandle = sosDIGIStartSample(hSOSDriverHandles[DIGI], &sSOSSampleData[wIndex]);
    #endif

    SampleRay[i].x = x;
    SampleRay[i].y = y;
//    SampleRay[i].playing = 1;
    SampleRay[i].number = nSound;
    SampleRay[i].priority = prioritize;
    // TODO	ActiveSampleBits |= (0x01 << wIndex);

    return SampleRay[i].nFXHandle;
}

void playsound_loc(int nSound, int32_t xplc, int32_t yplc)
{
    SND_PlaySound(nSound, xplc, yplc, 1, 0);
}

void SND_UpdateSoundLoc(int which, int Volume, int Pan)
{
    #if 0
    gVol = Volume;
    gPan = sosDIGISetPanLocation(hSOSDriverHandles[DIGI], SampleRay[which].SOSHandle, PanArray[Pan]);
    sosDIGISetSampleVolume(hSOSDriverHandles[DIGI], SampleRay[which].SOSHandle, Volume);
    #endif
}

void updatesound_loc()
{
    #if 0
    unsigned wVol, wPan;
    long sqrdist;

    if (!SoundMode)
        return;

    for (int i = 0; i < MAX_ACTIVE_SAMPLES; i++)
    {
        if (SampleRay[i].playing && SampleRay[i].x && SampleRay[i].y)
        {
            if (sSOSSampleData[wIndex].dwSampleSize != 0)
            {
                sqrdist = labs(player[pyrn].x - SampleRay[wIndex].x) +
                    labs(player[pyrn].y - SampleRay[wIndex].y);

                if (sqrdist < 1500)
                    wVol = 0x7fff;
                else if (sqrdist > 8500)
                    wVol = 0x1f00;
                else
                    wVol = 39000 - (sqrdist << 2);

                wPan = ((getangle(player[pyrn].x - SampleRay[wIndex].x, player[pyrn].y - SampleRay[wIndex].y) + (2047 - player[pyrn].ang)) % kAngleMask) >> 6;
                SND_UpdateSoundLoc(wIndex, wVol, wPan);
                //sprintf(displaybuf,"%dVol %x Pan %x Dist %ld",SampleRay[wIndex].number,wVol,wPan,sqrdist);
                //displaytime=100;
            }
        }
    }
    #endif
}

void SND_CheckLoops()
{
}

void SND_StopLoop(int16_t nSound)
{
}

void SND_LoadSongs(uint16_t which)
{
    static int index;

    index = songsperlevel * which;                  //vanilla

    //if digi_midi used skip to those songs
    // if (wMIDIDeviceID == _MIDI_AWE32)
    //     index += songelements;                            //skip past vanilla

    //if soundcanvas skip to those songs
    if (wMIDIDeviceID == _MIDI_MPU_401/* || wMIDIDeviceID == _MIDI_GUS*/)
        index += songelements * 2;

    BaseSongPtr = SND_LoadMIDISong(index + BASE_SONG);
    EmbSongPtr = SND_LoadMIDISong(index + EMB_SONG);
    SpiceSongPtr = SND_LoadMIDISong(index + SPICE_SONG);
}

void SND_StartMusic(int16_t level)
{
    if ((!MusicMode) || !SD_Started)
        return;

    if (level > 5)
        level = rand() % 6;

    SND_SongFlush();
    SND_LoadSongs(level);
    SongPending = SND_PrepareMIDISong(BASE_SONG);
    SND_StartMIDISong(SongPending);
    SongPending = 0;
}

void SND_Mixer(int16_t wSource, int16_t wVolume)
{
    if (wSource == MIDI) {
        wMIDIVol = (wVolume << 3);
        HMISetMasterVolume(wMIDIVol);
    }
}

void SND_Sting(int16_t nSound)
{
}

void SND_FadeMusic()
{
}

// temp
int nMusicSize = 0;

char* SND_LoadMIDISong(int nSong)
{
    uint32_t nLength = SongList[(nSong * 3) + 1];
    uint32_t SeekIndex = (SongList[(nSong * 3) + 0] * 4096);
    char* pData = (char*)malloc(nLength);

    klseek(hSongsFile, SeekIndex, SEEK_SET);
    kread(hSongsFile, pData, nLength);

    nMusicSize = nLength;

    return pData;
}

void SND_MenuMusic(int nSong)
{
    if (!MusicMode || !SD_Started)
        return;

    if ((nSong == DEATHSONG) && (wMIDIDeviceID == _MIDI_FM))
        return;
    SND_SongFlush();

    if (nSong == MENUSONG)
    {
        if (wMIDIDeviceID == _MIDI_MPU_401/* || wMIDIDeviceID == _MIDI_AWE32 || wMIDIDeviceID == _MIDI_GUS*/)
        {
            BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + BASE_SONG + 2);
        }
        else
        {
            BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + BASE_SONG);
        }
    }
    else if (wMIDIDeviceID == _MIDI_MPU_401/* || wMIDIDeviceID == _MIDI_AWE32 || wMIDIDeviceID == _MIDI_GUS*/)
    {
        BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + 3 + BASE_SONG + 2);
    }

    SongPending = SND_PrepareMIDISong(BASE_SONG);
    SND_StartMIDISong(SongPending);
    SongPending = 0;
}

void sosMIDISongCallback(uint32_t hSong)
{
    int i;
    for (i = 0; i < MAX_ACTIVE_SONGS; i++)
        if (hSong == hSOSSongHandles[i])
            break;

    HMIUnInitSong(hSOSSongHandles[i]);
    hSOSSongHandles[i] = 0x7fff;
}

int SND_PrepareMIDISong(int SongIndex)
{
    int status;
    if (!MusicMode)
        return(0x7fff);

    if (hSOSSongHandles[SongIndex] != 0x7fff)
        return(0x7fff);

    if (SongIndex == BASE_SONG)
        sSOSInitSongs[SongIndex].songptr = (uint8_t*)BaseSongPtr;
    if (SongIndex == EMB_SONG)
        sSOSInitSongs[SongIndex].songptr = (uint8_t*)EmbSongPtr;
    if (SongIndex == SPICE_SONG)
        sSOSInitSongs[SongIndex].songptr = (uint8_t*)SpiceSongPtr;

    sSOSInitSongs[SongIndex].callback = sosMIDISongCallback;
    if ((status = HMIInitSong(&sSOSInitSongs[SongIndex], &sSOSTrackMap[SongIndex], (uint32_t*)&hSOSSongHandles[SongIndex])))
    {
        crash("Init Song Failed!");
    }

    return((int)hSOSSongHandles[SongIndex]);
}
int SND_StartMIDISong(int wSongHandle)
{
    HMISetMasterVolume(wMIDIVol);
    return(HMIStartSong(wSongHandle));
}

void SND_StopMIDISong(int wSongHandle)
{
    int i;
    for (i = 0; i < MAX_ACTIVE_SONGS; i++)
        if (hSOSSongHandles[i] == wSongHandle)
            break;

    if (i >= MAX_ACTIVE_SONGS)
        return;

    if (!HMISongDone(hSOSSongHandles[i]))
    {
        HMIStopSong(hSOSSongHandles[i]);
        HMIUnInitSong(hSOSSongHandles[i]);
        hSOSSongHandles[i] = 0x7fff;
        free(sSOSInitSongs[i].songptr);
    }
}

void SND_SongFlush()
{
    if (!MusicMode)
        return;

    if (hSOSSongHandles[BASE_SONG] != 0x7fff)
        SND_StopMIDISong(hSOSSongHandles[BASE_SONG]);
    if (hSOSSongHandles[EMB_SONG] != 0x7fff)
        SND_StopMIDISong(hSOSSongHandles[EMB_SONG]);
    if (hSOSSongHandles[SPICE_SONG] != 0x7fff)
        SND_StopMIDISong(hSOSSongHandles[SPICE_SONG]);
}

void SND_MIDIFlush(void)
{
    int i;
    for (i = 0; i < MAX_ACTIVE_SONGS; i++) {
        if (!HMISongDone(hSOSSongHandles[i]))
            HMIStopSong(hSOSSongHandles[i]);
        if (hSOSSongHandles[i] != 0x7fff)
            HMIUnInitSong(hSOSSongHandles[i]);
        hSOSSongHandles[i] = 0x7fff;
    }

    free(BaseSongPtr);
    free(EmbSongPtr);
    free(SpiceSongPtr);
}
