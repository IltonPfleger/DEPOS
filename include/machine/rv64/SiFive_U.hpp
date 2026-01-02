#pragma once
#include <architecture/rv/64/RV64.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <memory/Memory.hpp>

class SiFive_U {
    static_assert(Traits<MemoryMap>::UART == 0x10010000ULL);
    static_assert(Traits<MemoryMap>::CLINT == 0x02000000ULL);

  public:
    using ISA = RV64<SiFiveCLINT>;
    using CPU = ISA::CPU;
    using MMU = typename ISA::SV39_MMU<Memory>;
    using IO = SiFiveUART<Traits<MemoryMap>::UART, 31250000, 115200>;
};
