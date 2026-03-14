#pragma once

#include <architecture/riscv64/PLIC.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

namespace DEPOS {

namespace riscv64 {

class IC {
    using Handler                 = void (*)(unsigned int);
    static constexpr int External = Traits<PLIC>::NumberOfInterruptions;
    static constexpr int Total    = 11 + External;

  public:
    enum { INTERRUPT = 1UL << 63 };

    static void dispatch(unsigned int id) {
        ERROR(!s_handlers[id]);
        s_handlers[id](id);
    }

    static void bind(unsigned int id, Handler handler) {
        s_handlers[id] = handler;
        if (id > 11) {
            PLIC::priority(id - 11, 1);
            PLIC::enable(id - 11);
        }
    }

  private:
    static inline Handler s_handlers[Total];
};

} // namespace riscv64

} // namespace DEPOS
