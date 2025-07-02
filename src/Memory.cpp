#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::Page *pages = nullptr;
static Memory::Heap heaps[Memory::COUNT];

void Memory::init() {
    const uintptr_t PSIZE        = Machine::Memory::Page::SIZE;
    const uintptr_t KERNEL_START = uintptr_t(__KERNEL_START__);
    const uintptr_t KERNEL_END   = uintptr_t(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_START   = (KERNEL_END + PSIZE + 1) & ~(PSIZE - 1);
    const uintptr_t HEAP_SIZE    = Machine::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END     = HEAP_START + HEAP_SIZE;

    Logger::log("Memory::init()\n");
    Logger::log("KernelStart=%p\n", KERNEL_START);
    Logger::log("KernelEnd=%p\n", KERNEL_END);
    Logger::log("KernelSize=%d\n", KERNEL_SIZE);
    Logger::log("HeapStart=%p\n", HEAP_START);
    Logger::log("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    Logger::log("HeapEnd=%p\n", HEAP_END);

    uintptr_t n_pages = 0;
    uintptr_t current = HEAP_START;
    while (current + PSIZE < HEAP_END) {
        Page *page = reinterpret_cast<Page *>(current);
        page->next = pages;
        pages      = page;
        n_pages++;
        current += Machine::Memory::Page::SIZE;
    };

    Logger::log("NumberOfPages=%d\n", n_pages);
    Logger::log("Memory::init(done)\n");
}

void *Memory::kmalloc() {
    Page *page = pages;
    if (page != nullptr) pages = page->next;
    Logger::log("Memory::kmalloc()[return=%p]\n", page);
    return reinterpret_cast<void *>(page);
}

void Memory::kfree(void *addr) {
    if (addr == nullptr) return;
    Page *page = reinterpret_cast<Page *>(addr);
    page->next = pages;
    pages      = page;
    Logger::log("Memory::kfree(%p)\n", page);
}

void *operator new(unsigned long bytes, void *ptr) { return ptr; }

void *operator new(unsigned long bytes, Memory::Role role) {
    using Block = Memory::Block;
    if (bytes == 0) return 0;

    Block *current = heaps[role].start;
    while (current) {
        if (current->flags.free && current->size >= bytes) {
            if (current->size >= bytes + sizeof(Block) + 1) {
                uintptr_t addr = reinterpret_cast<uintptr_t>(current);
                addr += sizeof(Block) + bytes;
                Block *block      = reinterpret_cast<Block *>(addr);
                block->size       = current->size - bytes - sizeof(Block);
                block->flags.free = true;
                block->flags.role = role;
                block->next       = current->next;
                current->size     = bytes;
                current->next     = block;
            }
            current->flags.free = false;
            return reinterpret_cast<void *>(current + 1);
        }
        current = current->next;
    }

    void *page = Memory::kmalloc();
    if (!page) return 0;
    Block *block      = reinterpret_cast<Block *>(page);
    block->size       = Machine::Memory::Page::SIZE - sizeof(Block);
    block->flags.free = true;
    block->next       = heaps[role].start;
    heaps[role].start = block;
    return ::operator new(bytes, role);
}

void operator delete(void *ptr, unsigned long) {
    using Block       = Memory::Block;
    Block *block      = reinterpret_cast<Block *>(ptr) - 1;
    Memory::Role role = static_cast<Memory::Role>(block->flags.role);
    block->flags.free = true;

    Block *current  = heaps[role].start;
    Block *previous = nullptr;
    while (current) {
        if (current->flags.free) {
            const uintptr_t tsize = current->size + sizeof(Block);
            if (tsize == Machine::Memory::Page::SIZE) {
                if (!previous) {
                    heaps[role].start = heaps[role].start->next;
                } else {
                    previous->next = current->next;
                }
                Block *temporary = current;
                current          = current->next;
                Memory::kfree(temporary);
                continue;
            }
            if (current->next && current->next->flags.free) {
                current->size += current->next->size + sizeof(Block);
                current->next = current->next->next;
                continue;
            }
        }
        previous = current;
        current  = current->next;
    }
}

void operator delete(void *ptr) { ::operator delete(ptr, 0); }
