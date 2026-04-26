#pragma once

/* Drivers */
#include <drivers/uart/UART16550.hpp>

#include <architecture/riscv64/init.hpp>
#include <machine/virt/Traits.hpp>

namespace DEPOS {

class Machine {
  public:
    static void init() {
        riscv64::init();
        meta::forEach(Traits<UART>::Devices{}, ([]<typename T>() { T::init(); }));
    }
};

} // namespace DEPOS
