#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/init.hpp>
#include <drivers/uart/SiFiveUART.hpp>

namespace DEPOS {

class sifive_u {
  public:
    static void init() {
        riscv64::init();
        Meta::forEach(Traits<UART>::Devices{}, []<typename T>() { T::init(); });
    }
    static void shutdown() {
        *reinterpret_cast<volatile uint32_t *>(0x100000) = (0 << 16) | 0x5555;
        CPU::halt();
    }
};

} // namespace DEPOS
