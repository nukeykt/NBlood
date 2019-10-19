#ifndef cpuid_h__
#define cpuid_h__
#include "compat.h"

auto constexpr CPU_UNKNOWN = 0;
auto constexpr CPU_INTEL   = 1;
auto constexpr CPU_AMD     = 2;

struct cpuinfo_t
{
    int type;
    char *vendorIDString;
    char *brandString;

    struct
    {
        int invariant_tsc : 1;
    } features;
};

extern cpuinfo_t cpu;

void sysReadCPUID(void);

#endif // cpuid_h__
