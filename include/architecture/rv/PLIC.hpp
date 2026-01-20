#pragma once

#include <architecture/rv/ic/IC.hpp>
#include <drivers/Driver.hpp>
#include <utils/Debug.hpp>

namespace rv {
class PLIC : Driver {
    enum {
        PRIORITY = 0x000000,
        PENDING = 0x001000,
        ENABLED = 0x002000,
        THRESHOLD = 0x200000,
        CLAIM = 0x200004,
    };

    static void priority(unsigned int source, unsigned int value) { Reg32(Base, PRIORITY + source * 4) = value; }

    static void threshold(unsigned int context, unsigned int value) {
        Reg32(Base, THRESHOLD + (context * 0x1000)) = value;
    }

    static unsigned int claim(unsigned int context) { return Reg32(Base, CLAIM + (context * 0x1000)); }
    static void complete(unsigned int context, unsigned int id) { Reg32(Base, CLAIM + (context * 0x1000)) = id; }

    static unsigned int context() { return 0; }
    static void enable(unsigned int context, unsigned int source) {
        unsigned int bank = source / 32;
        unsigned int bit = source % 32;
        Reg32(Base, ENABLED + (context * 0x80) + (bank * 4)) |= (1 << bit);
    }

  public:
    static void handler(unsigned int) {
        unsigned int id = claim(context());
        complete(context(), id);
        IC::dispatch(id + First);
    }

    static void init() {
        TraceIn();
        csrs<KernelMode::IE>(KernelMode::EI);
        threshold(context(), 0);
        for (unsigned int id = 5; id <= 9; id++) {
            priority(id, 1);
            enable(context(), id);
        }
        TraceOut();
    }

    static constexpr unsigned long Enable = Traits<::PLIC>::Enable;
    static constexpr unsigned long Base = Traits<::PLIC>::Addr;
    static constexpr unsigned int First = Traits<::PLIC>::First;
};
} // namespace rv

namespace rv64 {
using PLIC = rv::PLIC;
}
