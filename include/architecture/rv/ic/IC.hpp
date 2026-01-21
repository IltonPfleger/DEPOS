#pragma once

#include <architecture/rv/PLIC.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

namespace rv {
class IC : public DispatchTable<0, 11, IC> {
    using Base = DispatchTable<0, 11, IC>;

  public:
    static void bind(unsigned int id, Base::Handler handler) {
        if (id > 11)
            PLIC::bind(id - 12, handler);
        else
            Base::bind(id, handler);
    }

    static constexpr unsigned long INTERRUPT = 1UL << 63;
};

// class Syscall {
//     friend MIC;
//
//   public:
//     enum : unsigned int { TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E' };
//
//   private:
//     static void handle(void *function) {
//         auto core = CPU::id();
//         auto code = reinterpret_cast<uintptr_t>(function);
//         if (code == TIME) {
//             CLINT::reset(core);
//             csrs<MachineMode::IE>(MachineMode::TI);
//             csrc<MachineMode::IP>(SupervisorMode::TI);
//         }
//     }
// };
} // namespace rv

namespace rv64 {
using IC = rv::IC;
} // namespace rv64
