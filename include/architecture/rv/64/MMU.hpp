#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/Modes.hpp>
#include <architecture/rv/csrs.hpp>
#include <memory/Memory.hpp>

namespace rv64 {
class SV39_MMU {
  public:
    class TLB {
      public:
        static auto flush() { asm("sfence.vma zero, zero"); }
    };

    class PageTable {
        friend SV39_MMU;

      public:
        PageTable() = default;
        enum Flags {
            V = 1 << 0, // Valid
            R = 1 << 1, // Readable
            W = 1 << 2, // Writable
            X = 1 << 3, // Executable
            U = 1 << 4, // User accessible
            A = 1 << 6, // Accessed
            D = 1 << 7, // Dirty
            UserRO = A | D | R | U | V | X,
            UserRW = A | D | R | U | V | W | X,
            KernelRO = A | D | R | V | X,
            KernelRW = A | D | R | V | W | X,
            Default = UserRW
        };

        void load() const {
            rv::csrw<rv::SupervisorMode::SATP>(Mode | reinterpret_cast<uintptr_t>(this) >> 12);
            TLB::flush();
        }

        bool map(uintptr_t va, uintptr_t pa, Flags flags) {
            uintptr_t vpn2 = (va >> 30) & 0x1FF;
            uintptr_t vpn1 = (va >> 21) & 0x1FF;
            uintptr_t vpn0 = (va >> 12) & 0x1FF;

            PageTable *l1;
            PageTable *l0;

            if (!entries[vpn2]) {
                l1 = new (Memory::alloc(Size)) PageTable();
                set(vpn2, reinterpret_cast<uintptr_t>(l1), V);
            } else {
                l1 = walk(vpn2);
            }
            if (!l1->entries[vpn1]) {
                l0 = new (Memory::alloc(Size)) PageTable();
                l1->set(vpn1, reinterpret_cast<uintptr_t>(l0), V);
            } else {
                l0 = l1->walk(vpn1);
            }

            return l0->set(vpn0, reinterpret_cast<uintptr_t>(pa), flags);
        }
        bool map(uintptr_t va, Flags flags) { return map(va, va, flags); }

        void map(uintptr_t va, uintptr_t pa, size_t size, Flags flags) {
            if ((size % Giga == 0) && ((va % Giga == 0) && (pa % Giga == 0))) {
                uintptr_t vpn2 = (va >> 30) & 0x1FF;
                this->set(vpn2, pa, flags);
            } else {
                uintptr_t end = pa + size;
                for (; pa < end;) {
                    map(va, pa, flags);
                    va += Size;
                    pa += Size;
                }
            }
        }

      private:
        bool set(int vpn, uintptr_t addr, Flags flags) {
            if (entries[vpn]) return false;
            entries[vpn] = (addr >> 2) | flags;
            return true;
        }

        PageTable *walk(int vpn) {
            uintptr_t pte = entries[vpn];
            uintptr_t addr = (pte >> 10) << 12;
            return reinterpret_cast<PageTable *>(addr);
        }

      private:
        static constexpr auto Size = 4096;
        static constexpr auto EntriesNumber = 512;
        alignas(Size) uintptr_t entries[EntriesNumber];
    };

    // TODO: Don't need to be here
    // class KernelPageTable {
    //  private:
    //    KernelPageTable();
    //    static inline PageTable *s_pt = nullptr;

    //  public:
    //    static void init() {
    //        s_pt = new (Memory::alloc(PageTable::Size)) PageTable();
    //        s_pt->map(Traits<MemoryMap>::RamBas,
    //                        Traits<MemoryMap>::RamBas, Traits<Memory>::SIZE,
    //                        PageTable::KernelRW);
    //        s_pt->map(Traits<MemoryMap>::UART, Traits<MemoryMap>::UART,
    //                        PageTable::KernelRW);
    //        s_pt->load();
    //    }
    //    static const PageTable *get() { return s_pt; }
    //};

  public:
    static void init() {
        if (CPU::id() == Traits<CPUS>::BSP) {
            s_kernel_page_table = reinterpret_cast<PageTable *>(Memory::alloc(sizeof(PageTable)));
            s_kernel_page_table->map(Traits<MemoryMap>::VirtualRamStart, Traits<MemoryMap>::PhysicalRamStart,
                                     Traits<Memory>::Size, PageTable::KernelRW);
            s_kernel_page_table->map(Traits<MemoryMap>::PhysicalRamStart, Traits<MemoryMap>::PhysicalRamStart,
                                     Traits<Memory>::Size, PageTable::KernelRW);
            s_kernel_page_table->map(Traits<MemoryMap>::UART0, Traits<MemoryMap>::UART0, PageTable::KernelRW);
        }

        CPU::barrier();
        s_kernel_page_table->load();
    }

  private:
    static constexpr unsigned long Mode = 8UL << 60;
    static constexpr unsigned long Giga = (1 << 30);
    static inline PageTable *s_kernel_page_table;
};
} // namespace rv64
