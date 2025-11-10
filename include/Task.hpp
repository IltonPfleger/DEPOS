#pragma once

#include <IO/Debug.hpp>
#include <memory/AddressSpace.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <utils/Console.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];

class Task {
    // public:
    //  Task() : heap(new Heap()), as(new(Memory::kmalloc()) AddressSpace()) {
    //      uintptr_t PAGE_SIZE    = Traits::Memory::Page::SIZE;
    //      uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    //      uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    //      KERNEL_END             = (KERNEL_END + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    //      for (uintptr_t pa = KERNEL_START; pa < KERNEL_END; pa += PAGE_SIZE) as->map(pa, pa, SV39_MMU::X);
    //      as->map(Machine::IO::Addr, Machine::IO::Addr);
    //  }

    // // private:
    // //  Task(Heap* h, AddressSpace* a) : heap(h), as(a) {}

   public:
    static void init() {
        TraceIn();
        AddressSpace* as = new (Heap::SYSTEM) AddressSpace();

        for (uintptr_t pa = Traits::Memory::RAM_BASE; pa < Traits::Memory::RAM_END; pa += Traits::Memory::Page::SIZE) {
            as->map(pa, pa);
        }

        as->map(Machine::IO::Addr, Machine::IO::Addr);

        as->load();
        TraceOut();
    }

    // void* attach(void* addr) { return as->map(addr); }

    // public:
    //  Heap* const heap;
    //  AddressSpace* const as;
};
