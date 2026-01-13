#pragma once

#include "Traits.hpp"

#include <architecture/rv/64/RV64.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/SiFiveUART.hpp>

class sifive_u {
  public:
    using Timer = SiFiveCLINT<Traits<MemoryMap>::CLINT>;
    using ISA = RV64<Timer>;
    using CPU = ISA::CPU;
    using MMU = ISA::MMU;
    using IO = SiFiveUART<Traits<MemoryMap>::UART, 31250000, 115200>;

    __attribute__((always_inline)) static inline void init() {
        CPU::probe();
        CPU::jmode();
        CPU::init();
        IO::init();
    }
};
