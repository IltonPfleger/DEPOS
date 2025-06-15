#ifndef CPU_HPP
#define CPU_HPP
#include <definitions.hpp>

struct CPU {
    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }
    __attribute__((always_inline)) static inline void begin_atomic() { __asm__ volatile("csrci mstatus, 0x8"); }
    __attribute__((always_inline)) static inline void end_atomic() { __asm__ volatile("csrsi mstatus, 0x8"); }

    __attribute__((always_inline)) static inline uint32_t id() {
        uint32_t id;
        __asm__ volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    __attribute__((always_inline)) static inline void stack(char* ptr) {
        __asm__ volatile("add sp, %0, zero" ::"r"(ptr));
    }

    __attribute__((always_inline)) static inline void context(char* ptr) {
        __asm__ volatile("add tp, %0, zero" ::"r"(ptr));
    }

    __attribute__((always_inline)) static inline void trap(void (*ptr)()) {
        __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
    }

    __attribute__((always_inline)) static inline void save() {
        __asm__ volatile(
            "sd ra, 0(tp)\n"
            "sd sp, 8(tp)\n"
            "sd gp, 16(tp)\n"
            "sd t0, 24(tp)\n"
            "sd t1, 32(tp)\n"
            "sd t2, 40(tp)\n"
            "sd s0, 48(tp)\n"
            "sd s1, 56(tp)\n"
            "sd a0, 64(tp)\n"
            "sd a1, 72(tp)\n"
            "sd a2, 80(tp)\n"
            "sd a3, 88(tp)\n"
            "sd a4, 96(tp)\n"
            "sd a5, 104(tp)\n"
            "sd a6, 112(tp)\n"
            "sd a7, 120(tp)\n"
            "sd s2, 128(tp)\n"
            "sd s3, 136(tp)\n"
            "sd s4, 144(tp)\n"
            "sd s5, 152(tp)\n"
            "sd s6, 160(tp)\n"
            "sd s7, 168(tp)\n"
            "sd s8, 176(tp)\n"
            "sd s9, 184(tp)\n"
            "sd s10, 192(tp)\n"
            "sd s11, 200(tp)\n"
            "sd t3, 208(tp)\n"
            "sd t4, 216(tp)\n"
            "sd t5, 224(tp)\n"
            "sd t6, 232(tp)\n"
            "sd tp, 240(tp)\n"
            :
            : "i"(Machine::Memory::Page::SIZE)
            : "memory");
    }

    __attribute__((always_inline)) static inline void load() {
        __asm__ volatile(
            "ld ra, 0(tp)\n"
            "ld sp, 8(tp)\n"
            "ld gp, 16(tp)\n"
            "ld t0, 24(tp)\n"
            "ld t1, 32(tp)\n"
            "ld t2, 40(tp)\n"
            "ld s0, 48(tp)\n"
            "ld s1, 56(tp)\n"
            "ld a0, 64(tp)\n"
            "ld a1, 72(tp)\n"
            "ld a2, 80(tp)\n"
            "ld a3, 88(tp)\n"
            "ld a4, 96(tp)\n"
            "ld a5, 104(tp)\n"
            "ld a6, 112(tp)\n"
            "ld a7, 120(tp)\n"
            "ld s2, 128(tp)\n"
            "ld s3, 136(tp)\n"
            "ld s4, 144(tp)\n"
            "ld s5, 152(tp)\n"
            "ld s6, 160(tp)\n"
            "ld s7, 168(tp)\n"
            "ld s8, 176(tp)\n"
            "ld s9, 184(tp)\n"
            "ld s10, 192(tp)\n"
            "ld s11, 200(tp)\n"
            "ld t3, 208(tp)\n"
            "ld t4, 216(tp)\n"
            "ld t5, 224(tp)\n"
            "ld t6, 232(tp)\n"
            "ld tp, 240(tp)\n"
            :
            : "i"(Machine::Memory::Page::SIZE)
            : "memory");
    }
};

#endif
