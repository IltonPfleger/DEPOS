#pragma once

#include <Machine.hpp>

struct CPU {
    __attribute__((always_inline)) static inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((naked)) static unsigned int core() { __asm__ volatile("csrr a0, mhartid\nret"); }

    __attribute__((naked)) static void *thread() { __asm__ volatile("mv a0, tp\nret"); }

    __attribute__((always_inline)) static inline void thread(void *ptr) {
        __asm__ volatile("mv tp, %0" ::"r"(ptr) : "tp", "memory");
    }

    struct Context {
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
                __asm__ volatile("sd ra, 232(sp)" ::: "memory");
            }
        }

        __attribute__((naked)) static void pop() {
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
                "addi sp, sp, %0\n"
                :
                : "i"(sizeof(Context)));
            iret();
        }

        __attribute__((naked)) static void jump(Context *c) {
            __asm__ volatile("mv sp, %0" ::"r"(c));
            __asm__ volatile("li t0, 0x1880\ncsrs mstatus, t0" ::: "t0");

            pop();
        }

        __attribute__((naked)) static Context *get() { __asm__ volatile("mv a0, sp\nret" ::: "a0"); }
    };

    struct Atomic {
        static int tsl(volatile int *value) {
            int old;
            __asm__ volatile(
                "1:\n"
                "lr.w %0, (%1)\n"
                "sc.w t3, %2, (%1)\n"
                "bnez t3, 1b\n"
                : "=&r"(old)
                : "r"(value), "r"(1)
                : "t3", "cc", "memory");
            return old;
        }

        static int fdec(volatile int &value) {
            int old;
            __asm__ volatile(
                "1: lr.w %0, (%1)\n"
                "addi %0, %0, -1\n"
                "sc.w t3, %0, (%1)\n"
                "bnez t3, 1b\n"
                : "=&r"(old)
                : "r"(&value)
                : "t3", "cc", "memory");
            return old + 1;
        }

        static int finc(volatile int &value) {
            int old;
            __asm__ volatile(
                "1: lr.w %0, (%1)\n"
                "addi %0, %0, 1\n"
                "sc.w t3, %0, (%1)\n"
                "bnez t3, 1b\n"
                : "=&r"(old)
                : "r"(&value)
                : "t3", "cc", "memory");
            return old - 1;
        }
    };

    struct Trap {
        enum class Type { INTERRUPT = 1, EXCEPTION = 0 };

        __attribute__((naked)) static uintmax_t cause() { __asm__ volatile("csrr a0, mcause\nret"); }

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
