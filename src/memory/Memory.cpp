#include <machine/Machine.hpp>
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
    unsigned long BootStart = MemoryMap::__bmm.start;
    unsigned long BootEnd = MemoryMap::__bmm.end;
    unsigned long BootSize = BootEnd - BootStart;

    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize, BootStart, BootEnd,
            BootSize);

    new (&s_allocator) Allocator();

    unsigned long c = RamEnd - (PageSize * Traits<CPUS>::ACTIVE);

    for (; c > RamStart; c -= PageSize) {
        if (c + PageSize >= KernelStart && c < KernelEnd) continue;
        if (c + PageSize >= ApplicationStart && c < ApplicationEnd) continue;
        if (c + PageSize >= BootStart && c < BootEnd) continue;
        s_allocator.insert(reinterpret_cast<void *>(c), PageSize);
    }

    s_init = true;
    TraceOut();
}

size_t Memory::max() { return s_allocator.max(); }

uintptr_t Memory::virt2phys(uintptr_t va) {
    if constexpr (Traits<System>::Multitask)
        return va - Traits<MemoryMap>::VirtualRamStart + Traits<MemoryMap>::PhysicalRamStart;
    else
        return va;
}

void *Memory::alloc(size_t size) {
    TraceIn(size);
    void *block;
    if (!s_init) {
        block = reinterpret_cast<void *>(virt2phys(CPU::Atomic::fdec(MemoryMap::__bmm.start, size)));
    } else {
        s_spin.lock();
        block = s_allocator.remove(size);
        s_spin.unlock();
    }
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
