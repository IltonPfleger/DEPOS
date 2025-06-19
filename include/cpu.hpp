#ifndef CPU_HPP
#define CPU_HPP
#include <definitions.hpp>

struct CPU {
    struct Context {
        uintptr_t ra;
        uintptr_t sp;
        uintptr_t gp;
        uintptr_t t0;
        uintptr_t t1;
        uintptr_t t2;
        uintptr_t t3;
        uintptr_t t4;
        uintptr_t t5;
        uintptr_t t6;
        uintptr_t a0;
        uintptr_t a1;
        uintptr_t a2;
        uintptr_t a3;
        uintptr_t a4;
        uintptr_t a5;
        uintptr_t a6;
        uintptr_t a7;
        uintptr_t s0;
        uintptr_t s1;
        uintptr_t s2;
        uintptr_t s3;
        uintptr_t s4;
        uintptr_t s5;
        uintptr_t s6;
        uintptr_t s7;
        uintptr_t s8;
        uintptr_t s9;
        uintptr_t s10;
        uintptr_t s11;
        uintptr_t pc;

        __attribute__((always_inline)) static inline void set(void* p) {
            __asm__ volatile("mv tp, %0" ::"r"(p));
        }

        __attribute__((always_inline)) static inline void save() {
            __asm__ volatile(
                "sd ra, 0(tp)\n"
                "sd sp, 8(tp)\n"
                "sd gp, 16(tp)\n"
                "sd t0, 24(tp)\n"
                "sd t1, 32(tp)\n"
                "sd t2, 40(tp)\n"
                "sd t3, 48(tp)\n"
                "sd t4, 56(tp)\n"
                "sd t5, 64(tp)\n"
                "sd t6, 72(tp)\n"
                "sd a0, 80(tp)\n"
                "sd a1, 88(tp)\n"
                "sd a2, 96(tp)\n"
                "sd a3, 104(tp)\n"
                "sd a4, 112(tp)\n"
                "sd a5, 120(tp)\n"
                "sd a6, 128(tp)\n"
                "sd a7, 136(tp)\n"
                "sd s0, 144(tp)\n"
                "sd s1, 152(tp)\n"
                "sd s2, 160(tp)\n"
                "sd s3, 168(tp)\n"
                "sd s4, 176(tp)\n"
                "sd s5, 184(tp)\n"
                "sd s6, 192(tp)\n"
                "sd s7, 200(tp)\n"
                "sd s8, 208(tp)\n"
                "sd s9, 216(tp)\n"
                "sd s10, 224(tp)\n"
                "sd s11, 232(tp)\n");
        }

        __attribute__((always_inline)) static inline void load() {
            __asm__ volatile(
                "ld ra, 0(tp)\n"
                "ld sp, 8(tp)\n"
                "ld gp, 16(tp)\n"
                "ld t0, 24(tp)\n"
                "ld t1, 32(tp)\n"
                "ld t2, 40(tp)\n"
                "ld t3, 48(tp)\n"
                "ld t4, 56(tp)\n"
                "ld t5, 64(tp)\n"
                "ld t6, 72(tp)\n"
                "ld a0, 80(tp)\n"
                "ld a1, 88(tp)\n"
                "ld a2, 96(tp)\n"
                "ld a3, 104(tp)\n"
                "ld a4, 112(tp)\n"
                "ld a5, 120(tp)\n"
                "ld a6, 128(tp)\n"
                "ld a7, 136(tp)\n"
                "ld s0, 144(tp)\n"
                "ld s1, 152(tp)\n"
                "ld s2, 160(tp)\n"
                "ld s3, 168(tp)\n"
                "ld s4, 176(tp)\n"
                "ld s5, 184(tp)\n"
                "ld s6, 192(tp)\n"
                "ld s7, 200(tp)\n"
                "ld s8, 208(tp)\n"
                "ld s9, 216(tp)\n"
                "ld s10, 224(tp)\n"
                "ld s11, 232(tp)\n");
        }

        __attribute__((always_inline)) static inline void dispatch(
            Context* target) {
            //__asm__ volatile("mv gp, %0" ::"r"(target->pc));
            target->gp = target->pc;
            CPU::Context::set(target);
            CPU::Context::load();
            __asm__ volatile("jr gp");
        }
    };

    __attribute__((always_inline)) static inline void idle() {
        __asm__ volatile("wfi");
    }
    __attribute__((always_inline)) static inline void disable_interrupts() {
        __asm__ volatile("csrci mstatus, 0x8");
    }
    __attribute__((always_inline)) static inline void enable_interrupts() {
        __asm__ volatile("csrsi mstatus, 0x8");
    }

    __attribute__((always_inline)) static inline unsigned int get_id() {
        unsigned int id;
        __asm__ volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    __attribute__((always_inline)) static inline void set_stack(void* ptr) {
        __asm__ volatile("mv sp, %0" ::"r"(ptr));
    }

    __attribute__((always_inline)) static inline uintptr_t get_pc() {
        uintptr_t pc;
        __asm__ volatile("auipc %0, 0" ::"r"(pc));
        return pc;
    }

    __attribute__((always_inline)) static inline void set_trap_handler(
        void (*ptr)()) {
        __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
    }
};

#endif
