#pragma once
#include <IO/UART/SiFiveUART.hpp>
#include <Types.hpp>
#include <cpus/riscv/cpu.hpp>

namespace Machine {
    using IO  = SiFiveUART;
    using CPU = RISCV;
};
