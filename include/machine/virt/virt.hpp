#pragma once

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

using CPU = rv64::CPU;
template <typename... Tickers> using TimerTemplate = rv::CLINT<Tickers...>;

class virt {
  public:
    static void init() { rv64::Initializer::init(); }
};
