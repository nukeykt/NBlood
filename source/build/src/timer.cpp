
#include "timer.h"

#include "build.h"
#include "compat.h"

#ifdef _WIN32
#include "winbits.h"
#include <mmsystem.h>
#endif

#include <chrono>

using namespace std;
using namespace chrono;

static time_point<steady_clock> timerlastsample;
static int timerticspersec;
static void(*usertimercallback)(void) = NULL;

int timerGetRate(void) { return timerticspersec; }

uint32_t timerGetTicks(void)
{
#ifdef _WIN32
    return timeGetTime();
#else
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
#endif
}
// Returns the time since an unspecified starting time in milliseconds (fractional).
// (May be not monotonic for certain configurations.)
double timerGetHiTicks(void) { return duration<double, nano>(steady_clock::now().time_since_epoch()).count() / 1000000.0; }

EDUKE32_STATIC_ASSERT((high_resolution_clock::period::den/high_resolution_clock::period::num) >= 1000000000);
EDUKE32_STATIC_ASSERT(steady_clock::is_steady);

uint64_t timerGetTicksU64(void) { return high_resolution_clock::now().time_since_epoch().count() * high_resolution_clock::period::num; }
uint64_t timerGetFreqU64(void)  { return high_resolution_clock::period::den; }

int timerInit(int const tickspersecond)
{
    timerticspersec = tickspersecond;
    timerlastsample = steady_clock::now();

    usertimercallback = NULL;

    return 0;
}

ATTRIBUTE((flatten)) void timerUpdate(void)
{
    auto time = steady_clock::now();
    auto elapsedTime = time - timerlastsample;

    uint64_t numerator = (elapsedTime.count() * (uint64_t) timerticspersec * steady_clock::period::num);
    uint64_t const freq = steady_clock::period::den;
    int n = tabledivide64(numerator, freq);
    totalclock.setFraction(tabledivide64((numerator - n*freq) * 65536, freq));

    if (n <= 0) return;

    totalclock += n;
    timerlastsample += n*nanoseconds(1000000000/timerticspersec);

    if (usertimercallback)
        for (; n > 0; n--) usertimercallback();
}

void(*timerSetCallback(void(*callback)(void)))(void)
{
    void(*oldtimercallback)(void);

    oldtimercallback = usertimercallback;
    usertimercallback = callback;

    return oldtimercallback;
}
