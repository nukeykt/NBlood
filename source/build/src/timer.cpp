// Build engine timer stuff

#include "timer.h"

#include "build.h"
#include "compat.h"
#include "cpuid.h"
#include "renderlayer.h"

#ifdef _WIN32
#include "winbits.h"
#include <mmsystem.h>
#endif

#include <chrono>

using namespace std;
using namespace chrono;

static int sys_timer;

EDUKE32_STATIC_ASSERT(steady_clock::is_steady);

static time_point<steady_clock> clockLastSampleTime;
static int clockTicksPerSecond;
static void(*usertimercallback)(void) = nullptr;

#ifdef EDUKE32_PLATFORM_INTEL
static uint64_t tsc_freq;
#endif

int timerGetClockRate(void) { return clockTicksPerSecond; }

ATTRIBUTE((flatten)) void timerUpdateClock(void)
{
    auto time = steady_clock::now();
    auto elapsedTime = time - clockLastSampleTime;

    uint64_t numerator = (elapsedTime.count() * (uint64_t) clockTicksPerSecond * steady_clock::period::num);
    uint64_t const freq = steady_clock::period::den;
    int n = tabledivide64(numerator, freq);
    totalclock.setFraction(tabledivide64((numerator - n*freq) * 65536, freq));

    if (n <= 0) return;

    totalclock += n;
    clockLastSampleTime += n*nanoseconds(1000000000/clockTicksPerSecond);

    if (usertimercallback)
        for (; n > 0; n--) usertimercallback();
}

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

uint64_t timerGetTicksU64(void)
{
    switch (sys_timer)
    {
#ifdef RENDERTYPESDL
        default:
        case TIMER_AUTO:
        case TIMER_SDL:
            return SDL_GetPerformanceCounter();
#elif !defined _WIN32 && !defined RENDERTYPESDL
        default:
        case TIMER_AUTO:
#endif // RENDERTYPESDL
        case TIMER_CHRONO:
            return high_resolution_clock::now().time_since_epoch().count() * high_resolution_clock::period::num;
#ifdef _WIN32
#if !defined RENDERTYPESDL
        default:
        case TIMER_AUTO:
#endif // !RENDERTYPESDL
        case TIMER_QPC:
            LARGE_INTEGER li;
            QueryPerformanceCounter(&li);
            return li.QuadPart;
#endif // _WIN32
#ifdef EDUKE32_PLATFORM_INTEL
        case TIMER_RDTSC:
            _mm_mfence();
            return __rdtsc();
#endif // EDUKE32_PLATFORM_INTEL
    }
}

uint64_t timerGetFreqU64(void)
{
    switch (sys_timer)
    {
#ifdef RENDERTYPESDL
        default:
        case TIMER_AUTO:
        case TIMER_SDL:
        {
            static uint64_t freq;
            if (freq == 0)
                freq = SDL_GetPerformanceFrequency();
            return freq;
        }
#elif !defined _WIN32 && !defined RENDERTYPESDL
        default:
        case TIMER_AUTO:
#endif // RENDERTYPESDL
        case TIMER_CHRONO:
            return high_resolution_clock::period::den;
#ifdef _WIN32
#if !defined RENDERTYPESDL
        default:
        case TIMER_AUTO:
#endif // !RENDERTYPESDL
        case TIMER_QPC:
        {
            static LARGE_INTEGER li;
            if (li.QuadPart == 0)
                QueryPerformanceFrequency(&li);
            return li.QuadPart;
        }
#endif // _WIN32
#ifdef EDUKE32_PLATFORM_INTEL
        case TIMER_RDTSC:
            return tsc_freq;
#endif // EDUKE32_PLATFORM_INTEL
    }
}

static int osdcmd_sys_timer(osdcmdptr_t parm)
{
    static char constexpr const *s[] = { "auto", "QueryPerformanceCounter", "SDL", "std::chrono", "CPU TSC" };
    int const r = osdcmd_cvar_set(parm);

    if (r != OSDCMD_OK)
        goto print_and_return;

#ifndef _WIN32
    if (sys_timer == TIMER_QPC)
        sys_timer = TIMER_AUTO;
#endif
#ifndef RENDERTYPESDL
    if (sys_timer == TIMER_SDL)
        sys_timer = TIMER_AUTO;
#endif

    if (sys_timer == TIMER_RDTSC && !cpu.features.invariant_tsc)
    {
        sys_timer = TIMER_AUTO;
        OSD_Printf("Invariant TSC support not detected.\n");
    }

    if (sys_timer != TIMER_AUTO || !OSD_ParsingScript())
print_and_return:
        OSD_Printf("Using \"%s\" timer with %g MHz frequency\n", s[sys_timer], timerGetFreqU64() / 1000000.0);

    return r;
}

int timerInit(int const tickspersecond)
{
    static int initDone;

    if (initDone == 0)
    {
        static osdcvardata_t sys_timer_cvar = { "sys_timer",
                                                "engine frame timing backend:\n"
                                                "   0: auto\n"
#ifdef _WIN32
                                                "   1: QueryPerformanceCounter\n"
#endif
#ifdef RENDERTYPESDL
                                                "   2: SDL timer\n"
#endif
                                                "   3: std::chrono\n"
#ifdef EDUKE32_PLATFORM_INTEL
                                                "   4: CPU TSC\n",
#endif
                                                (void *)&sys_timer, CVAR_INT | CVAR_FUNCPTR, 0, 4 };

        OSD_RegisterCvar(&sys_timer_cvar, osdcmd_sys_timer);

#ifdef RENDERTYPESDL
        SDL_InitSubSystem(SDL_INIT_TIMER);
#endif

#ifdef EDUKE32_PLATFORM_INTEL
        if (tsc_freq == 0)
        {
            double const calibrationEndTime = timerGetHiTicks() + 100.0;
            _mm_mfence();
            auto time1 = __rdtsc();
            do { } while (timerGetHiTicks() < calibrationEndTime);
            _mm_mfence();
            auto time2 = __rdtsc();
            _mm_mfence();
            auto time3 = __rdtsc();

            tsc_freq = (time2 - time1 - (time3 - time2)) * 10;
        }
#endif
        initDone = 1;
        sysReadCPUID();
    }

    clockTicksPerSecond = tickspersecond;
    clockLastSampleTime = steady_clock::now();

    usertimercallback = nullptr;

    return 0;
}

void(*timerSetCallback(void(*callback)(void)))(void)
{
    void(*oldtimercallback)(void);

    oldtimercallback = usertimercallback;
    usertimercallback = callback;

    return oldtimercallback;
}
