#pragma once

#include <AddressSpace.hpp>
#include <Heap.hpp>
#include <Initializer.hpp>

class Task : Initializer<Task> {
    // friend void* operator new(unsigned long);

   private:
    Task(Heap* h, AddressSpace* a) : heap(h), as(a) {}
    //  Task() {
    //      _heap = new (Heap::SYSTEM) Heap();
    //      _as   = new (Heap::SYSTEM) AddressSpace();
    //  };

    // void* attach(uintptr_t addr) { return _as->attach(addr); }
   private:
    Heap* heap;
    AddressSpace* as;
};
