#pragma once

#include <utils/Debug.hpp>

namespace rv {
class IC {

  public:
    static constexpr unsigned long INTERRUPT = 1UL << 63;
    typedef void (*Handler)(unsigned int);

    static void bind(unsigned int id, Handler handler) { s_handlers[id] = handler; }

    static void dispatch(unsigned int id) {
        if (s_handlers[id])
            s_handlers[id](id);
        else
            ERROR(true, "Invalid Interrupt: %d\n", id);
    }

  private:
    static inline Handler s_handlers[Traits<PLIC>::Last];
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
