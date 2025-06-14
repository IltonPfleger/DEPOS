#include <definitions.h>

__attribute__((naked, section(".boot"))) void bootloader() {
    // disable interrupts.
    __asm__ volatile("csrci mstatus, 0x8");

    // stack.
    __asm__ volatile(
        "la t0, cpu_core_stacks\n"
        "csrr t1, mhartid\n"
        "li t2, %0\n"
        "mul t1, t1, t2\n"
        "add sp, t0, t1\n"
        :
        : "i"(MACHINE_MEMORY_PAGE_SIZE));

    // trap handler.
    __asm__ volatile("la t0, ktrap\ncsrw mtvec, t0" ::: "t0");

    // start.
    __asm__ volatile("j kmain");
}
