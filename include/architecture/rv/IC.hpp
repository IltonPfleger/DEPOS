#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/CPU.hpp>
#include <architecture/rv/Context.hpp>
#include <architecture/rv/Modes.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/csrs.hpp>
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
            ERROR(true);
    }

  private:
    static inline Handler s_handlers[100];
};

template <typename Mode = void> class Exception {
  public:
    static void dispatch() {
        char prefix = Mode::PREFIX;
        auto status = reinterpret_cast<void *>(csrr<Mode::STATUS>());
        auto epc = reinterpret_cast<void *>(csrr<Mode::EPC>());
        auto tval = reinterpret_cast<void *>(csrr<Mode::TVAL>());
        auto cause = reinterpret_cast<void *>(csrr<Mode::CAUSE>());
        ERROR(true,
              "Ohh it's a Trap!\n"
              "%ccause: %d\n"
              "%cepc: %p\n"
              "%ctval: %p\n"
              "%cstatus: %p\n",
              prefix, cause, prefix, epc, prefix, tval, prefix, status);
    }
};

class MIC {
    using Mode = MachineMode;
    using Context = ContextBase<Mode>;

    template <unsigned int ID> static void handler(unsigned int) {
        if constexpr (ID == 7) {
            if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
                csrc<Mode::IE>(Mode::TI);
                csrs<Mode::IP>(SupervisorMode::TI);
            } else {
                int core = CPU::id();
                CLINT::reset(core);
                Timer::handler(core);
            }
        }
    }

    static void dispatch(Context *) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            Exception<MachineMode>::dispatch();
        }
    }

  public:
    static void init() {
        CLINT::init();
        PLIC::init();
        IC::bind(7, handler<7>);
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(Context::push());
        Context::pop();
    }
};

class SIC {
    using Mode = SupervisorMode;
    using Context = ContextBase<Mode>;

    // enum { TIME = 5 };

    static void handle(Context *) {
        // uintmax_t scause = csrr<Mode::CAUSE>();
        // int code = (scause << 1) >> 1;
        // if (scause & Base::IS_INTERRUPT) {
        //     switch (code) {
        //     case TIME:
        //         auto core = CPU::id();
        //         CPU::syscall(Syscall::TIME);
        //         Timer::handler(core);
        //         break;
        //     }
        //     return;
        // }

        // Base::error();
    }

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handle(Context::push());
        Context::pop();
    }
};

class Syscall {
    friend MIC;

  public:
    enum : unsigned int { TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E' };

  private:
    static void handle(void *function) {
        auto core = CPU::id();
        auto code = reinterpret_cast<uintptr_t>(function);
        if (code == TIME) {
            CLINT::reset(core);
            csrs<MachineMode::IE>(MachineMode::TI);
            csrc<MachineMode::IP>(SupervisorMode::TI);
        }
    }
};
} // namespace rv

namespace rv64 {
using IC = rv::IC;
using MIC = rv::MIC;
using SIC = rv::SIC;
} // namespace rv64
