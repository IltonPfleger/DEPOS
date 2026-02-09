#pragma once

#include <architecture/riscv64/PLIC.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

namespace riscv64 {

class IC;
using IC_DispatchTable = DispatchTable<0, 11, IC>;

class IC : public IC_DispatchTable {

  public:
    enum { INTERRUPT = 1UL << 63 };

    static void bind(unsigned int id, Handler handler) {
        if constexpr (PLIC::Enable) {
            if (id > 11) {
                PLIC::bind(id - 12, handler);
                return;
            }
        }
        IC_DispatchTable::bind(id, handler);
    }
};

} // namespace riscv64
