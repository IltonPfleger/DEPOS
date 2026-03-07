#pragma once

#include <architecture/riscv64/init.hpp>
#include <drivers/uart/SiFiveUART.hpp>
#include <machine/sifive_u/Traits.hpp>

namespace DEPOS {

class sifive_u {
  public:
    static void init() {
        riscv64::init();
        Meta::ForEachTypeList(Traits<UART>::Devices{}, []<typename T>() { T::init(); });
    }
};

} // namespace DEPOS
