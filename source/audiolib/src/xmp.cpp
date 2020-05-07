
#include "compat.h"

#ifdef HAVE_XMP

#include "_multivc.h"
#include "multivoc.h"
#include "pitch.h"
#include "pragmas.h"

#define BUILDING_STATIC
#include "libxmp-lite/xmp.h"

typedef struct {
    void * ptr;
    VoiceNode *owner;
    size_t length;
    xmp_context context;
    int time;
} xmp_data;

int MV_GetXMPPosition(VoiceNode *voice)
{
    auto xmpd = (xmp_data *)voice->rawdataptr;
    return xmpd->time;
}

void MV_SetXMPPosition(VoiceNode *voice, int position)
{
    auto xmpd = (xmp_data *)voice->rawdataptr;
    xmp_seek_time(xmpd->context, position);
}

static playbackstatus MV_GetNextXMPBlock(VoiceNode *voice)
{
    if (voice->rawdataptr == nullptr)
    {
        MV_Printf("MV_GetNextXMPBlock(): rawdataptr is null?!\n");
        return NoMoreData;
    }

    auto xmpd = (xmp_data *)voice->rawdataptr;
    struct xmp_frame_info mi;

    if (xmp_play_frame(xmpd->context) != 0)
    {
        if (voice->LoopSize > 0)
        {
            xmp_restart_module(xmpd->context);
            if (xmp_play_frame(xmpd->context) != 0)
                return NoMoreData;
        }
        else
            return NoMoreData;
    }

    xmp_get_frame_info(xmpd->context, &mi);

    xmpd->time = mi.time;

    uint32_t const samples = mi.buffer_size / (2 * (16/8)); // since 2-channel, 16-bit is hardcoded
    // uint32_t const samples = mi.buffer_size / (voice->channels * (voice->bits / 8));

    voice->sound        = (char const *)mi.buffer;
    voice->length       = samples << 16;
    voice->position     = 0;
    voice->BlockLength  = 0;

    MV_SetVoiceMixMode(voice);

    return KeepPlaying;
}

int MV_PlayXMP3D(char *ptr, uint32_t length, int loophow, int pitchoffset, int angle, int distance, int priority, fix16_t volume, intptr_t callbackval)
{
    if (!MV_Installed)
        return MV_SetErrorCode(MV_NotInstalled);

    if (distance < 0)
    {
        distance  = -distance;
        angle    += MV_NUMPANPOSITIONS / 2;
    }

    int vol = MIX_VOLUME(distance);

    // Ensure angle is within 0 - 127
    angle &= MV_MAXPANPOSITION;

    int left  = MV_PanTable[angle][vol].left;
    int right = MV_PanTable[angle][vol].right;
    int mid   = max( 0, 255 - distance );

    return MV_PlayXMP(ptr, length, loophow, -1, pitchoffset, mid, left, right, priority, volume, callbackval);
}

int MV_PlayXMP(char *ptr, uint32_t length, int loopstart, int loopend, int pitchoffset, int vol, int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    UNREFERENCED_PARAMETER(loopend);

    if (!MV_Installed)
        return MV_SetErrorCode(MV_NotInstalled);

    auto xmpd = (xmp_data *)Xcalloc(1, sizeof(xmp_data));
    if (!xmpd)
        return MV_SetErrorCode(MV_InvalidFile);

    xmpd->ptr = ptr;
    xmpd->length = length;

    if ((xmpd->context = xmp_create_context()) == nullptr)
    {
        Xfree(xmpd);
        return MV_SetErrorCode(MV_InvalidFile);
    }

    int const xmp_status = xmp_load_module_from_memory(xmpd->context, ptr, length);

    if (xmp_status)
    {
        xmp_free_context(xmpd->context);
        Xfree(xmpd);
        MV_Printf("MV_PlayXMP: xmp_load_module_from_memory failed (%i)\n", xmp_status);
        return MV_SetErrorCode(MV_InvalidFile);
    }

    // Request a voice from the voice pool
    auto voice = MV_AllocVoice(priority);
    if (voice == nullptr)
    {
        xmp_release_module(xmpd->context);
        xmp_free_context(xmpd->context);
        Xfree(xmpd);
        return MV_SetErrorCode(MV_NoVoices);
    }

    xmpd->owner = voice;

    voice->length      = 0;
    voice->sound       = 0;

    voice->wavetype    = FMT_XMP;
    voice->rawdataptr  = (void*)xmpd;
    voice->GetSound    = MV_GetNextXMPBlock;
    voice->LoopCount   = 0;
    voice->BlockLength = 0;
    voice->PitchScale  = PITCH_GetScale(pitchoffset);
    voice->next        = nullptr;
    voice->prev        = nullptr;
    voice->priority    = priority;
    voice->callbackval = callbackval;

    voice->bits        = 16;
    voice->channels    = 2;
    voice->SamplingRate = MV_MixRate;

    voice->Paused      = FALSE;

    voice->LoopStart   = 0;
    voice->LoopEnd     = 0;
    voice->LoopSize    = loopstart >= 0 ? 1 : 0;

    xmp_start_player(xmpd->context, MV_MixRate, 0);
    xmp_set_player(xmpd->context, XMP_PLAYER_INTERP, MV_XMPInterpolation);

    // CODEDUP multivoc.c MV_SetVoicePitch
    voice->RateScale = divideu32(voice->SamplingRate * voice->PitchScale, MV_MixRate);
    voice->FixedPointBufferSize = (voice->RateScale * MV_MIXBUFFERSIZE) - voice->RateScale;
    MV_SetVoiceMixMode(voice);

    MV_SetVoiceVolume(voice, vol, left, right, volume);
    MV_PlayVoice(voice);

    return voice->handle;
}

void MV_ReleaseXMPVoice(VoiceNode * voice)
{
    if (voice->wavetype != FMT_XMP || voice->rawdataptr == nullptr)
        return;

    auto xmpd = (xmp_data *) voice->rawdataptr;
    voice->rawdataptr = nullptr;

    xmp_end_player(xmpd->context);
    xmp_release_module(xmpd->context);
    xmp_free_context(xmpd->context);
    Xfree(xmpd);
}

void MV_SetXMPInterpolation(void)
{
    if (!MV_Installed)
        return;

    for (VoiceNode *voice = VoiceList.next; voice != &VoiceList; voice = voice->next)
        if (voice->wavetype == FMT_XMP)
            xmp_set_player((xmp_context)voice->rawdataptr, XMP_PLAYER_INTERP, MV_XMPInterpolation);
}

#else

#include "_multivc.h"

static char const NoXMP[] = "MV_PlayXMP: libxmp-lite support not included in this binary.\n";

int MV_PlayXMP(char *ptr, uint32_t ptrlength, int loopstart, int loopend, int pitchoffset, int vol,
                   int left, int right, int priority, fix16_t volume, intptr_t callbackval)
{
    UNREFERENCED_PARAMETER(ptr);
    UNREFERENCED_PARAMETER(ptrlength);
    UNREFERENCED_PARAMETER(loopstart);
    UNREFERENCED_PARAMETER(loopend);
    UNREFERENCED_PARAMETER(pitchoffset);
    UNREFERENCED_PARAMETER(vol);
    UNREFERENCED_PARAMETER(left);
    UNREFERENCED_PARAMETER(right);
    UNREFERENCED_PARAMETER(priority);
    UNREFERENCED_PARAMETER(volume);
    UNREFERENCED_PARAMETER(callbackval);

    MV_Printf(NoXMP);
    return -1;
}

int MV_PlayXMP3D(char *ptr, uint32_t ptrlength, int loophow, int pitchoffset, int angle,
                     int distance, int priority, fix16_t volume, intptr_t callbackval)
{
    UNREFERENCED_PARAMETER(ptr);
    UNREFERENCED_PARAMETER(ptrlength);
    UNREFERENCED_PARAMETER(loophow);
    UNREFERENCED_PARAMETER(pitchoffset);
    UNREFERENCED_PARAMETER(angle);
    UNREFERENCED_PARAMETER(distance);
    UNREFERENCED_PARAMETER(priority);
    UNREFERENCED_PARAMETER(volume);
    UNREFERENCED_PARAMETER(callbackval);

    MV_Printf(NoXMP);
    return -1;
}

#endif

// KEEPINSYNC libxmp-lite/src/*_load.c

static int it_test_memory(char const *ptr, uint32_t ptrlength)
{
    static char const it_magic[] = "IMPM";
    return !!(ptrlength < sizeof(it_magic) - 1 || Bmemcmp(ptr, it_magic, sizeof(it_magic) - 1));
}

static int mod_test_memory(char const *ptr, uint32_t ptrlength)
{
    if (ptrlength < 1084)
        return -1;

    char const * const buf = ptr + 1080;

    if (!Bstrncmp(buf + 2, "CH", 2) && isdigit((int)buf[0]) && isdigit((int)buf[1]))
    {
        int i = (buf[0] - '0') * 10 + buf[1] - '0';
        if (i > 0 && i <= 32)
            return 0;
    }

    if (!Bstrncmp(buf + 1, "CHN", 3) && isdigit((int)*buf))
    {
        if (*buf >= '0' && *buf <= '9')
            return 0;
    }

    if (!Bmemcmp(buf, "M.K.", 4))
        return 0;

    return -1;
}

static int s3m_test_memory(char const *ptr, uint32_t ptrlength)
{
    static char const s3m_magic[] = "SCRM";
    #define s3m_magic_offset 44

    return !!(ptrlength < s3m_magic_offset + sizeof(s3m_magic)-1 ||
        Bmemcmp(ptr + s3m_magic_offset, s3m_magic, sizeof(s3m_magic)-1) ||
        ptr[29] != 0x10);
}

static int xm_test_memory(char const *ptr, uint32_t ptrlength)
{
    static char const xm_magic[] = "Extended Module: ";
    return !!(ptrlength < sizeof(xm_magic) - 1 || Bmemcmp(ptr, xm_magic, sizeof(xm_magic) - 1));
}

static int mtm_test_memory(char const *ptr, uint32_t ptrlength)
{
    static char const mtm_magic[] = "MTM\x10";
    return !!(ptrlength < sizeof(mtm_magic) - 1 || Bmemcmp(ptr, mtm_magic, sizeof(mtm_magic) - 1));
}

int MV_IdentifyXMP(char const *ptr, uint32_t ptrlength)
{
    static decltype(mod_test_memory) * const module_test_functions[] =
    {
        it_test_memory,
        mod_test_memory,
        s3m_test_memory,
        xm_test_memory,
        mtm_test_memory,
    };

    for (auto const test_module : module_test_functions)
    {
        if (test_module(ptr, ptrlength) == 0)
            return 1;
    }

    return 0;
}
