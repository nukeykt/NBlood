/*

  Taken from https://github.com/zpl-c/zpl

  This Software is licensed under the following licenses:

  Unlicense
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/

#ifndef rdtsc_h_
#define rdtsc_h_

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _MSC_VER && !defined __clang__ && !defined(_M_ARM64)
#define HAVE_TIMER_RDTSC
FORCE_INLINE uint64_t eduke32_rdtsc(void) { return __rdtsc( ); }
#elif defined __i386__
#define HAVE_TIMER_RDTSC
FORCE_INLINE uint64_t eduke32_rdtsc(void) {
    uint64_t x;
    __asm__ volatile(".byte 0x0f, 0x31" : "=A"(x));
    return x;
}
#elif defined __x86_64__
#define HAVE_TIMER_RDTSC
FORCE_INLINE uint64_t eduke32_rdtsc(void) {
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t) lo) | (((uint64_t) hi) << 32);
}
#elif defined EDUKE32_CPU_PPC
#define HAVE_TIMER_RDTSC
FORCE_INLINE uint64_t eduke32_rdtsc(void) {
    uint64_t result = 0;
    uint32_t upper, lower, tmp;
    __asm__ volatile("0:                   \n"
                     "\tmftbu   %0         \n"
                     "\tmftb    %1         \n"
                     "\tmftbu   %2         \n"
                     "\tcmpw    %2,%0      \n"
                     "\tbne     0b         \n"
                     : "=r"(upper), "=r"(lower), "=r"(tmp));
    result = upper;
    result = result << 32;
    result = result | lower;

    return result;
}
#elif defined EDUKE32_CPU_ARM
#define HAVE_TIMER_RDTSC
FORCE_INLINE uint64_t eduke32_rdtsc(void) {
#if defined(__aarch64__)
    int64_t r = 0;
    asm volatile("mrs %0, cntvct_el0" : "=r"(r));
#elif (__ARM_ARCH >= 6)
    uint32_t r = 0;
    uint32_t pmccntr;
    uint32_t pmuseren;
    uint32_t pmcntenset;

    // Read the user mode perf monitor counter access permissions.
    asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
    if (pmuseren & 1) { // Allows reading perfmon counters for user mode code.
        asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
        if (pmcntenset & 0x80000000ul) { // Is it counting?
            asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
            // The counter is set up to count every 64th cycle
            return ((int64_t)pmccntr) * 64; // Should optimize to << 6
        }
    }
#else
#error "No rdtsc implementation defined for this architecture."
#endif
    return r;
}
#endif


#if defined(__cplusplus)
}
#endif

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif /* rdtsc_h_ */
