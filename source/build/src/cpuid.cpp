#include "compat.h"
#include "cpuid.h"
#include "osd.h"

cpuinfo_t cpu;

#ifdef EDUKE32_PLATFORM_INTEL

#ifndef _MSC_VER
# include <cpuid.h>
#endif

static char g_cpuVendorIDString[16];
static char g_cpuBrandString[64];

void sysReadCPUID(void)
{
    int32_t regs[4];

    __cpuid(regs, 0);

    // CPUID returns things out of order...
    Bmemcpy(g_cpuVendorIDString,   regs+1, 4);
    Bmemcpy(g_cpuVendorIDString+8, regs+2, 4);
    Bmemcpy(g_cpuVendorIDString+4, regs+3, 4);
    g_cpuVendorIDString[12] = '\0';

#ifdef DEBUGGINGAIDS
    OSD_Printf("CPUID Vendor ID: %s\n", g_cpuVendorIDString);
#endif
    cpu.vendorIDString = g_cpuVendorIDString;

    if (!Bstrcmp(g_cpuVendorIDString, "GenuineIntel"))
        cpu.type = CPU_INTEL;
    else if (!Bstrcmp(g_cpuVendorIDString, "AuthenticAMD"))
        cpu.type = CPU_AMD;
    else cpu.type = CPU_UNKNOWN;

    __cpuid(regs, 0x80000000);
    int const subleaves = regs[0];

    if (subleaves >= 0x80000004)
    {
        __cpuid((int *)g_cpuBrandString,   0x80000002);
        __cpuid((int *)g_cpuBrandString+4, 0x80000003);
        __cpuid((int *)g_cpuBrandString+8, 0x80000004);

        for (int i=ARRAY_SIZE(g_cpuBrandString)-1;i>=0;--i)
        {
            if (isalnum(g_cpuBrandString[i]))
                break;

            g_cpuBrandString[i] = '\0';
        }

        OSD_Printf("CPU: %s\n", g_cpuBrandString);
        cpu.brandString = g_cpuBrandString;

        if (subleaves >= 0x80000007)
        {
            __cpuid(regs, 0x80000007);
            cpu.features.invariant_tsc = (regs[3] & (1 << 8)) != 0;
        }
    }
}
#endif  // EDUKE32_PLATFORM_INTEL
