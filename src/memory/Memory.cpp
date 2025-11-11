#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>

static Spin _lock;

void Memory::init() {
    constexpr size_t PageSize   = Traits::Memory::Page::SIZE;
    constexpr uintptr_t RamBase = Traits::Memory::RAM_BASE;
    constexpr uintptr_t RamEnd  = Traits::Memory::RAM_END;
    uintptr_t KernelStart       = reinterpret_cast<uintptr_t>(__mm.kernel.start);
    uintptr_t KernelEnd         = reinterpret_cast<uintptr_t>(__mm.kernel.end);
    uintptr_t ApplicationStart  = reinterpret_cast<uintptr_t>(__mm.app.start);
    uintptr_t ApplicationEnd    = reinterpret_cast<uintptr_t>(__mm.app.end);
    uintptr_t KernelSize        = KernelEnd - KernelStart;
    uintptr_t ApplicationSize   = ApplicationEnd - ApplicationStart;

    TraceIn(__mm.kernel.start, __mm.kernel.end, KernelSize, __mm.app.start, __mm.app.end, ApplicationSize);

    uintptr_t c;
    for (c = RamEnd - PageSize; c > RamBase; c -= PageSize) {
        if (c >= KernelStart && c < KernelEnd) continue;
        if (c >= ApplicationStart && c < ApplicationEnd) continue;
        buddy_.insert(reinterpret_cast<void *>(c), PageSize);
    }

    TraceOut();
}

void *Memory::kmalloc(size_t size) {
    //TraceIn(size);
    _lock.lock();
    void *page = buddy_.remove(size);
    _lock.unlock();
    ERROR(!page, "Out of pages.");
    //TraceOut(page);
    return page;
}

void Memory::kfree(void *addr, size_t size) {
    TraceIn(addr, size);
    ERROR(addr == nullptr, "}\n");
    _lock.lock();
    buddy_.insert(addr, size);
    _lock.unlock();
    TraceOut();
}
