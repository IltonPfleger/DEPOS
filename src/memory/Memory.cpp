#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

void Memory::init() {
    unsigned int PageSize = Traits<Memory>::PageSize;
    unsigned long RamStart = Traits<MemoryMap>::RamStart;
    unsigned long RamEnd = Traits<MemoryMap>::RamEnd;
    unsigned long KernelStart = __kmm.start;
    unsigned long KernelEnd = __kmm.end;
    unsigned long KernelSize = KernelEnd - KernelStart;
    unsigned long ApplicationStart = __mm.start;
    unsigned long ApplicationEnd = __mm.end;
    unsigned long ApplicationSize = ApplicationEnd - ApplicationStart;
    unsigned long BootStart = __bmm.start;
    unsigned long BootEnd = __bmm.end;

    s_spin.lock();

    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize, BootStart);

    new (&s_allocator) Allocator();

    for (unsigned long c = RamEnd - PageSize; c + PageSize > RamStart; c -= PageSize) {
        if (c + PageSize >= KernelStart && c < KernelEnd) continue;
        if (c + PageSize >= ApplicationStart && c < ApplicationEnd) continue;
        if (c + PageSize >= BootStart && c < BootEnd) continue;
        s_allocator.insert(reinterpret_cast<void *>(c), PageSize);
    }

    __bmm.start = 0;

    TraceOut();

    s_spin.unlock();
}

uintptr_t Memory::virt2phys(uintptr_t address) {
    if constexpr (Traits<System>::Multitask)
        return address - Traits<MemoryMap>::VirtualRamStart + Traits<MemoryMap>::PhysicalRamStart;
    else
        return address;
}

void *Memory::alloc(size_t size) {
    s_spin.lock();
    TraceIn(size);
    void *block;
    if (__bmm.start) {
        __bmm.start -= size;
        block = reinterpret_cast<void *>(virt2phys(__bmm.start));
    } else {
        block = s_allocator.remove(size);
    }
    ERROR(!block, "Out of Memory.");
    TraceOut(block);
    s_spin.unlock();
    return block;
}

void Memory::free(void *addr, size_t size) {
    s_spin.lock();
    TraceIn(addr, size);
    ERROR(addr == nullptr, "}\n");
    s_allocator.insert(addr, size);
    TraceOut();
    s_spin.unlock();
}

void *operator new(size_t, void *ptr) { return ptr; }
