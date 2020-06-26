#include "compat.h"
#include "multivoc.h"
#include "reality.h"

extern int MV_Channels, MV_MixRate;

#define RTSEMITONE 1.05946309

//
// Uses info and snippets from this code:
// https://github.com/jombo23/N64-Tools/blob/master/N64%20Midi%20Tool/N64MidiLibrary/MidiParse.cpps
//

rt_CTL_t *musicCtl;
char *seqBuffer;
uint32_t seqDivision;
uint32_t seqTempo;

struct trackinfo_t {
    int status;
    uint32_t counter;
    uint8_t lastcmd;
    char *ptr;
    char *seq;
    char *altPattern;
    int altOffset;
    int altLength;
};

trackinfo_t tracks[16];

#define RTMUSICVOICES 32

struct channel_t {
    int volume;
    int pan;
    int program;
};

enum {
    VS_FREE = 0,
    VS_RELEASED,
    VS_ACTIVE
};

struct voice_t {
    int status;
    rt_soundinstance_t decodeState;
    int duration;
    double phase;
    double step;
    int16_t *ptr;
    uint32_t length;
    channel_t *chan;
    int note;
};

channel_t channel[16];

voice_t voicePool[RTMUSICVOICES];

voice_t *RT_FindFreeVoice(void)
{
    for (int i = 0; i < RTMUSICVOICES; i++)
    {
        auto voice = &voicePool[i];
        if (voice->status == VS_FREE)
        {
            return voice;
        }
    }
    for (int i = 0; i < RTMUSICVOICES; i++)
    {
        auto voice = &voicePool[i];
        if (voice->status == VS_RELEASED)
        {
            return voice;
        }
    }
    return nullptr;
}

void RT_LoadRTSound(rt_sound_t *snd)
{
    if (snd->ptr)
        return;
    lseek(rt_group, snd->wave->base, SEEK_SET);
    int l = snd->wave->len;
    snd->lock = 200;
    g_cache.allocateBlock((intptr_t *)&snd->ptr, l, &snd->lock);
    read(rt_group, snd->ptr, l);
}

void RT_ChanNoteOn(channel_t *chan, int note, int vel, int duration, int perc)
{
    auto voice = RT_FindFreeVoice();
    if (!voice)
        return;

    rt_instrument_t *inst = perc ? musicCtl->bank[0]->perc : musicCtl->bank[0]->inst[chan->program];
    rt_sound_t *rsnd = nullptr;

    for (int i = 0; i < inst->sound_count; i++)
    {
        if (inst->sounds[i]->key->keymin <= note && inst->sounds[i]->key->keymax >= note
            && inst->sounds[i]->key->velocitymin <= vel && inst->sounds[i]->key->velocitymax >= vel)
            rsnd = inst->sounds[i];
    }

    if (!rsnd)
        return;

    RT_LoadRTSound(rsnd);

    auto snd = &voice->decodeState;
    memset(snd, 0, sizeof(rt_soundinstance_t));
    snd->status = 1;
    snd->ptr = snd->snd = rsnd->ptr;
    snd->rtsound = rsnd;
    if (rsnd->wave->adpcm && rsnd->wave->adpcm->loop)
    {
        snd->loop = rsnd->wave->adpcm->loop->count;
        snd->loop_start = rsnd->wave->adpcm->loop->start;
        snd->loop_end = rsnd->wave->adpcm->loop->end;
    }

    voice->chan = chan;
    voice->note = note;
    voice->duration = duration;
    voice->status = VS_ACTIVE;


    voice->step = pow(RTSEMITONE, note - rsnd->key->keybase) * musicCtl->bank[0]->rate / MV_MixRate;
}

void RT_ChanNoteOff(channel_t *chan, int note)
{
    for (int i = 0; i < RTMUSICVOICES; i++)
    {
        auto voice = &voicePool[i];
        if (voice->status == VS_ACTIVE && voice->chan == chan && voice->note == note)
        {
            voice->status = VS_RELEASED;
        }
    }
}

uint8_t RT_ReadMidiByte(trackinfo_t *track)
{
    uint8_t b;
    if (track->altPattern)
    {
        b = track->altPattern[track->altOffset++];
        if (track->altOffset == track->altLength)
        {
            track->altPattern = nullptr;
            track->altOffset = 0;
            track->altLength = 0;
        }
    }
    else
    {
        b = *track->ptr++;
        if (b == 0xfe && *track->ptr != 0xfe)
        {
            uint8_t repeatFirstByte = *track->ptr++;
            uint16_t repeatDistanceFromBeginningMarker = (repeatFirstByte << 8) | (*track->ptr++);
            uint8_t repeatCount = *track->ptr++;
            track->altPattern = track->ptr - 4 - repeatDistanceFromBeginningMarker;
            track->altOffset = 0;
            track->altLength = repeatCount;
            b = track->altPattern[track->altOffset++];
        }
        else if (b == 0xfe && *track->ptr == 0xfe)
        {
            track->ptr++;
        }
        if (track->altOffset == track->altLength && track->altPattern)
        {
            track->altPattern = nullptr;
            track->altOffset = 0;
            track->altLength = 0;
        }
    }
    return b;
}

uint32_t RT_ReadMidiDelay(trackinfo_t *track)
{
    uint8_t b;
    uint32_t ret = 0;
    do
    {
        b = RT_ReadMidiByte(track);
        ret <<= 7;
        ret |= (b & 0x7f);
    } while (b & 0x80);
    return ret;
}

void RT_TrackAdvance(trackinfo_t *track)
{
    if (!track->status)
        return;
    if (track->counter > 0)
    {
        track->counter--;
        return;
    }
    while(track->counter == 0)
    {
        uint8_t cmd = RT_ReadMidiByte(track);
        uint8_t b1, b2;
        if ((cmd & 0x80) == 0)
        {
            b1 = cmd;
            cmd = track->lastcmd;
        }
        else
        {
            b1 = RT_ReadMidiByte(track);
        }
        track->lastcmd = cmd;
        if (cmd == 0xff)
        {
            uint8_t metacmd = b1;
            switch (metacmd)
            {
            case 0x51: // Tempo
            {
                uint8_t b1 = RT_ReadMidiByte(track);
                uint8_t b2 = RT_ReadMidiByte(track);
                uint8_t b3 = RT_ReadMidiByte(track);
                seqTempo = (b1 << 16) | (b2 << 8) | b3;
                break;
            }
            case 0x2e: // Loop start
                RT_ReadMidiByte(track);
                RT_ReadMidiByte(track);
                break;
            case 0x2d: // Loop start
            {
                track->ptr += 2;
                uint8_t b1 = *track->ptr++;
                uint8_t b2 = *track->ptr++;
                uint8_t b3 = *track->ptr++;
                uint8_t b4 = *track->ptr++;
                uint32_t offset;
                offset = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
                track->ptr -= offset;
                break;
            }
            case 0x2f: // EOT
                track->status = 0;
                return;
            }
        }
        else
        {
            uint8_t ch = cmd & 0x0f;
            auto chan = &channel[ch];
            switch (cmd & 0xf0)
            {
            case 0xb0: // Control
                b2 = RT_ReadMidiByte(track);
                break;
            case 0xc0: // Program change
                chan->program = b1;
                break;
            case 0x90: // Note On
            {
                b2 = RT_ReadMidiByte(track);
                int duration = RT_ReadMidiDelay(track);

                if (b2 == 0) // Note Off
                {
                    RT_ChanNoteOff(chan, b1);
                }
                else
                {
                    RT_ChanNoteOn(chan, b1, b2, duration, ch == 9);
                }

                break;
            }
            }
        }
        track->counter = RT_ReadMidiDelay(track);
    }
}

void RT_AdvanceNotes(void)
{
    for (int i = 0; i < RTMUSICVOICES; i++)
    {
        auto voice = &voicePool[i];
        if (voice->status == VS_ACTIVE && voice->duration)
        {
            if (--voice->duration == 0)
            {
                voice->status = VS_RELEASED;
            }
        }
    }
}

int serviceTimer = 0;

void RT_MusicService(void)
{
    auto routinebuf = MV_GetMusicRoutineBuffer();
    int16_t *buffer = (int16_t*)routinebuf.buffer;
    int sampless = routinebuf.size / (MV_Channels *2);

    for (int j = 0; j < sampless; j++)
    {
        while (serviceTimer >= MV_MixRate)
        {
            for (int j = 0; j < 16; j++)
            {
                if (!tracks[j].seq)
                    continue;
                RT_TrackAdvance(&tracks[j]);
            }
            RT_AdvanceNotes();
            serviceTimer -= MV_MixRate;
        }
        serviceTimer += (1000000LL * seqDivision / seqTempo);
        int left = 0, right = 0;
        for (int i = 0; i < RTMUSICVOICES; i++)
        {
            auto voice = &voicePool[i];
            if (voice->status)
            {
                if (voice->length == 0)
                {
                    RT_SoundDecode((const char**)&voice->ptr, &voice->length, &voice->decodeState);
                    if (voice->length == 0)
                    {
                        voice->status = VS_FREE;
                        continue;
                    }
                }
                left += *voice->ptr * 0.1;
                right += *voice->ptr * 0.1;
                int64_t cphase = (int64_t)voice->phase;
                voice->phase += voice->step;
                int todo = (int64_t)voice->phase - cphase;
                while (todo > 0)
                {
                    int samples = min<int>(todo, voice->length);
                    voice->length -= samples;
                    voice->ptr += samples;
                    voice->phase -= samples;
                    todo -= samples;
                    if (voice->length == 0)
                    {
                        RT_SoundDecode((const char**)&voice->ptr, &voice->length, &voice->decodeState);
                        if (voice->length == 0)
                        {
                            voice->status = VS_FREE;
                            break;
                        }
                    }
                }
            }
        }
        *buffer++ = clamp(left, INT16_MIN, INT16_MAX);
        *buffer++ = clamp(right, INT16_MIN, INT16_MAX);
    }
}

void RT_MusicInit(void)
{
    static const uint32_t musicCtlOffset = 0x7bc6e0;
    static const uint32_t musicTblOffset = 0x7bd580;
    musicCtl = RT_LoadCTL(musicCtlOffset, musicTblOffset);
    static const uint32_t musicSeqOffset = 0x7bbfc0;
    uint32_t offset;
    lseek(rt_group, musicSeqOffset + 4, SEEK_SET);
    read(rt_group, &offset, sizeof(uint32_t));
    offset = B_BIG32(offset);
    uint32_t siz;
    lseek(rt_group, musicSeqOffset + 8, SEEK_SET);
    read(rt_group, &siz, sizeof(uint32_t));
    siz = B_BIG32(siz);
    seqBuffer = (char*)Xmalloc(siz);
    if (!seqBuffer)
        return;
    lseek(rt_group, musicSeqOffset + offset, SEEK_SET);
    read(rt_group, seqBuffer, siz);
    for (int i = 0; i < 16; i++)
    {
        uint32_t trackOffset = *(uint32_t*)&seqBuffer[i * 4];
        trackOffset = B_BIG32(trackOffset);
        tracks[i].seq = trackOffset ? seqBuffer + trackOffset : nullptr;
    }
    seqDivision = *(uint32_t*)&seqBuffer[64];
    seqDivision = B_BIG32(seqDivision);
}

void RT_PlaySong(void)
{
    if (!musicCtl || !seqBuffer)
        return;

    seqTempo = 650050;
    for (int i = 0; i < 16; i++)
    {
        tracks[i].ptr = tracks[i].seq;
        if (tracks[i].ptr)
        {
            tracks[i].counter = RT_ReadMidiDelay(&tracks[i]);
            tracks[i].status = 1;
        }
        tracks[i].altPattern = nullptr;
    }
    MV_HookMusicRoutine(RT_MusicService);
}
