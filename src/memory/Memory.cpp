#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>

static Spin _lock;

void Memory::init() {
    //(void)__kmm;
    // Console::out << reinterpret_cast<void *>(__kmm.text.start) << "\n";
    constexpr size_t PageSize   = Traits::Memory::Page::SIZE;
    constexpr uintptr_t RamBase = Traits::Memory::RAM_BASE;
    constexpr uintptr_t RamEnd  = Traits::Memory::RAM_END;
    uintptr_t KernelStart       = __kmm.start;
    uintptr_t KernelEnd         = __kmm.end;
    uintptr_t KernelSize        = KernelEnd - KernelStart;
    uintptr_t ApplicationStart  = __mm.start;
    uintptr_t ApplicationEnd    = __mm.end;
    uintptr_t ApplicationSize   = ApplicationEnd - ApplicationStart;

    Console::out << ApplicationStart << " " << Traits::Application::ADDR << "\n";

    Console::out << reinterpret_cast<void *>(ApplicationStart) << " "
                 << reinterpret_cast<void *>(Traits::Application::ADDR) << "\n";

    TraceIn(KernelStart, KernelEnd, KernelSize, ApplicationStart, ApplicationEnd, ApplicationSize);

    uintptr_t c;
    for (c = RamEnd - PageSize; c > RamBase; c -= PageSize) {
        if (c >= KernelStart && c < KernelEnd)
            continue;
        if (c >= ApplicationStart && c < ApplicationEnd)
            continue;
        buddy_.insert(reinterpret_cast<void *>(c), PageSize);
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
