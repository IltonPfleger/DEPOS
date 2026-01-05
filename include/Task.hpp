#pragma once

#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <memory/Segment.hpp>
#include <utils/Debug.hpp>

class Task {
    using AddressSpace = MMU::PageTable;

  public:
    static void init() {
        TraceIn();
        s_system = new (Heap::SYSTEM) Task();
        AddressSpace *as = s_system->m_as;

        as->map(Traits<MemoryMap>::RAM_BASE, Traits<MemoryMap>::RAM_BASE, Traits<Memory>::SIZE, AddressSpace::KernelRW);

        as->map(Traits<MemoryMap>::UART, Traits<MemoryMap>::UART, AddressSpace::KernelRW);

        // as->load();
        TraceOut();
    }

  private:
    Task() : m_as(new(Heap::SYSTEM) AddressSpace()) {}

  private:
    static inline Task *s_system;
    AddressSpace *m_as;
    //     using AddressSpace = Machine::MMU::PageTable;
    //     Task() : as(new(Heap::SYSTEM) AddressSpace()) {
    //         // uintptr_t KernelStart =
    //         // reinterpret_cast<uintptr_t>(__mm.kernel.start); uintptr_t
    //         KernelEnd
    //         // = reinterpret_cast<uintptr_t>(__mm.kernel.end); Segment k =
    //         // Segment(__mm.kernel.start, KernelEnd - KernelStart); for (int
    //         i = 0;
    //         // i < Traits<Machine>::CPUS; i++) {
    //         //     as->map(Traits<Memory>::RAM_END - (i + 1) *
    //         //     Machine::MMU::PageSize, AddressSpace::KernelRW);
    //         // };
    //
    //         // attach(k, AddressSpace::KernelRW);
    //         // as->map(Machine::IO::Addr, AddressSpace::KernelRW);
    //     }
    //     // Task() : heap(new Heap()), as(new(Heap::SYSTEM) AddressSpace()) {}
    //     //       uintptr_t PAGE_SIZE    = Traits<Memory>::Page::SIZE;
    //     //       uintptr_t KERNEL_START =
    //     //       reinterpret_cast<uintptr_t>(__KERNEL_START__); uintptr_t
    //     KERNEL_END
    //     //       = reinterpret_cast<uintptr_t>(__KERNEL_END__); KERNEL_END =
    //     //       (KERNEL_END + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); for
    //     (uintptr_t pa
    //     //       = KERNEL_START; pa < KERNEL_END; pa += PAGE_SIZE)
    //     as->map(pa, pa,
    //     //       SV39_MMU::X); as->map(Machine::IO::Addr, Machine::IO::Addr);
    //     //   }
    //     //
    //   public:
    //     void attach(Segment &s, AddressSpace::Flags f) {
    //		(void)s;
    //		(void)f;
    //         // TraceIn(reinterpret_cast<void *>(s.base()), s.size());
    //         // ERROR(s.size() % AddressSpace::Size != 0);
    //         //(void)f;
    //         //// for (auto addr = s.base(); addr <= s.end(); addr +=
    //         //// AddressSpace::PageSize) {
    //         //// as->map(reinterpret_cast<uintptr_t>(addr), f);
    //         //// }
    //         // TraceOut();
    //     }
    //
    //   public:
    //     static void init() {
    //         // TraceIn();
    //         // SYSTEM = new (Heap::SYSTEM) Task();
    //         // char buffer[sizeof(Segment)];
    //         // Segment *s = reinterpret_cast<Segment *>(buffer);
    //         // new (buffer) Segment(Traits<Memory>::RAM_BASE,
    //         Traits<Memory>::SIZE);
    //         // SYSTEM->attach(*s, AddressSpace::Flags::KernelRW);
    //         // new (buffer) Segment(Machine::IO::Addr,
    //         AddressSpace::PageSize);
    //         // SYSTEM->attach(*s, AddressSpace::Flags::KernelRW);
    //         // SYSTEM->load();
    //         // TraceOut();
    //     }
    //
    //     void load() { as->load(); };
    //
    //     // void attach(Segment &s) { attach(s, AddressSpace::KernelRW); }
    //
    //   public:
    //     Heap *heap;
    //     AddressSpace *as;
    //     // static inline Task *SYSTEM;
};
