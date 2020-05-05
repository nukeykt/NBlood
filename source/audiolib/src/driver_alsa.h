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

#include "music.h"

int ALSADrv_GetError(void);
const char *ALSADrv_ErrorString( int ErrorNumber );

int  ALSADrv_MIDI_Init(midifuncs *);
void ALSADrv_MIDI_Shutdown(void);
int  ALSADrv_MIDI_StartPlayback(void);
void ALSADrv_MIDI_HaltPlayback(void);
unsigned int ALSADrv_MIDI_GetTick(void);
void ALSADrv_MIDI_SetTempo(int tempo, int division);
void ALSADrv_MIDI_Lock(void);
void ALSADrv_MIDI_Unlock(void);
void ALSADrv_MIDI_Service(void);
void ALSADrv_MIDI_QueueStart(void);
void ALSADrv_MIDI_QueueStop(void);
