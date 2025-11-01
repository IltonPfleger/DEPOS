#pragma once

#include <AddressSpace.hpp>
#include <Heap.hpp>
#include <IO/Console.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];

class Task {
   public:
    Task() : Task(new Heap(), new(Memory::kmalloc()) AddressSpace()) {}

   private:
    Task(Heap* h, AddressSpace* a) : heap(h), as(a) {}

   public:
    static void init() {
        TRACE(__PRETTY_FUNCTION__, "{\n");
        Task* SYSTEM     = new (Heap::SYSTEM) Task(&Heap::SYSTEM, new (Memory::kmalloc()) AddressSpace());
        AddressSpace* as = SYSTEM->as;

        for (uintptr_t pa = Traits::Memory::RAM_BASE; pa < Traits::Memory::RAM_END; pa += Traits::Memory::Page::SIZE) {
            as->attach(pa, pa);
        }

        as->attach(Machine::IO::Addr, Machine::IO::Addr);

        as->load();

        // current = SYSTEM;

        TRACE("}\n");
    }

    // static inline Task* running() { return current; }
    // void* attach(void* addr) { return as->attach(addr); }

   public:
    Heap* const heap;
    AddressSpace* const as;

    // private:
    // static inline Task* current;
};
