//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#include "sounds.h"
#include "al_midi.h"
#include "compat.h"
#include "duke3d.h"
#include "renderlayer.h"  // for win_gethwnd()
#include "vfs.h"

#include <atomic>

int32_t g_numEnvSoundsPlaying, g_highestSoundIdx;
static bool g_dukeTalk;

static char *MusicPtr;

int32_t MusicIsWaveform;
int32_t MusicVoice = -1;

static bool MusicPaused;
static bool SoundPaused;

static uint32_t localQueueIndex;
static uint32_t freeSlotQueue[MAXVOICES];
#ifndef NDEBUG
static std::atomic<int> freeSlotPendingCnt;
#endif
static std::atomic<uint32_t> freeSlotReadIndex, freeSlotWriteIndex;

sound_t **g_sounds;
sound_t nullsound;
voiceinfo_t nullvoice = { -1, FX_Ok, UINT16_MAX };

void S_AllocIndexes(int sndidx)
{
    Bassert(sndidx < MAXSOUNDS);

    if (sndidx > g_highestSoundIdx || !g_highestSoundIdx)
    {
        int const lastSize = g_highestSoundIdx;
        g_highestSoundIdx  = sndidx;

        g_sounds = (sound_t **)Xrealloc(g_sounds, (g_highestSoundIdx + 1) * sizeof(intptr_t));

        if (!lastSize)
            g_sounds[0] = &nullsound;

        for (int i = lastSize + 1; i <= g_highestSoundIdx; i++)
            g_sounds[i] = &nullsound;
    }
}

static FORCE_INLINE void S_FillVoiceInfo(voiceinfo_t *snd, int16_t const owner, int16_t const voice, uint16_t const dist)
{
    *snd = { owner, voice, dist };
}

void S_SoundStartup(void)
{
#ifdef _WIN32
    void *initdata = (void *) win_gethwnd(); // used for DirectSound
#else
    void *initdata = NULL;
#endif

    int status = FX_Init(ud.config.NumVoices, ud.config.NumChannels, ud.config.MixRate, initdata);

    if (status != FX_Ok)
    {
        LOG_F(ERROR, "Failed initializing sound subsystem: %s", FX_ErrorString(status));
        return;
    }
    
    freeSlotReadIndex = freeSlotWriteIndex = localQueueIndex = 0;
#ifndef NDEBUG
    freeSlotPendingCnt = 0;
#endif
    nullsound.voices = &nullvoice;

    if (!g_sounds)
    {
        Bassert(g_highestSoundIdx == 0);
        g_sounds = (sound_t **)Xmalloc(sizeof(intptr_t));
        g_sounds[0] = &nullsound;
    }

    for (int i = 0; i <= g_highestSoundIdx; ++i)
    {
        if (g_sounds[i] == &nullsound)
            continue;

        g_sounds[i]->playing = 0;
        g_sounds[i]->voices = &nullvoice;

#ifdef CACHING_DOESNT_SUCK
        g_sounds[i]->lock = CACHE1D_UNLOCKED;
#endif
    }

    FX_SetVolume(ud.config.FXVolume);
    //S_MusicVolume(ud.config.MusicVolume);

#ifdef ASS_REVERSESTEREO
    FX_SetReverseStereo(ud.config.ReverseStereo);
#endif
    FX_SetCallBack(S_Callback);
}

void S_SoundShutdown(void)
{
    if (MusicVoice >= 0)
        S_MusicShutdown();

    int status = FX_Shutdown();
    if (status != FX_Ok)
    {
        Bsprintf(tempbuf, "Failed tearing down sound subsystem: %s", FX_ErrorString(status));
        LOG_F(ERROR, "%s", tempbuf);
        G_GameExit(tempbuf);
    }
}

void S_MusicStartup(void)
{
    int status;
    if ((status = MUSIC_Init(ud.config.MusicDevice)) == MUSIC_Ok)
    {
        if (ud.config.MusicDevice == ASS_AutoDetect)
            ud.config.MusicDevice = MIDI_GetDevice();
    }
    else if ((status = MUSIC_Init(ASS_AutoDetect)) == MUSIC_Ok)
    {
        ud.config.MusicDevice = MIDI_GetDevice();
    }
    else
    {
        LOG_F(ERROR, "Failed initializing music subsystem: %s", MUSIC_ErrorString(status));
        return;
    }

    MUSIC_SetVolume(ud.config.MusicVolume);

    auto const fil = kopen4load("d3dtimbr.tmb", 0);

    if (fil != buildvfs_kfd_invalid)
    {
        int l = kfilelength(fil);
        auto tmb = (uint8_t *)Xmalloc(l);
        kread(fil, tmb, l);
        AL_RegisterTimbreBank(tmb);
        Xfree(tmb);
        kclose(fil);
    }
}

void S_MusicShutdown(void)
{
    S_StopMusic();

    int status = MUSIC_Shutdown();
    if (status != MUSIC_Ok)
        LOG_F(ERROR, "Failed tearing down music subsystem: %s", MUSIC_ErrorString(status));
}

void S_PauseMusic(bool paused)
{
    if (MusicPaused == paused || (MusicIsWaveform && MusicVoice < 0))
        return;

    MusicPaused = paused;

    if (MusicIsWaveform)
    {
        FX_PauseVoice(MusicVoice, paused);
        return;
    }

    if (paused)
        MUSIC_Pause();
    else
        MUSIC_Continue();
}

void S_PauseSounds(bool paused)
{
    if (SoundPaused == paused)
        return;

    SoundPaused = paused;

    for (int i = 0; i <= g_highestSoundIdx; ++i)
    {
        if (g_sounds[i]->voices == &nullvoice)
            continue;

        for (int j = 0; j < MAXSOUNDINSTANCES; ++j)
        {
            auto &voice = g_sounds[i]->voices[j];
            if (bitmap_test(&g_sounds[i]->playing, j) && voice.handle > FX_Ok)
                FX_PauseVoice(voice.handle, paused);
        }
    }
}


void S_MusicVolume(int32_t volume)
{
    if (MusicIsWaveform && MusicVoice >= 0)
        FX_SetPan(MusicVoice, volume, volume, volume);

    MUSIC_SetVolume(volume);
}

void S_RestartMusic(void)
{
    if (ud.recstat != 2 && g_player[myconnectindex].ps->gm&MODE_GAME)
    {
        S_PlayLevelMusicOrNothing(g_musicIndex);
    }
    else if (G_GetLogoFlags() & LOGO_PLAYMUSIC)
    {
        S_PlaySpecialMusicOrNothing(MUS_INTRO);
    }
}

void S_MenuSound(void)
{
#ifndef EDUKE32_STANDALONE
    static int SoundNum;
    int const menusnds[] = {
        LASERTRIP_EXPLODE, DUKE_GRUNT,       DUKE_LAND_HURT,   CHAINGUN_FIRE, SQUISHED,      KICK_HIT,
        PISTOL_RICOCHET,   PISTOL_BODYHIT,   PISTOL_FIRE,      SHOTGUN_FIRE,  BOS1_WALK,     RPG_EXPLODE,
        PIPEBOMB_BOUNCE,   PIPEBOMB_EXPLODE, NITEVISION_ONOFF, RPG_SHOOT,     SELECT_WEAPON,
    };
    int s = VM_OnEventWithReturn(EVENT_OPENMENUSOUND, g_player[screenpeek].ps->i, screenpeek, FURY ? -1 : menusnds[SoundNum++ % ARRAY_SIZE(menusnds)]);
#else
    int s = VM_OnEventWithReturn(EVENT_OPENMENUSOUND, g_player[screenpeek].ps->i, screenpeek, -1);
#endif
    if (s != -1)
        S_PlaySound(s);
}

static int S_PlayMusic(const char *fn)
{
    if (!ud.config.MusicToggle)
        return 0;

    if (fn == NULL)
        return 1;

    buildvfs_kfd fp = S_OpenAudio(fn, 0, 1);
    if (EDUKE32_PREDICT_FALSE(fp == buildvfs_kfd_invalid))
    {
        LOG_F(ERROR, "Unable to play %s: file not found.",fn);
        return 2;
    }

    int32_t MusicLen = kfilelength(fp);

    if (EDUKE32_PREDICT_FALSE(MusicLen < 4))
    {
        LOG_F(ERROR, "Unable to play %s: no data.",fn);
        kclose(fp);
        return 3;
    }

    char * MyMusicPtr = (char *)Xaligned_alloc(16, MusicLen);
    int MyMusicSize = kread(fp, MyMusicPtr, MusicLen);

    if (EDUKE32_PREDICT_FALSE(MyMusicSize != MusicLen))
    {
        LOG_F(ERROR, "Unable to play %s: read %d of %d bytes expected.", fn, MyMusicSize, MusicLen);
        kclose(fp);
        ALIGNED_FREE_AND_NULL(MyMusicPtr);
        return 4;
    }

    kclose(fp);

    if (!Bmemcmp(MyMusicPtr, "MThd", 4))
    {
        int32_t retval = MUSIC_PlaySong(MyMusicPtr, MyMusicSize, MUSIC_LoopSong, fn);

        if (retval != MUSIC_Ok)
        {
            ALIGNED_FREE_AND_NULL(MyMusicPtr);
            return 5;
        }

        if (MusicIsWaveform && MusicVoice >= 0)
        {
            FX_StopSound(MusicVoice);
            MusicVoice = -1;
        }

        MusicIsWaveform = 0;
        ALIGNED_FREE_AND_NULL(MusicPtr);
        MusicPtr    = MyMusicPtr;
        g_musicSize = MyMusicSize;
    }
    else
    {
        int MyMusicVoice = FX_Play(MyMusicPtr, MusicLen, 0, 0, 0, ud.config.MusicVolume, ud.config.MusicVolume, ud.config.MusicVolume,
                                   FX_MUSIC_PRIORITY, fix16_one, MUSIC_ID);

        if (MyMusicVoice <= FX_Ok)
        {
            ALIGNED_FREE_AND_NULL(MyMusicPtr);
            return 5;
        }

        if (MusicIsWaveform && MusicVoice >= 0)
            FX_StopSound(MusicVoice);

        MUSIC_StopSong();

        MusicVoice      = MyMusicVoice;
        MusicIsWaveform = 1;
        ALIGNED_FREE_AND_NULL(MusicPtr);
        MusicPtr    = MyMusicPtr;
        g_musicSize = MyMusicSize;
    }

    return 0;
}

static void S_SetMusicIndex(unsigned int m)
{
    g_musicIndex = m;
    ud.music_episode = m / MAXLEVELS;
    ud.music_level   = m % MAXLEVELS;
}

bool S_TryPlayLevelMusic(unsigned int m)
{
    ud.returnvar[0] = m / MAXLEVELS;
    ud.returnvar[1] = m % MAXLEVELS;

    int retval = VM_OnEvent(EVENT_PLAYLEVELMUSICSLOT, g_player[myconnectindex].ps->i, myconnectindex);

    if (retval < 0)
        return false;

    char const * musicfn = g_mapInfo[m].musicfn;

    if (musicfn != NULL)
    {
        if (!S_PlayMusic(musicfn))
        {
            S_SetMusicIndex(m);
            return false;
        }
    }

    return true;
}

void S_PlayLevelMusicOrNothing(unsigned int m)
{
    if (S_TryPlayLevelMusic(m))
    {
        S_StopMusic();
        S_SetMusicIndex(m);
    }
}

int S_TryPlaySpecialMusic(unsigned int m)
{
    char const * musicfn = g_mapInfo[m].musicfn;
    if (musicfn != NULL)
    {
        if (!S_PlayMusic(musicfn))
        {
            S_SetMusicIndex(m);
            return 0;
        }
    }

    return 1;
}

void S_PlaySpecialMusicOrNothing(unsigned int m)
{
    if (S_TryPlaySpecialMusic(m))
    {
        S_StopMusic();
        S_SetMusicIndex(m);
    }
}

void S_ContinueLevelMusic(void)
{
    VM_OnEvent(EVENT_CONTINUELEVELMUSICSLOT, g_player[myconnectindex].ps->i, myconnectindex);
}

int32_t S_GetMusicPosition(void)
{
    int32_t position = 0;

    if (MusicIsWaveform)
        FX_GetPosition(MusicVoice, &position);

    return position;
}

void S_SetMusicPosition(int32_t position)
{
    if (MusicIsWaveform)
        FX_SetPosition(MusicVoice, position);
}

void S_StopMusic(void)
{
    MusicPaused = 0;

    if (MusicIsWaveform && MusicVoice >= 0)
    {
        FX_StopSound(MusicVoice);
        MusicVoice = -1;
        MusicIsWaveform = 0;
    }

    MUSIC_StopSong();

    ALIGNED_FREE_AND_NULL(MusicPtr);
    g_musicSize = 0;
}

void S_Cleanup(void)
{
    while (localQueueIndex != freeSlotReadIndex.load(std::memory_order_acquire))
    {
        uint32_t num = freeSlotQueue[++localQueueIndex & (MAXVOICES-1)];

        // I don't think this can actually happen
        while (num == 0xdeadbeef)
        {
            num = freeSlotQueue[(localQueueIndex-1) & (MAXVOICES-1)];
            LOG_F(ERROR, "Unexpected magic beef!");
        }

        freeSlotQueue[localQueueIndex & (MAXVOICES-1)] = 0xdeadbeef;
#ifndef NDEBUG
        int cnt = freeSlotPendingCnt.fetch_sub(1, std::memory_order_relaxed);
        Bassert(cnt >= 0);
#endif
        // negative index is RTS playback
        if ((int32_t)num < 0)
        {
            int const rtsindex = klabs((int32_t)num);

            Bassert((unsigned)rtsindex < sizeof(rts_lumplockbyte));
            if (rts_lumplockbyte[rtsindex] >= CACHE1D_LOCKED)
                --rts_lumplockbyte[rtsindex];
            continue;
        }

        // num + (MAXSOUNDS*MAXSOUNDINSTANCES) is a sound played globally
        // for which there was no open slot to keep track of the voice
        if (num >= (MAXSOUNDS*MAXSOUNDINSTANCES))
        {
            if (g_sounds[num-(MAXSOUNDS*MAXSOUNDINSTANCES)]->flags & SF_TALK)
                g_dukeTalk = false;
#ifdef CACHING_DOESNT_SUCK
            --g_soundlocks[num-(MAXSOUNDS*MAXSOUNDINSTANCES)];
#endif
            continue;
        }

        int const voiceindex = num & (MAXSOUNDINSTANCES - 1);

        num = (num - voiceindex) / MAXSOUNDINSTANCES;

        auto &snd   = g_sounds[num];
        auto &voice = snd->voices[voiceindex];

        int const spriteNum = voice.owner;

        Bassert(bitmap_test(&snd->playing, voiceindex));
        bitmap_clear(&snd->playing, voiceindex);

        if (snd->flags & SF_TALK && g_dukeTalk)
            g_dukeTalk = false;

        // MUSICANDSFX uses t_data[0] to control restarting the sound
        // CLEAR_SOUND_T0
        if (spriteNum != -1 && S_IsAmbientSFX(spriteNum) && sector[SECT(spriteNum)].lotag < 3)  // ST_2_UNDERWATER
            actor[spriteNum].t_data[0] = 0;

        S_FillVoiceInfo(&voice, -1, 0, UINT16_MAX);

#ifdef CACHING_DOESNT_SUCK
        --g_soundlocks[num];
#endif
    }
}

// returns number of bytes read
int32_t S_LoadSound(int num)
{
    if ((unsigned)num > (unsigned)g_highestSoundIdx || EDUKE32_PREDICT_FALSE(g_sounds[num] == &nullsound))
        return 0;

    auto &snd = g_sounds[num];

    buildvfs_kfd fp = S_OpenAudio(snd->filename, g_loadFromGroupOnly, 0);

    if (EDUKE32_PREDICT_FALSE(fp == buildvfs_kfd_invalid))
    {
        DVLOG_F(LOG_DEBUG, "Unable to load sound #%d: file %s not found.", num, snd->filename);
        return 0;
    }

    int32_t l = kfilelength(fp);
    g_sounds[num]->lock = CACHE1D_PERMANENT;
    snd->len = l;
    g_cache.allocateBlock((intptr_t *)&snd->ptr, l, (char *)&g_sounds[num]->lock);
    l = kread(fp, snd->ptr, l);
    kclose(fp);

    return l;
}

void cacheAllSounds(void)
{
    if (!g_sounds)
        return;

    for (int i=0, j=0; i <= g_highestSoundIdx; ++i)
    {
        if (g_sounds[i] != &nullsound && g_sounds[i]->ptr == nullptr)
        {
            S_LoadSound(i);
            j += g_sounds[i]->len;
            if (j >= 4 * 1024 * 1024)
                gameHandleEvents(), j = 0;
        }
    }
}

static int32_t S_DefineAudioIfSupported(char** fn, const char* name)
{
#if !defined HAVE_FLAC || !defined HAVE_VORBIS
    const char* extension = Bstrrchr(name, '.');
# if !defined HAVE_FLAC
    if (extension && !Bstrcasecmp(extension, ".flac"))
        return -2;
# endif
# if !defined HAVE_VORBIS
    if (extension && !Bstrcasecmp(extension, ".ogg"))
        return -2;
# endif
#endif
    realloc_copy(fn, name);
    return 0;
}

int32_t S_DefineSound(int sndidx, const char *name, int minpitch, int maxpitch, int priority, int type, int distance, float volume)
{
    if (EDUKE32_PREDICT_FALSE((unsigned)sndidx >= MAXSOUNDS))
        return -1;

    S_AllocIndexes(sndidx);

    if (g_sounds[sndidx] == &nullsound)
        g_sounds[sndidx] = (sound_t *)Xcalloc(1, sizeof(sound_t));

    auto &snd = g_sounds[sndidx];

    if (snd->filename == NULL)
    {
        int const len = Bstrlen(name);
        snd->filename = (char *)Xcalloc(len+1, sizeof(uint8_t));
        Bstrcpy(snd->filename, name);
    }

    if (S_DefineAudioIfSupported(&snd->filename, name))
        return -1;

    snd->minpitch   = clamp(minpitch, INT16_MIN, INT16_MAX);
    snd->maxpitch   = clamp(maxpitch, INT16_MIN, INT16_MAX);
    snd->priority   = priority & 0xFF;
    snd->flags       = type & ~SF_ONEINST_INTERNAL;
    snd->distOffset = clamp(distance, INT16_MIN, INT16_MAX);
    snd->volume     = volume * fix16_one;
    snd->voices     = &nullvoice;

    if (snd->flags & SF_LOOP)
        snd->flags |= SF_ONEINST_INTERNAL;

    return 0;
}

// Returns:
//   0: all OK
//  -1: ID declaration was invalid:
int32_t S_DefineMusic(const char* ID, const char* name)
{
    int32_t sel = MUS_FIRST_SPECIAL;

    Bassert(ID != NULL);

    if (!Bstrcmp(ID, "intro"))
    {
        // nothing
    }
    else if (!Bstrcmp(ID, "briefing"))
    {
        sel++;
    }
    else if (!Bstrcmp(ID, "loading"))
    {
        sel += 2;
    }
    else if (!Bstrcmp(ID, "usermap"))
    {
        sel += 3;
    }
    else
    {
        sel = G_GetMusicIdx(ID);
        if (sel < 0)
            return -1;
    }

    return S_DefineAudioIfSupported(&g_mapInfo[sel].musicfn, name);
}

static inline int S_GetPitch(int num)
{
    auto const &snd   = g_sounds[num];
    int const   range = klabs(snd->maxpitch - snd->minpitch);

    return (range == 0) ? snd->minpitch : min(snd->minpitch, snd->maxpitch) + wrand() % range;
}

static int S_GetSlot(int soundNum)
{
    int slot = 0;
    int bestslot = 0;
    uint16_t dist = 0;
    int position = 0;

    auto &snd = g_sounds[soundNum];

    S_Cleanup();

    do
    {
        if (bitmap_test(&snd->playing, slot) == 0) return slot;

        auto &voice = snd->voices[slot];
        int   fxpos = 0;

        if (FX_SoundValidAndActive(voice.handle))
        {
            if (voice.dist == dist)
                FX_GetPosition(voice.handle, &fxpos);

            if (voice.dist > dist || (voice.dist == dist && fxpos > position))
            {
                dist     = voice.dist;
                position = fxpos;
                bestslot = slot;
            }
        }
    }
    while (++slot < MAXSOUNDINSTANCES);

    if (!FX_SoundValidAndActive(snd->voices[bestslot].handle))
        return MAXSOUNDINSTANCES;

    slot = bestslot;
    FX_StopSound(snd->voices[slot].handle);
    S_Cleanup();

    return slot;
}

static FORCE_INLINE int S_GetAngle(int ang, const vec3_t &cam, const vec3_t &pos)
{
    return (2048 + ang - getangle(cam.x - pos.x, cam.y - pos.y)) & 2047;
}

static int S_CalcDistAndAng(int spriteNum, int soundNum, int sectNum, int angle,
                             const vec3_t &cam, const vec3_t &pos, int *distPtr, int *angPtr)
{
    int sndang = 0, sndist = 0, explosion = 0;

    if (PN(spriteNum) == APLAYER && P_Get(spriteNum) == screenpeek)
        goto sound_further_processing;

    sndang = S_GetAngle(angle, cam, pos);
    sndist = FindDistance3D(cam.x-pos.x, cam.y-pos.y, (cam.z-pos.z));

#ifdef SPLITSCREEN_MOD_HACKS
    if (g_fakeMultiMode==2)
    {
        // HACK for splitscreen mod: take the min of sound distances
        // to 1st and 2nd player.

        if (PN(spriteNum) == APLAYER && P_Get(spriteNum) == 1)
        {
            sndist = sndang = 0;
            goto sound_further_processing;
        }

        {
            const vec3_t &cam2 = g_player[1].ps->pos;
            int32_t sndist2 = FindDistance3D(cam2.x-pos.x, cam2.y-pos.y, (cam2.z-pos.z));

            if (sndist2 < sndist)
            {
                sectNum = g_player[1].ps->cursectnum;
                angle = fix16_to_int(g_player[1].ps->q16ang);

                sndist = sndist2;
                sndang = S_GetAngle(angle, cam2, pos);
            }
        }
    }
#endif

    if ((g_sounds[soundNum]->flags & (SF_GLOBAL|SF_DTAG)) != SF_GLOBAL && S_IsAmbientSFX(spriteNum) && (sector[SECT(spriteNum)].lotag&0xff) < 9)  // ST_9_SLIDING_ST_DOOR
        sndist = divscale14(sndist, SHT(spriteNum)+1);

sound_further_processing:
    sndist += g_sounds[soundNum]->distOffset;
    if (sndist < 0)
        sndist = 0;

#ifndef EDUKE32_STANDALONE
    if (!FURY && ((unsigned)sectNum >= (unsigned)numsectors || (unsigned)SECT(spriteNum) >= (unsigned)numsectors
        || (sndist && PN(spriteNum) != MUSICANDSFX
            && (!sectorsareconnected(sectNum, SECT(spriteNum))
                || !cansee(cam.x, cam.y, cam.z - (24 << 8), sectNum, SX(spriteNum), SY(spriteNum), SZ(spriteNum) - (24 << 8), SECT(spriteNum))))))
        sndist += sndist>>5;
#else
    UNREFERENCED_PARAMETER(sectNum);
#endif

    if ((g_sounds[soundNum]->flags & (SF_GLOBAL|SF_DTAG)) == (SF_GLOBAL|SF_DTAG))
    {
#ifndef EDUKE32_STANDALONE
boost:
#endif
        int const sdist = g_sounds[soundNum]->distOffset > 0 ? g_sounds[soundNum]->distOffset : 6144;

        explosion = true;

        if (sndist > sdist)
            sndist = sdist;
    }

#ifndef EDUKE32_STANDALONE
    else if (!FURY)
    {
        switch (soundGetMapping(soundNum))
        {
            case PIPEBOMB_EXPLODE__:
            case LASERTRIP_EXPLODE__:
            case RPG_EXPLODE__:
                goto boost;
        }
    }
#endif

    if ((g_sounds[soundNum]->flags & (SF_GLOBAL|SF_DTAG)) == SF_GLOBAL || sndist < ((255-LOUDESTVOLUME) << 6))
        sndist = ((255-LOUDESTVOLUME) << 6);

    *distPtr = sndist;
    *angPtr  = sndang;

    return explosion;
}

void S_AllocVoiceInfo(sound_t *snd)
{
    if (snd->voices == &nullvoice)
    {
        snd->voices = (voiceinfo_t*) Xcalloc(MAXSOUNDINSTANCES, sizeof(voiceinfo_t));
        for (int i=0; i<MAXSOUNDINSTANCES; ++i)
            snd->voices[i] = nullvoice;
    }
}

int S_PlaySound3D(int num, int spriteNum, const vec3_t& pos)
{
    int const sndNum = VM_OnEventWithReturn(EVENT_SOUND, spriteNum, screenpeek, num);

    if ((sndNum == -1 && num != -1) || !ud.config.SoundToggle) // check that the user returned -1, but only if -1 wasn't playing already (in which case, warn)
        return -1;

    if (EDUKE32_PREDICT_FALSE(!S_SoundIsValid(sndNum)))
    {
        LOG_F(WARNING, "Invalid sound #%d", sndNum);
        return -1;
    }

    auto &snd = g_sounds[sndNum];
    auto const pPlayer = g_player[myconnectindex].ps;

    if (((snd->flags & SF_ADULT) && ud.lockout) || (unsigned)spriteNum >= MAXSPRITES || (pPlayer->gm & MODE_MENU) || !FX_VoiceAvailable(snd->priority)
        || (pPlayer->timebeforeexit > 0 && pPlayer->timebeforeexit <= GAMETICSPERSEC * 3))
        return -1;

    S_AllocVoiceInfo(snd);

    // Duke talk
    if (snd->flags & SF_TALK)
    {
        if ((g_netServer || ud.multimode > 1) && PN(spriteNum) == APLAYER && P_Get(spriteNum) != screenpeek) // other player sound
        {
            if ((ud.config.VoiceToggle & 4) != 4)
                return -1;
        }
        // don't play if any Duke talk sounds are already playing
        else if (g_dukeTalk || !(ud.config.VoiceToggle & 1))
            return -1;
    }
    else if ((snd->flags & (SF_DTAG|SF_GLOBAL)) == SF_DTAG)  // Duke-Tag sound
    {
        int const voice = S_PlaySound(sndNum);

        if (voice <= FX_Ok)
            return -1;

        int slot = 0;
        while (slot < MAXSOUNDINSTANCES && snd->voices[slot].handle != voice)
            slot++;
        Bassert(slot < MAXSOUNDINSTANCES);
        snd->voices[slot].owner = spriteNum;

        return voice;
    }

    int32_t    sndist, sndang;
    int const  explosionp = S_CalcDistAndAng(spriteNum, sndNum, CAMERA(sect), fix16_to_int(CAMERA(q16ang)), CAMERA(pos), pos, &sndist, &sndang);
    int        pitch      = S_GetPitch(sndNum);
    auto const pOther     = g_player[screenpeek].ps;

#ifdef SPLITSCREEN_MOD_HACKS
    if (g_fakeMultiMode==2)
    {
        // splitscreen HACK
        if (g_player[1].ps->i == spriteNum)
            pOther = g_player[1].ps;
}
#endif

    if (pOther->sound_pitch)
        pitch += pOther->sound_pitch;

    if (explosionp)
    {
        if (pOther->cursectnum > -1 && sector[pOther->cursectnum].lotag == ST_2_UNDERWATER)
            pitch -= 1024;
    }
    else
    {
#if 0
        if (sndist > 32767 && PN(spriteNum) != MUSICANDSFX && (snd->mode & (SF_LOOP|SF_MSFX)) == 0)
            return -1;
#endif

        if (pOther->cursectnum > -1 && sector[pOther->cursectnum].lotag == ST_2_UNDERWATER
            && (snd->flags & SF_TALK) == 0)
            pitch = -768;
    }

    if (snd->playing && PN(spriteNum) != MUSICANDSFX)
        S_StopEnvSound(sndNum, spriteNum);

#ifdef CACHING_DOESNT_SUCK
    if (++g_soundlocks[sndNum] < CACHE1D_LOCKED)
        g_soundlocks[sndNum] = CACHE1D_LOCKED;
#endif

    int const sndSlot = S_GetSlot(sndNum);

    if (sndSlot >= MAXSOUNDINSTANCES)
    {
#ifdef CACHING_DOESNT_SUCK
        g_soundlocks[sndNum]--;
#endif
        return -1;
    }

    int const repeatp = (snd->flags & SF_LOOP);

    if (repeatp && (snd->flags & SF_ONEINST_INTERNAL) && snd->playing)
    {
#ifdef CACHING_DOESNT_SUCK
        g_soundlocks[sndNum]--;
#endif
        return -1;
    }

    Bassert(bitmap_test(&snd->playing, sndSlot) == 0);
    bitmap_set(&snd->playing, sndSlot);

    S_FillVoiceInfo(&snd->voices[sndSlot], spriteNum, -1, sndist >> 6);

    if (snd->flags & SF_TALK)
        g_dukeTalk = true;

    int const voice = FX_Play3D(snd->ptr, snd->len, repeatp ? FX_LOOP : FX_ONESHOT, pitch, sndang >> 4, sndist >> 6,
                                                        snd->priority, snd->volume, (sndNum * MAXSOUNDINSTANCES) + sndSlot);

    if (voice <= FX_Ok)
    {
        LOG_F(ERROR, "Unable to play %s: unsupported format or file corrupt.", snd->filename);
        bitmap_clear(&snd->playing, sndSlot);
#ifdef CACHING_DOESNT_SUCK
        g_soundlocks[sndNum]--;
#endif
        return -1;
    }

    snd->voices[sndSlot].handle = voice;

    return voice;
}

int S_PlaySound(int num)
{
    int sndnum = VM_OnEventWithReturn(EVENT_SOUND, g_player[screenpeek].ps->i, screenpeek, num);

    if ((sndnum == -1 && num != -1) || !ud.config.SoundToggle) // check that the user returned -1, but only if -1 wasn't playing already (in which case, warn)
        return -1;

    if (EDUKE32_PREDICT_FALSE(!S_SoundIsValid(sndnum)))
    {
        LOG_F(WARNING, "Invalid sound #%d", sndnum);
        return -1;
    }

    num = sndnum;

    auto &snd = g_sounds[num];

    if ((!(ud.config.VoiceToggle & 1) && (snd->flags & SF_TALK)) || ((snd->flags & SF_ADULT) && ud.lockout) || !FX_VoiceAvailable(snd->priority))
        return -1;

    int const pitch = S_GetPitch(num);

#ifdef CACHING_DOESNT_SUCK
    if (++g_soundlocks[num] < CACHE1D_LOCKED)
        g_soundlocks[num] = CACHE1D_LOCKED;
#endif

    S_AllocVoiceInfo(snd);

    sndnum = S_GetSlot(num);

    if (sndnum >= MAXSOUNDINSTANCES)
    {
#ifdef CACHING_DOESNT_SUCK
        g_soundlocks[num]--;
#endif
        return -1;
    }

    Bassert(bitmap_test(&snd->playing, sndnum) == 0);
    bitmap_set(&snd->playing, sndnum);

    if (snd->flags & SF_TALK)
        g_dukeTalk = true;

    int const voice = (snd->flags & SF_LOOP) ? FX_Play(snd->ptr, snd->len, 0, -1, pitch, LOUDESTVOLUME, LOUDESTVOLUME,
                                                  LOUDESTVOLUME, snd->len, snd->volume, (num * MAXSOUNDINSTANCES) + sndnum)
                                        : FX_Play3D(snd->ptr, snd->len, FX_ONESHOT, pitch, 0, 255 - LOUDESTVOLUME, snd->priority, snd->volume,
                                                    (num * MAXSOUNDINSTANCES) + sndnum);

    if (voice <= FX_Ok)
    {
        LOG_F(ERROR, "Error attempting to play %s", snd->filename);
        bitmap_clear(&snd->playing, sndnum);
#ifdef CACHING_DOESNT_SUCK
        g_soundlocks[num]--;
#endif
        return -1;
    }

    snd->voices[sndnum].handle = voice;

    return voice;
}

int A_PlaySound(int soundNum, int spriteNum)
{
    return (unsigned)spriteNum >= MAXSPRITES ? S_PlaySound(soundNum) :
        S_PlaySound3D(soundNum, spriteNum, sprite[spriteNum].xyz);
}

void S_StopEnvSound(int soundNum, int spriteNum)
{
    if (EDUKE32_PREDICT_FALSE(!S_SoundIsValid(soundNum)) || g_sounds[soundNum]->playing == 0)
        return;

    int j;

    do 
    {
        for (j=0; j < MAXSOUNDINSTANCES; ++j)
        {
            if (bitmap_test(&g_sounds[soundNum]->playing, j) == 0)
                continue;

            auto &voice = g_sounds[soundNum]->voices[j];

            if ((spriteNum == -1 && voice.handle > FX_Ok) || (spriteNum != -1 && voice.owner == spriteNum))
            {
                Bassert(spriteNum == -1 || voice.handle > FX_Ok);
                if (EDUKE32_PREDICT_FALSE(spriteNum >= 0 && voice.handle <= FX_Ok))
                    LOG_F(ERROR, "Invalid voice for sound #%d index #%d! (%d)", soundNum, j, voice.handle);
                else if (FX_SoundValidAndActive(voice.handle))
                    FX_StopSound(voice.handle);
                else S_Cleanup();
                break;
            }
        }
    } while (j < MAXSOUNDINSTANCES);
}

// Do not remove this or make it inline.
void S_StopAllSounds(void)
{
    FX_StopAllSounds();
    S_Cleanup();

    for (int i = 0; i <= g_highestSoundIdx; ++i)
    {
        if (g_sounds[i] == &nullsound)
            continue;

        g_sounds[i]->playing = 0;

        if (g_sounds[i]->voices != &nullvoice)
            for (int j = 0; j < MAXSOUNDINSTANCES; ++j)
                g_sounds[i]->voices[j] = nullvoice;

#ifdef CACHING_DOESNT_SUCK
        g_sounds[i]->lock = CACHE1D_UNLOCKED;
#endif
    }

    g_dukeTalk = false;
}

void S_ChangeSoundPitch(int soundNum, int spriteNum, int pitchoffset)
{
    if (EDUKE32_PREDICT_FALSE(g_sounds[soundNum] == &nullsound) || g_sounds[soundNum]->playing == 0)
        return;

    for (int j=0; j < MAXSOUNDINSTANCES; ++j)
    {
        if (bitmap_test(&g_sounds[soundNum]->playing, j) == 0)
            continue;

        auto &voice = g_sounds[soundNum]->voices[j];

        if ((spriteNum == -1 && voice.handle > FX_Ok) || (spriteNum != -1 && voice.owner == spriteNum))
        {
            Bassert(spriteNum == -1 || voice.handle > FX_Ok);
            if (EDUKE32_PREDICT_FALSE(spriteNum >= 0 && voice.handle <= FX_Ok))
                LOG_F(ERROR, "Invalid voice for sound #%d index #%d! (%d)", soundNum, j, voice.handle);
            else
                FX_SetPitch(voice.handle, pitchoffset);
        }
    }
}

void S_Update(void)
{
    if ((g_player[myconnectindex].ps->gm & (MODE_GAME|MODE_DEMO)) == 0)
        return;

    g_numEnvSoundsPlaying = 0;

    const vec3_t *c;
    int32_t ca,cs;

    if (ud.camerasprite == -1)
    {
        if (ud.overhead_on != 2)
        {
            c = &CAMERA(pos);
            cs = CAMERA(sect);
            ca = fix16_to_int(CAMERA(q16ang));
        }
        else
        {
            auto pPlayer = g_player[screenpeek].ps;
            c = &pPlayer->pos;
            cs = pPlayer->cursectnum;
            ca = fix16_to_int(pPlayer->q16ang);
        }
    }
    else
    {
        c = &sprite[ud.camerasprite].xyz;
        cs = sprite[ud.camerasprite].sectnum;
        ca = sprite[ud.camerasprite].ang;
    }

    int sndnum = 0;

    S_Cleanup();

    do
    {
        if (g_sounds[sndnum] == &nullsound || g_sounds[sndnum]->playing == 0)
            continue;

        int spriteNum, sndist, sndang;

        for (int j = 0; j < MAXSOUNDINSTANCES; ++j)
        {
            auto &voice = g_sounds[sndnum]->voices[j];

            if (bitmap_test(&g_sounds[sndnum]->playing, j) == 0)
                continue;
            
            // this is incremented here as a way to track how long the sound has been playing for ownerless sounds
            if ((unsigned)(spriteNum = voice.owner) >= MAXSPRITES)
            {
                if (voice.dist < UINT16_MAX)
                    voice.dist++;
                continue;
            }

            // AMBIENT_SOUND
            if (S_IsAmbientSFX(spriteNum))
                g_numEnvSoundsPlaying++;

            S_CalcDistAndAng(spriteNum, sndnum, cs, ca, *c, sprite[spriteNum].xyz, &sndist, &sndang);
            voice.dist = sndist >> 6;
            FX_Pan3D(voice.handle, sndang >> 4, sndist >> 6);
        }
    } while (++sndnum <= g_highestSoundIdx);
}

// when playing back a new sound needs an existing sound to be stopped first
// S_Callback() can be called from either the audio thread when a sound ends, or the main thread
// this is essentially a multiple producer single consumer queue
void S_Callback(intptr_t num)
{
    if ((int32_t)num == MUSIC_ID)
        return;

    if ((unsigned)num < (MAXSOUNDS * MAXSOUNDINSTANCES))
    {
        int const voiceindex = num & (MAXSOUNDINSTANCES-1);
        int anum = (num - voiceindex) / MAXSOUNDINSTANCES;    

        if (!bitmap_test(&g_sounds[anum]->playing, voiceindex))
        {
            LOG_F(WARNING, "Got callback for sound #%d index #%d that wasn't playing!", anum, voiceindex);
            return;
        }
    }

#ifndef NDEBUG
    int cnt = freeSlotPendingCnt.fetch_add(1, std::memory_order_relaxed);
    Bassert(cnt < MAXVOICES);
#endif

    freeSlotQueue[(freeSlotWriteIndex.fetch_add(1, std::memory_order_relaxed) + 1) & (MAXVOICES-1)] = (uint32_t)num;
    freeSlotReadIndex.fetch_add(1, std::memory_order_release);
}

void S_ClearSoundLocks(void)
{
#ifdef CACHING_DOESNT_SUCK
    int32_t i;
    int32_t const msp = g_highestSoundIdx;

    for (native_t i = 0; i < 11; ++i)
        if (rts_lumplockbyte[i] >= CACHE1D_LOCKED)
            rts_lumplockbyte[i] = CACHE1D_UNLOCKED;

    int32_t const msp = g_highestSoundIdx;

    for (native_t i = 0; i <= msp; ++i)
        if (g_soundlocks[i] >= CACHE1D_LOCKED)
            g_soundlocks[i] = CACHE1D_UNLOCKED;
#endif
}

int A_CheckSoundPlaying(int spriteNum, int soundNum)
{
    if (EDUKE32_PREDICT_FALSE((unsigned)soundNum > (unsigned)g_highestSoundIdx)) return 0;

    auto &snd = g_sounds[soundNum];

    if (snd->playing && spriteNum >= 0)
    {
        for (int j = 0; j < MAXSOUNDINSTANCES; ++j)
            if (bitmap_test(&snd->playing, j) && snd->voices[j].owner == spriteNum)
                return 1;
    }

    return (spriteNum == -1) ? (snd->playing != 0) : 0;
}

// Check if actor <i> is playing any sound.
int A_CheckAnySoundPlaying(int spriteNum)
{
    int const msp = g_highestSoundIdx;

    for (int i = 0; i <= msp; ++i)
    {
        auto &snd = g_sounds[i];

        for (int j = 0; j < MAXSOUNDINSTANCES; ++j)
            if (bitmap_test(&snd->playing, j) && snd->voices[j].owner == spriteNum)
                return 1;
    }

    return 0;
}

int S_CheckSoundPlaying(int soundNum)
{
    if (EDUKE32_PREDICT_FALSE((unsigned)soundNum > (unsigned)g_highestSoundIdx)) return false;
    return (g_sounds[soundNum]->playing != 0);
}
