#include <IO/Debug.hpp>
#include <Memory.hpp>
#include <Spin.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Spin _lock;

void Memory::init() {
    TRACE(__PRETTY_FUNCTION__, "{");

    constexpr unsigned int PAGE_SIZE = Traits::Memory::Page::SIZE;
    char* RAM_BASE = reinterpret_cast<char*>(Traits::Memory::RAM_BASE);
    char* RAM_END                    = reinterpret_cast<char*>(Traits::Memory::RAM_END);
    char* KERNEL_START               = const_cast<char*>(__KERNEL_START__);
    char* KERNEL_END                 = const_cast<char*>(__KERNEL_END__);
    const unsigned int KERNEL_SIZE   = KERNEL_END - KERNEL_START;
    const unsigned int HEAP_SIZE     = Traits::Memory::SIZE - KERNEL_SIZE;

    char* c;
    for (c = RAM_BASE; c + PAGE_SIZE < RAM_END; c += PAGE_SIZE) {
        if (c >= KERNEL_START && c < KERNEL_END) continue;
        pages.insert(reinterpret_cast<Page*>(c));
    }

    TRACE("KernelStart=", static_cast<void*>(KERNEL_START), ", ");
    TRACE("KernelEnd=", static_cast<void*>(KERNEL_END), ", ");
    TRACE("KernelSize=", KERNEL_SIZE, ", ");
    TRACE("HeapSize=", HEAP_SIZE);
    TRACE("}\n");
}

void* Memory::kmalloc() {
    TRACE(__PRETTY_FUNCTION__, "{");
    _lock.lock();
    Page* page = pages.remove();
    _lock.unlock();
    ERROR(!page, "Out of pages}");
    TRACE("return=", page, "}\n");
    return page;
}

void Memory::kfree(void* addr) {
    TRACE(__PRETTY_FUNCTION__, "{", addr);
    ERROR(addr == nullptr, "}\n");
    _lock.lock();
    pages.insert(reinterpret_cast<Page*>(addr));
    _lock.unlock();
    TRACE("}\n");
}
