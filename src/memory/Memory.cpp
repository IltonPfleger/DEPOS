#include <memory/Memory.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

void Memory::init() {
    const auto PageSize = Traits<Memory>::PageSize;
    const auto RamStart = Traits<MemoryMap>::RamStart;
    const auto RamEnd = Traits<MemoryMap>::RamEnd;
    const auto KernelSize = __kmm.end - __kmm.start;
    const auto ApplicationSize = __mm.end - __mm.start;
    const auto BootMemorySize = __bmm.end - __bmm.start;

    TraceIn(Console::Hex(__kmm.start), Console::Hex(__kmm.end), KernelSize, Console::Hex(__mm.start), Console::Hex(__mm.end),
            ApplicationSize, Console::Hex(__bmm.start), Console::Hex(__bmm.end), BootMemorySize);

    s_allocator = new (alloc(sizeof(Allocator))) Allocator();

    for (unsigned long c = RamEnd - PageSize; c + PageSize > RamStart; c -= PageSize) {
        if (c + PageSize >= __kmm.start && c < __kmm.end) continue;
        if (c + PageSize >= __mm.start && c < __mm.end) continue;
        if (c + PageSize >= __bmm.start && c < __bmm.end) continue;
        s_allocator->insert(reinterpret_cast<void *>(c), PageSize);
    }

    TraceOut();
}

uintptr_t Memory::virt2phys(uintptr_t chunkess) {
    if constexpr (Traits<System>::Multitask)
        return chunkess - Traits<MemoryMap>::VirtualRamStart + Traits<MemoryMap>::PhysicalRamStart;
    else
        return chunkess;
}

void *Memory::alloc(size_t size) {
    if (size == 0) return 0;

    s_spin.acquire();

    TraceIn(size);

    void *chunk = nullptr;

    if (!s_allocator) {
        Trace("Ussing Boot Memory!\n");
        __bmm.start -= size;
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
