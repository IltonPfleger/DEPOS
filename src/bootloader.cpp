#include <cpu.hpp>
#include <definitions.hpp>

extern void kmain();
extern void ktrap();

__attribute__((naked, section(".boot"))) void bootloader() {

    CPU::load_stack_per_id();

    // trap handler.
    __asm__ volatile("csrw mtvec, %0" ::"r"(ktrap));

    // start.
    __asm__ volatile("jr %0" ::"r"(kmain));
}
