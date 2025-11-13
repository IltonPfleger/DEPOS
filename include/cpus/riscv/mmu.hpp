#pragma once

#include <Types.hpp>

class SV39_MMU {
  private:
    static constexpr uintptr_t Mode = 8UL << 60;

  public:
    class PageTable {
      public:
        static constexpr size_t PageSize      = 4096;
        static constexpr size_t EntriesNumber = 512;
        typedef uintptr_t Flags;
        enum {
            V        = 1 << 0, // Valid
            R        = 1 << 1, // Readable
            W        = 1 << 2, // Writable
            X        = 1 << 3, // Executable
            U        = 1 << 4, // User accessible
            A        = 1 << 6, // Accessed
            D        = 1 << 7, // Dirty
            UserRO   = A | D | R | U | V | X,
            UserRW   = A | D | R | U | V | W | X,
            KernelRO = A | D | R | V | X,
            KernelRW = A | D | R | V | W | X,
            Default  = UserRW
        };

      private:
        bool set(int vpn, uintptr_t addr, Flags flags) {
            if (entries[vpn])
                return false;
            entries[vpn] = (addr >> 2) | flags;
            return true;
        }

        PageTable *walk(int vpn) {
            uintptr_t pte  = entries[vpn];
            uintptr_t addr = (pte >> 10) << 12;
            return reinterpret_cast<PageTable *>(addr);
        }

      public:
        void load() { SV39_MMU::set(reinterpret_cast<uintptr_t>(this)); }
        bool map(uintptr_t va, uintptr_t pa, Flags);
        bool map(uintptr_t va, Flags flags) { return map(va, va, flags); }

        PageTable() {
            for (auto &e : entries)
                e = 0;
        }

      private:
        alignas(PageSize) uintptr_t entries[EntriesNumber];
    };

    static void set(uintptr_t);
};
