#pragma once

#ifndef timer_h__
#define timer_h__

#include "compat.h"

// for compatibility
#define timerUninit()

auto constexpr TIMER_AUTO   = 0;
auto constexpr TIMER_QPC    = 1;
auto constexpr TIMER_SDL    = 2;
auto constexpr TIMER_CHRONO = 3;
auto constexpr TIMER_RDTSC  = 4;

int      timerInit(int const tickspersecond);
void     timerUpdateClock(void);
int      timerGetClockRate(void);
uint64_t timerGetTicksU64(void);
uint64_t timerGetFreqU64(void);
double   timerGetHiTicks(void);
uint32_t timerGetTicks(void);

void (*timerSetCallback(void (*callback)(void)))(void);

#endif // timer_h__
