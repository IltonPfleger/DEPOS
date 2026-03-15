#pragma once

#include <architecture/riscv64/csrs.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace riscv64 {

class Exception {
    static constexpr const char *Descriptions[] = {
        "Instruction Address Misaligned", // 0
        "Instruction Access Fault",       // 1
        "Illegal Instruction",            // 2
        "Breakpoint",                     // 3
        "Load Address Misaligned",        // 4
        "Load Access Fault",              // 5
        "Store/AMO Address Misaligned",   // 6
        "Store/AMO Access Fault",         // 7
        "Environment Call from U-mode",   // 8
        "Environment Call from S-mode",   // 9
        "Reserved (10)",                  // 10
        "Environment Call from M-mode",   // 11
        "Instruction Page Fault",         // 12
        "Load Page Fault",                // 13
        "Reserved (14)",                  // 14
        "Store/AMO Page Fault"            // 15
    };

  public:
    static void dispatch(unsigned int id, Context *c) {
        Console::cout << Console::panic;
        Console::cout << "Ohh, It's a Trap!" << Console::endl;
        Console::cout << Descriptions[c->cause] << Console::endl;
        Console::cout << "PC: " << Console::hex << c->pc << Console::endl;
        Console::cout << "Cause: " << id << Console::endl;
        Console::cout << "Status: " << Console::hex << c->status << Console::endl;
        Console::cout << "Value: " << Console::hex << c->tval << Console::endl;
        CPU::halt();
    }
};

} // namespace riscv64

} // namespace DEPOS
