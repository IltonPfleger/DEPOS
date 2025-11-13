#pragma once

#include <cpus/default/mmu.hpp>

class SV39_MMU : public DefaultMMU<4096, 512> {
   private:
    static constexpr unsigned long Mode = 8UL << 60;
    using Base                          = DefaultMMU<4096, 512>;

   public:
    class PageTable : public Base::PageTable {
       public:
        enum Flags {
            V        = 1 << 0,  // Valid
            R        = 1 << 1,  // Readable
            W        = 1 << 2,  // Writable
            X        = 1 << 3,  // Executable
            U        = 1 << 4,  // User accessible
            A        = 1 << 6,  // Accessed
            D        = 1 << 7,  // Dirty
            UserRO   = A | D | R | U | V | X,
            UserRW   = A | D | R | U | V | W | X,
            KernelRO = A | D | R | V | X,
            KernelRW = A | D | R | V | W | X,
            Default  = UserRW
        };

       private:
        bool set(int vpn, uintptr_t addr, Flags flags) {
            if (entries[vpn]) return false;
            entries[vpn] = (addr >> 2) | flags;
            return true;
        }

        PageTable *walk(int vpn) {
            uintptr_t pte  = entries[vpn];
            uintptr_t addr = (pte >> 10) << 12;
            return reinterpret_cast<PageTable *>(addr);
        }

       public:
        PageTable() = default;
        void load() { SV39_MMU::set(reinterpret_cast<uintptr_t>(this)); }
        bool map(uintptr_t va, uintptr_t pa, Flags);
        bool map(uintptr_t va, Flags flags) { return map(va, va, flags); }
    };

    static void set(uintptr_t);
};
