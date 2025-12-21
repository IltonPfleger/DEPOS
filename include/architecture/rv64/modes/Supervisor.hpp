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
        MACHINE2ME = 1ULL << 11, // Machine to Supervisor
        ME2ME = 1ULL << 8,       // Supervisor to Supervisor
        ME2USER = 0ULL << 8,     // Supervisor to User
        IRQE = 1ULL << 1,        // Interrupt Enable
        TI = 1ULL << 5,          // Timer Interrupt Enable
        PIRQE = 1ULL << 5,       // Previous Interrupt Enable
        SUM = 1ULL << 18         // Supervisor User Memory access
    };

    class IC {
        enum Interruption {
            IS_INTERRUPTION = 1ULL << (Traits<::Machine>::XLEN - 1),
            TIMER = 5,
        };

        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Supervisor>());
            Context::pop<Supervisor>();
        }

        static void handler(Context *) {
            uintmax_t scause = csrr<Supervisor::CAUSE>();

            if (scause & IS_INTERRUPTION) {
                s_irqs.dispatch(scause & ~IS_INTERRUPTION);
                return;
            }
        }

        static void reset_timer() {
            Timer::handler(CPU::id());
            CPU::syscall(Machine::Syscall::TIME);
        }

      public:
        static void init() {
            csrw<TVEC>(entry);

            if constexpr (Traits<Timer>::Enable) {
                s_irqs.bind(Interruption::TIMER, reset_timer);
                csrs<IE>(TI);
            }
        }

        static inline DispatchTable<Traits<IRQ>::MinSupervisorModeIRQ, Traits<IRQ>::MaxSupervisorModeIRQ> s_irqs;
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }

    static void init() {
        Machine::init();
        IC::init();
    }
};
