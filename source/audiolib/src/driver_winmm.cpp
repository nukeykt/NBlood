/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 Copyright (C) EDuke32 developers and contributors

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
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */

/**
 * WinMM MIDI output driver
 */

#include "driver_winmm.h"

#include "baselayer.h"
#include "linklist.h"
#include "midi.h"
#include "midifuncs.h"
#include "multivoc.h"
#include "osd.h"
#include "winbits.h"

#include <mmsystem.h>

#ifdef _MSC_VER
#define inline _inline
#endif

UINT WinMM_DeviceID = MIDI_MAPPER;

static int ErrorCode = WinMMErr_Ok;

static BOOL      midiInstalled;
static HMIDISTRM midiStream;
static uint32_t midiThreadTimer;
static uint32_t midiLastEventTime;
static uint32_t midiThreadQueueTimer;
static uint32_t midiThreadQueueTicks;
static HANDLE   midiThread;
static HANDLE   midiThreadQuitEvent;
static HANDLE   midiBufferFinishedEvent;
static HANDLE   midiThreadResetEvent;
static HANDLE   midiMutex;
static int      midiStreamRunning;
static int      midiLastDivision;

#define MME_THREAD_QUEUE_INTERVAL 10        // 1/10 sec
#define MME_MIDI_BUFFER_SPACE (12 * 128u)  // 128 note-on events

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#endif
typedef struct MidiBuffer
{
    struct MidiBuffer *next;
    struct MidiBuffer *prev;
    MIDIHDR hdr;
    DWORD data[];
} MidiBuffer;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

static MidiBuffer finishedBufferList;
static MidiBuffer activeBufferList;
static MidiBuffer spareBufferList;
static MidiBuffer *currentMidiBuffer;

static int maxActiveMidiBuffers, maxSpareMidiBuffers;
static int numActiveMidiBuffers, numSpareMidiBuffers;

static void midi_destroy_thread(void);
static void CALLBACK midi_callback(HMIDIOUT out, UINT msg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int WinMMDrv_GetError(void) { return ErrorCode; }

const char *WinMMDrv_ErrorString(int ErrorNumber)
{
    switch (ErrorNumber)
    {
        case WinMMErr_Error:             return WinMMDrv_ErrorString(ErrorCode);
        case WinMMErr_Ok:                return "MME ok.";
        case WinMMErr_MIDIStreamOpen:    return "MIDI error: failed opening stream.";
        case WinMMErr_MIDIStreamRestart: return "MIDI error: failed starting stream.";
        case WinMMErr_MIDICreateEvent:   return "MIDI error: failed creating play thread quit event.";
        case WinMMErr_MIDIPlayThread:    return "MIDI error: failed creating play thread.";
        case WinMMErr_MIDICreateMutex:   return "MIDI error: failed creating play mutex.";
        default:                         return "Unknown MME error code.";
    }
}


// will append "err nnn (ssss)\n" to the end of the string it emits
static void midi_error(MMRESULT rv, const char *str)
{
    const char * errtxt = "?";

    switch (rv)
    {
        case MMSYSERR_NOERROR:      errtxt = "MMSYSERR_NOERROR";      break;
        case MMSYSERR_BADDEVICEID:  errtxt = "MMSYSERR_BADDEVICEID";  break;
        case MMSYSERR_NOTENABLED:   errtxt = "MMSYSERR_NOTENABLED";   break;
        case MMSYSERR_ALLOCATED:    errtxt = "MMSYSERR_ALLOCATED";    break;
        case MMSYSERR_INVALHANDLE:  errtxt = "MMSYSERR_INVALHANDLE";  break;
        case MMSYSERR_NODRIVER:     errtxt = "MMSYSERR_NODRIVER";     break;
        case MMSYSERR_NOMEM:        errtxt = "MMSYSERR_NOMEM";        break;
        case MMSYSERR_NOTSUPPORTED: errtxt = "MMSYSERR_NOTSUPPORTED"; break;
        case MMSYSERR_BADERRNUM:    errtxt = "MMSYSERR_BADERRNUM";    break;
        case MMSYSERR_INVALFLAG:    errtxt = "MMSYSERR_INVALFLAG";    break;
        case MMSYSERR_INVALPARAM:   errtxt = "MMSYSERR_INVALPARAM";   break;
        case MMSYSERR_HANDLEBUSY:   errtxt = "MMSYSERR_HANDLEBUSY";   break;
        case MMSYSERR_INVALIDALIAS: errtxt = "MMSYSERR_INVALIDALIAS"; break;
        case MMSYSERR_BADDB:        errtxt = "MMSYSERR_BADDB";        break;
        case MMSYSERR_KEYNOTFOUND:  errtxt = "MMSYSERR_KEYNOTFOUND";  break;
        case MMSYSERR_READERROR:    errtxt = "MMSYSERR_READERROR";    break;
        case MMSYSERR_WRITEERROR:   errtxt = "MMSYSERR_WRITEERROR";   break;
        case MMSYSERR_DELETEERROR:  errtxt = "MMSYSERR_DELETEERROR";  break;
        case MMSYSERR_VALNOTFOUND:  errtxt = "MMSYSERR_VALNOTFOUND";  break;
        case MMSYSERR_NODRIVERCB:   errtxt = "MMSYSERR_NODRIVERCB";   break;
        default:                                                      break;
    }
    debug_break();

    LOG_F(ERROR, "%s: %s (0x%08x)", str, errtxt, rv);
}

// AddressSanitizer bug causes heap destruction with some functions from winmm.lib
// see https://vsf-prod.westus.cloudapp.azure.com/content/problem/1100813/false-positive-bad-free-when-using-asan-with-winmm.html
static void midi_dispose_buffer(MidiBuffer *node, const char *caller)
{
    if (node->hdr.dwFlags & MHDR_PREPARED)
    {
#if __SANITIZE_ADDRESS__ != 1
        VLOG_F(LOG_DEBUG, "%s/midi_dispose_buffer unpreparing buffer %p", caller, node);
        auto rv = midiOutUnprepareHeader((HMIDIOUT)midiStream, &node->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "midi_dispose_buffer: error in midiOutUnprepareHeader");
#endif
       node->hdr.dwFlags &= ~MHDR_PREPARED;
    }

    node->hdr.dwBufferLength = max<DWORD>(MME_MIDI_BUFFER_SPACE, node->hdr.dwBufferLength);
    LL::Move(node, &spareBufferList);

    if (++numSpareMidiBuffers > maxSpareMidiBuffers)
        maxSpareMidiBuffers = numSpareMidiBuffers;
    VLOG_F(LOG_DEBUG, "MME %s/midi_dispose_buffer recycling buffer %p", caller, node);
}

static void midi_gc_buffers(void)
{
    if (!midiStreamRunning)
        return;

    for (auto node = finishedBufferList.next, next = node->next; node != &finishedBufferList; node = next, next = node->next)
    {
        if (node->hdr.dwFlags & MHDR_DONE)
        {
            midi_dispose_buffer(node, "midi_gc_buffers");
        }
    }

    // prune excess spare buffers for when users hold down buttons on the music toggle in the menu
    for (auto node = spareBufferList.next, next = node->next; node != &spareBufferList; node = next, next = node->next)
    {
        if (numSpareMidiBuffers < numActiveMidiBuffers)
            break;

        LL::Remove(node);
        Xfree(node);
        numSpareMidiBuffers--;
        VLOG_F(LOG_DEBUG, "MME midi_gc_buffers pruning spare buffer %p", node);
    }
}

static void midi_free_buffers(void)
{
    //Bassert(activeBufferList.next == activeBufferList.prev);
    while (activeBufferList.next != activeBufferList.prev)
        WaitForSingleObject(midiBufferFinishedEvent, INFINITE);

    for (auto node = finishedBufferList.next, next = node->next; node != &finishedBufferList; node = next, next = node->next)
    {
        LL::Move(node, &spareBufferList);
        numSpareMidiBuffers++;
    }

    for (auto node = spareBufferList.next, next = node->next; node != &spareBufferList; node = next, next = node->next)
    {
        LL::Remove(node);
        Xfree(node);
        numSpareMidiBuffers--;
        VLOG_F(LOG_DEBUG, "MME midi_free_buffers pruning spare buffer %p", node);
    }

    Bassert(numSpareMidiBuffers == 0);

    numActiveMidiBuffers = 0;
    currentMidiBuffer = nullptr;
}

static void midi_flush_current_buffer(void)
{
    BOOL needsPrepare = FALSE;
    BOOL running = midiStreamRunning;

    if (!currentMidiBuffer)
    {
        //LOG_F(INFO, "no buffer");
        return;
    }
    auto evt = (MIDIEVENT *)&currentMidiBuffer->data[0];

    if (!running)
    {
        // immediate messages don't use a MIDIEVENT header so strip it off and
        // make some adjustments

        if (currentMidiBuffer->hdr.dwBytesRecorded)
        {
            currentMidiBuffer->hdr.dwBufferLength  = currentMidiBuffer->hdr.dwBytesRecorded - 12;
            currentMidiBuffer->hdr.dwBytesRecorded = 0;
        }

        if (currentMidiBuffer->hdr.dwBufferLength > 0)
            needsPrepare = TRUE;
    }
    else
        needsPrepare = TRUE;

    if (needsPrepare)
    {
        Bassert((currentMidiBuffer->hdr.dwFlags & MHDR_PREPARED) == 0);
        // playing a file, or sending a sysex when not playing means
        // we need to prepare the buffer
        currentMidiBuffer->hdr.dwBufferLength = currentMidiBuffer->hdr.dwBytesRecorded;
        auto rv = midiOutPrepareHeader((HMIDIOUT)midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
        {
            midi_error(rv, "midi_flush_current_buffer: error in midiOutPrepareHeader");
            return;
        }
    }

    if (running)
    {
        // midi file playing, so send events to the stream
        auto rv = midiStreamOut(midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "midi_flush_current_buffer: error in midiStreamOut");

        //LOG_F(INFO, "MME midi_flush_current_buffer queued buffer %p", currentMidiBuffer);
    }
    else
    {
        // midi file not playing, so send immediately
        if (currentMidiBuffer->hdr.dwBufferLength > 0)
        {
            currentMidiBuffer->hdr.dwUser = 0x1337;
            auto rv = midiOutLongMsg((HMIDIOUT)midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
            if (rv == MMSYSERR_NOERROR)
            {
                // busy-wait for Windows to be done with it
                WaitForSingleObject(midiBufferFinishedEvent, INFINITE);

                //LOG_F(INFO, "MME midi_flush_current_buffer sent immediate long");
            }
            else
                midi_error(rv, "midi_flush_current_buffer: error in midiOutLongMsg");

            midi_dispose_buffer(currentMidiBuffer, "midi_flush_current_buffer");
            currentMidiBuffer = 0;
        }
        else
        {
            auto rv = midiOutShortMsg((HMIDIOUT)midiStream, evt->dwEvent);
            if (rv != MMSYSERR_NOERROR)
                midi_error(rv, "midi_flush_current_buffer: error in midiOutShortMsg");
        }
        return;
    }

    LL::Insert((MidiBuffer *)&activeBufferList, currentMidiBuffer);

    if (++numActiveMidiBuffers > maxActiveMidiBuffers)
        maxActiveMidiBuffers = numActiveMidiBuffers;

    currentMidiBuffer = 0;
}

static void midi_setup_event(int length, unsigned char **data)
{
    int i = currentMidiBuffer->hdr.dwBytesRecorded / sizeof(DWORD);
    auto evt = (MIDIEVENT *)&currentMidiBuffer->data[i];

    evt->dwDeltaTime = midiThread ? (midiThreadTimer - midiLastEventTime) : 0;
    evt->dwStreamID  = 0;

    if (length <= 3)
    {
        evt->dwEvent = (DWORD)MEVT_SHORTMSG << 24;
        *data        = (unsigned char *)&evt->dwEvent;
    }
    else
    {
        evt->dwEvent = ((DWORD)MEVT_LONGMSG << 24) | (length & 0x00ffffff);
        *data        = (unsigned char *)&evt->dwParms[0];
    }
}

/* Gets space in the buffer presently being filled.
   If insufficient space can be found in the buffer,
   what is there is flushed to the stream and a new
   buffer large enough is allocated.

   Returns a pointer to starting writing at in 'data'.
 */
static BOOL midi_get_buffer(int length, unsigned char **data)
{
    BOOL const running = midiStreamRunning;
    uint32_t    datalen;

    // determine the space to alloc.
    // the size of a MIDIEVENT is 3*sizeof(DWORD) = 12.
    // short messages need only that amount of space.
    // long messages need additional space equal to the length of
    //    the message, padded to 4 bytes

    if (length <= 3)
        datalen = 12;
    else
        datalen = 12 + ((length + 3) & ~3);

    if (midiStreamRunning && currentMidiBuffer && currentMidiBuffer->hdr.dwBytesRecorded + datalen <= MME_MIDI_BUFFER_SPACE)
    {
        // there was enough space in the current buffer, so hand that back
        midi_setup_event(length, data);
        currentMidiBuffer->hdr.dwBytesRecorded += datalen;
        return TRUE;
    }

    if (currentMidiBuffer)
    {
        // not enough space in the current buffer to accommodate the
        // new data, so flush it to the stream
        midi_flush_current_buffer();
        currentMidiBuffer = 0;
    }

    // check if there's a spare buffer big enough to hold the message
    if (running)
    {
        for (auto node = spareBufferList.next; node != &spareBufferList; node = node->next)
        {
            if (node->hdr.dwBufferLength >= datalen)
            {
                // yes!
                LL::Remove(node);
                numSpareMidiBuffers--;
                node->hdr.dwBytesRecorded = 0;
                Bmemset(node->hdr.lpData, 0, node->hdr.dwBufferLength);

                currentMidiBuffer = node;

                VLOG_F(LOG_DEBUG, "MME midi_get_buffer fetched buffer %p", node);
                break;
            }
        }
    }

    if (!currentMidiBuffer)
    {
        // there were no spare buffers, or none were big enough, so allocate a new one
        int const size = max(MME_MIDI_BUFFER_SPACE, datalen);
        auto      node = (MidiBuffer *)Xcalloc(1, sizeof(MidiBuffer) + size);

        node->hdr.dwUser = (DWORD_PTR)node;
        node->hdr.lpData = (LPSTR)node->data;

        node->hdr.dwBufferLength  = size;
        node->hdr.dwBytesRecorded = 0;

        currentMidiBuffer = node;
        LL::Reset(node);
        VLOG_F(LOG_DEBUG, "MME midi_get_buffer allocated buffer %p", node);
    }

    midi_setup_event(length, data);

    currentMidiBuffer->hdr.dwBytesRecorded += datalen;

    return TRUE;
}

static inline void midi_sequence_event(void)
{
    if (!midiThread)
    {
        // a midi event being sent out of playback (streaming) mode
        midi_flush_current_buffer();
        return;
    }

    //LOG_F(INFO, "MME midi_sequence_event buffered");

    midiLastEventTime = midiThreadTimer;
}

static void MME_NoteOff(int channel, int key, int velocity)
{
    unsigned char *data;

    if (midi_get_buffer(3, &data))
    {
        data[0] = WINMM_NOTE_OFF | channel;
        data[1] = key;
        data[2] = velocity;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_NoteOff()");
}

static void MME_NoteOn(int channel, int key, int velocity)
{
    unsigned char *data;

    if (midi_get_buffer(3, &data))
    {
        data[0] = WINMM_NOTE_ON | channel;
        data[1] = key;
        data[2] = velocity;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_NoteOn()");
}

static void MME_PolyAftertouch(int channel, int key, int pressure)
{
    unsigned char *data;

    if (midi_get_buffer(3, &data))
    {
        data[0] = WINMM_POLY_AFTER_TCH | channel;
        data[1] = key;
        data[2] = pressure;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_PolyAftertouch()");
}

static void MME_ControlChange(int channel, int number, int value)
{
    unsigned char *data;

    if (midi_get_buffer(3, &data))
    {
        data[0] = WINMM_CONTROL_CHANGE | channel;
        data[1] = number;
        data[2] = value;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_ControlChange()");
}

static void MME_ProgramChange(int channel, int program)
{
    unsigned char *data;

    if (midi_get_buffer(2, &data))
    {
        data[0] = WINMM_PROGRAM_CHANGE | channel;
        data[1] = program;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_ProgramChange()");
}

static void MME_ChannelAftertouch(int channel, int pressure)
{
    unsigned char *data;

    if (midi_get_buffer(2, &data))
    {
        data[0] = WINMM_AFTER_TOUCH | channel;
        data[1] = pressure;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_ChannelAftertouch()");
}

static void MME_PitchBend(int channel, int lsb, int msb)
{
    unsigned char *data;

    if (midi_get_buffer(3, &data))
    {
        data[0] = WINMM_PITCH_BEND | channel;
        data[1] = lsb;
        data[2] = msb;
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_PitchBend()");
}

static void MME_SysEx(const unsigned char *data, int length)
{
    unsigned char *wdata;

    if (midi_get_buffer(length, &wdata))
    {
        Bmemcpy(wdata, data, length);
        midi_sequence_event();
    }
    else
        LOG_F(ERROR, "Error in MME_SysEx()");
}

void WinMMDrv_MIDI_PrintDevices(void)
{
    auto numDevices = (int)midiOutGetNumDevs();
    MIDIOUTCAPS midicaps;

    for (int i = -1; i < numDevices; i++)
    {
        if (!midiOutGetDevCaps(i, &midicaps, sizeof(MIDIOUTCAPS)))
            LOG_F(INFO, "%d: %s  ", i, midicaps.szPname);
    }
}

int WinMMDrv_MIDI_PrintBufferInfo(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    LOG_F(INFO, "MME MIDI buffers:");
    LOG_F(INFO, "%6s: %d (max %d)", "active", numActiveMidiBuffers, maxActiveMidiBuffers);
    LOG_F(INFO, "%6s: %d (max %d)", "spare", numSpareMidiBuffers, maxSpareMidiBuffers);
    return OSDCMD_OK;
}

int WinMMDrv_MIDI_GetNumDevices(void) { return midiOutGetNumDevs(); }

int WinMMDrv_MIDI_Init(midifuncs * funcs)
{
    if (midiInstalled)
        WinMMDrv_MIDI_Shutdown();

    LL::Reset(&finishedBufferList);
    LL::Reset(&activeBufferList);
    LL::Reset(&spareBufferList);

    Bmemset(funcs, 0, sizeof(midifuncs));

    if ((midiMutex = CreateMutex(0, FALSE, 0)) == 0)
    {
        ErrorCode = WinMMErr_MIDICreateMutex;
        return WinMMErr_Error;
    }

    if ((midiBufferFinishedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr)) == 0)
    {
        ErrorCode = WinMMErr_MIDICreateEvent;
        return WinMMErr_Error;
    }

    MIDIOUTCAPS midicaps;

    if (WinMM_DeviceID > midiOutGetNumDevs() || midiOutGetDevCaps(WinMM_DeviceID, &midicaps, sizeof(MIDIOUTCAPS)))
        WinMM_DeviceID = MIDI_MAPPER;

    if (!midiOutGetDevCaps(WinMM_DeviceID, &midicaps, sizeof(MIDIOUTCAPS)))
        LOG_F(INFO, ": [%d] %s", WinMM_DeviceID, midicaps.szPname);

    auto rv = midiStreamOpen(&midiStream, &WinMM_DeviceID, 1, (DWORD_PTR)midi_callback, (DWORD_PTR)0, CALLBACK_FUNCTION);

    if (rv != MMSYSERR_NOERROR)
    {
        WinMMDrv_MIDI_Shutdown();
        midi_error(rv, "WinMMDrv_MIDI_Init: error in midiStreamOpen");
        ErrorCode = WinMMErr_MIDIStreamOpen;
        return WinMMErr_Error;
    }

    funcs->NoteOff           = MME_NoteOff;
    funcs->NoteOn            = MME_NoteOn;
    funcs->PolyAftertouch    = MME_PolyAftertouch;
    funcs->ControlChange     = MME_ControlChange;
    funcs->ProgramChange     = MME_ProgramChange;
    funcs->ChannelAftertouch = MME_ChannelAftertouch;
    funcs->PitchBend         = MME_PitchBend;
    funcs->SysEx             = MME_SysEx;

    midiInstalled = TRUE;

    return WinMMErr_Ok;
}

void WinMMDrv_MIDI_Shutdown(void)
{
    WinMMDrv_MIDI_HaltPlayback();
    midi_free_buffers();

    if (midiStream)
    {
        // LOG_F(INFO, "stopping stream");
        auto rv = midiStreamClose(midiStream);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "WinMMDrv_MIDI_Shutdown: error in midiStreamClose");
        // LOG_F(INFO, "stream stopped");

        midiStream = 0;
    }

    midi_destroy_thread();

    if (midiMutex)
    {
        CloseHandle(midiMutex);
        midiMutex = 0;
    }

    if (midiBufferFinishedEvent)
    {
        CloseHandle(midiBufferFinishedEvent);
        midiBufferFinishedEvent = 0;
    }

    midiInstalled = FALSE;

    VLOG_F(LOG_DEBUG, "MME finished, max active buffers: %d max spare buffers: %d", maxActiveMidiBuffers, maxSpareMidiBuffers);
}

static DWORD midi_get_tick(void)
{
    if (!midiStreamRunning)
        return 0;

    MMTIME mmtime = { TIME_TICKS, 0 };

    auto rv = midiStreamPosition(midiStream, &mmtime, sizeof(MMTIME));
    if (rv != MMSYSERR_NOERROR)
    {
        midi_error(rv, "midi_get_tick: error in midiStreamPosition");
        return 0;
    }

    return mmtime.u.ticks;
}

static void CALLBACK midi_callback(HMIDIOUT out, UINT msg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    UNREFERENCED_PARAMETER(out);
    UNREFERENCED_PARAMETER(dwInstance);
    UNREFERENCED_PARAMETER(dwParam2);

    if (msg != MOM_DONE)
        return;

    WinMMDrv_MIDI_Lock();
    for (auto node = activeBufferList.next; node != &activeBufferList; node = node->next)
    {
        if (&node->hdr == (MIDIHDR *)dwParam1)
        {
            Bassert(node->hdr.dwUser != 0x1337);
            numActiveMidiBuffers--;
            LL::Move(node, &finishedBufferList);
            SetEvent(midiBufferFinishedEvent);
            WinMMDrv_MIDI_Unlock();
            return;
        }
    }
    debug_break();
    WinMMDrv_MIDI_Unlock();
}

static unsigned WINAPI midiDataThread(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);

    debugThreadName("midiDataThread");

    DWORD sleepAmount = 100 / MME_THREAD_QUEUE_INTERVAL;

    do
    {
        HANDLE const events[3] = { midiThreadQuitEvent, midiBufferFinishedEvent, midiThreadResetEvent };
        auto result = WaitForMultipleObjects(3, events, false, sleepAmount);

        if (result != WAIT_OBJECT_0 && midiStreamRunning == false)
            continue;

        switch (result)
        {
        case WAIT_OBJECT_0+2: // midiThreadResetEvent
            midiThreadTimer      = midi_get_tick();
            midiLastEventTime    = midiThreadTimer;
            midiThreadQueueTimer = midiThreadTimer + midiThreadQueueTicks;
            break;
        case WAIT_OBJECT_0+1: // midiBufferFinishedEvent
            sleepAmount = 0;
            WinMMDrv_MIDI_Lock();
            midi_gc_buffers();
            WinMMDrv_MIDI_Unlock();
            break;
        case WAIT_TIMEOUT:
        {
            // queue a tick
            WinMMDrv_MIDI_Lock();
            DWORD sequenceTime = midi_get_tick();
            sleepAmount = 100 / MME_THREAD_QUEUE_INTERVAL;

            if (((int64_t)midiThreadTimer - (int64_t)sequenceTime) > midiThreadQueueTicks)
            {
                // we're running ahead, so sleep for half the usual
                // amount and try again
                sleepAmount /= 2;
                WinMMDrv_MIDI_Unlock();
                continue;
            }

            midiThreadQueueTimer = sequenceTime + midiThreadQueueTicks;
            while (midiThreadTimer < midiThreadQueueTimer)
            {
                WinMMDrv_MIDI_Service();
                midiThreadTimer++;
            }
            midi_flush_current_buffer();
            WinMMDrv_MIDI_Unlock();
            break;
        }
        case WAIT_OBJECT_0: // midiThreadQuitEvent
            return 0;
        }
    } while (1);

    return 0;
}

int midi_create_thread(void)
{
    midiThreadQuitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!midiThreadQuitEvent)
    {
        ErrorCode = WinMMErr_MIDICreateEvent;
        return WinMMErr_Error;
    }

    midiThreadResetEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!midiThreadResetEvent)
    {
        ErrorCode = WinMMErr_MIDICreateEvent;
        return WinMMErr_Error;
    }

    midiThread = (HANDLE)_beginthreadex(nullptr, 0, midiDataThread, 0, 0, 0);
    if (!midiThread)
    {
        ErrorCode = WinMMErr_MIDIPlayThread;
        return WinMMErr_Error;
    }

    return WinMMErr_Ok;
}

void midi_destroy_thread(void)
{
    if (midiThread)
    {
        SetEvent(midiThreadQuitEvent);
        WaitForSingleObject(midiThread, INFINITE);
        // LOG_F(INFO,"MME MIDI_HaltPlayback synched");
        CloseHandle(midiThread);
        midiThread = 0;
        CloseHandle(midiThreadQuitEvent);
        midiThreadQuitEvent = 0;
        CloseHandle(midiThreadResetEvent);
        midiThreadResetEvent = 0;
    }
}

int WinMMDrv_MIDI_StartPlayback(void)
{
    WinMMDrv_MIDI_HaltPlayback();

    auto rv = midiStreamRestart(midiStream);
    if (rv != MMSYSERR_NOERROR)
    {
        midi_error(rv, "WinMMDrv_MIDI_StartPlayback: error in midiStreamRestart");
        WinMMDrv_MIDI_HaltPlayback();
        ErrorCode = WinMMErr_MIDIStreamRestart;
        return WinMMErr_Error;
    }

    if (!midiThread && midi_create_thread() != WinMMErr_Ok)
        return WinMMErr_Error;

    midiLastDivision = 0;

    if (midiThread)
        SetEvent(midiThreadResetEvent);

    midiStreamRunning = TRUE;

    return WinMMErr_Ok;
}


void WinMMDrv_MIDI_HaltPlayback(void)
{
    midiStreamRunning = FALSE;
}

void WinMMDrv_MIDI_SetTempo(int tempo, int division)
{
    //LOG_F(INFO, "MIDI_SetTempo %d/%d", tempo, division);
    MIDIPROPTEMPO   propTempo   = { sizeof(MIDIPROPTEMPO), (DWORD)(60000000l / tempo) };
    MIDIPROPTIMEDIV propTimediv = { sizeof(MIDIPROPTIMEDIV), (DWORD)division };

    if (midiLastDivision != division)
    {
        auto rv = midiStreamStop(midiStream);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "WinMMDrv_MIDI_SetTempo: error in midiStreamStop");

        rv = midiStreamProperty(midiStream, (LPBYTE)&propTimediv, MIDIPROP_SET | MIDIPROP_TIMEDIV);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "WinMMDrv_MIDI_SetTempo: error in midiStreamProperty (MIDIPROP_TIMEDIV)");
    }

    auto rv = midiStreamProperty(midiStream, (LPBYTE)&propTempo, MIDIPROP_SET | MIDIPROP_TEMPO);
    if (rv != MMSYSERR_NOERROR)
        midi_error(rv, "WinMMDrv_MIDI_SetTempo: error in midiStreamProperty (MIDIPROP_TEMPO)");

    if (midiLastDivision != division)
    {
        if (midiStreamRunning)
            midiStreamRestart(midiStream);

        midiLastDivision = division;
    }

    midiThreadQueueTicks = (int)ceil((((double)tempo * (double)division) / 60.0) / (double)MME_THREAD_QUEUE_INTERVAL);
    if (midiThreadQueueTicks <= 0)
        midiThreadQueueTicks = 1;
}

void WinMMDrv_MIDI_Lock(void)
{
    DWORD err = WaitForSingleObject(midiMutex, INFINITE);
    if (err != WAIT_OBJECT_0)
        LOG_F(ERROR, "Error in WinMMDrv_MIDI_Lock(): WaitForSingleObject() returned %d", (int) err);
}

void WinMMDrv_MIDI_Unlock(void)  { ReleaseMutex(midiMutex); }
void WinMMDrv_MIDI_Service(void) { MIDI_ServiceRoutine(); }
