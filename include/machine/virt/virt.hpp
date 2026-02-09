#pragma once

#include <architecture/riscv64/init.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

// class CPU : public rv64::CPU {};

class virt {
  public:
    template <typename... Tickers> using Timer = riscv64::CLINT<Tickers...>;

    static void init() { riscv64::init(); }
};
