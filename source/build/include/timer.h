#pragma once

#ifndef timer_h_
#define timer_h_

#include "compat.h"

// for compatibility
#define timerUninit()

enum buildtimertype
{
    TIMER_AUTO   = 0,
    TIMER_QPC    = 1,
    TIMER_SDL    = 2,
    TIMER_CHRONO = 3,
    TIMER_RDTSC  = 4,
};

int      timerInit(int const tickspersecond);
void     timerUpdateClock(void);
int      timerGetClockRate(void);
uint64_t timerGetTicksU64(void);
uint64_t timerGetFreqU64(void);
double   timerGetHiTicks(void);
uint32_t timerGetTicks(void);

void (*timerSetCallback(void (*callback)(void)))(void);

#endif /* timer_h_ */
