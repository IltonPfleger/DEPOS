#pragma once

#include <architecture/rv/64/RV64.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

class CPU : public rv64::CPU {};

class virt {
  public:
    template <typename... Tickers> using Timer = rv::CLINT<Tickers...>;

    static void init() { rv64::Initializer::init(); }
};
