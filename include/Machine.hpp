#pragma once
#include <IO/UART/SiFiveUART.hpp>
#include <Types.hpp>

struct Machine {
    struct IO {
        using UART = SiFiveUART;
    };
    static constexpr const int XLEN = 64;
    static constexpr const int CPUS = 4;
    struct Memory {
        static constexpr const unsigned int ORDER = 30;
        static constexpr const unsigned int SIZE  = (1 << ORDER);
    };
    struct CLINT {
        static constexpr const unsigned long long CLOCK = 1'000'000;
        static constexpr const uintptr_t ADDR           = 0x02000000;
        static constexpr const uintptr_t MTIMECMP       = ADDR + 0x4000;
        static volatile inline uintmax_t *MTIME         = reinterpret_cast<volatile uintmax_t *>(ADDR + 0xBFF8);
    };
};
