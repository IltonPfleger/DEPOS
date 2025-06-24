#include <io/logger.hpp>
#include <io/uart.hpp>
#include <memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::Page *pages = nullptr;

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

void *operator new(unsigned long size, Memory::Heap &target) {
    using Block = Memory::Heap::Block;
    if (size == 0) return nullptr;

    Block *current = target.start;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size >= size + sizeof(Block) + 1) {
                uintptr_t addr = reinterpret_cast<uintptr_t>(current);
                addr += sizeof(Block) + size;
                Block *block  = reinterpret_cast<Block *>(addr);
                block->size   = current->size - size - sizeof(Block);
                block->free   = true;
                block->next   = current->next;
                current->size = size;
                current->next = block;
            }
            current->free = false;
            return reinterpret_cast<void *>(current + 1);
        }
        current = current->next;
    }

    void *page = Memory::kmalloc();
    if (!page) return nullptr;
    Block *block = reinterpret_cast<Block *>(page);
    block->size  = Machine::Memory::Page::SIZE - sizeof(Block);
    block->free  = true;
    block->next  = target.start;
    target.start = block;
    return ::operator new(size, target);
}

void operator delete(void *ptr, Memory::Heap &target) {
    using Block  = Memory::Heap::Block;
    Block *block = reinterpret_cast<Block *>(ptr) - 1;
    block->free  = true;

    Block *current  = target.start;
    Block *previous = nullptr;

    while (current) {
        if (current->free) {
            const uintptr_t tsize = current->size + sizeof(Block);
            if (tsize == Machine::Memory::Page::SIZE) {
                if (!previous) {
                    target.start = target.start->next;
                } else {
                    previous->next = current->next;
                }
                Block *temporary = current;
                current          = current->next;
                Memory::kfree(temporary);
                continue;
            }
            if (current->next && current->next->free) {
                current->size += current->next->size + sizeof(Block);
                current->next = current->next->next;
                continue;
            }
        }
        previous = current;
        current  = current->next;
    }
}
