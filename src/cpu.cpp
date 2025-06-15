#include <cpu.hpp>

uint8_t __attribute__((aligned(Machine::Memory::Page::SIZE))) CPU::stack[Machine::CPUS][Machine::Memory::Page::SIZE];

//uint32_t CPU::id() {
//    uint32_t mhartid;
//    __asm__ volatile("csrr %0, mhartid" : "=r"(mhartid));
//    return mhartid;
//}
//
//void CPU::idle() { __asm__ volatile("wfi"); }
//void CPU::begin_atomic() { __asm__ volatile("csrci mstatus, 0x8"); }
//void CPU::end_atomic() { __asm__ volatile("csrsi mstatus, 0x8"); }
//
