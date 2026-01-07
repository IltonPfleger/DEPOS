#pragma once
#include <architecture/rv/64/RV64.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <memory/Memory.hpp>

class SiFive_U {
  public:
    using CLINT = SiFiveCLINT;
    using ISA = RV64<CLINT>;
    using CPU = ISA::CPU;
    using MMU = typename ISA::SV39_MMU<Memory>;
    using IO = SiFiveUART<0x10010000, 31250000, 115200>;

    __attribute__((always_inline)) static inline void init() {
        CPU::probe();
        CPU::jmode();
        CPU::init();
        IO::init();
    }
};
