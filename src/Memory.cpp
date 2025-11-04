#include <ELF.hpp>
#include <IO/Debug.hpp>
#include <Memory.hpp>
#include <Spin.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Spin _lock;

void Memory::init() {
    TRACE(__PRETTY_FUNCTION__, "{");
    while (1);
    uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(const_cast<char *>(__KERNEL_START__));
    uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(const_cast<char *>(__KERNEL_END__));
    KERNEL_END             = (KERNEL_END + (Traits::Memory::Page::SIZE - 1)) & ~(Traits::Memory::Page::SIZE - 1);
    uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;

    // static char _buffer[sizeof(Buddy)];
    // void *buffer = reinterpret_cast<void *>(_buffer);
    // m_buddy      = new (buffer) Buddy(reinterpret_cast<void *>(KERNEL_END), Traits::Memory::SIZE - KERNEL_SIZE);

    // constexpr unsigned int PAGE_SIZE = Traits::Memory::Page::SIZE;
    // constexpr uintptr_t RAM_BASE     = Traits::Memory::RAM_BASE;
    // constexpr uintptr_t RAM_END      = Traits::Memory::RAM_END;

    //// TODO: Move this math to another function
    // ELF* elf             = reinterpret_cast<ELF*>(KERNEL_END);
    // uintptr_t HEAP_START = KERNEL_END;
    // while (elf->valid()) {
    //     size_t size = elf->size();
    //     HEAP_START += size;
    //     elf = reinterpret_cast<ELF*>(reinterpret_cast<uintptr_t>(elf) + size);
    // }

    // uintptr_t c;
    // for (c = RAM_BASE; c + PAGE_SIZE < RAM_END; c += PAGE_SIZE) {
    //     if (c >= KERNEL_START && c < HEAP_START) continue;
    //     pages.insert(reinterpret_cast<Page*>(c));
    // }

    TRACE("KernelStart=", KERNEL_START, ", ");
    TRACE("KernelEnd=", KERNEL_END, ", ");
    TRACE("KernelSize=", KERNEL_SIZE);
    TRACE("}\n");
}

void *Memory::kmalloc() {
    TRACE(__PRETTY_FUNCTION__, "{");
    _lock.lock();
    void *page = m_buddy->remove(Traits::Memory::Page::SIZE);
    _lock.unlock();
    ERROR(!page, "Out of pages}");
    TRACE("return=", page, "}\n");
    return page;
}

void Memory::kfree(void *addr) {
    TRACE(__PRETTY_FUNCTION__, "{", addr);
    ERROR(addr == nullptr, "}\n");
    _lock.lock();
    m_buddy->insert(addr, Traits::Memory::Page::SIZE);
    _lock.unlock();
    TRACE("}\n");
}
