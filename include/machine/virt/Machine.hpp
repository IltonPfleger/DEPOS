#pragma once

/* Drivers */
#include <architecture/riscv64/init.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

namespace DEPOS {

class virt {
  public:
    static void init() {
        riscv64::init();
        Meta::forEach(Traits<UART>::Devices{}, ([]<typename T>() { T::init(); }));
    }
};

} // namespace DEPOS
