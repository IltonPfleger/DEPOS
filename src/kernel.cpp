#include <cpu.hpp>
#include <definitions.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>

void kmain() {
    if (CPU::id() == 0) {
        CPU::begin_atomic();
        IO<UART>::init();
        IO<UART>::out("\nQ U A R K | [μKernel]\n");
        __asm__ volatile(".word 0xffffffff");
        // IO<UART0>::init();
        // Memory::init(reinterpret_cast<uintptr_t>(__KERNEL_END__));
        // Memory::alloc(13);
        CPU::end_atomic();
    } else {
        CPU::idle();
    }
    CPU::idle();
}

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::save();
    IO<UART>::out("Ohh it's a Trap!\n");
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    IO<UART>::out("mcause: %x\n", mcause);
    IO<UART>::out("mepc: %x\n", mepc);
    IO<UART>::out("mtval: %x\n", mtval);

    if ((mcause >> (Machine::XLEN - 1)) == 0) {
        IO<UART>::out("Exception Detected!\n");
    } else {
        IO<UART>::out("Interruption Detected!\n");
    }

    CPU::load();
    CPU::idle();
}
