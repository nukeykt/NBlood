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

int      timerInit(int const tickspersecond);
void     timerUpdateClock(void);
int      timerGetClockRate(void);
uint64_t timerGetPerformanceCounter(void);
uint64_t timerGetPerformanceFrequency(void);
double   timerGetHiTicks(void);
uint32_t timerGetTicks(void);

void (*timerSetCallback(void (*callback)(void)))(void);

#endif /* timer_h_ */
