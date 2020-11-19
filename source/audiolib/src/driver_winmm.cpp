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

#include "compat.h"
#include "ll.h"
#include "midi.h"
#include "midifuncs.h"
#include "multivoc.h"

#include <mmsystem.h>

#ifdef _MSC_VER
#define inline _inline
#endif

UINT WinMM_DeviceID = MIDI_MAPPER;

static int ErrorCode = WinMMErr_Ok;

static BOOL      midiInstalled;
static HMIDISTRM midiStream;
static void    (*midiThreadService)(void);
static uint32_t midiThreadTimer;
static uint32_t midiLastEventTime;
static uint32_t midiThreadQueueTimer;
static uint32_t midiThreadQueueTicks;
static HANDLE   midiThread;
static HANDLE   midiThreadQuitEvent;
static HANDLE   midiMutex;
static BOOL     midiStreamRunning;
static int      midiLastDivision;

#define MME_THREAD_QUEUE_INTERVAL 10       // 1/10 sec
#define MME_MIDI_BUFFER_SPACE (12 * 128u)  // 128 note-on events

typedef struct MidiBuffer
{
    struct MidiBuffer *next;
    struct MidiBuffer *prev;

    BOOL prepared;
    MIDIHDR hdr;
} MidiBuffer;

static MidiBuffer activeMidiBuffers;
static MidiBuffer spareMidiBuffers;
static MidiBuffer *currentMidiBuffer;

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
static void midi_error(MMRESULT rv, const char * fmt, ...)
{
    va_list va;
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
    
    va_start(va, fmt);
    MV_Printf(fmt, va);
    va_end(va);
    
    MV_Printf(" err %d (%s)\n", (int)rv, errtxt);
}

static void midi_dispose_buffer(MidiBuffer *node, const char *caller)
{
    if (node->prepared)
    {
        auto rv = midiOutUnprepareHeader((HMIDIOUT)midiStream, &node->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "MME %s/midi_dispose_buffer midiOutUnprepareHeader", caller);
        node->prepared = FALSE;
    }

    if (midiThread)
    {
        // remove the node from the activeMidiBuffers list
        LL_Remove(node, next, prev);

        // when playing, we keep the buffers
        LL_Add((MidiBuffer *)&spareMidiBuffers, node, next, prev);
        //MV_Printf("MME %s/midi_dispose_buffer recycling buffer %p\n", caller, node);
    }
    else
    {
        // when not, we throw them away
        Xfree(node);
        //MV_Printf("MME %s/midi_dispose_buffer freeing buffer %p\n", caller, node);
    }
}

static void midi_gc_buffers(void)
{
    for (auto node = activeMidiBuffers.next, next = node->next; node != &activeMidiBuffers; node = next, next = node->next)
    {
        if (node->hdr.dwFlags & MHDR_DONE)
            midi_dispose_buffer(node, "midi_gc_buffers");
    }
}

static void midi_free_buffers(void)
{
    //MV_Printf("waiting for active buffers to return\n");
    while (!LL_ListEmpty(&activeMidiBuffers, next, prev))
    {
        // wait for Windows to finish with all the buffers queued
        midi_gc_buffers();
        //MV_Printf("waiting...\n");
        Sleep(10);
    }
    //MV_Printf("waiting over\n");

    for (auto node = spareMidiBuffers.next, next = node->next; node != &spareMidiBuffers; node = next, next = node->next)
    {
        LL_Remove(node, next, prev);
        Xfree(node);
        //MV_Printf("MME midi_free_buffers freeing buffer %p\n", node);
    }

    Bassert(currentMidiBuffer == 0);
}

static void midi_flush_current_buffer(void)
{
    BOOL needsPrepare = FALSE;

    if (!currentMidiBuffer)
        return;

    auto evt = (MIDIEVENT *)currentMidiBuffer->hdr.lpData;

    if (!midiThread)
    {
        // immediate messages don't use a MIDIEVENT header so strip it off and
        // make some adjustments

        currentMidiBuffer->hdr.dwBufferLength  = currentMidiBuffer->hdr.dwBytesRecorded - 12;
        currentMidiBuffer->hdr.dwBytesRecorded = 0;
        currentMidiBuffer->hdr.lpData          = (LPSTR)&evt->dwParms[0];

        if (currentMidiBuffer->hdr.dwBufferLength > 0)
            needsPrepare = TRUE;
    }
    else
        needsPrepare = TRUE;

    if (needsPrepare)
    {
        // playing a file, or sending a sysex when not playing means
        // we need to prepare the buffer
        auto rv = midiOutPrepareHeader((HMIDIOUT)midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
        {
            midi_error(rv, "MME midi_flush_current_buffer midiOutPrepareHeader");
            return;
        }

        currentMidiBuffer->prepared = TRUE;
    }

    if (midiThread)
    {
        // midi file playing, so send events to the stream

        LL_Add((MidiBuffer *)&activeMidiBuffers, currentMidiBuffer, next, prev);

        auto rv = midiStreamOut(midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
        if (rv != MMSYSERR_NOERROR)
        {
            midi_error(rv, "MME midi_flush_current_buffer midiStreamOut");
            midi_dispose_buffer(currentMidiBuffer, "midi_flush_current_buffer");
            return;
        }

        //MV_Printf("MME midi_flush_current_buffer queued buffer %p\n", currentMidiBuffer);
    }
    else
    {
        // midi file not playing, so send immediately

        if (currentMidiBuffer->hdr.dwBufferLength > 0)
        {
            auto rv = midiOutLongMsg((HMIDIOUT)midiStream, &currentMidiBuffer->hdr, sizeof(MIDIHDR));
            if (rv == MMSYSERR_NOERROR)
            {
                // busy-wait for Windows to be done with it
                while (!(currentMidiBuffer->hdr.dwFlags & MHDR_DONE));

                //MV_Printf("MME midi_flush_current_buffer sent immediate long\n");
            }
            else
                midi_error(rv, "MME midi_flush_current_buffer midiOutLongMsg");
        }
        else
        {
            auto rv = midiOutShortMsg((HMIDIOUT)midiStream, evt->dwEvent);
            if (rv != MMSYSERR_NOERROR)
                midi_error(rv, "MME midi_flush_current_buffer midiOutShortMsg");
        }

        midi_dispose_buffer(currentMidiBuffer, "midi_flush_current_buffer");
    }

    currentMidiBuffer = 0;
}

static void midi_setup_event(int length, unsigned char **data)
{
    auto evt = (MIDIEVENT *)((intptr_t)currentMidiBuffer->hdr.lpData + currentMidiBuffer->hdr.dwBytesRecorded);

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
    uint32_t    datalen;

    // determine the space to alloc.
    // the size of a MIDIEVENT is 3*sizeof(DWORD) = 12.
    // short messages need only that amount of space.
    // long messages need additional space equal to the length of
    //    the message, padded to 4 bytes

    if (length <= 3)
        datalen = 12;
    else
    {
        datalen = 12 + length;
        if ((datalen & 3) > 0)
            datalen += 4 - (datalen & 3);
    }

    if (!midiThread)
        Bassert(currentMidiBuffer == 0);

    if (currentMidiBuffer && (currentMidiBuffer->hdr.dwBufferLength - currentMidiBuffer->hdr.dwBytesRecorded) >= datalen)
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
    if (midiThread)
    {
        for (auto node = spareMidiBuffers.next; node != &spareMidiBuffers; node = node->next)
        {
            if (node->hdr.dwBufferLength >= datalen)
            {
                // yes!
                LL_Remove(node, next, prev);

                node->hdr.dwBytesRecorded = 0;
                Bmemset(node->hdr.lpData, 0, node->hdr.dwBufferLength);

                currentMidiBuffer = node;

                //MV_Printf("MME midi_get_buffer fetched buffer %p\n", node);
                break;
            }
        }
    }

    if (!currentMidiBuffer)
    {
        // there were no spare buffers, or none were big enough, so allocate a new one
        int const size = midiThread ? max(MME_MIDI_BUFFER_SPACE, datalen) : datalen;
        auto      node = (MidiBuffer *)Xmalloc(sizeof(MidiBuffer) + size);

        Bmemset(node, 0, sizeof(MidiBuffer) + datalen);

        node->hdr.dwUser = (DWORD_PTR)node;
        node->hdr.lpData = (LPSTR)((intptr_t)node + sizeof(MidiBuffer));

        node->hdr.dwBufferLength  = size;
        node->hdr.dwBytesRecorded = 0;

        currentMidiBuffer = node;

        //MV_Printf("MME midi_get_buffer allocated buffer %p\n", node);
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

    //MV_Printf("MME midi_sequence_event buffered\n");

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
        MV_Printf("MME_NoteOff error\n");
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
        MV_Printf("MME_NoteOn error\n");
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
        MV_Printf("MME_PolyAftertouch error\n");
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
        MV_Printf("MME_ControlChange error\n");
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
        MV_Printf("MME_ProgramChange error\n");
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
        MV_Printf("MME_ChannelAftertouch error\n");
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
        MV_Printf("MME_PitchBend error\n");
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
        MV_Printf("MME_SysEx error\n");
}

void WinMMDrv_MIDI_PrintDevices(void)
{
    auto numDevices = (int)midiOutGetNumDevs();
    MIDIOUTCAPS midicaps;

    for (int i = -1; i < numDevices; i++)
    {
        if (!midiOutGetDevCaps(i, &midicaps, sizeof(MIDIOUTCAPS)))
            MV_Printf("%d: %s  ", i, midicaps.szPname);
    }

    MV_Printf("\n");
}

int WinMMDrv_MIDI_GetNumDevices(void) { return midiOutGetNumDevs(); }

int WinMMDrv_MIDI_Init(midifuncs * funcs)
{
    if (midiInstalled)
        WinMMDrv_MIDI_Shutdown();

    Bmemset(funcs, 0, sizeof(midifuncs));

    LL_Reset((MidiBuffer *)&activeMidiBuffers, next, prev);
    LL_Reset((MidiBuffer *)&spareMidiBuffers, next, prev);

    if ((midiMutex = CreateMutex(0, FALSE, 0)) == 0)
    {
        ErrorCode = WinMMErr_MIDICreateMutex;
        return WinMMErr_Error;
    }

    MIDIOUTCAPS midicaps;

    if (WinMM_DeviceID > midiOutGetNumDevs() || midiOutGetDevCaps(WinMM_DeviceID, &midicaps, sizeof(MIDIOUTCAPS)))
        WinMM_DeviceID = MIDI_MAPPER;

    if (!midiOutGetDevCaps(WinMM_DeviceID, &midicaps, sizeof(MIDIOUTCAPS)))
        MV_Printf(": [%d] %s", WinMM_DeviceID, midicaps.szPname);

    auto rv = midiStreamOpen(&midiStream, &WinMM_DeviceID, 1, (DWORD_PTR)0, (DWORD_PTR)0, CALLBACK_NULL);

    if (rv != MMSYSERR_NOERROR)
    {
        CloseHandle(midiMutex);
        midiMutex = 0;

        midi_error(rv, "MME MIDI_Init midiStreamOpen");
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
    if (!midiInstalled)
        return;

    WinMMDrv_MIDI_HaltPlayback();

    if (midiStream)
    {
        auto rv = midiStreamClose(midiStream);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "MME MIDI_Shutdown midiStreamClose");
    }

    if (midiMutex)
        CloseHandle(midiMutex);

    midiStream = 0;
    midiMutex  = 0;

    midiInstalled = FALSE;
}

static DWORD midi_get_tick(void)
{
    MMTIME mmtime = { TIME_TICKS, 0 };

    auto rv = midiStreamPosition(midiStream, &mmtime, sizeof(MMTIME));
    if (rv != MMSYSERR_NOERROR)
    {
        midi_error(rv, "MME midi_get_tick midiStreamPosition");
        return 0;
    }

    return mmtime.u.ticks;
}

static DWORD WINAPI midiDataThread(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);

    midiThreadTimer      = midi_get_tick();
    midiLastEventTime    = midiThreadTimer;
    midiThreadQueueTimer = midiThreadTimer + midiThreadQueueTicks;

    WinMMDrv_MIDI_Lock();
    midi_gc_buffers();
    while (midiThreadTimer < midiThreadQueueTimer)
    {
        midiThreadService();
        midiThreadTimer++;
    }
    midi_flush_current_buffer();
    WinMMDrv_MIDI_Unlock();

    DWORD sleepAmount = 100 / MME_THREAD_QUEUE_INTERVAL;

    do
    {
        auto waitret = WaitForSingleObject(midiThreadQuitEvent, sleepAmount);

        if (waitret == WAIT_OBJECT_0)
            break;
        else if (waitret == WAIT_TIMEOUT)
        {
            // queue a tick
            auto sequenceTime = midi_get_tick();

            sleepAmount = 100 / MME_THREAD_QUEUE_INTERVAL;
            if (((int64_t)midiThreadTimer - (int64_t)sequenceTime) > midiThreadQueueTicks)
            {
                // we're running ahead, so sleep for half the usual
                // amount and try again
                sleepAmount /= 2;
                continue;
            }

            midiThreadQueueTimer = sequenceTime + midiThreadQueueTicks;

            WinMMDrv_MIDI_Lock();
            midi_gc_buffers();
            while (midiThreadTimer < midiThreadQueueTimer)
            {
                midiThreadService();
                midiThreadTimer++;
            }
            midi_flush_current_buffer();
            WinMMDrv_MIDI_Unlock();
        }
        else
            MV_Printf("MME midiDataThread: wfmo err %d\n", (int)waitret);
    } while (1);

    return 0;
}

int WinMMDrv_MIDI_StartPlayback(void)
{
    WinMMDrv_MIDI_HaltPlayback();
    midiThreadService = WinMMDrv_MIDI_Service;

    midiThreadQuitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!midiThreadQuitEvent)
    {
        ErrorCode = WinMMErr_MIDICreateEvent;
        return WinMMErr_Error;
    }

    if (!midiStreamRunning)
    {
        auto rv = midiStreamRestart(midiStream);
        if (rv != MMSYSERR_NOERROR)
        {
            midi_error(rv, "MIDI_StartPlayback midiStreamRestart");
            WinMMDrv_MIDI_HaltPlayback();
            ErrorCode = WinMMErr_MIDIStreamRestart;
            return WinMMErr_Error;
        }

        midiStreamRunning = TRUE;
    }

    midiThread = CreateThread(nullptr, 0, midiDataThread, 0, 0, 0);
    if (!midiThread)
    {
        WinMMDrv_MIDI_HaltPlayback();
        ErrorCode = WinMMErr_MIDIPlayThread;
        return WinMMErr_Error;
    }

    midiLastDivision = 0;

    return WinMMErr_Ok;
}

void WinMMDrv_MIDI_HaltPlayback(void)
{
    if (midiThread)
    {
        SetEvent(midiThreadQuitEvent);

        WaitForSingleObject(midiThread, INFINITE);
        // MV_Printf("MME MIDI_HaltPlayback synched\n");

        CloseHandle(midiThread);
    }

    if (midiThreadQuitEvent)
        CloseHandle(midiThreadQuitEvent);

    if (midiStreamRunning)
    {
        // MV_Printf("stopping stream\n");
        auto rv = midiStreamStop(midiStream);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "MME MIDI_HaltPlayback midiStreamStop");
        // MV_Printf("stream stopped\n");

        midiStreamRunning = FALSE;
    }

    midi_free_buffers();

    midiThread          = 0;
    midiThreadQuitEvent = 0;
}

void WinMMDrv_MIDI_SetTempo(int tempo, int division)
{
    BOOL const running = midiStreamRunning;

    //MV_Printf("MIDI_SetTempo %d/%d\n", tempo, division);
    MIDIPROPTEMPO   propTempo   = { sizeof(MIDIPROPTEMPO), (DWORD)(60000000l / tempo) };
    MIDIPROPTIMEDIV propTimediv = { sizeof(MIDIPROPTIMEDIV), (DWORD)division };

    if (midiLastDivision != division)
    {
        // changing the division means halting the stream
        WinMMDrv_MIDI_HaltPlayback();

        auto rv = midiStreamProperty(midiStream, (LPBYTE)&propTimediv, MIDIPROP_SET | MIDIPROP_TIMEDIV);
        if (rv != MMSYSERR_NOERROR)
            midi_error(rv, "MME MIDI_SetTempo midiStreamProperty timediv");
    }

    auto rv = midiStreamProperty(midiStream, (LPBYTE)&propTempo, MIDIPROP_SET | MIDIPROP_TEMPO);
    if (rv != MMSYSERR_NOERROR)
        midi_error(rv, "MME MIDI_SetTempo midiStreamProperty tempo");

    if (midiLastDivision != division)
    {
        if (running && WinMMDrv_MIDI_StartPlayback() != WinMMErr_Ok)
            return;

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
        MV_Printf("MME midiMutex lock: wfso %d\n", (int) err);
}

void WinMMDrv_MIDI_Unlock(void)  { ReleaseMutex(midiMutex); }
void WinMMDrv_MIDI_Service(void) { MIDI_ServiceRoutine(); }
