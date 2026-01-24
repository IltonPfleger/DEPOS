#pragma once

#include "Traits.hpp"

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/SiFiveUART.hpp>

typedef rv64::CPU CPU;

class sifive_u {
  public:
    using Initializer = rv64::Initializer;

    static void init() { Initializer::init(); }
};
