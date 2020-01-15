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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#ifndef driver_sdl_h__
#define driver_sdl_h__

extern char SDLAudioDriverName[16];
const char *SDLDrv_ErrorString(int ErrorNumber);

int  SDLDrv_GetError(void);
int  SDLDrv_PCM_Init(int *mixrate, int *numchannels, void *initdata);
void SDLDrv_PCM_Shutdown(void);
int  SDLDrv_PCM_BeginPlayback(char *BufferStart, int BufferSize, int NumDivisions, void (*CallBackFunc)(void));
void SDLDrv_PCM_StopPlayback(void);
void SDLDrv_PCM_Lock(void);
void SDLDrv_PCM_Unlock(void);

void SDLDrv_PCM_PrintDevices(void);
int  SDLDrv_PCM_CheckDevice(char const *dev);
char const *SDLDrv_PCM_GetDevice(void);

#endif // driver_sdl_h__
