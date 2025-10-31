#pragma once

#include <AddressSpace.hpp>
#include <Heap.hpp>
#include <Resource.hpp>

class Task : public SystemResource<Task, Traits::System::MULTITASK> {
    // friend void* operator new(unsigned long);

   public:
    Task() : heap(Heap::SYSTEM()), as(AddressSpace::SYSTEM()) {}
    //  Task() {
    //      _heap = new (Heap::SYSTEM) Heap();
    //      _as   = new (Heap::SYSTEM) AddressSpace();
    //  };

    // void* attach(uintptr_t addr) { return _as->attach(addr); }
   private:
    Heap* heap;
    AddressSpace* as;
};
