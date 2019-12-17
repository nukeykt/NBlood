#include "baselayer.h"
#include "build_cpuid.h"
#include "compat.h"
#include "osd.h"

cpuinfo_t cpu;

#if defined EDUKE32_CPU_X86

#ifndef _MSC_VER
# include <cpuid.h>
#endif

static char g_cpuVendorIDString[16];
static char g_cpuBrandString[64];

void sysReadCPUID()
{
    int32_t regs[4];

#ifdef _MSC_VER
    __cpuid(regs, 0);
#else
    __cpuid(0, regs[0], regs[1], regs[2], regs[3]);
#endif

    // CPUID returns things out of order...
    Bmemcpy(g_cpuVendorIDString,   regs+1, 4);
    Bmemcpy(g_cpuVendorIDString+8, regs+2, 4);
    Bmemcpy(g_cpuVendorIDString+4, regs+3, 4);
    g_cpuVendorIDString[12] = '\0';

#ifdef DEBUGGINGAIDS
    initprintf("CPUID Vendor ID: %s\n", g_cpuVendorIDString);
#endif
    cpu.vendorIDString = g_cpuVendorIDString;

    if (!Bstrcmp(g_cpuVendorIDString, "GenuineIntel"))
        cpu.type = CPU_INTEL;
    else if (!Bstrcmp(g_cpuVendorIDString, "AuthenticAMD"))
        cpu.type = CPU_AMD;
    else
        cpu.type = CPU_UNKNOWN;

#ifdef _MSC_VER
    __cpuid(regs, 0x80000000);
#else
    __cpuid(0x80000000, regs[0], regs[1], regs[2], regs[3]);
#endif

    auto const subleaves = (unsigned)regs[0];

    if (subleaves >= 0x80000004)
    {
#ifdef _MSC_VER
        __cpuid((int *)g_cpuBrandString,   0x80000002);
        __cpuid((int *)g_cpuBrandString+4, 0x80000003);
        __cpuid((int *)g_cpuBrandString+8, 0x80000004);
#else
        __cpuid(0x80000002, *(int *)&g_cpuBrandString[0], *(int *)&g_cpuBrandString[4],
                            *(int *)&g_cpuBrandString[8], *(int *)&g_cpuBrandString[16]);
        __cpuid(0x80000003, *(int *)&g_cpuBrandString[20], *(int *)&g_cpuBrandString[24],
                            *(int *)&g_cpuBrandString[28], *(int *)&g_cpuBrandString[32]);
        __cpuid(0x80000004, *(int *)&g_cpuBrandString[36], *(int *)&g_cpuBrandString[40],
                            *(int *)&g_cpuBrandString[44], *(int *)&g_cpuBrandString[48]);
#endif
        for (int i=ARRAY_SIZE(g_cpuBrandString)-1;i>=0;--i)
        {
            if (isalnum(g_cpuBrandString[i]))
                break;

            g_cpuBrandString[i] = '\0';
        }

        initprintf("CPU: %s\n", g_cpuBrandString);
        cpu.brandString = g_cpuBrandString;

        if (subleaves >= 0x80000007)
        {
#ifdef _MSC_VER
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
