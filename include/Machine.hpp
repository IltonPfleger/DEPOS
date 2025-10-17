#pragma once
#include <IO/UART/SiFiveUART.hpp>
#include <Types.hpp>
#include <cpus/riscv/cpu.hpp>
#include <cpus/riscv/mmu.hpp>

namespace Machine {
    using IO  = SiFiveUART;
    using CPU = RISCV;
    using MMU = SV39_MMU;
};
