#pragma once

#ifndef timer_h_
#define timer_h_

#include "compat.h"

// for compatibility
#define timerUninit()

enum buildtimertype
{
    TIMER_AUTO = 0,
    TIMER_QPC,
    TIMER_SDL,
    TIMER_RDTSC,
    NUMTIMERS,
};

uint64_t timerGetPerformanceCounter(void);
uint64_t timerGetPerformanceFrequency(void);

int  timerInit(int const tickspersecond);
void timerUpdateClock(void);

int      timerGetClockRate(void);
double   timerGetFractionalTicks(void);
uint32_t timerGetTicks(void);
uint32_t timer120(void);
uint64_t timerGetNanoTicks(void);
uint64_t timerGetNanoTickRate(void);

void (*timerSetCallback(void (*callback)(void)))(void);

#endif /* timer_h_ */
