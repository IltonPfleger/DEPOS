#include <kernel.h>

__attribute__((naked, section(".boot"))) int bootloader()
{
	// disable interrupts.
    __asm__ volatile("csrci mstatus, 0x8");

    // stack.
    __asm__ volatile("li sp, 0x80200000");

    // trap handler.
    __asm__ volatile("la t0, ktrap\ncsrw mtvec, t0" ::: "t0");

    kmain();

    return 1;
}
