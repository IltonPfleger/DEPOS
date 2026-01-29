#pragma once

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

typedef rv64::CPU CPU;

class virt {
  public:
    static void init() { rv64::Initializer::init(); }
};
