#include "build.h"
#include "compat.h"
#include "music.h"
#include "fx_man.h"
#include "al_midi.h"
#include "common_game.h"
#include "config.h"
#include "resource.h"
#include "sound.h"
#include "renderlayer.h"

Resource gSoundRes;

int soundRates[13] = {
    11025,
    11025,
    11025,
    11025,
    11025,
    22050,
    22050,
    22050,
    22050,
    44100,
    44100,
    44100,
    44100,
};
#define kMaxChannels 32

int sndGetRate(int format)
{
    if (format < 13)
        return soundRates[format];
    return 11025;
}

SAMPLE2D Channel[kMaxChannels];

SAMPLE2D * FindChannel(void)
{
    for (int i = kMaxChannels - 1; i >= 0; i--)
        if (Channel[i].at5 == 0)
            return &Channel[i];
    ThrowError("No free channel available for sample");
    return NULL;
}

DICTNODE *hSong;
char *pSong;
int songSize;

void sndPlaySong(const char *songName, bool bLoop)
{
    if (*pSong)
        sndStopSong();
    if (!songName || strlen(songName) == 0)
        return;
    hSong = gSoundRes.Lookup(songName, "MID");
    if (!hSong)
        return;
    songSize = hSong->size;
    if (songSize > 65535)
        return;
    gSoundRes.Load(hSong, pSong);
    MUSIC_SetVolume(MusicVolume);
    MUSIC_PlaySong(pSong, songSize, bLoop);
}

bool sndIsSongPlaying(void)
{
    //return MUSIC_SongPlaying();
    return false;
}

void sndFadeSong(int nTime)
{
    // NUKE-TODO:
    //if (MusicDevice == -1)
    //    return;
    //if (gEightyTwoFifty && sndMultiPlayer)
    //    return;
    //MUSIC_FadeVolume(0, nTime);
    MUSIC_SetVolume(0);
    MUSIC_StopSong();
}

void sndSetMusicVolume(int nVolume)
{
    MusicVolume = nVolume;
    MUSIC_SetVolume(nVolume);
}

void sndSetFXVolume(int nVolume)
{
    FXVolume = nVolume;
    FX_SetVolume(nVolume);
}

void sndStopSong(void)
{
    MUSIC_StopSong();
    hSong = NULL;
    *pSong = 0;
}

void SoundCallback(intptr_t val)
{
    SAMPLE2D *pChannel = (SAMPLE2D*)val;
    pChannel->at0 = 0;
}

void sndKillSound(SAMPLE2D *pChannel);

void sndStartSample(const char *pzSound, int nVolume, int nChannel)
{
    if (!strlen(pzSound))
        return;
    dassert(nChannel >= -1 && nChannel < kMaxChannels);
    SAMPLE2D *pChannel;
    if (nChannel == -1)
        pChannel = FindChannel();
    else
        pChannel = &Channel[nChannel];
    if (pChannel->at0 > 0)
        sndKillSound(pChannel);
    pChannel->at5 = gSoundRes.Lookup(pzSound, "RAW");
    if (!pChannel->at5)
        return;
    int nSize = pChannel->at5->size;
    char *pData = (char*)gSoundRes.Lock(pChannel->at5);
    pChannel->at0 = FX_PlayRaw(pData, nSize, sndGetRate(1), 0, nVolume, nVolume, nVolume, nVolume, 1.f, (intptr_t)&pChannel->at0);
}

void sndStartSample(unsigned long nSound, int nVolume, int nChannel, bool bLoop)
{
    dassert(nChannel >= -1 && nChannel < kMaxChannels);
    DICTNODE *hSfx = gSoundRes.Lookup(nSound, "SFX");
    if (!hSfx)
        return;
    SFX *pEffect = (SFX*)gSoundRes.Lock(hSfx);
    dassert(pEffect != NULL);
    SAMPLE2D *pChannel;
    if (nChannel == -1)
        pChannel = FindChannel();
    else
        pChannel = &Channel[nChannel];
    if (pChannel->at0 > 0)
        sndKillSound(pChannel);
    pChannel->at5 = gSoundRes.Lookup(pEffect->rawName, "RAW");
    if (!pChannel->at5)
        return;
    if (nVolume < 0)
        nVolume = pEffect->relVol;
    int nSize = pChannel->at5->size;
    int nLoopEnd = nSize - 1;
    if (nLoopEnd < 0)
        nLoopEnd = 0;
    if (nSize <= 0)
        return;
    char *pData = (char*)gSoundRes.Lock(pChannel->at5);
    if (nChannel < 0)
        bLoop = false;
    if (bLoop)
    {
        pChannel->at0 = FX_PlayLoopedRaw(pData, nSize, pData + pEffect->loopStart, pData + nLoopEnd, sndGetRate(pEffect->format),
            0, nVolume, nVolume, nVolume, nVolume, 1.f, (intptr_t)&pChannel->at0);
        pChannel->at4 |= 1;
    }
    else
    {
        pChannel->at0 = FX_PlayRaw(pData, nSize, sndGetRate(pEffect->format), 0, nVolume, nVolume, nVolume, nVolume, 1.f, (intptr_t)&pChannel->at0);
        pChannel->at4 &= ~1;
    }
}

void sndStartWavID(unsigned long nSound, int nVolume, int nChannel)
{
    dassert(nChannel >= -1 && nChannel < kMaxChannels);
    SAMPLE2D *pChannel;
    if (nChannel == -1)
        pChannel = FindChannel();
    else
        pChannel = &Channel[nChannel];
    if (pChannel->at0 > 0)
        sndKillSound(pChannel);
    pChannel->at5 = gSoundRes.Lookup(nSound, "WAV");
    if (!pChannel->at5)
        return;
    char *pData = (char*)gSoundRes.Lock(pChannel->at5);
    pChannel->at0 = FX_Play(pData, pChannel->at5->size, 0, -1, 0, nVolume, nVolume, nVolume, nVolume, 1.f, (intptr_t)&pChannel->at0);
}

void sndKillSound(SAMPLE2D *pChannel)
{
    if (pChannel->at4 & 1)
    {
        FX_EndLooping(pChannel->at0);
        pChannel->at4 &= ~1;
    }
    FX_StopSound(pChannel->at0);
}

void sndStartWavDisk(const char *pzFile, int nVolume, int nChannel)
{
    // NUKE-TODO:
#if 0
    if (FXDevice == -1)
        return;
    dassert(nChannel >= -1 && nChannel < kMaxChannels);
    SAMPLE2D *pChannel;
    if (nChannel == -1)
        pChannel = FindChannel();
    else
        pChannel = &Channel[nChannel];
    if (pChannel->at0 > 0)
        sndKillSound(pChannel);
    struct find_t dosFind;
    if (_dos_findfirst(pzFile, NULL, &dosFind) != 0)
        return;
    char *pData = gSoundRes.Alloc(dosFind.size);
    if (!pData)
        return;
    if (!FileLoad(pzFile, pData, dosFind.size))
        return;
    pChannel->at5 = (DICTNODE*)pData;
    pChannel->at4 |= 2;
    pChannel->at0 = FX_PlayWAV(pData, 0, nVolume, nVolume, nVolume, nVolume, &pChannel->at0);
#endif
}

void sndKillAllSounds(void)
{
    for (int i = 0; i < kMaxChannels; i++)
    {
        SAMPLE2D *pChannel = &Channel[i];
        if (pChannel->at0 > 0)
            sndKillSound(pChannel);
        if (pChannel->at5)
        {
            if (pChannel->at4 & 2)
            {
#if 0
                free(pChannel->at5);
#else
                gSoundRes.Free(pChannel->at5);
#endif
                pChannel->at4 &= ~2;
            }
            else
            {
                gSoundRes.Unlock(pChannel->at5);
            }
            pChannel->at5 = 0;
        }
    }
}

void sndProcess(void)
{
    for (int i = 0; i < kMaxChannels; i++)
    {
        if (Channel[i].at0 <= 0 && Channel[i].at5)
        {
            if (Channel[i].at4 & 2)
            {
                gSoundRes.Free(Channel[i].at5);
                Channel[i].at4 &= ~2;
            }
            else
            {
                gSoundRes.Unlock(Channel[i].at5);
            }
            Channel[i].at5 = 0;
        }
    }
}

void InitSoundDevice(void)
{
#ifdef MIXERTYPEWIN
    void *initdata = (void *)win_gethwnd(); // used for DirectSound
#else
    void *initdata = NULL;
#endif
    int nStatus;
    nStatus = FX_Init(NumVoices, NumChannels, MixRate, initdata);
    if (nStatus != 0)
        ThrowError(FX_ErrorString(nStatus));
    if (ReverseStereo == 1)
        FX_SetReverseStereo(!FX_GetReverseStereo());
    FX_SetVolume(FXVolume);
    FX_SetCallBack(SoundCallback);
}

void DeinitSoundDevice(void)
{
    int nStatus = FX_Shutdown();
    if (nStatus != 0)
        ThrowError(FX_ErrorString(nStatus));
}

void InitMusicDevice(void)
{
    int nStatus = MUSIC_Init(0, 0);
    if (nStatus != 0)
        ThrowError(MUSIC_ErrorString(nStatus));
    DICTNODE *hTmb = gSoundRes.Lookup("GMTIMBRE", "TMB");
    if (hTmb)
        OPLMusic::AL_RegisterTimbreBank((unsigned char*)gSoundRes.Load(hTmb));
    MUSIC_SetVolume(MusicVolume);
}

void DeinitMusicDevice(void)
{
    FX_StopAllSounds();
    int nStatus = MUSIC_Shutdown();
    if (nStatus != 0)
        ThrowError(MUSIC_ErrorString(nStatus));
}

bool sndActive = false;

void sndTerm(void)
{
    if (!sndActive)
        return;
    sndActive = false;
    sndStopSong();
    Resource::Free(pSong);
    DeinitSoundDevice();
    DeinitMusicDevice();
}
extern char *pUserSoundRFF;
void sndInit(void)
{
    gSoundRes.Init(pUserSoundRFF ? pUserSoundRFF : "SOUNDS.RFF");
    memset(Channel, 0, sizeof(Channel));
    pSong = (char*)Resource::Alloc(65535);
    if (pSong)
        *pSong = 0;
    InitSoundDevice();
    InitMusicDevice();
    atexit(sndTerm);
    sndActive = true;
}
