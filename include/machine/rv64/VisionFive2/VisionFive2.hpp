#pragma once
#include <architecture/rv64/RV64.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>

namespace VisionFive2 {
using CPU = RV64::CPU;
using MMU = typename RV64::SV39_MMU<Memory>;
using IO = DW8250<Traits<MemoryMap>::UART, 1000000 / 2, 115200>;
}; // namespace VisionFive2
