#include <BootInformation.hpp>
#include <memory/Memory.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

void Memory::init() {
    constexpr auto PageSize = Traits<Memory>::PageSize;
    constexpr auto RamStart = Traits<MemoryMap>::RamStart;
    constexpr auto RamEnd   = Traits<MemoryMap>::RamEnd;

    TraceIn();

    for (uintptr_t c = RamEnd - PageSize; c >= RamStart; c -= PageSize) {
        Chunk page(c, PageSize);
        if (page.overlaps(__kmm)) continue;
        if (page.overlaps(__mm)) continue;
        if (page.overlaps(__bmm)) continue;
        s_allocator.insert(reinterpret_cast<void *>(page.start()), page.size());
    }

    TraceOut();
}

uintptr_t Memory::virt2phys(uintptr_t address) {
    if constexpr (Traits<Kernel>::Multitask) return address - (Traits<MemoryMap>::RamStart - __amm.start());
    return address;
}

void *Memory::alloc(size_t size) {
    if (size == 0) return 0;

    s_spin.acquire();

    TraceIn(size);

    void *chunk = s_allocator.remove(size);

    ERROR(!chunk, "Out of Memory.");

    TraceOut(chunk);

    s_spin.release();

    return chunk;
}

void Memory::free(void *chunk, size_t size) {
    s_spin.acquire();
    TraceIn(chunk, size);
    ERROR(chunk == nullptr);
    s_allocator.insert(chunk, size);
    TraceOut();
    s_spin.release();
}

} // namespace DEPOS
