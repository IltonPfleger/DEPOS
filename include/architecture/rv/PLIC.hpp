#pragma once

#include <drivers/Driver.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

namespace rv {
class PLIC : public DispatchTable<Traits<::PLIC>::First, Traits<::PLIC>::Last, PLIC>, Driver {
    using Base = DispatchTable<Traits<::PLIC>::First, Traits<::PLIC>::Last, PLIC>;

    enum {
        PRIORITY = 0x000000,
        PENDING = 0x001000,
        ENABLED = 0x002000,
        THRESHOLD = 0x200000,
        CLAIM = 0x200004,
    };

    static void priority(unsigned int source, unsigned int value) { Reg32(Addr, PRIORITY + source * 4) = value; }

    static void threshold(unsigned int context, unsigned int value) {
        Reg32(Addr, THRESHOLD + (context * 0x1000)) = value;
    }

    static unsigned int claim(unsigned int context) { return Reg32(Addr, CLAIM + (context * 0x1000)); }
    static void complete(unsigned int context, unsigned int id) { Reg32(Addr, CLAIM + (context * 0x1000)) = id; }

    static unsigned int context() { return 0; }
    static void enable(unsigned int context, unsigned int source) {
        unsigned int bank = source / 32;
        unsigned int bit = source % 32;
        Reg32(Addr, ENABLED + (context * 0x80) + (bank * 4)) |= (1 << bit);
    }

  public:
    static void bind(unsigned int id, Base::Handler handler) {
        Base::bind(id, handler);
        priority(id, 1);
        enable(context(), id);
    }

    static void handler(unsigned int) {
        unsigned int id = claim(context());
        dispatch(id);
        complete(context(), id);
    }

    static void init() {
        csrs<KernelMode::IE>(KernelMode::EI);
        threshold(context(), 0);
    }

    static constexpr unsigned long Enable = Traits<::PLIC>::Enable;
    static constexpr unsigned long Addr = Traits<::PLIC>::Addr;
};
} // namespace rv

namespace rv64 {
using PLIC = rv::PLIC;
}
