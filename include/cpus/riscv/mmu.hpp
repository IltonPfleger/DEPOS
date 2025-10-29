#pragma once

#include <Types.hpp>

class SV39_MMU {
   public:
    enum Flags {
        V = 1 << 0,  // Valid
        R = 1 << 1,  // Readable
        W = 1 << 2,  // Writable
        X = 1 << 3,  // Executable
        U = 1 << 4,  // User accessible
        A = 1 << 6,  // Accessed
        D = 1 << 7,  // Dirty
    };

    struct PageTable {
        PageTable() {
            for (auto& e : entries) e = 0;
        }

        bool attach(int vpn, uintptr_t addr, Flags flags) {
            if (entries[vpn]) return false;
            entries[vpn] = (addr >> 2) | flags;
            return true;
        }

        PageTable* walk(int vpn) {
            uintptr_t pte  = entries[vpn];
            uintptr_t addr = (pte >> 10) << 12;
            return reinterpret_cast<PageTable*>(addr);
        }

        alignas(4096) uintptr_t entries[512];
    };

    static PageTable* base();
    static bool map(PageTable*, uintptr_t, uintptr_t, Flags);
    static void init();
    static void set(uintptr_t);
    static uintptr_t attach(uintptr_t);

   private:
    static constexpr const uintptr_t MODE = 8UL << 60;
};
