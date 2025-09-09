#include <Heap.hpp>
#include <IO/Debug.hpp>
#include <Memory.hpp>
#include <Spin.hpp>

static Spin _lock;
Heap Heap::SYSTEM;

unsigned long Heap::index_of(unsigned long bytes) {
    unsigned long order = 1;
    while ((1UL << order) < bytes) order++;
    return order;
}

void *Heap::alloc(unsigned long bytes) {
    if (bytes == 0 || bytes >= Traits::Memory::Page::SIZE) return 0;

    auto order = index_of(bytes);
    auto i     = order;

    while (i < Traits::Memory::Page::ORDER && !_chunks[i]) i++;
    if (i == Traits::Memory::Page::ORDER) {
        auto page = reinterpret_cast<Chunk *>(Memory::kmalloc());
        ERROR(!page, "[operator new] Out of memory.");
        i          = Traits::Memory::Page::ORDER;
        page->next = _chunks[i];
        _chunks[i] = reinterpret_cast<Chunk *>(page);
    }

    while (i != order) {
        Chunk *block = _chunks[i];
        _chunks[i--] = block->next;

        block->next       = reinterpret_cast<Chunk *>(reinterpret_cast<uintptr_t>(block) ^ (1 << i));
        block->next->next = _chunks[i];
        _chunks[i]        = block;
    }

    Chunk *block   = _chunks[order];
    _chunks[order] = block->next;
    return reinterpret_cast<void *>(block);
}

void *Heap::free(void *ptr, unsigned long bytes) {
    if (!ptr) return nullptr;

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto order = index_of(bytes);

    while (order < Traits::Memory::Page::ORDER) {
        auto buddy = addr ^ (1UL << order);

        Chunk **current = &_chunks[order];

        while (*current && reinterpret_cast<uintptr_t>(*current) != buddy) {
            current = &(*current)->next;
        }

        if (!*current) break;

        (*current) = (*current)->next;

        if (buddy < addr) addr = buddy;
        order++;
    }

    Chunk *new_block = reinterpret_cast<Chunk *>(addr);
    new_block->next  = _chunks[order];
    _chunks[order]   = new_block;

    if (order == Traits::Memory::Page::ORDER) {
        Memory::kfree(_chunks[order]);
        _chunks[order] = _chunks[order]->next;
    }
    return nullptr;
}

void *operator new(unsigned long, void *ptr) { return ptr; }

void *operator new[](unsigned long bytes, Heap &heap) {
    _lock.lock();
    void *addr = heap.alloc(bytes);
    if (!addr) {
        heap.free(Memory::kmalloc(), Traits::Memory::Page::SIZE);
        addr = heap.alloc(bytes);
    }
    _lock.unlock();
    return addr;
}

void *operator new(unsigned long bytes, Heap &heap) { return operator new[](bytes, heap); }

void *operator new(unsigned long bytes) {
    if constexpr (Traits::System::MULTITASK) {
        // return operator new[](bytes, Thread::running()->task->heap);
        return operator new[](bytes, Heap::SYSTEM);
    } else {
        return operator new[](bytes, Heap::SYSTEM);
    }
}
