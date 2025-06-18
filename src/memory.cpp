#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>

extern "C" const char KERNEL_START[];
extern "C" const char KERNEL_END[];
static Memory::MemoryBlock *pages = nullptr;

void Memory::init() {
    const uintptr_t KERNEL_SIZE = uintptr_t(KERNEL_END - KERNEL_START);
    const uintptr_t HEAP_START  = uintptr_t(KERNEL_END);
    const uintptr_t HEAP_SIZE   = Machine::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END    = HEAP_START + HEAP_SIZE;

    IO::out("Memory::init()\n");
    IO::out("KernelStart=%p\n", KERNEL_START);
    IO::out("KernelEnd=%p\n", KERNEL_END);
    IO::out("KernelSize=%d\n", KERNEL_SIZE);
    IO::out("HeapStart=%p\n", HEAP_START);
    IO::out("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    IO::out("HeapEnd=%p\n", HEAP_END);

    uintptr_t n_pages = 0;
    uintptr_t current = HEAP_START;
    while (current < HEAP_END) {
        MemoryBlock *block = reinterpret_cast<MemoryBlock *>(current);
        block->next        = pages;
        pages              = block;
        n_pages++;
        current += Machine::Memory::Page::SIZE;
    };

    IO::out("NumberOfPages=%d\n", n_pages);
    IO::out("Memory::init(done)\n");
}

void *Memory::kmalloc() {
    MemoryBlock *block = pages;
    if (block != nullptr) pages = block->next;
    IO::out("Memory::kmalloc()[return=%p]\n", block);
    return reinterpret_cast<void *>(block);
}

void Memory::kfree(void *addr) {
    if (addr == nullptr) return;
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(addr);
    block->next        = pages;
    pages              = block;
    IO::out("Memory::kfree(%p)\n", block);
}

void *Memory::Heap::malloc(uintptr_t size) {
    if (size == 0 || size % 8 != 0) return nullptr;

    HeapBlock *current = start;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size >= size + sizeof(HeapBlock) + 8) {
                uintptr_t addr = reinterpret_cast<uintptr_t>(current);
                addr += sizeof(HeapBlock) + size;
                HeapBlock *block = reinterpret_cast<HeapBlock *>(addr);
                block->size      = current->size - size - sizeof(HeapBlock);
                block->free      = true;
                block->next      = current->next;
                current->size    = size;
                current->next    = block;
            }
            current->free = false;
            return reinterpret_cast<void *>(current + 1);
        }
        current = current->next;
    }

    void *page = Memory::kmalloc();
    if (!page) return nullptr;
    HeapBlock *block = reinterpret_cast<HeapBlock *>(page);
    block->size      = Machine::Memory::Page::SIZE - sizeof(HeapBlock);
    block->free      = true;
    block->next      = start;
    start            = block;
    return malloc(size);
}

void Memory::Heap::free(void *addr) {
    HeapBlock *block = reinterpret_cast<HeapBlock *>(addr) - 1;
    block->free      = true;

    HeapBlock *current  = start;
    HeapBlock *previous = nullptr;

    while (current) {
        if (current->free) {
            const uintptr_t tsize = current->size + sizeof(HeapBlock);
            if (tsize == Machine::Memory::Page::SIZE) {
                if (!previous) {
                    start = start->next;
                } else {
                    previous->next = current->next;
                }
                HeapBlock *temporary = current;
                current              = current->next;
                Memory::kfree(temporary);
                continue;
            }
            if (current->next && current->next->free) {
                current->size += current->next->size + sizeof(HeapBlock);
                current->next = current->next->next;
                continue;
            }
        }
        previous = current;
        current  = current->next;
    }
}
