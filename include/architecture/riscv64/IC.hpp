#pragma once

#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace riscv64 {

class IC {

    /*
     * -----------------------------
     * | PLIC | Exceptions | CLINT |
     * -----------------------------
     * 0      N         N+Exceptions
     * -----------------------------
     */

    using Handler                   = void (*)(unsigned int);
    static constexpr int Exceptions = 16;
    static constexpr int Internal   = 16;
    static constexpr int External   = Traits<PLIC>::NumberOfInterruptions;
    static constexpr int Offset     = External;
    static constexpr int Total      = Exceptions + Internal + External;

  public:
    static void dispatch(unsigned int id, bool interruption, bool external) {
        unsigned int index = irq2index(id, interruption, external);
        ERROR(!s_handlers[index], "ID: ", id, " Index: ", index);
        s_handlers[index](id);
    }

    static unsigned int irq2index(unsigned int id, bool interruption, bool external) {
        if (!interruption) return id + External;
        if (external) return id;
        return id + External + Exceptions;
    }

    static void bind(unsigned int id, Handler handler, bool interruption = true, bool external = true) {
        s_handlers[irq2index(id, interruption, external)] = handler;
        if (interruption && external) {
            PLIC::priority(id, 1);
            PLIC::enable(id);
        }
    }

    static void init() {
        for (int i = 0; i < Exceptions; i++)
            bind(i, Exception::dispatch, false, false);
    }

  private:
    static inline Handler s_handlers[Total];
};

} // namespace riscv64

} // namespace DEPOS
