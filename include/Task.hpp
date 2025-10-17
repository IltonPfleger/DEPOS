#pragma once

#include <Heap.hpp>
#include <Multitask/AddressSpace.hpp>

class Task {
    friend void* operator new(unsigned long);

   public:
    Task() {
        _heap = new (Heap::SYSTEM) Heap();
        _as   = new (Heap::SYSTEM) AddressSpace();
    };

    void* attach(uintptr_t addr) { return _as->attach(addr); }

   private:
    Heap* _heap;
    AddressSpace* _as;

   public:
    static Task* SYSTEM;
};
