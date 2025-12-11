#include <Spin.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <utils/Debug.hpp>

static Spin s_lock;

void Memory::init() {
    constexpr size_t PageSize = Traits<Memory>::PAGE_SIZE;
    uintptr_t KernelStart = __kmm.start;
    uintptr_t KernelEnd = __kmm.end;
    uintptr_t KernelSize = KernelEnd - KernelStart;
    uintptr_t ApplicationStart = __mm.start;
    uintptr_t ApplicationEnd = __mm.end;
    uintptr_t ApplicationSize = ApplicationEnd - ApplicationStart;
    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize);
    new (&s_allocator) Allocator();
    new (&s_lock) Spin;

    uintptr_t c = Traits<MemoryMap>::RamStart;
    for (; c < Traits<MemoryMap>::RamEnd; c += PageSize) {
        if (c + PageSize > KernelStart && c < KernelEnd)
            continue;
        if (c + PageSize > ApplicationStart && c < ApplicationEnd)
            continue;
        s_allocator.insert(reinterpret_cast<void *>(c), PageSize);
    }
    TraceOut();
}

void *Memory::kmalloc(size_t size) {
    TraceIn(size);
    s_lock.lock();
    void *page = s_allocator.remove(size);
    s_lock.unlock();
    ERROR(!page, "Out of pages.");
    TraceOut(page);
    return page;
}

void Memory::kfree(void *addr, size_t size) {
    TraceIn(addr, size);
    ERROR(addr == nullptr, "}\n");
    s_lock.lock();
    s_allocator.insert(addr, size);
    s_lock.unlock();
    TraceOut();
}
