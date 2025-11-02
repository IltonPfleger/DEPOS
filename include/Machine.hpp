#pragma once
#include <Meta.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <cpus/dummy/mmu.hpp>
#include <cpus/riscv/cpu.hpp>
#include <cpus/riscv/mmu.hpp>
#include <drivers/uart/SiFiveUART.hpp>

namespace Machine {
    using IO  = SiFiveUART;
    using CPU = RISCV;
    using MMU = Meta::IF<Traits::System::MULTITASK, SV39_MMU, DummyMMU>::Result;
};
