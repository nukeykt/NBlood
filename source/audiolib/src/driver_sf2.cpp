#include "driver_sf2.h"
#include "compat.h"
#include "midi.h"
#include "_multivc.h"
#include "vfs.h"

#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#define TSF_MALLOC Bmalloc
#define TSF_REALLOC Brealloc
#define TSF_FREE Bfree
#define TSF_MEMCPY Bmemcpy
#define TSF_MEMSET Bmemset
#include "tsf.h"

char SF2_BankFile[BMAX_PATH];
static int SF2_Volume = MIDI_MaxVolume;
static tsf *synth;

static void SF2_NoteOff(int channel, int key, int velocity)
{
    UNREFERENCED_PARAMETER(velocity);
    tsf_channel_note_off(synth, channel, key);
}

static void SF2_NoteOn(int channel, int key, int velocity)
{
    tsf_channel_note_on(synth, channel, key, velocity * (1.f/127.f));
}

static void SF2_ControlChange(int channel, int number, int value)
{
    tsf_channel_midi_control(synth, channel, number, value);
}

static void SF2_ProgramChange(int channel, int program)
{
    tsf_channel_set_presetnumber(synth, channel, program, channel == 9);
}

static void SF2_SetPitchBend(int channel, int lsb, int msb)
{
    tsf_channel_set_pitchwheel(synth, channel, (msb << 7) | lsb);
}

static void SF2_SetVolume(int volume)
{
    SF2_Volume = clamp(volume, 0, MIDI_MaxVolume);
}

static int ErrorCode = SF2_Ok;

#define SF2_SetErrorCode(status) ErrorCode = (status);

int SF2_GetError(void) { return ErrorCode; }

const char* SF2_ErrorString(int ErrorNumber)
{
    const char *ErrorString;
    
    switch( ErrorNumber )
    {
        case SF2_Warning :
        case SF2_Error :
            ErrorString = SF2_ErrorString( ErrorCode );
            break;

        case SF2_Ok :
            ErrorString = "SF2 ok.";
            break;

        case SF2_BankError:
            ErrorString = "SF2 bank error.";
            break;
            
        default:
            ErrorString = "Unknown SF2 error.";
            break;
    }
        
    return ErrorString;
}

static int sf2_stream_read(buildvfs_kfd *handle, void *ptr, unsigned int size)
{
    return kread(*handle, ptr, size);
};

static int sf2_stream_skip(buildvfs_kfd *handle, unsigned int size)
{
    return !klseek(*handle, size, SEEK_CUR);
};

int SF2_MIDI_Init(midifuncs * const funcs)
{
    SF2_MIDI_Shutdown();

    auto sf2_handle = kopen4load(SF2_BankFile, 0);
    if (sf2_handle == buildvfs_kfd_invalid)
        return SF2_BankError;

    struct tsf_stream sf2_stream = { &sf2_handle, (int(*)(void*,void*,unsigned int)) & sf2_stream_read, (int(*)(void*,unsigned int)) & sf2_stream_skip };

    synth = tsf_load(&sf2_stream);
    if (!synth)
        return SF2_BankError;
    
    kclose(sf2_handle);

    Bmemset(funcs, 0, sizeof(midifuncs));

    funcs->NoteOff           = SF2_NoteOff;
    funcs->NoteOn            = SF2_NoteOn;
    funcs->PolyAftertouch    = nullptr;
    funcs->ControlChange     = SF2_ControlChange;
    funcs->ProgramChange     = SF2_ProgramChange;
    funcs->ChannelAftertouch = nullptr;
    funcs->PitchBend         = SF2_SetPitchBend;
    funcs->SetVolume         = SF2_SetVolume;

    SF2_Volume = MIDI_MaxVolume;
    
    return SF2_Ok;
}

void SF2_MIDI_HaltPlayback(void) { MV_UnhookMusicRoutine(); }

void SF2_MIDI_Shutdown(void)
{
    SF2_MIDI_HaltPlayback();

    if (synth != nullptr)
        tsf_close(synth);
    synth = nullptr;
}

int SF2_MIDI_StartPlayback(void)
{
    SF2_MIDI_HaltPlayback();

    tsf_set_output(synth, MV_Channels == 1 ? TSF_MONO : TSF_STEREO_INTERLEAVED, MV_MixRate, 0);
    tsf_reset(synth);
    for (int channel = 0; channel < 16; channel++)
    {
        SF2_ProgramChange(channel, 0);
    }
    MV_HookMusicRoutine(SF2_MIDI_Service);

    return MIDI_Ok;
}

void SF2_MIDI_SetTempo(int const tempo, int const division)
{
    MV_MIDIRenderTempo = tempo * division / 60;
    MV_MIDIRenderTimer = 0;
}

void SF2_MIDI_Service(void)
{
    int16_t * buffer16 = (int16_t *)MV_MusicBuffer;
    int32_t const samples = MV_MIXBUFFERSIZE;
    float buf[samples * 2];

    for (int32_t i = 0; i < samples;)
    {
        while (MV_MIDIRenderTimer >= MV_MixRate)
        {
            if (MV_MIDIRenderTempo >= 0)
                MIDI_ServiceRoutine();
            MV_MIDIRenderTimer -= MV_MixRate;
        }
        int32_t samplesrender = MV_MIDIRenderTempo > 0 ? (MV_MixRate - MV_MIDIRenderTimer + MV_MIDIRenderTempo - 1) / MV_MIDIRenderTempo : INT32_MAX;
        samplesrender = min(samplesrender, samples - i);
        tsf_render_float(synth, buf, samplesrender);
        for (int32_t j = 0; j < samplesrender * MV_Channels; j++)
        {
            *buffer16++ = clamp(int32_t(buf[j]*SF2_Volume*(32768.f/MIDI_MaxVolume)), INT16_MIN, INT16_MAX);
        }
        if (MV_MIDIRenderTempo >= 0) MV_MIDIRenderTimer += MV_MIDIRenderTempo * samplesrender;
        i += samplesrender;
    }
}
