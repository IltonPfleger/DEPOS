#pragma once
#include <IO/UART/SiFiveUART.hpp>
#include <Meta.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <cpus/dummy/mmu.hpp>
#include <cpus/riscv/cpu.hpp>
#include <cpus/riscv/mmu.hpp>

namespace Machine {
    using IO  = SiFiveUART;
    using CPU = RISCV;
    using MMU = Meta::IF<Traits::System::MULTITASK, SV39_MMU, DummyMMU>::Result;
};
