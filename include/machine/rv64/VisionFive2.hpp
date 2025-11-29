#pragma once
#include <architecture/rv64/RV64.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>

namespace VisionFive2 {
static_assert(Traits<MemoryMap>::PhysicalBootAddr == 0x40000000ULL);
static_assert(Traits<MemoryMap>::UART == 0x10000000ULL);
static_assert(Traits<MemoryMap>::CLINT == 0x02000000ULL);

static_assert(Traits<Timer>::Clock == 10'000'000);

using CPU = RV64::CPU;
using MMU = typename RV64::SV39_MMU<Memory>;
using IO = DW8250<Traits<MemoryMap>::UART, 1000000 / 2, 115200>;
}; // namespace VisionFive2
