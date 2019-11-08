
#define ZPL_IMPLEMENTATION

// zpl.h is included from compat.h
#include "compat.h"

#define zpl_inline
#define cast(Type) (Type)


zpl_virtual_memory zpl_vm(void *data, size_t size) {
    zpl_virtual_memory vm;
    vm.data = data;
    vm.size = size;
    return vm;
}

#if defined _WIN32

zpl_inline zpl_virtual_memory zpl_vm_alloc(void *addr, size_t size) {
    zpl_virtual_memory vm;
    ZPL_ASSERT(size > 0);
    vm.data = VirtualAlloc(addr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    vm.size = size;
    return vm;
}

zpl_inline zpl_b32 zpl_vm_free(zpl_virtual_memory vm) {
    MEMORY_BASIC_INFORMATION info;
    while (vm.size > 0) {
        if (VirtualQuery(vm.data, &info, zpl_size_of(info)) == 0) return false;
        if (info.BaseAddress != vm.data || info.AllocationBase != vm.data || info.State != MEM_COMMIT ||
            info.RegionSize > vm.size) {
            return false;
        }
        if (VirtualFree(vm.data, 0, MEM_RELEASE) == 0) return false;
        vm.data = zpl_pointer_add(vm.data, info.RegionSize);
        vm.size -= info.RegionSize;
    }
    return true;
}

#elif defined __linux__ || defined __APPLE__

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

zpl_inline zpl_virtual_memory zpl_vm_alloc(void *addr, size_t size) {
    zpl_virtual_memory vm;
    ZPL_ASSERT(size > 0);
    vm.data = mmap(addr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (EDUKE32_PREDICT_FALSE(vm.data == MAP_FAILED))
        vm.data = nullptr;
    vm.size = size;
    return vm;
}

zpl_inline zpl_b32 zpl_vm_free(zpl_virtual_memory vm) {
    munmap(vm.data, vm.size);
    return true;
}

#else

zpl_inline zpl_virtual_memory zpl_vm_alloc(void *addr, size_t size) {
    UNREFERENCED_PARAMETER(addr);
    zpl_virtual_memory vm;
    ZPL_ASSERT(size > 0);
    vm.data = Baligned_alloc(Bgetpagesize(), size);
    vm.size = size;
    return vm;
}

zpl_inline zpl_b32 zpl_vm_free(zpl_virtual_memory vm) {
    Baligned_free(vm.data);
    return true;
}

#endif
