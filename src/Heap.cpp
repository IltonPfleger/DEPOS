#include <Heap.hpp>
#include <IO/Debug.hpp>
#include <Memory.hpp>

unsigned long Heap::index_of(unsigned long bytes) {
    unsigned long order = 1;
    while ((1UL << order) < bytes) order++;
    return order;
}

void *Heap::alloc(unsigned long bytes) {
    if (bytes == 0 || bytes >= Traits::Memory::Page::SIZE) return 0;

    auto order = index_of(bytes);
    auto i     = order;

    _lock.lock();
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
    _lock.unlock();
    return reinterpret_cast<void *>(block);
}

void *Heap::free(void *ptr, unsigned long bytes) {
    if (!ptr) return nullptr;

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto order = index_of(bytes);

    _lock.lock();
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

    _lock.unlock();
    return nullptr;
}
