#include "baselayer.h"
#include "build_cpuid.h"
#include "compat.h"
#include "osd.h"

cpuinfo_t cpu;

#if defined EDUKE32_CPU_X86

#ifdef _WIN32
# include <intrin.h>
#else
# include <cpuid.h>
#endif

static char g_cpuVendorIDString[16];
static char g_cpuBrandString[48];

void sysReadCPUID()
{
    int32_t regs[4];

#ifdef _WIN32
    __cpuid(regs, 0);
#else
    __cpuid(0, regs[0], regs[1], regs[2], regs[3]);
#endif

    // CPUID returns things out of order...
    Bmemcpy(g_cpuVendorIDString,   regs+1, 4);
    Bmemcpy(g_cpuVendorIDString+8, regs+2, 4);
    Bmemcpy(g_cpuVendorIDString+4, regs+3, 4);
    g_cpuVendorIDString[12] = '\0';

    DVLOG_F(LOG_DEBUG, "CPUID Vendor ID: %s", g_cpuVendorIDString);

    cpu.vendorIDString = g_cpuVendorIDString;

    //if (!Bstrcmp(g_cpuVendorIDString, "GenuineIntel"))
    //    cpu.type = CPU_INTEL;
    //else if (!Bstrcmp(g_cpuVendorIDString, "AuthenticAMD"))
    //    cpu.type = CPU_AMD;
    //else
    //    cpu.type = CPU_UNKNOWN;

#ifdef _WIN32
    __cpuid(regs, 0x80000000);
#else
    __cpuid(0x80000000, regs[0], regs[1], regs[2], regs[3]);
#endif

    auto const subleaves = (unsigned)regs[0];

    if (subleaves >= 0x80000004)
    {
#ifdef _WIN32
        __cpuid((int *)g_cpuBrandString,   0x80000002);
        __cpuid((int *)g_cpuBrandString+4, 0x80000003);
        __cpuid((int *)g_cpuBrandString+8, 0x80000004);
#else
        __cpuid(0x80000002, *(int *)&g_cpuBrandString[0], *(int *)&g_cpuBrandString[4],
                            *(int *)&g_cpuBrandString[8], *(int *)&g_cpuBrandString[12]);
        __cpuid(0x80000003, *(int *)&g_cpuBrandString[16], *(int *)&g_cpuBrandString[20],
                            *(int *)&g_cpuBrandString[24], *(int *)&g_cpuBrandString[28]);
        __cpuid(0x80000004, *(int *)&g_cpuBrandString[32], *(int *)&g_cpuBrandString[36],
                            *(int *)&g_cpuBrandString[40], *(int *)&g_cpuBrandString[44]);
#endif
        LOG_F(INFO, "CPU: %s", g_cpuBrandString);
        cpu.brandString = g_cpuBrandString;

        if (subleaves >= 0x80000007)
        {
#ifdef _WIN32
            __cpuid(regs, 0x80000007);
#else
            __cpuid(0x80000007, regs[0], regs[1], regs[2], regs[3]);
#endif
            cpu.features.invariant_tsc = (regs[3] & (1 << 8)) != 0;
        }
    }
}
#else

void sysReadCPUID() { }

#endif  // EDUKE32_CPU_X86
