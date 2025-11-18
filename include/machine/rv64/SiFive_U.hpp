#pragma once
#include <architecture/rv64/RV64.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <memory/Memory.hpp>

namespace SiFive_U {
static_assert(Traits<MemoryMap>::UART == 0x10010000ULL);
static_assert(Traits<MemoryMap>::CLINT == 0x02000000ULL);

using CPU = RV64::CPU;
using MMU = typename RV64::SV39_MMU<Memory>;
using IO = SiFiveUART<Traits<MemoryMap>::UART, 31250000, 115200>;
}; // namespace SiFive_U
