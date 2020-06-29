#include "compat.h"
#include "multivoc.h"
#include "reality.h"
#include "../duke3d.h"

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

#define RTMUSICVOICES 64

struct channel_t {
    int volume;
    int pan;
    int program;
    int sustain;
};

enum {
    VS_FREE = 0,
    VS_RELEASED,
    VS_ACTIVE
};

enum {
    VE_ATTACK = 0,
    VE_DECAY,
    VE_SUSTAIN,
    VE_RELEASE
};

struct voice_t {
    int status;
    int vel;
    rt_sound_t *rsnd;
    rt_soundinstance_t decodeState;
    int duration;
    double phase;
    double step;
    int16_t *ptr;
    uint32_t length;
    channel_t *chan;
    int note;
    double envVol;
    double envStep;
    double envTarget;
    int envTimer;
    int envState;
};

channel_t channel[16];

voice_t voicePool[RTMUSICVOICES];

float pantable[128];

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

    voice->vel = vel;
    voice->rsnd = rsnd;
    voice->chan = chan;
    voice->note = note;
    voice->duration = duration;
    voice->status = VS_ACTIVE;
    voice->envState = VE_ATTACK;
    voice->envVol = 0.0;
    voice->envTarget = rsnd->env->attack_volume;
    voice->envTimer = MV_MixRate * (rsnd->env->attack_time / 1000000.0);
    voice->envStep = (double)(voice->envTarget - voice->envVol) / (double)voice->envTimer;

    voice->step = pow(RTSEMITONE, note - rsnd->key->keybase + rsnd->key->detune / 100.0) * musicCtl->bank[0]->rate / MV_MixRate;
}

void RT_ChanNoteOff(channel_t *chan, int note)
{
    for (int i = 0; i < RTMUSICVOICES; i++)
    {
        auto voice = &voicePool[i];
        if (voice->status == VS_ACTIVE && voice->chan == chan && voice->note == note)
        {
            voice->status = VS_RELEASED;
            
            voice->envState = VE_RELEASE;
            voice->envTarget = 0.0;
            voice->envTimer = MV_MixRate * (voice->rsnd->env->release_time / 1000000.0);
            voice->envStep = (double)(voice->envTarget - voice->envVol) / (double)voice->envTimer;
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
                switch (b1)
                {
                case 7:
                    chan->volume = b2;
                    break;
                case 10:
                    chan->pan = b2;
                    break;
                }
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
    track->counter--;
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

                voice->envState = VE_RELEASE;
                voice->envTarget = 0.0;
                voice->envTimer = MV_MixRate * (voice->rsnd->env->release_time / 1000000.0);
                voice->envStep = (double)(voice->envTarget - voice->envVol) / (double)voice->envTimer;
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
                float volume = voice->vel * voice->chan->volume * voice->rsnd->sample_volume / (128.f * 128.f * 256.f) * 0.5f;
                volume *= voice->envVol / 128.f;
                volume *= ud.config.MusicVolume / 256.f;
                int pan = voice->chan->pan + (voice->rsnd->sample_pan - 64);
                if (pan < 0)
                    pan = 0;
                if (pan > 127)
                    pan = 127;
                int s1 = *voice->ptr;
                int s2 = 0;
                if (voice->length <= 1)
                    s2 = s1; // FIXME
                else
                    s2 = *(voice->ptr + 1);
                int64_t cphase = (int64_t)voice->phase;
                double interp = voice->phase - cphase;
                left += (s2 * interp + s1 * (1.0 - interp)) * volume * pantable[127 - pan];
                right += (s2 * interp + s1 * (1.0 - interp)) * volume * pantable[pan];
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

                if (--voice->envTimer <= 0)
                {
                    if (voice->envState == VE_ATTACK)
                    {
                        voice->envState = VE_DECAY;
                        voice->envVol = voice->envTarget;
                        voice->envTarget = voice->rsnd->env->decay_volume;
                        voice->envTimer = MV_MixRate * (voice->rsnd->env->decay_time / 1000000.0);
                        voice->envStep = (double)(voice->envTarget - voice->envVol) / (double)voice->envTimer;
                    }
                    else if (voice->envState == VE_DECAY)
                    {
                        voice->envState = VE_SUSTAIN;
                        voice->envVol = voice->envTarget;
                        voice->envStep = 0;
                        voice->envTimer = 0;
                    }
                    else if (voice->envState == VE_RELEASE)
                    {
                        voice->status = VS_FREE;
                    }
                }
                else
                {
                    voice->envVol += voice->envStep;
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

    for (int i = 0; i < 128; i++)
    {
        pantable[i] = sinf((fPI / 2.f) * i / 128.f);
    }
}

int rt_musicactive = 0;

void RT_PlaySong(void)
{
    if (!musicCtl || !seqBuffer)
        return;

    memset(voicePool, 0, sizeof(voicePool));
    memset(channel, 0, sizeof(channel));
    seqTempo = 650050;
    for (int i = 0; i < 16; i++)
    {
        auto track = &tracks[i];
        auto seq = track->seq;
        memset(track, 0, sizeof(trackinfo_t));
        track->ptr = track->seq = seq;
        if (track->ptr)
        {
            track->counter = RT_ReadMidiDelay(track);
            track->status = 1;
        }
        track->altPattern = nullptr;
    }
    MV_HookMusicRoutine(RT_MusicService);
    rt_musicactive = 1;
}

void RT_PauseSong(bool paused)
{
    if (!rt_musicactive)
        return;
    if (paused)
        MV_UnhookMusicRoutine();
    else
        MV_HookMusicRoutine(RT_MusicService);
}

void RT_StopSong(void)
{
    if (!rt_musicactive)
        return;
    MV_UnhookMusicRoutine();
    rt_musicactive = 0;
}