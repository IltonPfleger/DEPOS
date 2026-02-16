#pragma once

#include <architecture/riscv64/csrs.hpp>
#include <utils/Debug.hpp>

namespace riscv64 {

class PMP {
    enum Registers { PMPADDR = 0x3B0, PMPCFG = 0x3A0 };

  public:
    enum Bits {
        R = 0x01,
        W = 0x02,
        X = 0x04,
        LOCK = 0x80,
    };

    template <unsigned int N> static void TOR(uintptr_t start, uintptr_t end, unsigned int flags) {
        static_assert(N > 0);
        ERROR(flags & ~0xFF);
        ERROR(start & 0x3);
        csrw<PMPADDR + N - 1>(start >> 2);
        csrw<PMPADDR + N>(end >> 2);
        unsigned int shift = (N % 4) * 8;
        uintptr_t value = (0x08 | (flags << shift));
        csrw<PMPCFG + (N / 4)>(value);
    }

    template <unsigned int N> static void NAPOT(uintptr_t start, size_t size, unsigned int flags) {
        if (size == 0) {
            ERROR(start != 0);
        } else {
            ERROR(size < 8);
            ERROR(start & (size - 1));
        }

        csrw<PMPADDR + N>((start >> 2) | ((size >> 3) - 1));
        csrw<PMPCFG + (N / 4)>((0x18 | flags) << ((N % 4) * 8));
    }
};
} // namespace riscv64
