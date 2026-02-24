#pragma once

#include <architecture/riscv64/init.hpp>
#include <drivers/uart/UART16550.hpp>
#include <machine/virt/Traits.hpp>

class virt {
    template <typename... Ts> static void initializer(Meta::TypeList<Ts...>) { (Ts::init(), ...); }

  public:
    static void init() {
        riscv64::init();
        Meta::ForEachTypeList(Traits<UART>::Devices{}, []<typename T>() { T::init(); });
    }
};
