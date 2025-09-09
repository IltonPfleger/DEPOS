#pragma once

#include <Lists.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

class Heap {
   public:
    void *alloc(unsigned long);
    void *free(void *, unsigned long);
    unsigned long index_of(unsigned long);

   private:
    using Chunk                   = Node<void>;
    static constexpr int CAPSTONE = Traits::Memory::Page::ORDER;
    Chunk *_chunks[CAPSTONE + 1];
    Spin _lock;
};
