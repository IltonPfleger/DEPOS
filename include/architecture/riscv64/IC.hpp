#pragma once

#include <architecture/riscv64/PLIC.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

namespace riscv64 {

class IC : public DispatchTable<Traits<IC>::First, Traits<IC>::Last, IC> {

  public:
    enum { INTERRUPT = 1UL << 63 };

    static void bind(unsigned int id, Handler handler) {
        DispatchTable<Traits<IC>::First, Traits<IC>::Last, IC>::bind(id, handler);

        if (id > 11) {
            PLIC::priority(id - 11, 1);
            PLIC::enable(id - 11);
        }
    }
};

} // namespace riscv64
