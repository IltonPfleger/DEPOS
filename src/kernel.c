// #include <utils/kprint.h>
// #include <utils/welcome.h>
// #include <memory.h>
#include <cpu.h>
#include <utils/io.h>
#include <utils/uart.h>

void kmain() {
    if (cpu_id() == 0) {
        IO_INIT(UART);
        // memory_init();
        // kprint(WELCOME);
    } else {
        cpu_idle();
    }
    cpu_idle();
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
