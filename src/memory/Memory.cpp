#include <ELF.hpp>
#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <memory/Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Spin _lock;

void Memory::init() {
    constexpr size_t PageSize   = Traits::Memory::Page::SIZE;
    constexpr uintptr_t RamBase = Traits::Memory::RAM_BASE;
    constexpr uintptr_t RamEnd  = Traits::Memory::RAM_END;
    uintptr_t KernelStart       = reinterpret_cast<uintptr_t>(const_cast<char *>(__KERNEL_START__));
    uintptr_t KernelEnd         = reinterpret_cast<uintptr_t>(const_cast<char *>(__KERNEL_END__));
    KernelEnd                   = (KernelEnd + (PageSize - 1)) & ~(PageSize - 1);
    uintptr_t KernelSize        = KernelEnd - KernelStart;
    TRACE(__PRETTY_FUNCTION__, "{");

    uintptr_t c;
    for (c = RamBase; c + PageSize < RamEnd; c += PageSize) {
        if (c >= KernelStart && c < KernelEnd) continue;
        m_buddy.insert(reinterpret_cast<void *>(c), PageSize);
    }

    TRACE("KernelStart=", KernelStart, ", ");
    TRACE("KernelEnd=", KernelEnd, ", ");
    TRACE("KernelSize=", KernelSize);
    TRACE("}\n");
}

void *Memory::kmalloc() {
    TRACE(__PRETTY_FUNCTION__, "{");
    _lock.lock();
    void *page = m_buddy.remove(Traits::Memory::Page::SIZE);
    _lock.unlock();
    ERROR(!page, "Out of pages}");
    TRACE("return=", page, "}\n");
    return page;
}

void Memory::kfree(void *addr) {
    TRACE(__PRETTY_FUNCTION__, "{", "a0=", addr);
    ERROR(addr == nullptr, "}\n");
    _lock.lock();
    m_buddy.insert(addr, Traits::Memory::Page::SIZE);
    _lock.unlock();
    TRACE("}\n");
}
