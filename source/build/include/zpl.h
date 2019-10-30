/*

  ZPL - Global module

Credits:
  Read AUTHORS.md

GitHub:
  https://github.com/zpl-c/zpl

  This Software is dual licensed under the following licenses:

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

  Apache 2.0
  Copyright 2017-2019 Dominik Madarász <zaklaus@outlook.com>
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#ifndef ZPL_INCLUDE_ZPL_H
#define ZPL_INCLUDE_ZPL_H


/*
 * This file has been gutted for EDuke32.
 * There is a lot of cool stuff in ZPL.
 * However, 10000 lines of everything and the kitchen sink is an unacceptable burden.
 * Additionally, implementations are often lacking outside of a few well-tested targets.
 * It is far from portable enough for our needs to include anything beyond the specific pieces we want to use.
 * Even then, changes are often required to allow portable use.
 */

#define ZPL_DEF extern

#define zpl_inline FORCE_INLINE

typedef int32_t zpl_i32;
typedef zpl_i32 zpl_b32;
typedef uint32_t zpl_u32;
typedef uint64_t zpl_u64;

#define zpl_size_of(x) sizeof(x)
#define zpl_pointer_add(ptr, bytes) ((void *)((char *)ptr + bytes))
#define cast(Type) (Type)

#define ZPL_ASSERT(x) Bassert(x)


/* Begin ZPL. */

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)
#pragma warning(disable : 4127) // Conditional expression is constant
#endif

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct zpl_virtual_memory {
    void *data;
    size_t size;
} zpl_virtual_memory;

ZPL_DEF zpl_virtual_memory zpl_vm(void *data, size_t size);
ZPL_DEF zpl_virtual_memory zpl_vm_alloc(void *addr, size_t size);
ZPL_DEF zpl_b32 zpl_vm_free(zpl_virtual_memory vm);


#if defined _WIN32 || defined __APPLE__ || defined EDUKE32_CPU_X86
#define ZPL_HAVE_FENCES

zpl_inline void zpl_yield_thread(void) {
#if defined _WIN32 || defined EDUKE32_CPU_X86
    _mm_pause();
#elif defined __APPLE__
    __asm__ volatile ("" : : : "memory");
#else
#error Unknown architecture
#endif
}

zpl_inline void zpl_mfence(void) {
#if defined _WIN32
    _ReadWriteBarrier();
#elif defined __APPLE__
    __sync_synchronize();
#elif defined EDUKE32_CPU_X86
    _mm_mfence();
#else
#error Unknown architecture
#endif
}

zpl_inline void zpl_sfence(void) {
#if defined _WIN32
    _WriteBarrier();
#elif defined __APPLE__
    __asm__ volatile ("" : : : "memory");
#elif defined EDUKE32_CPU_X86
    _mm_sfence();
#else
#error Unknown architecture
#endif
}

zpl_inline void zpl_lfence(void) {
#if defined _WIN32
    _ReadBarrier();
#elif defined __APPLE__
    __asm__ volatile ("" : : : "memory");
#elif defined EDUKE32_CPU_X86
    _mm_lfence();
#else
#error Unknown architecture
#endif
}

#endif


#if defined _MSC_VER && !defined __clang__
#define ZPL_HAVE_RDTSC
zpl_inline zpl_u64 zpl_rdtsc(void) { return __rdtsc( ); }
#elif defined __i386__
#define ZPL_HAVE_RDTSC
zpl_inline zpl_u64 zpl_rdtsc(void) {
    zpl_u64 x;
    __asm__ volatile(".byte 0x0f, 0x31" : "=A"(x));
    return x;
}
#elif defined __x86_64__
#define ZPL_HAVE_RDTSC
zpl_inline zpl_u64 zpl_rdtsc(void) {
    zpl_u32 hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (cast(zpl_u64) lo) | ((cast(zpl_u64) hi) << 32);
}
#elif defined EDUKE32_CPU_PPC
#define ZPL_HAVE_RDTSC
zpl_inline zpl_u64 zpl_rdtsc(void) {
    zpl_u64 result = 0;
    zpl_u32 upper, lower, tmp;
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
#define ZPL_HAVE_RDTSC
zpl_inline zpl_u64 zpl_rdtsc(void) {
#if defined(__aarch64__)
    int64_t r = 0;
    asm volatile("mrs %0, cntvct_el0" : "=r"(r));
#elif defined(__ARM_ARCH_7A__)
    uint32_t r = 0;
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(r));
#elif (__ARM_ARCH >= 6)
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
#error "No suitable method for zpl_rdtsc for this cpu type"
#endif
    return r;
}
#endif


#if defined(__cplusplus)
}
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif


/* End ZPL. */

#undef cast
#undef ZPL_DEF
#undef zpl_inline

static FORCE_INLINE zpl_virtual_memory xvm_alloc(void * const ptr, const size_t size)
{
    zpl_virtual_memory vm = zpl_vm_alloc(ptr, size);

    if (EDUKE32_PREDICT_FALSE(vm.data == NULL))
        vm.data = handle_memerr(vm.data);

    return vm;
}
#define Xvm_alloc(ptr, size) (EDUKE32_PRE_XALLOC xvm_alloc(ptr, size))
#define Xvm_free(ptr) (zpl_vm_free(ptr))
#define Xvm_free(ptr) (zpl_vm_free(ptr))

#endif /* ZPL_INCLUDE_ZPL_H */
