#pragma once

#include "Machine.hpp"

// class SupervisorFirmware {
//     using Stack = char[Traits<Memory>::PAGE_SIZE];
//     static constexpr bool Enable = Traits<System>::MULTITASK;
//
//   public:
//     class IC {
//         static void handler(Context *c) {
//             uintmax_t mcause = csrr<Machine::CAUSE>();
//
//             if (mcause >> (Traits<::Machine>::XLEN - 1)) {
//                 Machine::Interruption code = static_cast<Machine::Interruption>((mcause << 1) >> 1);
//                 interruption(code);
//                 return;
//             }
//
//             if (mcause == Machine::SUPERVISOR_SYSCALL) {
//                 syscall(c);
//                 return;
//             }
//
//             Machine::IC::error();
//         }
//
//         static void interruption(Machine::Interruption code) {
//             switch (code) {
//             case Machine::Interruption::TIMER:
//                 csrc<Machine::IE>(Machine::TI);
//                 csrs<Machine::IP>(Supervisor::TI);
//             }
//         }
//
//         static void syscall(Context *c) {
//             c->pc += 4;
//             switch (c->a7) {
//             case Syscall::TIME:
//                 CLINT::reset(CPU::id());
//                 csrs<Machine::IE>(Machine::TI);
//                 csrc<Machine::IP>(Supervisor::TI);
//             }
//         }
//
//       public:
//         enum Syscall {
//             TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E',
//         };
//
//         __attribute__((naked, aligned(4))) static void entry() {
//             handler(Context::push<Machine, true>());
//             Context::pop<Machine, true>();
//         }
//     };
//
//   public:
//     static void init() {
//         csrc<Machine::STATUS>(Machine::IRQE);
//
//         if (!(csrr<Machine::MISA>() & (1UL << ('S' - 'A')))) {
//             CPU::kill();
//             CPU::idle();
//         }
//
//         if constexpr (Traits<Timer>::Enable) {
//             csrs<Machine::IE>(Machine::TI);
//         }
//
//         if constexpr (Enable) {
//             csrw<Machine::SCRATCH>(s_stack[CPU::id()].Result);
//         }
//
//         csrw<Machine::TVEC>(IC::entry);
//         csrw<Machine::MIDELEG>(0x222);
//         csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
//         csrw<Machine::PMPCFG0>(0b11111);
//         csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIRQE);
//         csrc<Machine::STATUS>(Supervisor::PIRQE);
//         csrw<Machine::EPC>(__builtin_return_address(0));
//         Machine::ret();
//     }
//
//     static inline Meta::ConditionalValue<Stack, Enable> s_stack[Traits<::Machine>::CPUS];
// };
//
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
        ME2ME = 1ULL << 8,   // Supervisor to Supervisor
        ME2USER = 0ULL << 8, // Supervisor to User
        IRQE = 1ULL << 1,    // Interrupt Enable
        TI = 1ULL << 5,      // Timer Interrupt Enable
        PIRQE = 1ULL << 5,   // Previous Interrupt Enable
        SUM = 1ULL << 18     // Supervisor User Memory access
    };
    //
    //     class IC {
    //         enum Interruption { TIMER = 5 };
    //
    //         static void handler(Context *) {
    //             uintmax_t scause = csrr<Supervisor::CAUSE>();
    //
    //             if (scause >> (Traits<::Machine>::XLEN - 1)) {
    //                 Interruption code = static_cast<Interruption>((scause << 1) >> 1);
    //                 interruption(code);
    //                 return;
    //             }
    //         }
    //
    //         static void interruption(Interruption code) {
    //             switch (code) {
    //             case Interruption::TIMER:
    //                 Timer::handler(CPU::id());
    //                 CPU::syscall(SupervisorFirmware::IC::Syscall::TIME);
    //                 break;
    //             }
    //         }
    //
    //       public:
    //         __attribute__((naked, aligned(4))) static void entry() {
    //             handler(Context::push<Supervisor>());
    //             Context::pop<Supervisor>();
    //         }
    //     };
    //
    //     __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }
    //
    //     static void init() {
    //         SupervisorFirmware::init();
    //         csrw<Supervisor::TVEC>(Supervisor::IC::entry);
    //         csrs<Supervisor::IE>(Supervisor::TI);
    //     }
};
