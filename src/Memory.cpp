#include <IO/Debug.hpp>
#include <Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::Page *pages               = nullptr;
static Memory::Heap heaps[Memory::COUNT] = {0, 0};

static constexpr unsigned long ORDER(unsigned long bytes) {
    unsigned long order = 1;
    while ((1UL << order) < bytes) order++;
    return order;
}

static constexpr unsigned char &ROLE(uintptr_t page) {
    return *(reinterpret_cast<unsigned char *>(page) + Traits<Memory>::Page::SIZE - 1);
}

void Memory::init() {
    const uintptr_t PSIZE        = Traits<Memory>::Page::SIZE;
    const uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    const uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_START   = (KERNEL_END + PSIZE + 1) & ~(PSIZE - 1);
    const uintptr_t HEAP_SIZE    = Machine::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END     = HEAP_START + HEAP_SIZE;

    TRACE("Memory::init()\n");
    TRACE("KernelStart=%p\n", KERNEL_START);
    TRACE("KernelEnd=%p\n", KERNEL_END);
    TRACE("KernelSize=%d\n", KERNEL_SIZE);
    TRACE("HeapStart=%p\n", HEAP_START);
    TRACE("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    TRACE("HeapEnd=%p\n", HEAP_END);

    int n_pages       = 0;
    uintptr_t current = HEAP_START;
    while (current + PSIZE < HEAP_END) {
        Page *page = reinterpret_cast<Page *>(current);
        page->next = pages;
        pages      = page;
        n_pages++;
        current += Traits<Memory>::Page::SIZE;
    };

    TRACE("NumberOfPages=%d\n", n_pages);
    TRACE("Memory::init(done)\n");
}

void *Memory::kmalloc() {
    lock.lock();
    Page *page = pages;
    if (page != nullptr) pages = page->next;
    TRACE("[Memory::kmalloc] {return=%p}\n", page);
    lock.unlock();
    return reinterpret_cast<void *>(page);
}

void Memory::kfree(void *addr) {
    Page *page = reinterpret_cast<Page *>(addr);
    lock.lock();
    ERROR(page == nullptr, "[Memory::free] Free nullptr");
    page->next = pages;
    pages      = page;
    TRACE("Memory::kfree(%p)\n", page);
    lock.unlock();
}

void *operator new(unsigned long, void *ptr) { return ptr; }

void *operator new(unsigned long bytes, Memory::Role role) {
    if (bytes == 0 || bytes >= Traits<Memory>::Page::SIZE) return 0;

    auto order = ORDER(bytes);
    auto i     = order;

    heaps[role].lock.lock();
    while (i < Traits<Memory>::Page::ORDER && !heaps[role].blocks[i]) i++;
    if (i == Traits<Memory>::Page::ORDER) {
        auto raw  = reinterpret_cast<uintptr_t>(Memory::kmalloc());
        auto page = reinterpret_cast<Memory::Block *>(raw);
        if (!page) {
            heaps[role].lock.unlock();
            return nullptr;
        };
        ROLE(raw)             = role;
        i                     = Traits<Memory>::Page::ORDER;
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
    heaps[role].lock.unlock();
    return reinterpret_cast<void *>(block);
}

void operator delete(void *ptr, unsigned long bytes) {
    if (!ptr) return;

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto mask  = ~0ULL << Traits<Memory>::Page::ORDER;
    auto role  = ROLE(addr & mask);
    auto order = ORDER(bytes);

    heaps[role].lock.lock();
    while (order < Traits<Memory>::Page::ORDER) {
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

    if (order == Traits<Memory>::Page::ORDER) {
        Memory::kfree(heaps[role].blocks[order]);
        heaps[role].blocks[order] = nullptr;
    }
    heaps[role].lock.unlock();
}
