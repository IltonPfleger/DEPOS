#include <CPU.hpp>
#include <kernel.hpp>
#include <utils/definitions.hpp>


__attribute__((naked, section(".boot"))) int bootloader() {
    // stacks.
    __asm__ volatile(
        "la t0, %[stack_base]\n"
        "csrr t1, mhartid\n"
        "mul t1, t1, %[stack_size]\n"
        "add sp, t0, t1\n"
        :
        : [stack_base] "i"(CPU::stack), [stack_size] "r"(Machine::Memory::STACK_SIZE)
        : "t0", "t1" );

    // trap handler.
    __asm__ volatile("csrw mtvec, %0" : : "r"(ktrap) : "memory");

	__asm__ volatile("call kmain");

	kmain();

    return 1;
}
