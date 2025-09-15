#include <IO/Debug.hpp>
#include <Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];

Memory::PageList Memory::pages;
static Spin _lock;

void Memory::init() {
    // TRACE("%d\n", Traits::Memory::SIZE / Traits::Memory::Page::SIZE);
    // while (1);
    const uintptr_t PAGE_SIZE    = Traits::Memory::Page::SIZE;
    const uintptr_t RAM_BASE     = Traits::System::RAM_BASE;
    const uintptr_t RAM_END      = Traits::System::RAM_END;
    const uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    const uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_SIZE    = Traits::Memory::SIZE - KERNEL_SIZE;

    uintptr_t c = RAM_BASE;
    for (; c + PAGE_SIZE < RAM_END; c += PAGE_SIZE) {
        if (c >= KERNEL_START && c < KERNEL_END) continue;
        pages.insert(reinterpret_cast<Page *>(c));
    }

    TRACE("[Memory::init]{\n");
    TRACE("KernelStart=%p\n", KERNEL_START);
    TRACE("KernelEnd=%p\n", KERNEL_END);
    TRACE("KernelSize=%d\n", KERNEL_SIZE);
    TRACE("HeapStart=%p\n", RAM_BASE);
    TRACE("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    TRACE("HeapEnd=%p\n", c);
    TRACE("}\n");
}

void *Memory::kmalloc() {
    _lock.lock();
    Page *page = pages.remove();
    _lock.unlock();
    ERROR(page == nullptr, "[Memory::kmalloc] Out of Memory.");
    TRACE("[Memory::kmalloc] {return=%p}\n", page);
    return page;
}

void Memory::kfree(void *addr) {
    ERROR(addr == nullptr, "[Memory::free] Free nullptr.");
    _lock.lock();
    pages.insert(reinterpret_cast<Page *>(addr));
    _lock.unlock();
    TRACE("[Memory::kfree] %p\n", addr);
}
