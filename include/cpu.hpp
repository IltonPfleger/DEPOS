#ifndef CPU_HPP
#define CPU_HPP
#include <definitions.hpp>

struct CPU {
    static uint8_t stack[Machine::CPUS][Machine::Memory::Page::SIZE];

    static uint32_t id();
    static void idle();
    static void begin_atomic();
    static void end_atomic();

    __attribute__((always_inline)) static inline void save() {
        __asm__ volatile(
            "csrw mscratch, a0\n"
            "csrw sscratch, a1\n"
            "csrr a0, mhartid\n"
            "li a1, %0\n"
            "mul a1, a1, a0\n"
            "la a0, _ZN3CPU5stackE\n"
            "add a0, a0, a1\n"
            "csrr a1, sscratch\n"
            "sd ra, 0(a0)\n"
            "sd sp, 8(a0)\n"
            "sd gp, 16(a0)\n"
            "sd tp, 24(a0)\n"
            "sd t0, 32(a0)\n"
            "sd t1, 40(a0)\n"
            "sd t2, 48(a0)\n"
            "sd s0, 56(a0)\n"
            "sd s1, 64(a0)\n"
            "sd a1, 72(a0)\n"
            "sd a2, 80(a0)\n"
            "sd a3, 88(a0)\n"
            "sd a4, 96(a0)\n"
            "sd a5, 104(a0)\n"
            "sd a6, 112(a0)\n"
            "sd a7, 120(a0)\n"
            "sd s2, 128(a0)\n"
            "sd s3, 136(a0)\n"
            "sd s4, 144(a0)\n"
            "sd s5, 152(a0)\n"
            "sd s6, 160(a0)\n"
            "sd s7, 168(a0)\n"
            "sd s8, 176(a0)\n"
            "sd s9, 184(a0)\n"
            "sd s10, 192(a0)\n"
            "sd s11, 200(a0)\n"
            "sd t3, 208(a0)\n"
            "sd t4, 216(a0)\n"
            "sd t5, 224(a0)\n"
            "sd t6, 232(a0)\n"
            "add a1, a0, zero\n"
            "csrr a0, mscratch\n"
            "sd a0, 240(a1)\n"
            :
            : "i"(Machine::Memory::Page::SIZE)
            : "memory");
    }

    __attribute__((always_inline)) static inline void load() {
        __asm__ volatile(
            "csrr a0, mhartid\n"
            "li a1, %0\n"
            "mul a1, a1, a0\n"
            "la a0, _ZN3CPU5stackE\n"
            "add a0, a0, a1\n"
            "ld ra, 0(a0)\n"
            "ld sp, 8(a0)\n"
            "ld gp, 16(a0)\n"
            "ld tp, 24(a0)\n"
            "ld t0, 32(a0)\n"
            "ld t1, 40(a0)\n"
            "ld t2, 48(a0)\n"
            "ld s0, 56(a0)\n"
            "ld s1, 64(a0)\n"
            "ld a1, 72(a0)\n"
            "ld a2, 80(a0)\n"
            "ld a3, 88(a0)\n"
            "ld a4, 96(a0)\n"
            "ld a5, 104(a0)\n"
            "ld a6, 112(a0)\n"
            "ld a7, 120(a0)\n"
            "ld s2, 128(a0)\n"
            "ld s3, 136(a0)\n"
            "ld s4, 144(a0)\n"
            "ld s5, 152(a0)\n"
            "ld s6, 160(a0)\n"
            "ld s7, 168(a0)\n"
            "ld s8, 176(a0)\n"
            "ld s9, 184(a0)\n"
            "ld s10, 192(a0)\n"
            "ld s11, 200(a0)\n"
            "ld t3, 208(a0)\n"
            "ld t4, 216(a0)\n"
            "ld t5, 224(a0)\n"
            "ld t6, 232(a0)\n"
            "ld a0, 240(a0)\n"
            :
            : "i"(Machine::Memory::Page::SIZE)
            : "memory");
    }
};

#endif
