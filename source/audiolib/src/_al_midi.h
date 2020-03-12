/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

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
#ifndef ___AL_MIDI_H
#define ___AL_MIDI_H

#include <inttypes.h>
#include "_midi.h"

#define lobyte(num) ((uint32_t)*((char *)&(num)))
#define hibyte(num) ((uint32_t)*(((char *)&(num)) + 1))

#define AL_MaxVolume             127
#define AL_DefaultChannelVolume  90
#define AL_DefaultPitchBendRange 200
#define AL_VoiceNotFound -1

#define ADLIB_PORT 0x388

/* Number of slots for the voices on the chip */
#define AL_NumChipSlots 18

#define NUMADLIBVOICES 9
#define NUMADLIBCHANNELS 16

#define NOTE_ON  0x2000 /* Used to turn note on or toggle note */
#define NOTE_OFF 0x0000

#define MAX_VELOCITY 0x7f
#define MAX_OCTAVE 7
#define MAX_NOTE (MAX_OCTAVE * 12 + 11)
#define FINETUNE_MAX 31
#define FINETUNE_RANGE (FINETUNE_MAX + 1)

#define PITCHBEND_CENTER 1638400

typedef struct AdLibVoice
{
    struct AdLibVoice *next;
    struct AdLibVoice *prev;

    uint32_t num;
    uint32_t key;
    uint32_t velocity;
    uint32_t channel;
    uint32_t pitchleft;
    uint32_t pitchright;
    int      timbre;
    int      port;
    uint32_t status;
} AdLibVoice;

typedef struct
{
    AdLibVoice *start;
    AdLibVoice *end;
} AdLibVoiceList;

typedef struct
{
    AdLibVoiceList Voices;

    int      Timbre;
    int      Pitchbend;
    int      KeyOffset;
    uint32_t KeyDetune;
    uint32_t Volume;
    uint32_t EffectiveVolume;
    int      Pan;
    int      Detune;
    uint32_t RPN;
    int16_t  PitchBendRange;
    int16_t  PitchBendSemiTones;
    int16_t  PitchBendHundreds;
} AdLibChannel;

static int  AL_Init(int rate);
static void AL_NoteOff(int channel, int key, int velocity);
static void AL_NoteOn(int channel, int key, int vel);
static void AL_ControlChange(int channel, int type, int data);
static void AL_ProgramChange(int channel, int patch);
static void AL_SetPitchBend(int channel, int lsb, int msb);
static void AL_SetVolume(int volume);

#endif
