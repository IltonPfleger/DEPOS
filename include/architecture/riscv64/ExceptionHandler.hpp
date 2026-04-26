#pragma once

#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <shared/console/Console.hpp>

namespace DEPOS {

namespace riscv64 {

class ExceptionHandler {
    // static constexpr const char *Descriptions[] = {
    //     "Instruction Address Misaligned", // 0
    //     "Instruction Access Fault",       // 1
    //     "Illegal Instruction",            // 2
    //     "Breakpoint",                     // 3
    //     "Load Address Misaligned",        // 4
    //     "Load Access Fault",              // 5
    //     "Store/AMO Address Misaligned",   // 6
    //     "Store/AMO Access Fault",         // 7
    //     "Environment Call from U-mode",   // 8
    //     "Environment Call from S-mode",   // 9
    //     "Reserved (10)",                  // 10
    //     "Environment Call from M-mode",   // 11
    //     "Instruction Page Fault",         // 12
    //     "Load Page Fault",                // 13
    //     "Reserved (14)",                  // 14
    //     "Store/AMO Page Fault"            // 15
    // };

  public:
    static void onTrap(size_t id, Context *c) {
        Console::panic();
        Console::println("\nOhh, It's a Trap!");
        Console::println("context: ", c);
        Console::println("id: ", id);
        Console::println("pc: ", Console::Hex(c->pc));
        Console::println("cause: ", Console::Hex(c->cause));
        Console::println("status: ", Console::Hex(c->status));
        Console::println("tval: ", Console::Hex(c->value));
        CPU::halt();
    }
};

} // namespace riscv64

} // namespace DEPOS
