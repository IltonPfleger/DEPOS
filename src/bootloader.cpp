#include <definitions.hpp>

extern void kmain();
extern void ktrap();

__attribute__((naked, section(".boot"))) void bootloader() {
    // disable interrupts.
    __asm__ volatile("csrci mstatus, 0x8");

    // stack.
    __asm__ volatile(
        "la t0, _ZN3CPU5stackE\n"
        "csrr t1, mhartid\n"
        "li t2, %0\n"
        "mul t1, t1, t2\n"
        "add sp, t0, t1\n"
        :
        : "i"(Machine::Memory::Page::SIZE));

    // trap handler.
    __asm__ volatile("csrw mtvec, %0" ::"r"(ktrap));


    // enable interrupts.
    __asm__ volatile("csrsi mstatus, 0x8");

    // start.
    __asm__ volatile("jr %0" ::"r"(kmain));
}
