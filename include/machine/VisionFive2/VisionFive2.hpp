#pragma once
#include <architecture/rv/64/RV64.hpp>
#include <drivers/ethernet/DWMAC.hpp>
#include <drivers/timing/clint/SiFiveCLINT.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>

#include "Traits.hpp"

class VisionFive2 {
  public:
    using CLINT = SiFiveCLINT;
    using ISA = RV64<CLINT>;
    using CPU = ISA::CPU;
    using MMU = ISA::MMU;
    using Ethernet = DWMAC<Traits<MemoryMap>::GMAC0>::Ethernet;
    using IO = DW8250<Traits<MemoryMap>::UART>;

    __attribute__((always_inline)) static inline void init() {
        CPU::probe();
        BSS::init();
        CPU::barrier();
        CLINT::reset(CPU::id());
        CPU::init();
        IO::init();
    }
};
