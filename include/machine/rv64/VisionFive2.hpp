#pragma once
#include <architecture/rv/64/RV64.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>

class VisionFive2 {
  public:
    using CLINT = SiFiveCLINT;
    using ISA = RV64<CLINT>;
    using CPU = ISA::CPU;
    using MMU = typename ISA::SV39_MMU<Memory>;
    using IO = DW8250<0x10000000, 1000000 / 2, 115200>;

    __attribute__((always_inline)) static inline void init() {
        CPU::probe();
        if (CPU::id() >= Traits<CPUS>::COUNT)
            CPU::halt();
        BSS::init();
        CPU::jmode();
        CPU::init();
        IO::init();
    }
};
