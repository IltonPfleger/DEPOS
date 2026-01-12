#pragma once
#include <architecture/rv/64/RV64.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>

#include "Traits.hpp"

class VisionFive2 {
  public:
    using CLINT = SiFiveCLINT<Traits<MemoryMap>::CLINT>;
    using ISA = RV64<CLINT>;
    using CPU = ISA::CPU;
    using MMU = ISA::MMU;
    using IO = DW8250<Traits<MemoryMap>::UART, 1000000 / 2, 115200>;

    __attribute__((always_inline)) static inline void init() {
        CPU::probe();

        if (CPU::id() >= Traits<CPUS>::COUNT)
            CPU::halt();

        BSS::init();
        CPU::init();
        IO::init();
    }
};
