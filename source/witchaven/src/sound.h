
#ifndef __sound_h__
#define __sound_h__

#include <stdint.h>
#include "sndmod.h"

extern int SoundMode;
extern int MusicMode;

void SND_Startup();
void SND_Shutdown();

int SND_Sound(int nSound);
int SND_PlaySound(int nSound, int32_t x, int32_t y, int16_t Pan, int16_t loopcount);
void playsound_loc(int nSound, int32_t xplc, int32_t yplc);
void SND_CheckLoops();
void SND_StopLoop(int16_t nSound);
void SND_SongFlush();
void SND_StartMusic(int16_t level);
void SND_Mixer(int16_t wSource, int16_t wVolume);
void SND_Sting(int16_t nSound);
void SND_FadeMusic();
void SND_MenuMusic(int nSong);

#endif
