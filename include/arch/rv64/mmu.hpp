#pragma once

#include <arch/rv64/cpu.hpp>
#include <memory/Memory.hpp>

class SV39_MMU {
  private:
    static constexpr unsigned long Mode = 8UL << 60;
    static constexpr unsigned long Giga = (1 << 30);

  public:
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
            RISCV::csrw<RISCV::Supervisor::SATP>(
                Mode | reinterpret_cast<uintptr_t>(this) >> 12);
            RISCV::flush();
        }

        bool map(uintptr_t va, uintptr_t pa, Flags);
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
            if (entries[vpn])
                return false;
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
    class KernelPageTable {
      private:
        KernelPageTable();
        static inline PageTable *instance_s = nullptr;

      public:
        static void init() {
            instance_s = new (Memory::kmalloc(PageTable::Size)) PageTable();
            instance_s->map(Traits<MemoryMap>::RAM_BASE,
                            Traits<MemoryMap>::RAM_BASE, Traits<Memory>::SIZE,
                            PageTable::KernelRW);
            instance_s->map(Traits<MemoryMap>::UART0, Traits<MemoryMap>::UART0,
                            PageTable::KernelRW);
            instance_s->load();
        }
        static const PageTable *get() { return instance_s; }
    };
};
