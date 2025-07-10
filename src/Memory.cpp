#include <IO/Debug.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::Page *pages = nullptr;
static Memory::Heap heaps[Memory::COUNT];

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
    const uintptr_t KERNEL_START = uintptr_t(__KERNEL_START__);
    const uintptr_t KERNEL_END   = uintptr_t(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_START   = (KERNEL_END + PSIZE + 1) & ~(PSIZE - 1);
    const uintptr_t HEAP_SIZE    = Machine::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END     = HEAP_START + HEAP_SIZE;

    Logger::println("Memory::init()\n");
    Logger::println("KernelStart=%p\n", KERNEL_START);
    Logger::println("KernelEnd=%p\n", KERNEL_END);
    Logger::println("KernelSize=%d\n", KERNEL_SIZE);
    Logger::println("HeapStart=%p\n", HEAP_START);
    Logger::println("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    Logger::println("HeapEnd=%p\n", HEAP_END);

    uintptr_t n_pages = 0;
    uintptr_t current = HEAP_START;
    while (current + PSIZE < HEAP_END) {
        Page *page = reinterpret_cast<Page *>(current);
        page->next = pages;
        pages      = page;
        n_pages++;
        current += Traits<Memory>::Page::SIZE;
    };

    Logger::println("NumberOfPages=%d\n", n_pages);
    Logger::println("Memory::init(done)\n");
}

void *Memory::kmalloc() {
    Page *page = pages;
    if (page != nullptr) pages = page->next;
    Logger::println("Memory::kmalloc()[return=%p]\n", page);
    return reinterpret_cast<void *>(page);
}

void Memory::kfree(void *addr) {
    if (addr == nullptr) return;
    Page *page = reinterpret_cast<Page *>(addr);
    page->next = pages;
    pages      = page;
    Logger::println("Memory::kfree(%p)\n", page);
}

void *operator new(unsigned long, void *ptr) { return ptr; }
void *operator new(unsigned long bytes, Memory::Role role) {
    if (bytes == 0 || bytes >= Traits<Memory>::Page::SIZE) return 0;

    auto order = ORDER(bytes);
    auto i     = order;
    while (i < Traits<Memory>::Page::ORDER && !heaps[role][i]) i++;

    if (i == Traits<Memory>::Page::ORDER) {
        auto raw  = reinterpret_cast<uintptr_t>(Memory::kmalloc());
        auto page = reinterpret_cast<Memory::Block *>(raw);
        if (!page) return nullptr;
        ROLE(raw)      = role;
        i              = Traits<Memory>::Page::ORDER;
        page->next     = heaps[role][i];
        heaps[role][i] = reinterpret_cast<Memory::Block *>(page);
    }

    while (i != order) {
        Memory::Block *block = heaps[role][i];
        heaps[role][i--]     = block->next;

        block->next       = reinterpret_cast<Memory::Block *>(reinterpret_cast<uintptr_t>(block) ^ (1 << i));
        block->next->next = heaps[role][i];
        heaps[role][i]    = block;
    }

    Memory::Block *block = heaps[role][order];
    heaps[role][order]   = block->next;
    return reinterpret_cast<void *>(block);
}

void operator delete(void *) { ERROR(true, "::operator delete(void*)"); }
void operator delete(void *ptr, unsigned long bytes) {
    if (!ptr) return;

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto mask  = ~0ULL << Traits<Memory>::Page::ORDER;
    auto role  = ROLE(addr & mask);
    auto order = ORDER(bytes);

    while (order < Traits<Memory>::Page::ORDER) {
        auto buddy = addr ^ (1UL << order);

        Memory::Block **current = &heaps[role][order];

        while (*current && reinterpret_cast<uintptr_t>(*current) != buddy) {
            current = &(*current)->next;
        }

        if (!*current) break;

        (*current) = (*current)->next;

        if (buddy < addr) addr = buddy;
        order++;
    }

    Memory::Block *new_block = reinterpret_cast<Memory::Block *>(addr);
    new_block->next          = heaps[role][order];
    heaps[role][order]       = new_block;

    if (order == Traits<Memory>::Page::ORDER) {
        Memory::kfree(heaps[role][order]);
        heaps[role][order] = nullptr;
    }
}
