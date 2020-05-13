
#include "witchaven.h"
#include "sound.h"
#include "compat.h"
#include "baselayer.h"
#include "renderlayer.h" // for win_gethwnd()
#include "build.h"
#include "cache1d.h"
#include "music.h"
#include "fx_man.h"
#include "keyboard.h"
#include "control.h"
#include "config.h"
#include "player.h"

int lavasnd = -1;
int batsnd = -1;
int cartsnd = -1;

int SoundMode, wDIGIVol;
int MusicMode, wMIDIVol;
int use_rec_driver, voicecom_enabled;

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

void ASSCallback(intptr_t pSample)
{
    SampleType *sample = (SampleType*)pSample;
    sample->loopcount = 0;
    sample->nSize = 0;
    sample->nFXHandle = 0;
    sample->priority = 0;
    sample->number = -1;
}

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

    int status;
    if ((status = MUSIC_Init(MusicDevice)) == MUSIC_Ok)
    {
        if (MusicDevice == ASS_AutoDetect)
            MusicDevice = MIDI_GetDevice();
    }
    else if ((status = MUSIC_Init(ASS_AutoDetect)) == MUSIC_Ok)
    {
        MusicDevice = MIDI_GetDevice();
    }
    else
    {
        buildprintf("Music error: %s\n", MUSIC_ErrorString(status));
//        gs.MusicOn = FALSE;
        return;
    }

 //   MusicInitialized = TRUE;
    MUSIC_SetVolume(255); // gs.MusicVolume);

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

    if (hSoundFile != buildvfs_kfd_invalid) {
        kclose(hSoundFile);
    }

    if (hLoopsFile != buildvfs_kfd_invalid) {
        kclose(hLoopsFile);
    }

    if (hSongsFile != buildvfs_kfd_invalid) {
        kclose(hSongsFile);
    }
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

void SND_SongFlush()
{
}

void SND_StartMusic(int16_t level)
{
    #if 0
    if ((!MusicMode) || !SD_Started)
        return;

    if (level > 5)
        level = rand() % 6;

    if (MusicMode == _LOOP_MUSIC)
    {
        SND_LoadLoop(0);
        LoopPending = 1;
    }

    else
    {
        SND_SongFlush();
        SND_LoadSongs(level);
        SongPending = SND_PrepareMIDISong(BASE_SONG);
        SND_StartMIDISong(SongPending);
        SongPending = 0;
    }
    #endif
}

void SND_Mixer(int16_t wSource, int16_t wVolume)
{
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
    uint16_t nLength = (uint16_t)SongList[(nSong * 3) + 1];
    uint16_t SeekIndex = (SongList[(nSong * 3) + 0] * 4096);
    char* pData = new char[nLength];

    klseek(hSongsFile, SeekIndex, SEEK_SET);
    kread(hSongsFile, pData, nLength);

    nMusicSize = nLength;

    return pData;
}

void SND_MenuMusic(int nSong)
{
    char *pMusic = SND_LoadMIDISong((totallevels * songsperlevel) + BASE_SONG + 2);

    #if 0
    /* TODO
    if (!MusicMode || !SD_Started)
        return;

    if ((choose == DEATHSONG) && (wMIDIDeviceID == _MIDI_FM))
        return;
    */
    SND_SongFlush();

    if (nSong == MENUSONG)
    {
        if (wMIDIDeviceID == _MIDI_MPU_401 || wMIDIDeviceID == _MIDI_AWE32 || wMIDIDeviceID == _MIDI_GUS)
        {
            BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + BASE_SONG + 2);
        }
        else
        {
            BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + BASE_SONG);
        }
    }
    else if (wMIDIDeviceID == _MIDI_MPU_401 || wMIDIDeviceID == _MIDI_AWE32 || wMIDIDeviceID == _MIDI_GUS)
    {
        BaseSongPtr = SND_LoadMIDISong((totallevels * songsperlevel) + 3 + BASE_SONG + 2);
    }

    SongPending = SND_PrepareMIDISong(BASE_SONG);
    SND_StartMIDISong(SongPending);
    SongPending = 0;
    #endif
}