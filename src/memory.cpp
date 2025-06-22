#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static Memory::PBlock *pages = nullptr;

void Memory::init() {
    const uintptr_t PSIZE        = Machine::Memory::Page::SIZE;
    const uintptr_t KERNEL_START = uintptr_t(__KERNEL_START__);
    const uintptr_t KERNEL_END   = uintptr_t(__KERNEL_END__);
    const uintptr_t KERNEL_SIZE  = KERNEL_END - KERNEL_START;
    const uintptr_t HEAP_START   = (KERNEL_END + PSIZE + 1) & ~(PSIZE - 1);
    const uintptr_t HEAP_SIZE    = Machine::Memory::SIZE - KERNEL_SIZE;
    const uintptr_t HEAP_END     = HEAP_START + HEAP_SIZE;

    IO::out("Memory::init()\n");
    IO::out("KernelStart=%p\n", KERNEL_START);
    IO::out("KernelEnd=%p\n", KERNEL_END);
    IO::out("KernelSize=%d\n", KERNEL_SIZE);
    IO::out("HeapStart=%p\n", HEAP_START);
    IO::out("HeapSize=%d\n", HEAP_SIZE / (1024 * 1024));
    IO::out("HeapEnd=%p\n", HEAP_END);

    uintptr_t n_pages = 0;
    uintptr_t current = HEAP_START;
    while (current + PSIZE < HEAP_END) {
        PBlock *block = reinterpret_cast<PBlock *>(current);
        block->next   = pages;
        pages         = block;
        n_pages++;
        current += Machine::Memory::Page::SIZE;
    };

    IO::out("NumberOfPages=%d\n", n_pages);
    IO::out("Memory::init(done)\n");
}

void *Memory::kmalloc() {
    PBlock *block = pages;
    if (block != nullptr) pages = block->next;
    IO::out("Memory::kmalloc()[return=%p]\n", block);
    return reinterpret_cast<void *>(block);
}

void Memory::kfree(void *addr) {
    if (addr == nullptr) return;
    PBlock *block = reinterpret_cast<PBlock *>(addr);
    block->next   = pages;
    pages         = block;
    IO::out("Memory::kfree(%p)\n", block);
}

void *operator new(unsigned long size, Memory::Heap &target) {
    using Block = Memory::Heap::HBlock;
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
    using Block  = Memory::Heap::HBlock;
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
