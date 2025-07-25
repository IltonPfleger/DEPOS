#pragma once

#include <Machine.hpp>

struct CPU {
    __attribute__((always_inline)) static inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((always_inline)) static inline void *thread() {
        void *ptr;
        __asm__ volatile("mv %0, tp" : "=r"(ptr));
        return ptr;
    }

    __attribute__((always_inline)) static inline void thread(void *ptr) { __asm__ volatile("mv tp, %0" ::"r"(ptr)); }

    __attribute__((always_inline)) static inline unsigned int core() {
        unsigned int n;
        __asm__ volatile("csrr %0, mhartid" : "=&r"(n));
        return n;
    }

    struct __attribute__((packed)) Context {
        uintptr_t ra;
        uintptr_t tp;
        uintptr_t t0, t1, t2, t3, t4, t5, t6;
        uintptr_t a0, a1, a2, a3, a4, a5, a6, a7;
        uintptr_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uintptr_t epc;

        Context(int (*entry)(void *), void (*exit)(), void *thread, void *arg) {
            ra  = reinterpret_cast<uintptr_t>(exit);
            tp  = reinterpret_cast<uintptr_t>(thread);
            epc = reinterpret_cast<uintptr_t>(entry);
            a0  = reinterpret_cast<uintptr_t>(arg);
        }

        template <bool is_interrupt = false>
        __attribute__((always_inline)) static inline void push() {
            __asm__ volatile("addi sp, sp, %0" ::"i"(-sizeof(Context)));

            __asm__ volatile(
                "sd ra, 0(sp)\n"
                "sd tp, 8(sp)\n"
                "sd t0, 16(sp)\n"
                "sd t1, 24(sp)\n"
                "sd t2, 32(sp)\n"
                "sd t3, 40(sp)\n"
                "sd t4, 48(sp)\n"
                "sd t5, 56(sp)\n"
                "sd t6, 64(sp)\n"
                "sd a0, 72(sp)\n"
                "sd a1, 80(sp)\n"
                "sd a2, 88(sp)\n"
                "sd a3, 96(sp)\n"
                "sd a4, 104(sp)\n"
                "sd a5, 112(sp)\n"
                "sd a6, 120(sp)\n"
                "sd a7, 128(sp)\n"
                "sd s0, 136(sp)\n"
                "sd s1, 144(sp)\n"
                "sd s2, 152(sp)\n"
                "sd s3, 160(sp)\n"
                "sd s4, 168(sp)\n"
                "sd s5, 176(sp)\n"
                "sd s6, 184(sp)\n"
                "sd s7, 192(sp)\n"
                "sd s8, 200(sp)\n"
                "sd s9, 208(sp)\n"
                "sd s10, 216(sp)\n"
                "sd s11, 224(sp)\n" ::
                    : "memory");

            if constexpr (is_interrupt) {
                __asm__ volatile(
                    "csrr t0, mepc\n"
                    "sd t0, 232(sp)\n" ::
                        : "t0", "memory");
            } else {
                __asm__ volatile("sd ra, 232(sp)\n" ::: "memory");
            }
        }

        __attribute__((always_inline)) static inline void pop() {
            __asm__ volatile(
                "ld t0, 232(sp)\n"
                "csrw mepc, t0\n"
                "ld ra, 0(sp)\n"
                "ld tp, 8(sp)\n"
                "ld t0, 16(sp)\n"
                "ld t1, 24(sp)\n"
                "ld t2, 32(sp)\n"
                "ld t3, 40(sp)\n"
                "ld t4, 48(sp)\n"
                "ld t5, 56(sp)\n"
                "ld t6, 64(sp)\n"
                "ld a0, 72(sp)\n"
                "ld a1, 80(sp)\n"
                "ld a2, 88(sp)\n"
                "ld a3, 96(sp)\n"
                "ld a4, 104(sp)\n"
                "ld a5, 112(sp)\n"
                "ld a6, 120(sp)\n"
                "ld a7, 128(sp)\n"
                "ld s0, 136(sp)\n"
                "ld s1, 144(sp)\n"
                "ld s2, 152(sp)\n"
                "ld s3, 160(sp)\n"
                "ld s4, 168(sp)\n"
                "ld s5, 176(sp)\n"
                "ld s6, 184(sp)\n"
                "ld s7, 192(sp)\n"
                "ld s8, 200(sp)\n"
                "ld s9, 208(sp)\n"
                "ld s10, 216(sp)\n"
                "ld s11, 224(sp)\n"
                "addi sp, sp, %0\n" ::"i"(sizeof(Context))
                : "memory");
        }

        __attribute__((naked)) static void jump(volatile Context *next) {
            __asm__ volatile("mv sp, %0" ::"r"(next));
            __asm__ volatile("li t0, 0x1800\ncsrs mstatus, t0\n" ::: "t0");
            pop();
            iret();
        }

        __attribute__((naked)) static volatile Context *get() { __asm__ volatile("mv a0, sp\nret"); }
        __attribute__((naked)) static void set(volatile Context *) { __asm__ volatile("mv sp, a0\nret"); }
    };

    struct Atomic {
        static void lock(int *value) {
            __asm__ volatile(
                "li t0, 1\n"
                "1:\n"
                "   lr.w t1, (%0)\n"
                "   bnez t1, 1b\n"
                "   sc.w t1, t0, (%0)\n"
                "   bnez t1, 1b\n"
                :
                : "r"(value)
                : "t0", "t1", "memory");
        }

        static void unlock(int *value) { __asm__ volatile("sw zero, (%0)\n" : : "r"(value) : "memory"); }

        static int fdec(int *value) {
            int ret;
            __asm__ volatile(
                "1: lr.w %0, 0(%1)\n"
                "addi t0, %0, -1\n"
                "sc.w t0, t0, 0(%1)\n"
                "bnez t0, 1b\n"
                : "=&r"(ret)
                : "r"(value)
                : "memory");
            return ret - 1;
        }

        static int fadd(int *value) {
            int ret;
            __asm__ volatile(
                "1: lr.w %0, 0(%1)\n"
                "addi t0, %0, 1\n"
                "sc.w t0, t0, 0(%1)\n"
                "bnez t0, 1b\n"
                : "=&r"(ret)
                : "r"(value)
                : "memory");
            return ret + 1;
        }
    };

    struct Trap {
        enum class Type { INTERRUPT = 1, EXCEPTION = 0 };

        static uintptr_t cause() {
            uintptr_t r;
            __asm__ volatile("csrr %0, mcause" : "=r"(r));
            return r;
        }

        static Type type() { return static_cast<Type>(cause() >> (Machine::XLEN - 1)); }

        __attribute__((always_inline)) static inline void set(void (*ptr)()) {
            __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
        }
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

        struct Timer {
            static void enable() { __asm__ volatile("li t0, 0x80\ncsrs mie, t0" ::: "t0"); }
            static void disable() { __asm__ volatile("li t0, 0x80\ncsrc mie, t0" ::: "t0"); }
        };
    };

    struct Stack {
        __attribute__((always_inline)) static inline void set(void *ptr) { __asm__ volatile("mv sp, %0" ::"r"(ptr)); }
    };
};
