#pragma once

#include <Machine.hpp>

namespace Timer {
    void handler();
}

namespace Kernel {
    void exception();
}

struct CPU {
    __attribute__((always_inline)) static inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((always_inline)) static inline void stack(void *ptr) { __asm__ volatile("mv sp, %0" ::"r"(ptr)); }

    __attribute__((naked)) static unsigned int core() { __asm__ volatile("csrr a0, mhartid\nret"); }

    __attribute__((naked)) static void *thread() { __asm__ volatile("mv a0, tp\nret"); }

    struct Context {
        uintptr_t ra;
        uintptr_t tp;
        uintptr_t t0, t1, t2, t3, t4, t5, t6;
        uintptr_t a0, a1, a2, a3, a4, a5, a6, a7;
        uintptr_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uintptr_t epc;
        uintptr_t estatus;

        Context(int (*entry)(void *), void (*exit)(), void *tp, void *a0) {
            ra       = reinterpret_cast<uintptr_t>(exit);
            epc      = reinterpret_cast<uintptr_t>(entry);
            estatus  = reinterpret_cast<uintptr_t>(0ULL | (3 << 11) | (1 << 7));
            this->tp = reinterpret_cast<uintptr_t>(tp);
            this->a0 = reinterpret_cast<uintptr_t>(a0);
        }

        __attribute__((always_inline)) static inline void save() {
            __asm__ volatile(
                "addi sp, sp, %0\n"
                "sd ra, 0(sp)\n"
                "sd tp, 8(sp)\n"
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
                "sd s11, 224(sp)\n"
                "csrr t0, mstatus\n"
                "andi t0, t0, ~0x80\n"
                "li t1, 0x1800\n"
                "or t0, t0, t1\n"
                "sd t0, 240(sp)\n"
                "sd ra, 232(sp)" ::"i"(-sizeof(Context))
                : "memory");
        }

        __attribute__((naked)) static inline void load() {
            __asm__ volatile(
                "ld t0, 240(sp)\n"
                "csrw mstatus, t0\n"
                "ld t0, 232(sp)\n"
                "csrw mepc, t0\n"
                "ld ra, 0(sp)\n"
                "ld tp, 8(sp)\n"
                "ld a0, 72(sp)\n"
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
            CPU::iret();
        }

        __attribute__((always_inline)) static inline void push() {
            __asm__ volatile(
                "addi sp, sp, %0\n"
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
                "sd s11, 224(sp)\n"
                "csrr t0, mstatus\n"
                "sd t0, 240(sp)\n"
                "csrr t0, mepc\n"
                "sd t0, 232(sp)\n" ::"i"(-sizeof(Context))
                : "memory");
        }

        __attribute__((naked)) static void pop() {
            __asm__ volatile(
                "ld t0, 240(sp)\n"
                "csrw mstatus, t0\n"
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
            CPU::iret();
        }

        __attribute__((naked)) static void jump(Context *c) {
            __asm__ volatile("mv sp, %0" ::"r"(c));
            load();
            iret();
        }

        __attribute__((always_inline)) [[nodiscard]] static inline Context *get() {
            Context *sp;
            __asm__ volatile("mv %0, sp" : "=r"(sp));
            return sp;
        }
    };

    struct Atomic {
        static int tsl(volatile int &value) { return __atomic_test_and_set(&value, __ATOMIC_SEQ_CST); }
        static int fdec(volatile int &value) { return __atomic_fetch_sub(&value, 1, __ATOMIC_SEQ_CST); }
        static int finc(volatile int &value) { return __atomic_fetch_add(&value, 1, __ATOMIC_SEQ_CST); }
    };

    struct Trap {
        using Handler = void (*)();

        static inline Handler interrupts[8] = {
            nullptr,         // 0
            nullptr,         // 1
            nullptr,         // 2
            nullptr,         // 3
            nullptr,         // 4
            nullptr,         // 5
            nullptr,         // 6
            Timer::handler,  // 7
        };

        __attribute__((naked)) static uintmax_t icause() { __asm__ volatile("csrr a0, mcause\nret"); }

        static void handler() {
            auto cause        = icause();
            bool is_interrupt = cause >> (Machine::XLEN - 1);
            auto code         = (cause << 1) >> 1;

            if (is_interrupt) {
                interrupts[code]();
            } else {
                Kernel::exception();
            }
        }

        __attribute__((always_inline)) static inline void set(void (*ptr)()) {
            __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
        }
    };

    struct Interrupt {
        __attribute__((always_inline)) static inline void disable() { __asm__ volatile("csrci mstatus, 0x8"); }
        __attribute__((always_inline)) static inline void enable() { __asm__ volatile("csrsi mstatus, 0x8"); }

        struct Timer {
            static void enable() { __asm__ volatile("li t0, 0x80\ncsrs mie, t0" ::: "t0"); }
            static void disable() { __asm__ volatile("li t0, 0x80\ncsrc mie, t0" ::: "t0"); }
        };
    };
};
