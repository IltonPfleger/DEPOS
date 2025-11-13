#pragma once

#include <IO/Debug.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <memory/Segment.hpp>

class Task {
    using AddressSpace = Machine::MMU::PageTable;

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
    void attach(Segment &s, AddressSpace::Flags f) {
        TraceIn(reinterpret_cast<void *>(s.base()), s.size());
        ERROR(s.size() % AddressSpace::PageSize != 0);
        for (auto addr = s.base(); addr <= s.end(); addr += AddressSpace::PageSize) {
            as->map(reinterpret_cast<uintptr_t>(addr), f);
        }
        TraceOut();
    }

   public:
    static void init() {
        TraceIn();
        Task *SYSTEM = new (Heap::SYSTEM) Task();
        char buffer[sizeof(Segment)];
        Segment *s = reinterpret_cast<Segment *>(buffer);
        new (buffer) Segment(Traits::Memory::RAM_BASE, Traits::Memory::SIZE);
        SYSTEM->attach(*s, AddressSpace::Flags::KernelRW);
        new (buffer) Segment(Machine::IO::Addr, AddressSpace::PageSize);
        SYSTEM->attach(*s, AddressSpace::Flags::KernelRW);
        SYSTEM->load();
        TraceOut();
    }

    void load() { as->load(); };

    void attach(Segment &) {}

   public:
    Heap *heap;
    AddressSpace *as;
};
