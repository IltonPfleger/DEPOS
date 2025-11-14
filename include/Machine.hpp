#pragma once
#include <Meta.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <cpus/riscv/cpu.hpp>
#include <cpus/riscv/mmu.hpp>
#include <drivers/uart/SiFiveUART.hpp>

class Machine {
  public:
    using IO  = SiFiveUART<Traits<MemoryMap>::UART0, 31250000, 115200>;
    using CPU = RISCV;
    using MMU =
        Meta::IF<Traits<System>::MULTITASK, SV39_MMU, DefaultMMU<>>::Result;
};
