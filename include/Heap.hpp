#pragma once

#include <Lists.hpp>
#include <Traits.hpp>

class Heap {
    friend void *operator new[](unsigned long);

   public:
    void *alloc(unsigned long);
    void *free(void *, unsigned long);
    void grow(void *, unsigned long);
    unsigned long index_of(unsigned long);

   private:
    static constexpr int CAPSTONE = Traits::Memory::Page::ORDER;
    using Chunk                   = Node<void>;
    Chunk *_chunks[CAPSTONE + 1]  = {nullptr};

   public:
    static Heap *SYSTEM;
};

void *operator new(unsigned long, void *);
void *operator new(unsigned long);
void *operator new(unsigned long, Heap *);
void *operator new[](unsigned long, Heap *);
void operator delete(void *, unsigned long);
