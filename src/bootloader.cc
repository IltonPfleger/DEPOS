#include <kernel.h>

__attribute__((naked, section(".boot"))) int bootloader() {
    // stack.
    __asm__ volatile("li sp, 0x80200000");

    // trap handler.
    __asm__ volatile("csrw mtvec, %0" :: "r"(ktrap));

    kmain();

    return 1;
}
