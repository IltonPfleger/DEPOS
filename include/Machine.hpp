#pragma once
#include <IO/UART/SiFiveUART.hpp>
#include <Types.hpp>

struct Machine {
    struct IO {
        using UART = SiFiveUART;
    };
    static constexpr const unsigned int XLEN = 64;
    static constexpr const unsigned int CPUS = 4;
    struct Memory {
        static constexpr const unsigned int ORDER = 30;
        static constexpr const unsigned int SIZE  = (1 << ORDER);
    };
    struct CLINT {
        static constexpr const unsigned long long CLOCK = 1'000'000;
        static constexpr const uintptr_t ADDR           = 0x02000000;
        static volatile inline uintptr_t &MTIME         = *reinterpret_cast<volatile uintptr_t *>(ADDR + 0xBFF8);
        static volatile inline uintptr_t &MTIMECMP      = *reinterpret_cast<volatile uintptr_t *>(ADDR + 0x4000);
    };
};
