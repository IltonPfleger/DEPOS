#include <memory/Memory.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

void Memory::init() {
    const auto PageSize = Traits<Memory>::PageSize;
    const auto RamStart = Traits<MemoryMap>::RamStart;
    const auto RamEnd = Traits<MemoryMap>::RamEnd;
    // const auto KernelSize = __kmm.end - __kmm.start;
    // const auto ApplicationSize = __mm.end - __mm.start;
    // const auto BootMemorySize = __bmm.end - __bmm.start;

    TraceIn();

    s_allocator = new (alloc(sizeof(Allocator))) Allocator();

    for (unsigned long c = RamEnd - PageSize; c + PageSize > RamStart; c -= PageSize) {
        if (c + PageSize >= __kmm.start && c < __kmm.end) continue;
        if (c + PageSize >= __mm.start && c < __mm.end) continue;
        if (c + PageSize >= __bmm.start && c < __bmm.end) continue;
        s_allocator->insert(reinterpret_cast<void *>(c), PageSize);
    }

    TraceOut();
}

uintptr_t Memory::virt2phys(uintptr_t chunk) {
    if constexpr (Traits<Kernel>::Multitask)
        return chunk - Traits<MemoryMap>::VirtualRamStart + Traits<MemoryMap>::PhysicalRamStart;
    else
        return chunk;
}

void *Memory::alloc(size_t size) {
    if (size == 0) return 0;

    s_spin.acquire();

    TraceIn(size);

    void *chunk = nullptr;

    if (!s_allocator) {
        Trace("Ussing Boot Memory!");
        __bmm.start -= size;
        __bmm.start &= ~(size - 1);
        chunk = reinterpret_cast<void *>(virt2phys(__bmm.start));
    } else {
        chunk = s_allocator->remove(size);
    }

    ERROR(!chunk, "Out of Memory.");

    TraceOut(chunk);

    s_spin.release();
    return chunk;
}

void Memory::free(void *chunk, size_t size) {
    s_spin.acquire();
    TraceIn(chunk, size);
    ERROR(chunk == nullptr);
    s_allocator->insert(chunk, size);
    TraceOut();
    s_spin.release();
}

void *operator new(size_t, void *ptr) { return ptr; }
