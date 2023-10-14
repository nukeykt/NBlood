#ifndef cpuid_h_
#define cpuid_h_

#include "compat.h"

#if defined EDUKE32_CPU_X86
enum cpuinfo_vendor_x86
{
    CPU_UNKNOWN = 0,
    CPU_INTEL   = 1,
    CPU_AMD     = 2,
};
#endif

struct cpuinfo_t
{
    //int type;
    char *vendorIDString;
    char *brandString;

    struct
    {
        unsigned int invariant_tsc : 1;
    } features;
};

extern cpuinfo_t cpu;

void sysReadCPUID(void);

#endif /* cpuid_h_ */
