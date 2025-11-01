#pragma once

#include <Lists.hpp>
#include <Traits.hpp>

class Heap {
    // friend void *operator new[](unsigned long);

    // void *alloc(unsigned long);
    // void *free(void *, unsigned long);
    // void grow(void *, unsigned long);
    // unsigned long index_of(unsigned long);

    // private:
    //  using Chunk                            = Node<void>;
    //  static constexpr unsigned int CAPSTONE = Traits::Memory::Page::ORDER;

   public:
    static Heap SYSTEM;

    // private:
    //  Chunk *chunks[CAPSTONE + 1] = {nullptr};
};

void *operator new(unsigned long);
void *operator new(unsigned long, void *);
void *operator new(unsigned long, Heap &);
// void *operator new[](unsigned long, Heap *);
//  void operator delete(void *, unsigned long);
