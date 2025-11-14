#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>

static Spin _lock;

void Memory::init() {
    constexpr size_t PageSize   = Traits<Memory>::PAGE_SIZE;
    constexpr uintptr_t RamBase = Traits<MemoryMap>::RAM_BASE;
    constexpr uintptr_t RamEnd  = Traits<MemoryMap>::RAM_END;
    uintptr_t KernelStart       = __kmm.start;
    uintptr_t KernelEnd         = __kmm.end;
    uintptr_t KernelSize        = KernelEnd - KernelStart;
    uintptr_t ApplicationStart  = __mm.start;
    uintptr_t ApplicationEnd    = __mm.end;
    uintptr_t ApplicationSize   = ApplicationEnd - ApplicationStart;
    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart,
            ApplicationEnd, ApplicationSize);
    if (buddy_.empty()) {
        uintptr_t c;
        for (c = RamEnd - PageSize; c > RamBase; c -= PageSize) {
            if (c >= KernelStart && c < KernelEnd)
                continue;
            if (c >= ApplicationStart && c < ApplicationEnd)
                continue;
            buddy_.insert(reinterpret_cast<void *>(c), PageSize);
        }
    }
    TraceOut();
}

void *Memory::kmalloc(size_t size) {
    TraceIn(size);
    _lock.lock();
    void *page = buddy_.remove(size);
    _lock.unlock();
    ERROR(!page, "Out of pages.");
    TraceOut(page);
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
