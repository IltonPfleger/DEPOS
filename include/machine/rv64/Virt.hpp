#pragma once
#include <architecture/rv64/RV64.hpp>
#include <drivers/uart/UART16550.hpp>
#include <memory/Memory.hpp>

namespace Virt {
static_assert(Traits<MemoryMap>::PhysicalBootAddr == 0x80000000ULL);
static_assert(Traits<MemoryMap>::UART == 0x10000000ULL);
static_assert(Traits<MemoryMap>::CLINT == 0x02000000ULL);

using CPU = RV64::CPU;
using MMU = typename RV64::SV39_MMU<Memory>;
using IO = UART16550<Traits<MemoryMap>::UART, 3686400, 115200>;
}; // namespace Virt
