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

#include "_multivc.h"
#include "compat.h"

#ifdef _WIN32
#include "winbits.h"
#endif

#ifdef HAVE_VORBIS

#define BLOCKSIZE MV_MIXBUFFERSIZE

#include "libasync_config.h"

#define OGG_IMPL
#define VORBIS_IMPL
#define OV_EXCLUDE_STATIC_CALLBACKS

#include "minivorbis.h"

typedef struct {
   void * ptr;
   async::task<void> task;
   size_t length;
   size_t pos;

   OggVorbis_File vf;

   char block[BLOCKSIZE];
   int lastbitstream;
} vorbis_data;

// designed with multiple calls in mind
static void MV_GetVorbisCommentLoops(VoiceNode *voice, vorbis_comment *vc)
{
    const char *vc_loopstart = nullptr;
    const char *vc_loopend = nullptr;
    const char *vc_looplength = nullptr;

    for (int comment = 0; comment < vc->comments; ++comment)
    {
        auto entry = (const char *)vc->user_comments[comment];
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
        const ogg_int64_t ov_loopstart = Batol(vc_loopstart);
        if (ov_loopstart >= 0) // a loop starting at 0 is valid
        {
            voice->Loop.Start = (const char *) (intptr_t) ov_loopstart;
            voice->Loop.Size  = 1;
        }
    }
    if (vc_loopend != nullptr)
    {
        if (voice->Loop.Size > 0)
        {
            const ogg_int64_t ov_loopend = Batol(vc_loopend);
            if (ov_loopend > 0) // a loop ending at 0 is invalid
                voice->Loop.End = (const char *) (intptr_t) ov_loopend;
        }
    }
    if (vc_looplength != nullptr)
    {
        if (voice->Loop.Size > 0 && voice->Loop.End == 0)
        {
            const ogg_int64_t ov_looplength = Batol(vc_looplength);
            if (ov_looplength > 0) // a loop of length 0 is invalid
                voice->Loop.End = (const char *) ((intptr_t) ov_looplength + (intptr_t) voice->Loop.Start);
        }
    }
}

// callbacks

static size_t read_vorbis(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    auto vorb = (vorbis_data *)datasource;

    errno = 0;

    if (vorb->length == vorb->pos)
        return 0;

    int nread = 0;

    for (; nmemb > 0; nmemb--, nread++)
    {
        int bytes = vorb->length - vorb->pos;

        if ((signed)size < bytes)
            bytes = (int)size;

        memcpy(ptr, (uint8_t *)vorb->ptr + vorb->pos, bytes);
        vorb->pos += bytes;
        ptr = (uint8_t *)ptr + bytes;

        if (vorb->length == vorb->pos)
        {
            nread++;
            break;
        }
    }

    return nread;
}


static int seek_vorbis(void *datasource, ogg_int64_t offset, int whence)
{
    auto vorb = (vorbis_data *)datasource;

    switch (whence)
    {
        case SEEK_SET: vorb->pos = 0; break;
        case SEEK_CUR: break;
        case SEEK_END: vorb->pos = vorb->length; break;
    }

    vorb->pos += offset;

    if (vorb->pos > vorb->length)
        vorb->pos = vorb->length;

    return vorb->pos;
}

static int close_vorbis(void *) { return 0; }

static long tell_vorbis(void *datasource)
{
    auto vorb = (vorbis_data *)datasource;

    return vorb->pos;
}


int MV_GetVorbisPosition(VoiceNode *voice)
{
    auto vd = (vorbis_data *) voice->rawdataptr;

    return ov_pcm_tell(&vd->vf);
}

void MV_SetVorbisPosition(VoiceNode *voice, int position)
{
    auto vd = (vorbis_data *) voice->rawdataptr;

    ov_pcm_seek(&vd->vf, position);
}

static playbackstatus MV_GetNextVorbisBlock(VoiceNode *voice)
{
    int bitstream;
    int bytesread = 0;
    auto vd = (vorbis_data *)voice->rawdataptr;

    if (!vd)
    {
        MV_Printf("null rawdataptr\n");
        return NoMoreData;
    }

    do
    {
#ifdef USING_TREMOR
        int bytes = ov_read(&vd->vf, vd->block + bytesread, BLOCKSIZE - bytesread, &bitstream);
#else
        int bytes = ov_read(&vd->vf, vd->block + bytesread, BLOCKSIZE - bytesread, 0, 2, 1, &bitstream);
#endif
        // fprintf(stderr, "ov_read = %d\n", bytes);
        if (bytes > 0)
        {
            ogg_int64_t currentPosition;
            bytesread += bytes;
            if ((ogg_int64_t)(intptr_t)voice->Loop.End > 0 &&
                (currentPosition = ov_pcm_tell(&vd->vf)) >= (ogg_int64_t)(intptr_t)voice->Loop.End)
            {
                bytesread -=
                (currentPosition - (ogg_int64_t)(intptr_t)voice->Loop.End) * voice->channels * 2;  // (voice->bits>>3)

                int const err = ov_pcm_seek(&vd->vf, (ogg_int64_t)(intptr_t)voice->Loop.Start);

                if (err != 0)
                {
                    MV_Printf("MV_GetNextVorbisBlock ov_pcm_seek: LOOP_START %l, LOOP_END %l, err %d\n",
                              (ogg_int64_t)(intptr_t)voice->Loop.Start, (ogg_int64_t)(intptr_t)voice->Loop.End, err);
                }
            }
            continue;
        }
        else if (bytes == OV_HOLE)
            continue;
        else if (bytes == 0)
        {
            if (voice->Loop.Size > 0)
            {
                int const err = ov_pcm_seek(&vd->vf, (ogg_int64_t)(intptr_t)voice->Loop.Start);

                if (err != 0)
                {
                    MV_Printf("MV_GetNextVorbisBlock ov_pcm_seek: LOOP_START %l, err %d\n",
                              (ogg_int64_t)(intptr_t)voice->Loop.Start, err);
                }
                else
                    continue;
            }
            else
            {
                break;
            }
        }
        else if (bytes < 0)
        {
            MV_Printf("MV_GetNextVorbisBlock ov_read: err %d\n", bytes);
            voice->rawdataptr = nullptr;
            voice->rawdatasiz = 0;
            ov_clear(&vd->vf);
            ALIGNED_FREE_AND_NULL(vd);
            return NoMoreData;
        }
    } while (bytesread < BLOCKSIZE);

    if (bytesread == 0)
        return NoMoreData;

    if (bitstream != vd->lastbitstream)
    {
        vorbis_info *vi = ov_info(&vd->vf, -1);
        if (!vi || (vi->channels != 1 && vi->channels != 2))
            return NoMoreData;

        voice->channels     = vi->channels;
        voice->SamplingRate = vi->rate;
        voice->RateScale    = divideu64((uint64_t)voice->SamplingRate * voice->PitchScale, MV_MixRate);

        voice->FixedPointBufferSize = (voice->RateScale * MV_MIXBUFFERSIZE) - voice->RateScale;
        vd->lastbitstream = bitstream;
        MV_SetVoiceMixMode(voice);
    }

    uint32_t const samples = divideu32(bytesread, ((voice->bits>>3) * voice->channels));

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

static constexpr ov_callbacks vorbis_callbacks = { read_vorbis, seek_vorbis, close_vorbis, tell_vorbis };

int MV_PlayVorbis(char *ptr, uint32_t length, int loopstart, int loopend, int pitchoffset, int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    UNREFERENCED_PARAMETER(loopend);

    if (!MV_Installed)
        return MV_SetErrorCode(MV_NotInstalled);

    auto voice = MV_AllocVoice(priority, sizeof(vorbis_data));
    if (voice == nullptr)
        return MV_SetErrorCode(MV_NoVoices);

    auto vd = (vorbis_data *)voice->rawdataptr;

    vd->ptr    = ptr;
    vd->pos    = 0;
    vd->length = length;

    // pitchoffset is passed into the worker tasks using the preexisting lastbitstream member
    vd->lastbitstream = pitchoffset;

    voice->wavetype    = FMT_VORBIS;
    voice->bits        = 16;
    voice->NextBlock   = vd->block;
    voice->priority    = priority;
    voice->callbackval = callbackval;
    voice->Loop        = { nullptr, nullptr, 0, (loopstart >= 0) };
    voice->GetSound    = MV_GetNextVorbisBlock;
    voice->Paused      = true;

    MV_SetVoiceMixMode(voice);
    MV_SetVoiceVolume(voice, vol, left, right, volume);

    //if (vd->task.valid() && !vd->task.ready())
    //    vd->task.wait();

    vd->task = async::spawn([voice]
    {
#if defined _WIN32 && !defined NDEBUG
        debugThreadName("MV_PlayVorbis");
#endif
        auto vd = (vorbis_data *)voice->rawdataptr;

        // yoinked ptr indicates we're in some shitshow scenario where we tried to cancel
        // the voice before the decoder even got a chance to start initializing

        if (!vd)
        {
            voice->rawdatasiz = 0;
            MV_PlayVoice(voice);
            return;
        }

        int status = ov_open_callbacks((void *)vd, &vd->vf, 0, 0, vorbis_callbacks);
        vorbis_info *vi;

        if (status < 0 || ((vi = ov_info(&vd->vf, 0)) == nullptr) || vi->channels < 1 || vi->channels > 2)
        {
            if (status == 0)
                ov_clear(&vd->vf);
            else
                MV_Printf("MV_PlayVorbis: err %d\n", status);

            ALIGNED_FREE_AND_NULL(voice->rawdataptr);
            voice->rawdatasiz = 0;
            MV_SetErrorCode(MV_InvalidFile);
            MV_PlayVoice(voice);
            return;
        }

        voice->channels = vi->channels;

        // load loop tags from metadata
        if (auto comment = ov_comment(&vd->vf, 0))
            MV_GetVorbisCommentLoops(voice, comment);

        MV_SetVoicePitch(voice, vi->rate, vd->lastbitstream);
        vd->lastbitstream = -1;
        MV_PlayVoice(voice);
    });
    
    return voice->handle;
}

void MV_ReleaseVorbisVoice(VoiceNode *voice)
{
    Bassert(voice->wavetype == FMT_VORBIS && voice->rawdataptr != nullptr && voice->rawdatasiz == sizeof(vorbis_data));

    auto vd = (vorbis_data *)voice->rawdataptr;
    //vd->task.wait();

    ov_clear(&vd->vf);

    if (MV_LazyAlloc)
        return;

    voice->rawdataptr = nullptr;
    voice->rawdatasiz = 0;
    ALIGNED_FREE_AND_NULL(vd);
}
#else
#include "_multivc.h"

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
