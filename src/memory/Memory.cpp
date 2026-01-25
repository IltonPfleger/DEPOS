#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
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

    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize);
    new (&s_allocator) Allocator();

    unsigned long c = RamEnd - (PageSize * Traits<CPUS>::ACTIVE);

    for (; c > RamStart; c -= PageSize) {
        if (c + PageSize >= KernelStart && c < KernelEnd) continue;
        if (c + PageSize >= ApplicationStart && c < ApplicationEnd) continue;
        s_allocator.insert(reinterpret_cast<void *>(c), PageSize);
    }
    TraceOut();
}

void *Memory::alloc(size_t size) {
    TraceIn(size);
    s_spin.lock();
    void *block = s_allocator.remove(size);
    s_spin.unlock();
    ERROR(!block, "Out of Memory.");
    TraceOut(block);
    return block;
}

void Memory::free(void *addr, size_t size) {
    TraceIn(addr, size);
    ERROR(addr == nullptr, "}\n");
    s_spin.lock();
    s_allocator.insert(addr, size);
    s_spin.unlock();
    TraceOut();
}

void *operator new(size_t, void *ptr) { return ptr; }
