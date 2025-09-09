#include <IO/Debug.hpp>
#include <Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::PageList page_list;

// static Memory::Page *page_list                     = nullptr;
static Memory::Heap heaps[Memory::Role::COUNT] = {{}};
static Spin _lock;
static Spin _plock;

static constexpr unsigned long calculateOrder(unsigned long bytes) {
    unsigned long order = 1;
    while ((1UL << order) < bytes) order++;
    return order;
}

static unsigned char &getRole(void *page) {
    return *(reinterpret_cast<unsigned char *>(page) + Traits::Memory::Page::SIZE - 1);
}

void Memory::init() {
    const uintptr_t PSIZE        = Traits::Memory::Page::SIZE;
    const uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    const uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_START   = (KERNEL_END + PSIZE + 1) & ~(PSIZE - 1);
    const uintptr_t HEAP_SIZE    = Traits::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END     = HEAP_START + HEAP_SIZE;

    TRACE("[Memory::init]{\n");
    TRACE("KernelStart=%p\n", KERNEL_START);
    TRACE("KernelEnd=%p\n", KERNEL_END);
    TRACE("KernelSize=%d\n", KERNEL_SIZE);
    TRACE("HeapStart=%p\n", HEAP_START);
    TRACE("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    TRACE("HeapEnd=%p\n", HEAP_END);

    for (uintptr_t c = HEAP_START; c + PSIZE < HEAP_END; c += PSIZE) {
        page_list.insert(reinterpret_cast<Page *>(c));
    };

    TRACE("}\n");
}

void *Memory::kmalloc() {
    _plock.lock();
    Page *page = page_list.remove();
    _plock.unlock();
    ERROR(page == nullptr, "[Memory::kmalloc] Out of Memory.");
    TRACE("[Memory::kmalloc] {return=%p}\n", page);
    return page;
}

void Memory::kfree(void *addr) {
    ERROR(addr == nullptr, "[Memory::free] Free nullptr.");
    _plock.lock();
    page_list.insert(reinterpret_cast<Page *>(addr));
    _plock.unlock();
    TRACE("[Memory::kfree] %p\n", addr);
}

void *operator new(unsigned long, void *ptr) { return ptr; }

void *operator new(unsigned long bytes, Memory::Role role) {
    if (bytes == 0 || bytes >= Traits::Memory::Page::SIZE) return 0;

    auto order = calculateOrder(bytes);
    auto i     = order;

    _lock.lock();
    while (i < Traits::Memory::Page::ORDER && !heaps[role].blocks[i]) i++;
    if (i == Traits::Memory::Page::ORDER) {
        auto page = reinterpret_cast<Memory::Block *>(Memory::kmalloc());
        ERROR(!page, "[operator new] Out of memory.");
        getRole(page)         = role;
        i                     = Traits::Memory::Page::ORDER;
        page->next            = heaps[role].blocks[i];
        heaps[role].blocks[i] = reinterpret_cast<Memory::Block *>(page);
    }

    while (i != order) {
        Memory::Block *block    = heaps[role].blocks[i];
        heaps[role].blocks[i--] = block->next;

        block->next           = reinterpret_cast<Memory::Block *>(reinterpret_cast<uintptr_t>(block) ^ (1 << i));
        block->next->next     = heaps[role].blocks[i];
        heaps[role].blocks[i] = block;
    }

    Memory::Block *block      = heaps[role].blocks[order];
    heaps[role].blocks[order] = block->next;
    _lock.unlock();
    return reinterpret_cast<void *>(block);
}

void operator delete(void *ptr, unsigned long bytes) {
    if (!ptr) return;

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto mask  = ~0ULL << Traits::Memory::Page::ORDER;
    auto role  = getRole(reinterpret_cast<void *>(addr & mask));
    auto order = calculateOrder(bytes);

    _lock.lock();
    while (order < Traits::Memory::Page::ORDER) {
        auto buddy = addr ^ (1UL << order);

        Memory::Block **current = &heaps[role].blocks[order];

        while (*current && reinterpret_cast<uintptr_t>(*current) != buddy) {
            current = &(*current)->next;
        }

        if (!*current) break;

        (*current) = (*current)->next;

        if (buddy < addr) addr = buddy;
        order++;
    }

    Memory::Block *new_block  = reinterpret_cast<Memory::Block *>(addr);
    new_block->next           = heaps[role].blocks[order];
    heaps[role].blocks[order] = new_block;

    void *free = nullptr;
    if (order == Traits::Memory::Page::ORDER) {
        free                      = heaps[role].blocks[order];
        heaps[role].blocks[order] = heaps[role].blocks[order]->next;
    }

    _lock.unlock();
    if (free) Memory::kfree(free);
}
