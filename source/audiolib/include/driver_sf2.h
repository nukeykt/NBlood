#pragma once
#ifndef driver_sf2_h__
#define driver_sf2_h__

#include "compat.h"
#include "midifuncs.h"

enum
{
    SF2_Error     = -1,
    SF2_Ok        = 0,
    SF2_BankError = 1,
};

int         SF2Drv_GetError(void);
const char *SF2Drv_ErrorString(int ErrorNumber);

int  SF2Drv_MIDI_Init(midifuncs *);
void SF2Drv_MIDI_Shutdown(void);
int  SF2Drv_MIDI_StartPlayback(void);
void SF2Drv_MIDI_HaltPlayback(void);
void SF2Drv_MIDI_SetTempo(int tempo, int division);
void SF2Drv_MIDI_Service(void);
#endif // driver_sf2_h__
