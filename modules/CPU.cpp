export module CPU;
import Definitions;

export namespace CPU {
    __attribute__((always_inline)) inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((always_inline)) inline unsigned int id() {
        unsigned int id;
        __asm__ volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    struct __attribute__((packed)) Context {
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
        uintptr_t epc;

        static void create(Context *context, int (*entry)(void *), void (*exit)(), void *arg) {
            context->ra  = reinterpret_cast<uintptr_t>(exit);
            context->epc = reinterpret_cast<uintptr_t>(entry);
            context->a0  = reinterpret_cast<uintptr_t>(arg);
        }

        template <bool is_interrupt = false>
        __attribute__((always_inline)) static inline void push() {
            __asm__ volatile("addi sp, sp, %0" ::"i"(-sizeof(Context)));
            __asm__ volatile(
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
                "sd s11, 216(sp)\n");
            if constexpr (is_interrupt) {
                __asm__ volatile(
                    "csrr t0, mepc\n"
                    "sd t0, 224(sp)");
            } else {
                __asm__ volatile("sd ra, 224(sp)");
            }
        }

        __attribute__((always_inline)) static inline void pop() {
            __asm__ volatile(
                "ld t0, 224(sp)\n"
                "csrw mepc, t0\n"
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

        __attribute__((naked)) static void jump(Context *next) {
            __asm__ volatile("mv sp, %0" ::"r"(next));
            __asm__ volatile("li t0, 0x1800\ncsrs mstatus, t0\n" ::: "t0");
            pop();
            iret();
        }

        __attribute__((naked)) static Context *get() { __asm__ volatile("mv a0, sp\nret"); }

        __attribute__((naked)) static void transfer(CPU::Context **current, CPU::Context *next) {
            push();
            __asm__ volatile("sd sp, 0(%0)" ::"r"(current));
            jump(next);
        }
    };

    namespace Atomic {
        int fdec(int * value) {
            int ret;
            __asm__ volatile(
                "1: lr.w %0, 0(%1)\n"
                "addi t0, %0, -1\n"
                "sc.w t0, t0, 0(%1)\n"
                "bnez t0, 1b\n"
                : "=&r"(ret) : "r"(value));
            return ret - 1;
        }

        int fadd(int * value) {
            int ret;
            __asm__ volatile(
                "1: lr.w %0, 0(%1)\n"
                "addi t0, %0, 1\n"
                "sc.w t0, t0, 0(%1)\n"
                "bnez t0, 1b\n"
                : "=&r"(ret) : "r"(value));
            return ret + 1;
        }

    };  // namespace Atomic

    namespace Trap {
        enum class Type { INTERRUPT = 1, EXCEPTION = 0 };

        uintptr_t cause() {
            uintptr_t r;
            __asm__ volatile("csrr %0, mcause" : "=r"(r));
            return r;
        }

        Type type() { return static_cast<Type>(cause() >> (Machine::XLEN - 1)); }

        __attribute__((always_inline)) inline void set(void (*ptr)()) { __asm__ volatile("csrw mtvec, %0" ::"r"(ptr)); }

    };  // namespace Trap

    namespace Interrupt {
        enum class Type { TIMER = 7 };

        Type type() {
            uintptr_t r = Trap::cause();
            r           = (r << 1) >> 1;
            return static_cast<Type>(r);
        }

        __attribute__((always_inline)) inline void disable() { __asm__ volatile("csrci mstatus, 0x8"); }
        __attribute__((always_inline)) inline void enable() { __asm__ volatile("csrsi mstatus, 0x8"); }

        namespace Timer {
            void enable() { __asm__ volatile("li t0, 0x80\ncsrs mie, t0" ::: "t0"); }
            void disable() { __asm__ volatile("li t0, 0x80\ncsrc mie, t0" ::: "t0"); }
        };  // namespace Timer

    };  // namespace Interrupt

    namespace Stack {
        __attribute__((always_inline)) inline void set(void *ptr) { __asm__ volatile("mv sp, %0" ::"r"(ptr)); }
    };  // namespace Stack

};  // namespace CPU
