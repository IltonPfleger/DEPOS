#include <cpu.h>
#include <definitions.h>
// #include <memory.h>
#include <io/io.h>
#include <io/uart.h>
// #include <utils/welcome.h>

void kmain() {
    if (cpu_id() == 0) {
        io_init(UART());
        io_out("\nQ U A R K | [μKernel]\n");

        //  memory_init();
        // kprint(WELCOME);
    } else {
        cpu_sleep();
    }
    cpu_sleep();
}

__attribute__((naked)) void ktrap() {
    // cpu_scontext();
    // kprint("Trap!\n");
    /// wcpu_lcontext();
    /* INTERRUPT OR EXCEPTION */
    //__asm__ volatile("csrr t0, mcause\nsrli t0, t0, %0" ::"i"(XLEN - 1));
    //__asm__ goto("beq t0, zero, %l[EXCEPTION]\nj %l[INTERRUPT]" ::
    //::EXCEPTION, INTERRUPT);

    // EXCEPTION:
    //     kprint("EXCEPTION\n");
    //     goto EXIT;
    // INTERRUPT:
    //     kprint("INTERRUPT\n");
    //     goto EXIT;
    // EXIT:
    //     __asm__ volatile("j .");
}

__attribute__((naked, section(".boot"))) void kboot() {
    cpu_disable_interrupts();

    // stack.
    __asm__ volatile(
        "li t0, 0x80200000\n"
        "li t1, %0\n"
        "csrr t2, mhartid\n"
        "mul t1, t1, t2\n"
        "add sp, t0, t1\n"
        :
        : "i"(4096));

    cpu_trap(ktrap);
    cpu_enable_interrupts();

    kmain();
}
