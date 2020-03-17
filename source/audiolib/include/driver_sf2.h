#pragma once
#include "compat.h"
#include "midifuncs.h"

extern char SF2_BankFile[BMAX_PATH];

enum
{
    SF2_Warning = -2,
    SF2_Error = -1,
    SF2_Ok = 0,
    SF2_BankError = 1,
};

int         SF2_GetError(void);
const char *SF2_ErrorString(int ErrorNumber);

int  SF2_MIDI_Init(midifuncs *);
void SF2_MIDI_Shutdown(void);
int  SF2_MIDI_StartPlayback(void);
void SF2_MIDI_HaltPlayback(void);
void SF2_MIDI_SetTempo(int tempo, int division);
void SF2_MIDI_Service(void);
