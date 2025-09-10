#include <Heap.hpp>
#include <IO/Debug.hpp>
#include <Memory.hpp>
#include <Spin.hpp>
#include <Thread.hpp>

static Spin _lock;
Heap Heap::SYSTEM;

unsigned long Heap::index_of(unsigned long bytes) {
    unsigned long order = 1;
    while ((1UL << order) < bytes) order++;
    return order;
}

void *Heap::alloc(unsigned long bytes) {
    ERROR(bytes == 0 || bytes == CAPSTONE, "[Heap::alloc] Invalid size.");

    unsigned long order = index_of(bytes);
    unsigned long i     = order;

    while (i < CAPSTONE && !_chunks[i]) i++;
    if (i == CAPSTONE && !_chunks[i]) return nullptr;

    while (i != order) {
        Chunk *block = _chunks[i];
        _chunks[i--] = block->next;

        block->next       = reinterpret_cast<Chunk *>(reinterpret_cast<uintptr_t>(block) ^ (1UL << i));
        block->next->next = _chunks[i];
        _chunks[i]        = block;
    }

    Chunk *block   = _chunks[order];
    _chunks[order] = block->next;
    return block;
}

void Heap::grow(void *ptr, unsigned long bytes) {
    auto order       = index_of(bytes);
    Chunk *new_block = reinterpret_cast<Chunk *>(ptr);
    new_block->next  = _chunks[order];
    _chunks[order]   = new_block;
}

void *Heap::free(void *ptr, unsigned long bytes) {
    ERROR(!ptr, "[Heap::free] Invalid pointer.");

    auto addr  = reinterpret_cast<uintptr_t>(ptr);
    auto order = index_of(bytes);

    while (order < CAPSTONE) {
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

    // if (order == Traits::Memory::Page::ORDER) {
    //     Memory::kfree(_chunks[order]);
    //     _chunks[order] = _chunks[order]->next;
    // }
    return nullptr;
}

void *operator new(unsigned long, void *ptr) { return ptr; }

void *operator new(unsigned long bytes, Heap &heap) {
    _lock.lock();
    void *addr = heap.alloc(bytes);
    if (!addr) {
        heap.grow(Memory::kmalloc(), Traits::Memory::Page::SIZE);
        addr = heap.alloc(bytes);
        ERROR(!addr, "[operator new] Out of memory.");
    }
    _lock.unlock();
    return addr;
}

void *operator new(unsigned long bytes) {
    if constexpr (Traits::System::MULTITASK) {
        return operator new[](bytes, Thread::running()->task->heap);
    } else {
        return operator new[](bytes, Heap::SYSTEM);
    }
}

void *operator new[](unsigned long bytes, Heap &heap) { return operator new(bytes, heap); }
void *operator new[](unsigned long bytes) { return operator new(bytes); }
