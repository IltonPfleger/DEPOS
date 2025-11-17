#pragma once
#include <arch/rv64/RV64.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <memory/Memory.hpp>

class SiFive_U {
    static_assert(Traits<MemoryMap>::UART == 0x10010000ULL);
    static_assert(Traits<MemoryMap>::CLINT == 0x02000000ULL);

  private:
    using UART = SiFiveUART<Traits<MemoryMap>::UART, 31250000, 115200>;
    using Architecture = RISCV;

  public:
    using CPU = Architecture::CPU;
    using MMU = Architecture::SV39_MMU<Memory>;
    using IO = UART;
};
