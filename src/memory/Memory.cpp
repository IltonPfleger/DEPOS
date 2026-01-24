#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <utils/Debug.hpp>

void Memory::init() {
    constexpr size_t PageSize = Traits<Memory>::PageSize;
    constexpr uintptr_t RamStart = Traits<MemoryMap>::RamStart;
    constexpr uintptr_t RamEnd = Traits<MemoryMap>::RamEnd;
    uintptr_t KernelStart = __kmm.start;
    uintptr_t KernelEnd = __kmm.end;
    uintptr_t KernelSize = KernelEnd - KernelStart;
    uintptr_t ApplicationStart = __mm.start;
    uintptr_t ApplicationEnd = __mm.end;
    uintptr_t ApplicationSize = ApplicationEnd - ApplicationStart;
    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize);
    new (&s_allocator) Allocator();

    uintptr_t c = RamEnd - (PageSize * Traits<CPUS>::ONLINE);
    for (; c > RamStart; c -= PageSize) {
        if (c + PageSize >= KernelStart && c < KernelEnd)
            continue;
        if (c + PageSize >= ApplicationStart && c < ApplicationEnd) {
            continue;
        }
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
