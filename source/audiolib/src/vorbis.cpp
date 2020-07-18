/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 Copyright (C) 2020 EDuke32 developers and contributors

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

/**
 * OggVorbis source support for MultiVoc
 */

#include "compat.h"
#include "_multivc.h"

#ifdef HAVE_VORBIS

#define STB_VORBIS_MAX_CHANNELS 2

#include "stb_vorbis.c"

#define BLOCKSIZE MV_MIXBUFFERSIZE

typedef struct
{
    stb_vorbis *vf;
    char block[BLOCKSIZE];
} vorbis_data;

// designed with multiple calls in mind
static void MV_GetVorbisCommentLoops(VoiceNode *voice)
{
    const char *vc_loopstart  = nullptr;
    const char *vc_loopend    = nullptr;
    const char *vc_looplength = nullptr;

    auto vf = (*(vorbis_data *)voice->rawdataptr).vf;

    for (int comment = 0; comment < vf->comment_list_length; ++comment)
    {
        auto entry = (const char *)vf->comment_list[comment];
        if (entry != nullptr && entry[0] != '\0')
        {
            const char *value = Bstrchr(entry, '=');

            if (!value)
                continue;

            const size_t field = value - entry;
            value += 1;

            for (int t = 0; t < loopStartTagCount && vc_loopstart == nullptr; ++t)
            {
                auto tag = loopStartTags[t];
                if (field == Bstrlen(tag) && Bstrncasecmp(entry, tag, field) == 0)
                    vc_loopstart = value;
            }

            for (int t = 0; t < loopEndTagCount && vc_loopend == nullptr; ++t)
            {
                auto tag = loopEndTags[t];
                if (field == Bstrlen(tag) && Bstrncasecmp(entry, tag, field) == 0)
                    vc_loopend = value;
            }

            for (int t = 0; t < loopLengthTagCount && vc_looplength == nullptr; ++t)
            {
                auto tag = loopLengthTags[t];
                if (field == Bstrlen(tag) && Bstrncasecmp(entry, tag, field) == 0)
                    vc_looplength = value;
            }
        }
    }
    
    if (vc_loopstart != nullptr)
    {
        int const ov_loopstart = Batoi(vc_loopstart);
        if (ov_loopstart >= 0) // a loop starting at 0 is valid
        {
            voice->Loop.Start = (const char *)(intptr_t)ov_loopstart;
            voice->Loop.Size  = 1;
        }
    }
    if (vc_loopend != nullptr)
    {
        if (voice->Loop.Size > 0)
        {
            int const ov_loopend = Batoi(vc_loopend);
            if (ov_loopend > 0)  // a loop ending at 0 is invalid
                voice->Loop.End = (const char *)(intptr_t)ov_loopend;
        }
    }
    if (vc_looplength != nullptr)
    {
        if (voice->Loop.Size > 0 && voice->Loop.End == 0)
        {
            int const ov_looplength = Batoi(vc_looplength);
            if (ov_looplength > 0)  // a loop of length 0 is invalid
                voice->Loop.End = (const char *)((intptr_t)ov_looplength + (intptr_t)voice->Loop.Start);
        }
    }
}

int MV_GetVorbisPosition(VoiceNode *voice)
{
    auto vd = (vorbis_data *)voice->rawdataptr;
    return stb_vorbis_get_sample_offset(vd->vf);
}

void MV_SetVorbisPosition(VoiceNode *voice, unsigned int position)
{
    auto vd = (vorbis_data *)voice->rawdataptr;
    stb_vorbis_seek(vd->vf, position);
}

static playbackstatus MV_GetNextVorbisBlock(VoiceNode *voice)
{
    int bytesread = 0;
    auto vd = (vorbis_data *)voice->rawdataptr;
    
    if (!vd)
        return NoMoreData;

    do
    {
        int bytes = stb_vorbis_get_samples_short_interleaved(vd->vf, voice->channels, (short *)(vd->block + bytesread), (BLOCKSIZE - bytesread) >> 1) * (voice->channels << 1);
        Bassert(bytes <= BLOCKSIZE - bytesread);
        if (bytes > 0)
        {
            bytesread += bytes;
            int currentPosition;
            if ((intptr_t)voice->Loop.End > 0 && (currentPosition = stb_vorbis_get_sample_offset(vd->vf)) >= (int)(intptr_t)voice->Loop.End)
            {
                if (!stb_vorbis_seek(vd->vf, (int)(intptr_t)voice->Loop.Start))
                {
                    MV_Printf("MV_GetNextVorbisBlock: LOOP_START %l, LOOP_END %l, stb_vorbis_seek: err %d\n",
                              (int)(intptr_t)voice->Loop.Start, (int)(intptr_t)voice->Loop.End, stb_vorbis_get_error(vd->vf));
                }
            }
            continue;
        }
        else if (bytes == 0)
        {
            if (voice->Loop.Size > 0)
            {
                if (!stb_vorbis_seek(vd->vf, (int)(intptr_t)voice->Loop.Start))
                {
                    MV_Printf("MV_GetNextVorbisBlock: LOOP_START %l, stb_vorbis_seek: err %d\n",
                              (int)(intptr_t)voice->Loop.Start, stb_vorbis_get_error(vd->vf));
                }
                else
                    continue;
            }
            else
                break;
        }
        else if (bytes < 0)
        {
            MV_Printf("MV_GetNextVorbisBlock: stb_vorbis_get_samples_short_interleaved: err %d\n", stb_vorbis_get_error(vd->vf));
            voice->rawdataptr = nullptr;
            voice->rawdatasiz = 0;
            stb_vorbis_close(vd->vf);
            ALIGNED_FREE_AND_NULL(vd);
            return NoMoreData;
        }
    } while (bytesread < BLOCKSIZE);

    if (bytesread == 0)
        return NoMoreData;

    uint32_t const samples = divideu32(bytesread, voice->channels << 1);

    voice->position = 0;
    voice->sound = vd->block;
    voice->length = samples << 16;

#ifdef GEKKO
    // If libtremor had the three additional ov_read() parameters that libvorbis has,
    // this would be better handled using the endianness parameter.
    int16_t *data = (int16_t *)(vd->block);  // assumes signed 16-bit
    for (bytesread = 0; bytesread < BLOCKSIZE / 2; ++bytesread)
        data[bytesread] = (data[bytesread] & 0xff) << 8 | ((data[bytesread] & 0xff00) >> 8);
#endif

    return KeepPlaying;
}

int MV_PlayVorbis3D(char *ptr, uint32_t length, int loophow, int pitchoffset, int angle, int distance, int priority, fix16_t volume, intptr_t callbackval)
{
    if (!MV_Installed)
        return MV_SetErrorCode(MV_NotInstalled);

    if (distance < 0)
    {
        distance = -distance;
        angle += MV_NUMPANPOSITIONS / 2;
    }

    int const vol = MIX_VOLUME(distance);

    // Ensure angle is within 0 - 127
    angle &= MV_MAXPANPOSITION;

    return MV_PlayVorbis(ptr, length, loophow, -1, pitchoffset, max(0, 255 - distance),
                         MV_PanTable[angle][vol].left, MV_PanTable[angle][vol].right, priority, volume, callbackval);
}

int MV_PlayVorbis(char *ptr, uint32_t length, int loopstart, int loopend, int pitchoffset, int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    UNREFERENCED_PARAMETER(loopend);

    if (!MV_Installed)
        return MV_SetErrorCode(MV_NotInstalled);

    auto voice = MV_AllocVoice(priority, sizeof(vorbis_data));

    if (voice == nullptr)
        return MV_SetErrorCode(MV_NoVoices);

    auto vd = (vorbis_data *)voice->rawdataptr;

    int status;

    vd->vf = stb_vorbis_open_memory((unsigned char *)ptr, length, &status, nullptr);

    if (status != VORBIS__no_error || vd->vf->channels < 1 || vd->vf->channels > 2)
    {
        if (status == VORBIS__no_error)
            stb_vorbis_close(vd->vf);
        else
            MV_Printf("MV_PlayVorbis: stb_vorbis_open_memory: err %d\n", status);

        voice->rawdatasiz = 0;
        ALIGNED_FREE_AND_NULL(voice->rawdataptr);
        return MV_SetErrorCode(MV_InvalidFile);
    }

    voice->wavetype    = FMT_VORBIS;
    voice->bits        = 16;
    voice->channels    = vd->vf->channels;
    voice->GetSound    = MV_GetNextVorbisBlock;
    voice->NextBlock   = vd->block;
    voice->priority    = priority;
    voice->callbackval = callbackval;
    voice->Loop        = { nullptr, nullptr, 0, (loopstart >= 0) };

    // load loop tags from metadata
    if (vd->vf->comment_list_length)
        MV_GetVorbisCommentLoops(voice);

    MV_SetVoicePitch(voice, vd->vf->sample_rate, pitchoffset);
    MV_SetVoiceMixMode(voice);

    MV_SetVoiceVolume(voice, vol, left, right, volume);
    MV_PlayVoice(voice);

    return voice->handle;
}

void MV_ReleaseVorbisVoice(VoiceNode *voice)
{
    Bassert(voice->wavetype == FMT_VORBIS && voice->rawdataptr != nullptr && voice->rawdatasiz == sizeof(vorbis_data));

    auto vd = (vorbis_data *)voice->rawdataptr;

    if (MV_LazyAlloc)
    {
        stb_vorbis_close(vd->vf);
        return;
    }

    voice->rawdataptr = nullptr;
    voice->rawdatasiz = 0;
    stb_vorbis_close(vd->vf);
    ALIGNED_FREE_AND_NULL(vd);
}
#else

int MV_PlayVorbis(char *, uint32_t, int, int, int, int, int, int, int, fix16_t, intptr_t)
{
    MV_Printf("MV_PlayVorbis: OggVorbis support not included in this binary.\n");
    return -1;
}

int MV_PlayVorbis3D(char *, uint32_t, int, int, int, int, int, fix16_t, intptr_t)
{
    MV_Printf("MV_PlayVorbis: OggVorbis support not included in this binary.\n");
    return -1;
}
#endif //HAVE_VORBIS
