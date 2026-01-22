#pragma once

#include "Traits.hpp"

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/SiFiveUART.hpp>

class sifive_u {
  public:
    using CPU = rv64::CPU;
    using Initializer = rv64::Initializer;
    using IO = SiFiveUART<Traits<MemoryMap>::UART, 31250000, 115200>;

    __attribute__((always_inline)) static inline void init() {
        Initializer::prepare();
        Initializer::init();
    }
};
