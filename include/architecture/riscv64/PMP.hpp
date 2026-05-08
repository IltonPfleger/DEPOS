#pragma once

#include <architecture/riscv64/csrs.hpp>

namespace DEPOS {

namespace riscv64 {

class PMP {
    enum Registers { PMPADDR = 0x3B0, PMPCFG = 0x3A0 };

  public:
    enum Bits {
        R    = 0x01,
        W    = 0x02,
        X    = 0x04,
        LOCK = 0x80,
    };

    template <unsigned int N> static void TOR(uintptr_t start, uintptr_t end, unsigned int flags) {
        csrw<PMPADDR + N - 1>(start >> 2);
        csrw<PMPADDR + N>(end >> 2);

        unsigned int shift        = (N % 4) * 8;
        constexpr uintptr_t index = N / 4;
        uintptr_t cfg             = csrr<PMPCFG + index>();

        cfg &= ~(0xFFULL << shift);
        cfg |= (0x8 | flags) << shift;

        csrw<PMPCFG + index>(cfg);
    }

    template <unsigned int N> static void NAPOT(uintptr_t start, size_t size, unsigned int flags) {
        csrw<PMPADDR + N>((start >> 2) | ((size >> 3) - 1));

        unsigned int shift        = (N % 8) * 8;
        constexpr uintptr_t index = (N / 8) * 2;
        uintptr_t cfg             = csrr<PMPCFG + index>();

        cfg &= ~(0xFFULL << shift);
        cfg |= (0x18 | flags) << shift;

        csrw<PMPCFG + index>(cfg);
    }
};

} // namespace riscv64

} // namespace DEPOS
