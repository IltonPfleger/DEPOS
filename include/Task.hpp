#pragma once

#include <IO/Debug.hpp>
#include <memory/AddressSpace.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <memory/Segment.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];

class Task {
  public:
    Task() : as(new(Heap::SYSTEM) AddressSpace()) {
        // uintptr_t KernelStart = reinterpret_cast<uintptr_t>(__mm.kernel.start);
        // uintptr_t KernelEnd   = reinterpret_cast<uintptr_t>(__mm.kernel.end);
        // Segment k             = Segment(__mm.kernel.start, KernelEnd - KernelStart);
        // for (int i = 0; i < Traits::Machine::CPUS; i++) {
        //     as->map(Traits::Memory::RAM_END - (i + 1) * Machine::MMU::PageSize, AddressSpace::KernelRW);
        // };

        // attach(k, AddressSpace::KernelRW);
        // as->map(Machine::IO::Addr, AddressSpace::KernelRW);
    }
    // Task() : heap(new Heap()), as(new(Heap::SYSTEM) AddressSpace()) {}
    //       uintptr_t PAGE_SIZE    = Traits::Memory::Page::SIZE;
    //       uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    //       uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    //       KERNEL_END             = (KERNEL_END + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    //       for (uintptr_t pa = KERNEL_START; pa < KERNEL_END; pa += PAGE_SIZE) as->map(pa, pa, SV39_MMU::X);
    //       as->map(Machine::IO::Addr, Machine::IO::Addr);
    //   }

    // // private:
    // //  Task(Heap* h, AddressSpace* a) : heap(h), as(a) {}

  public:
    static void init() {
        TraceIn();
        // AddressSpace *as = new (Heap::SYSTEM) AddressSpace();

        // for (uintptr_t pa = Traits::Memory::RAM_BASE; pa < Traits::Memory::RAM_END; pa += Traits::Memory::Page::SIZE)
        // {
        //     as->map(pa, AddressSpace::KernelRW);
        // }

        // as->map(Machine::IO::Addr, AddressSpace::KernelRW);

        // as->load();
        TraceOut();
    }

    void load() { as->load(); };

    void attach(Segment &) {}
    // void attach(Segment &s, AddressSpace::Flags flags = AddressSpace::UserRW) {
    //     TraceIn(reinterpret_cast<void *>(s.base()), s.size());
    //     (void)flags;
    //     // ERROR(s.size() % Machine::MMU::PageSize != 0);
    //     // for (auto i = s.base(); i <= s.end(); i += Machine::MMU::PageSize) {
    //     //     as->map(reinterpret_cast<uintptr_t>(i), flags);
    //     // }
    //     TraceOut();
    // }

    // void* attach(void* addr) { return as->map(addr); }

  public:
    Heap *heap;
    AddressSpace *as;
};
