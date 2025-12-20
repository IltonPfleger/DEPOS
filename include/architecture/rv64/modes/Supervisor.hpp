#pragma once

#include "Machine.hpp"

class Supervisor {
  public:
    enum Registers : unsigned long {
        STATUS = 0x100,  // Supervisor Status
        IE = 0x104,      // Interrupt Enable
        TVEC = 0x105,    // Trap Vector Base Address
        SCRATCH = 0x140, // Temporary Register
        EPC = 0x141,     // Exception Program Counter
        CAUSE = 0x142,   // Trap Cause
        TVAL = 0x143,    // Trap Value
        IP = 0x144,      // Pending Interrupts
        SATP = 0x180     // Page Table Base And MMU Mode
    };

    enum Bits : unsigned long {
        ME2ME = 1ULL << 8,                        // Supervisor to Supervisor
        ME2USER = 0ULL << 8,                      // Supervisor to User
        IRQE = 1ULL << 1,                         // Interrupt Enable
        TI = 1ULL << 5,                           // Timer Interrupt Enable
        PIRQE = 1ULL << 5,                        // Previous Interrupt Enable
        SUM = 1ULL << 18                          // Supervisor User Memory access
    };

    class IC {
        enum Interruption { TIMER = 5 };

        static void handler(Context *) {
            uintmax_t scause = csrr<Supervisor::CAUSE>();

            if (scause >> (Traits<::Machine>::XLEN - 1)) {
                Interruption code = static_cast<Interruption>((scause << 1) >> 1);
                interruption(code);
                return;
            }
        }

        static void interruption(Interruption code) {
            switch (code) {
            case Interruption::TIMER:
                Timer::handler(CPU::id());
                CPU::syscall(Machine::Syscall::TIME);
                break;
            }
        }

      public:
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Supervisor>());
            Context::pop<Supervisor>();
        }
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }

    static void init() {
        Machine::init();
        csrw<Supervisor::TVEC>(Supervisor::IC::entry);
        csrs<Supervisor::IE>(Supervisor::TI);
    }
};
