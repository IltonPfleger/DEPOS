#pragma once

namespace rv {
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

} // namespace rv

namespace rv64 {
using SIC = rv::SIC;
}
