/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 
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
 * ALSA MIDI output
 */

#include "midifuncs.h"
#include "driver_alsa.h"
#include <pthread.h>
#include <alsa/asoundlib.h>
#include "midi.h"
#include <unistd.h>
#include <math.h>

enum {
   ALSAErr_Warning = -2,
   ALSAErr_Error   = -1,
   ALSAErr_Ok      = 0,
   ALSAErr_Uninitialised,
   ALSAErr_SeqOpen,
   ALSAErr_CreateSimplePort,
   ALSAErr_AllocQueue,
   ALSAErr_ConnectTo,
   ALSAErr_StartQueue,
   ALSAErr_StopQueue,
   ALSAErr_PlayThread
};

int ALSA_ClientID = 0;
int ALSA_PortID = 0;

static int ErrorCode = ALSAErr_Ok;

static snd_seq_t * seq = 0;
static int seq_port = -1;
static int seq_queue = -1;
static int queueRunning = 0;

static pthread_t thread;
static int threadRunning = 0;
static volatile int threadQuit = 0;
static void (* threadService)(void) = 0;

static unsigned int threadTimer = 0;
static unsigned int threadQueueTimer = 0;
static int threadQueueTicks = 0;
#define THREAD_QUEUE_INTERVAL 20    // 1/20 sec


int ALSADrv_GetError(void)
{
	return ErrorCode;
}

const char *ALSADrv_ErrorString( int ErrorNumber )
{
	const char *ErrorString;
	
    switch( ErrorNumber )
	{
        case ALSAErr_Warning :
        case ALSAErr_Error :
            ErrorString = ALSADrv_ErrorString( ErrorCode );
            break;

        case ALSAErr_Ok :
            ErrorString = "ALSA ok.";
            break;
			
		case ALSAErr_Uninitialised:
			ErrorString = "ALSA uninitialised.";
			break;

        case ALSAErr_SeqOpen:
            ErrorString = "ALSA error: failed in snd_seq_open.\n";
            break;

        case ALSAErr_CreateSimplePort:
            ErrorString = "ALSA error: failed in snd_seq_create_simple_port.\n";
            break;

        case ALSAErr_AllocQueue:
            ErrorString = "ALSA error: failed in snd_seq_alloc_queue.\n";
            break;

        case ALSAErr_ConnectTo:
            ErrorString = "ALSA error: failed in snd_seq_connect_to.\n";
            break;

        default:
            ErrorString = "Unknown ALSA error.";
            break;
    }
        
	return ErrorString;
}

static inline void sequence_event(snd_seq_event_t * ev, bool sync_output_queue)
{
    int result = 0;
    
    snd_seq_ev_set_subs(ev);
    snd_seq_ev_set_source(ev, seq_port);
    snd_seq_ev_schedule_tick(ev, seq_queue, 0, threadTimer);
    result = snd_seq_event_output(seq, ev);
    if (result < 0) {
        fprintf(stderr, "ALSA could not queue event: err %d\n", result);
    }
    else {
        while ((result = snd_seq_drain_output(seq)) > 0) ;
        if (result < 0) {
            fprintf(stderr, "ALSA could not drain output: err %d\n", result);
        }

        if (sync_output_queue)
            snd_seq_sync_output_queue(seq);
    }
}

static void Func_NoteOff( int channel, int key, int velocity )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_noteoff(&ev, channel, key, velocity);
    sequence_event(&ev, true);
}

static void Func_NoteOn( int channel, int key, int velocity )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_noteon(&ev, channel, key, velocity);
    sequence_event(&ev, true);
}

static void Func_PolyAftertouch( int channel, int key, int pressure )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_keypress(&ev, channel, key, pressure);
    sequence_event(&ev, true);
}

static void Func_ControlChange( int channel, int number, int value )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_controller(&ev, channel, number, value);
    sequence_event(&ev, false);
}

static void Func_ProgramChange( int channel, int program )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_pgmchange(&ev, channel, program);
    sequence_event(&ev, true);
}

static void Func_ChannelAftertouch( int channel, int pressure )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_chanpress(&ev, channel, pressure);
    sequence_event(&ev, true);
}

static void Func_PitchBend( int channel, int lsb, int msb )
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_pitchbend(&ev, channel, (lsb | (msb << 7)) - 8192);
    sequence_event(&ev, true);
}

static unsigned int get_tick(void)
{
    snd_seq_queue_status_t * status;
    int result;

    snd_seq_queue_status_alloca(&status);
    
    result = snd_seq_get_queue_status(seq, seq_queue, status);
    if (result < 0) {
        fprintf(stderr, "ALSA snd_seq_get_queue_status err %d\n", result);
        return 0;
    }

    return (unsigned int) snd_seq_queue_status_get_tick_time(status);
}

static void * threadProc(void *)
{
    struct timeval tv;
    int sleepAmount = 1000000 / THREAD_QUEUE_INTERVAL;
    unsigned int sequenceTime;

    // prime the pump
    threadTimer = get_tick();
    threadQueueTimer = threadTimer + threadQueueTicks;
    while (threadTimer < threadQueueTimer) {
        if (threadService) {
            threadService();
        }
        threadTimer++;
    }
    
    while (!threadQuit) {
        tv.tv_sec = 0;
        tv.tv_usec = sleepAmount;

        select(0, NULL, NULL, NULL, &tv);

        sequenceTime = get_tick();

        sleepAmount = 1000000 / THREAD_QUEUE_INTERVAL;
        if ((int)(threadTimer - sequenceTime) > threadQueueTicks) {
            // we're running ahead, so sleep for half the usual
            // amount and try again
            sleepAmount /= 2;
            continue;
        }

        threadQueueTimer = sequenceTime + threadQueueTicks;
        while (threadTimer < threadQueueTimer) {
            if (threadService) {
                threadService();
            }
            threadTimer++;
        }
    }

    return NULL;
}

int ALSADrv_MIDI_Init(midifuncs * funcs)
{
    int result;
    
    ALSADrv_MIDI_Shutdown();
    memset(funcs, 0, sizeof(midifuncs));

    result = snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if (result < 0) {
        fprintf(stderr, "ALSA snd_seq_open err %d\n", result);
        ErrorCode = ALSAErr_SeqOpen;
        return ALSAErr_Error;
    }
    
    seq_port = snd_seq_create_simple_port(seq, "output",
                  SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE,
                  SND_SEQ_PORT_TYPE_APPLICATION);
    if (seq_port < 0) {
        ALSADrv_MIDI_Shutdown();
        fprintf(stderr, "ALSA snd_seq_create_simple_port err %d\n", seq_port);
        ErrorCode = ALSAErr_CreateSimplePort;
        return ALSAErr_Error;
    }
    
    snd_seq_set_client_name(seq, "EDuke32");

    seq_queue = snd_seq_alloc_queue(seq);
    if (seq_queue < 0) {
        ALSADrv_MIDI_Shutdown();
        fprintf(stderr, "ALSA snd_seq_alloc_queue err %d\n", seq_queue);
        ErrorCode = ALSAErr_AllocQueue;
        return ALSAErr_Error;
    }
    
    result = snd_seq_connect_to(seq, seq_port, ALSA_ClientID, ALSA_PortID);
    if (result < 0) {
        ALSADrv_MIDI_Shutdown();
        fprintf(stderr, "ALSA snd_seq_connect_to err %d\n", result);
        ErrorCode = ALSAErr_ConnectTo;
        return ALSAErr_Error;
    }
    
    funcs->NoteOff = Func_NoteOff;
    funcs->NoteOn  = Func_NoteOn;
    funcs->PolyAftertouch = Func_PolyAftertouch;
    funcs->ControlChange = Func_ControlChange;
    funcs->ProgramChange = Func_ProgramChange;
    funcs->ChannelAftertouch = Func_ChannelAftertouch;
    funcs->PitchBend = Func_PitchBend;
    
    return ALSAErr_Ok;
}

void ALSADrv_MIDI_Shutdown(void)
{
    ALSADrv_MIDI_HaltPlayback();
    
    if (seq_queue >= 0) {
        snd_seq_free_queue(seq, seq_queue);
    }
    if (seq_port >= 0) {
        snd_seq_delete_simple_port(seq, seq_port);
    }
    if (seq) {
        snd_seq_close(seq);
    }
    seq_queue = -1;
    seq_port = -1;
    seq = 0;
}

int ALSADrv_MIDI_StartPlayback(void)
{
    ALSADrv_MIDI_HaltPlayback();

    threadService = ALSADrv_MIDI_Service;
    threadQuit = 0;

    ALSADrv_MIDI_QueueStart();

    if (pthread_create(&thread, NULL, threadProc, NULL)) {
        fprintf(stderr, "ALSA pthread_create returned error\n");

        ALSADrv_MIDI_HaltPlayback();
        
        return ALSAErr_PlayThread;
    }

    threadRunning = 1;

    return 0;
}

void ALSADrv_MIDI_HaltPlayback(void)
{
    void * ret;
    
    if (!threadRunning) {
        return;
    }

    threadQuit = 1;

    if (pthread_join(thread, &ret)) {
        fprintf(stderr, "ALSA pthread_join returned error\n");
    }

    ALSADrv_MIDI_QueueStop();

    threadRunning = 0;
}

void ALSADrv_MIDI_SetTempo(int tempo, int division)
{
    double tps;
    snd_seq_queue_tempo_t * t;

    ALSADrv_MIDI_QueueStop();

    snd_seq_queue_tempo_alloca(&t);
    snd_seq_queue_tempo_set_tempo(t, 60000000 / tempo);
    snd_seq_queue_tempo_set_ppq(t, division);
    snd_seq_set_queue_tempo(seq, seq_queue, t);

    tps = ( (double) tempo * (double) division ) / 60.0;
    threadQueueTicks = (int) ceil(tps / (double) THREAD_QUEUE_INTERVAL);

    ALSADrv_MIDI_QueueStart();
}

void ALSADrv_MIDI_Lock(void)
{
}

void ALSADrv_MIDI_Unlock(void)
{
}

void ALSADrv_MIDI_Service(void) { MIDI_ServiceRoutine(); }

void ALSADrv_MIDI_QueueStart(void)
{
    int result;

    if (!queueRunning) {
        result = snd_seq_start_queue(seq, seq_queue, NULL);
        if (result < 0) {
            fprintf(stderr, "ALSA snd_seq_start_queue err %d\n", result);
        }

        while ((result = snd_seq_drain_output(seq)) > 0);
        if (result < 0) {
            fprintf(stderr, "ALSA could not drain output: err %d\n", result);
        }

        snd_seq_sync_output_queue(seq);

        queueRunning = 1;
    }
}

void ALSADrv_MIDI_QueueStop(void)
{
    int result;

    if (queueRunning) {
        result = snd_seq_stop_queue(seq, seq_queue, NULL);
        if (result < 0) {
            fprintf(stderr, "ALSA snd_seq_stop_queue err %d\n", result);
        }

        while ((result = snd_seq_drop_output(seq)) > 0);
        if (result < 0) {
            fprintf(stderr, "ALSA could not drop output: err %d\n", result);
        }

        snd_seq_sync_output_queue(seq);

        queueRunning = 0;
    }
}

std::vector<alsa_mididevinfo_t> ALSADrv_MIDI_ListPorts(void)
{
    int result;
    bool requiredInit;
    bool devicesFound = false;
    std::vector<alsa_mididevinfo_t> validDevices;
    std::vector<alsa_mididevinfo_t> nullDevice = {
        {
            "No Devices Found",
            0,
            0,
        }
    };

    if (seq == 0) {
        requiredInit = true;

        result = snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0);
        if (result < 0) {
            fprintf(stderr, "ALSA snd_seq_open err %d\n", result);
            return nullDevice;
        }
    }
    else {
        requiredInit = false;
    }

    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);

    while (snd_seq_query_next_client(seq, cinfo) >= 0) {
        // Ignore 'ALSA oddities' that we don't want to use.
        int client = snd_seq_client_info_get_client(cinfo);
        if(client < 16) {
            continue;
        }

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seq, pinfo) >= 0) {
            /* port must understand MIDI messages */
            if (!(snd_seq_port_info_get_type(pinfo)
                  & SND_SEQ_PORT_TYPE_MIDI_GENERIC))
                continue;

            /* we need both WRITE and SUBS_WRITE */
            if ((snd_seq_port_info_get_capability(pinfo)
                 & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
                != (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
                continue;

            validDevices.push_back({
                snd_seq_port_info_get_name(pinfo),
                snd_seq_port_info_get_client(pinfo),
                snd_seq_port_info_get_port(pinfo),
            });

            devicesFound = true;
        }
    }

    if (requiredInit) {  
        ALSADrv_MIDI_Shutdown();
    }

    return (devicesFound) ? validDevices : nullDevice;
}
