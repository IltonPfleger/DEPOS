#include <cpu.hpp>
#include <definitions.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <process.hpp>

static struct Process kProcs[Machine::CPUS];

struct Kernel {
    static void init() {
        if (CPU::id() == 0) {
            CPU::disable_interrupts();
            IO::init();
            IO::out("\nQ U A R K | [μKernel]\n");
            Memory::init();
            void* mem  = Memory::alloc(25);
            void* mem2 = Memory::alloc(25);
            Memory::free(mem, 25);
            Memory::free(mem2, 25);
            mem  = Memory::alloc(25);
            mem2 = Memory::alloc(25);
            Memory::free(mem, 25);
            Memory::free(mem2, 25);

            __asm__ volatile(".word 0xffffffff");
            IO::out("Done!\n");
            CPU::enable_interrupts();
        }
        CPU::idle();
    }
};

// void kmain() {
//     if (CPU::id() == 0) {
//         CPU::disable_interrupts();
//         IO::init();
//         IO::out("\nQ U A R K | [μKernel]\n");
//         Memory::init();
//         void* mem  = Memory::alloc(25);
//         void* mem2 = Memory::alloc(25);
//         Memory::free(mem, 25);
//         Memory::free(mem2, 25);
//         mem  = Memory::alloc(25);
//         mem2 = Memory::alloc(25);
//         Memory::free(mem, 25);
//         Memory::free(mem2, 25);
//
//         __asm__ volatile(".word 0xffffffff");
//         IO::out("Done!\n");
//         CPU::enable_interrupts();
//     }
//     CPU::idle();
// }

__attribute__((naked)) void ktrap() {
    CPU::save();
    CPU::stack(kProcs[CPU::id()].stack);
    CPU::context(kProcs[CPU::id()].context);

    IO::out("Ohh it's a Trap!\n");
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    IO::out("mcause: %p\n", mcause);
    IO::out("mepc: %p\n", mepc);
    IO::out("mtval: %p\n", mtval);

    if ((mcause >> (Machine::XLEN - 1)) == 0) {
        IO::out("Exception Detected!\n");
    } else {
        IO::out("Interruption Detected!\n");
    }

    CPU::load();
    CPU::idle();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::stack(kProcs[CPU::id()].stack);
    CPU::context(kProcs[CPU::id()].context);
    CPU::trap(ktrap);
    Kernel::init();
}
