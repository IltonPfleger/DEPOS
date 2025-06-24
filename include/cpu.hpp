#ifndef CPU_HPP
#define CPU_HPP
#include <definitions.hpp>

struct CPU {
    struct Context {
        uintptr_t ra;
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

        // __attribute__((always_inline)) static inline void set(Context *c) { __asm__ volatile("mv tp, %0" ::"r"(c)); }

        __attribute__((always_inline)) static inline Context *get() {
            Context *c;
            __asm__ volatile("mv %0, sp" : "=r"(c));
            return c;
        }

        __attribute__((always_inline)) static inline void push() {
            __asm__ volatile(
                "addi sp, sp, %0\n"
                "sd ra, 0(sp)\n"
                "sd t0, 8(sp)\n"
                "sd t1, 16(sp)\n"
                "sd t2, 24(sp)\n"
                "sd t3, 32(sp)\n"
                "sd t4, 40(sp)\n"
                "sd t5, 48(sp)\n"
                "sd t6, 56(sp)\n"
                "sd a0, 64(sp)\n"
                "sd a1, 72(sp)\n"
                "sd a2, 80(sp)\n"
                "sd a3, 88(sp)\n"
                "sd a4, 96(sp)\n"
                "sd a5, 104(sp)\n"
                "sd a6, 112(sp)\n"
                "sd a7, 120(sp)\n"
                "sd s0, 128(sp)\n"
                "sd s1, 136(sp)\n"
                "sd s2, 144(sp)\n"
                "sd s3, 152(sp)\n"
                "sd s4, 160(sp)\n"
                "sd s5, 168(sp)\n"
                "sd s6, 176(sp)\n"
                "sd s7, 184(sp)\n"
                "sd s8, 192(sp)\n"
                "sd s9, 200(sp)\n"
                "sd s10, 208(sp)\n"
                "sd s11, 216(sp)\n" ::"i"(-sizeof(Context)));
        }

        __attribute__((always_inline)) static inline void pop() {
            __asm__ volatile(
                "ld ra, 0(sp)\n"
                "ld t0, 8(sp)\n"
                "ld t1, 16(sp)\n"
                "ld t2, 24(sp)\n"
                "ld t3, 32(sp)\n"
                "ld t4, 40(sp)\n"
                "ld t5, 48(sp)\n"
                "ld t6, 56(sp)\n"
                "ld a0, 64(sp)\n"
                "ld a1, 72(sp)\n"
                "ld a2, 80(sp)\n"
                "ld a3, 88(sp)\n"
                "ld a4, 96(sp)\n"
                "ld a5, 104(sp)\n"
                "ld a6, 112(sp)\n"
                "ld a7, 120(sp)\n"
                "ld s0, 128(sp)\n"
                "ld s1, 136(sp)\n"
                "ld s2, 144(sp)\n"
                "ld s3, 152(sp)\n"
                "ld s4, 160(sp)\n"
                "ld s5, 168(sp)\n"
                "ld s6, 176(sp)\n"
                "ld s7, 184(sp)\n"
                "ld s8, 192(sp)\n"
                "ld s9, 200(sp)\n"
                "ld s10, 208(sp)\n"
                "ld s11, 216(sp)\n"
                "addi sp, sp, %0\n" ::"i"(sizeof(Context)));
        }

        __attribute__((naked)) static void swap(CPU::Context **current, CPU::Context *next) {
            CPU::Context::push();
            __asm__ volatile("beq a0, zero, 1f\nsd sp, 0(a0)\n1:");
            __asm__ volatile("mv sp, %0" ::"r"(next));
            __asm__ volatile(
                "csrw mepc, %0\n"
                "csrr t0, mstatus\n"
                "li   t1, 0x1800\n"
                "or   t0, t0, t1\n"
                "csrw mstatus, t0\n" ::"r"((next)->pc)
                : "t1", "t0");
            CPU::Context::pop();
            CPU::iret();
        }
    };

    struct Trap {
        enum class Type { INTERRUPT = 1, EXCEPTION = 0 };

        static inline uintptr_t ra() {
            uintptr_t r;
            __asm__ volatile("csrr %0, mepc" : "=r"(r));
            return r;
        };

        static inline uintptr_t cause() {
            uintptr_t r;
            __asm__ volatile("csrr %0, mcause" : "=r"(r));
            return r;
        }

        static inline Type type() { return static_cast<Type>(cause() >> (Machine::XLEN - 1)); }
    };

    struct Interrupt {
        enum class Type { TIMER = 7 };

        static Type type() {
            uintptr_t r = Trap::cause();
            r           = (r << 1) >> 1;
            return static_cast<Type>(r);
        }

        __attribute__((always_inline)) static inline void disable() { __asm__ volatile("csrci mstatus, 0x8"); }
        __attribute__((always_inline)) static inline void enable() { __asm__ volatile("csrsi mstatus, 0x8"); }
    };

    __attribute__((always_inline)) static inline void enable_timer_interrupts() {
        __asm__ volatile("li t0, 0x80\ncsrs mie, t0" ::: "t0");
    }
    __attribute__((always_inline)) static inline void disable_timer_interrupts() {
        __asm__ volatile("li t0, 0x80\ncsrc mie, t0" ::: "t0");
    }

    __attribute__((always_inline)) static inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((always_inline)) static inline void halt() { for (;;); }

    __attribute__((always_inline)) static inline unsigned int id() {
        unsigned int id;
        __asm__ volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    __attribute__((always_inline)) static inline void stack(void *ptr) { __asm__ volatile("mv sp, %0" ::"r"(ptr)); }

    __attribute__((always_inline)) static inline void trap(void (*ptr)()) {
        __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
    }
};

#endif
