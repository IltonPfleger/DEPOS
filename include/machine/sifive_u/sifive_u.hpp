#pragma once

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <machine/sifive_u/Traits.hpp>

typedef rv64::CPU CPU;

class sifive_u {
  public:
    static void init() { rv64::Initializer::init(); }
};
