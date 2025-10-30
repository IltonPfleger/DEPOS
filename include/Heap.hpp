#pragma once

#include <Initializer.hpp>
#include <Lists.hpp>
#include <Traits.hpp>

class Heap : public Initializer<Heap> {
    // friend void *operator new[](unsigned long);

    // void *alloc(unsigned long);
    // void *free(void *, unsigned long);
    // void grow(void *, unsigned long);
    // unsigned long index_of(unsigned long);

   public:
    static constexpr unsigned int CAPSTONE = Traits::Memory::Page::ORDER;

   private:
    using Chunk                 = Node<void>;
    Chunk *chunks[CAPSTONE + 1] = {nullptr};
};

void *operator new(unsigned long);
void *operator new(unsigned long, Heap *);
void *operator new[](unsigned long, Heap *);
void operator delete(void *, unsigned long);
