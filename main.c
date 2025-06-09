#include <kernel.h>

__attribute__((naked, section(".boot"))) int main()
{
    /* DISABLE ALL INTERRUPTS */
    __asm__ volatile("csrci mstatus, 0x8");

    /* SETUP TEMPORARY STACK */
    __asm__ volatile("li sp, 0x80200000");

    /* SETUP MACHINE TRAP VECTOR (TRAP HANDLER) */
    __asm__ volatile("la t0, ktrap\ncsrw mtvec, t0" ::: "t0");

    /* GIVE INITIAL ACESS TO THE HOLE MEMORY */
    //__asm__ volatile("li t0, 0xFFFFFFFFFFFFFFF\ncsrw pmpaddr0, t0\nli t0, 0x9F\ncsrw pmpcfg0, t0" ::: "t0");

    kmain();

    return 1;
}
