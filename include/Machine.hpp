#pragma once
#include <Meta.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <arch/rv64/mmu.hpp>
#include <arch/rv64/cpu.hpp>
#include <drivers/uart/SiFiveUART.hpp>

class Machine {
  public:
    using IO = SiFiveUART<Traits<MemoryMap>::UART0, 31250000, 115200>;
    using CPU = RISCV;
    using MMU = SV39_MMU;
};
