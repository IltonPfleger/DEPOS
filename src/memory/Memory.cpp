#include <BootInformation.hpp>
#include <architecture/CPU.hpp>
#include <memory/Memory.hpp>
#include <utility/Debug.hpp>

namespace QUARK {

void Memory::init() {
    constexpr auto PageSize = Traits<Memory>::PageSize;
    constexpr auto RamStart = Traits<MemoryMap>::RamStart;
    constexpr auto RamEnd   = Traits<MemoryMap>::RamEnd;

    TraceIn();

    Chunk payload(Traits<Payload>::Address, Traits<Payload>::Size);

    for (uintptr_t c = RamEnd - PageSize; c >= RamStart; c -= PageSize) {
        Chunk page(c, PageSize);
        if (page.overlaps(__kmm)) continue;
        if (page.overlaps(__bmm)) continue;
        if (page.overlaps(payload)) continue;
        s_allocator.insert(reinterpret_cast<void *>(page.start()), page.size());
    }

    TraceOut();
}

uintptr_t Memory::virt2phys(uintptr_t address) {
    if constexpr (Traits<Kernel>::Multitask) return address - (Traits<MemoryMap>::RamStart - __amm.start());
    return address;
}

void *Memory::alloc(size_t size) {
    if (size == 0) return nullptr;

    CPU::IRQ::Guard _;

    s_spin.acquire();

    void *chunk = s_allocator.remove(size);

    assert(chunk, "Out of Memory.");

    s_spin.release();

    return chunk;
}

void Memory::free(void *chunk, size_t size) {
    CPU::IRQ::Guard _;

    s_spin.acquire();

    assert(chunk != nullptr);

    s_allocator.insert(chunk, size);

    s_spin.release();
}

} // namespace QUARK
