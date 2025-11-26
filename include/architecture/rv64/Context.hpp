class Context {
  public:
    uint64_t ra;
    uint64_t tp;
    uint64_t gp;
    uint64_t sp;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t t0, t1, t2, t3, t4, t5, t6;
    uint64_t status;
    uint64_t pc;

    Context() = default;
    Context(int (*entry)(void *), void *a0, void *sp, void (*exit)()) {
        this->ra = reinterpret_cast<uint64_t>(exit);
        this->pc = reinterpret_cast<uint64_t>(entry);
        this->status = static_cast<uint64_t>(KernelMode::ME2ME | KernelMode::PIRQE);
        this->a0 = reinterpret_cast<uint64_t>(a0);
        this->sp = reinterpret_cast<uint64_t>(sp);
    }

    template <typename F, typename... Args, typename T = KernelMode>
    __attribute__((naked)) void load(F f, Args... args) {
        asm("ld sp, %[sp](a0)" ::[sp] "i"(offsetof(Context, sp)));
        asm("mv s0, a0");
        asm("addi sp, sp, %0" ::"i"(-sizeof(Context)));
        f(args...);
        asm("addi sp, sp, %0" ::"i"(sizeof(Context)));
        asm("mv a1, s0");
        asm("ld ra, %[ra](a1)\n"
            "ld gp, %[gp](a1)\n"
            "ld s0, %[s0](a1)\n"
            "ld a0, %[a0](a1)\n"
            "ld s1, %[s1](a1)\n"
            "ld s2, %[s2](a1)\n"
            "ld s3, %[s3](a1)\n"
            "ld s4, %[s4](a1)\n"
            "ld s5, %[s5](a1)\n"
            "ld s6, %[s6](a1)\n"
            "ld s7, %[s7](a1)\n"
            "ld s8, %[s8](a1)\n"
            "ld s9, %[s9](a1)\n"
            "ld s10, %[s10](a1)\n"
            "ld s11, %[s11](a1)\n"
            "ld t0, %[pc](a1)\n"
            "ld t1, %[status](a1)\n"
            "csrw %[csrepc], t0\n"
            "csrw %[csrstatus], t1\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [s0] "i"(offsetof(Context, s0)),
              [s1] "i"(offsetof(Context, s1)), [a0] "i"(offsetof(Context, a0)), [s2] "i"(offsetof(Context, s2)),
              [s3] "i"(offsetof(Context, s3)), [s4] "i"(offsetof(Context, s4)), [s5] "i"(offsetof(Context, s5)),
              [s6] "i"(offsetof(Context, s6)), [s7] "i"(offsetof(Context, s7)), [s8] "i"(offsetof(Context, s8)),
              [s9] "i"(offsetof(Context, s9)), [s10] "i"(offsetof(Context, s10)), [s11] "i"(offsetof(Context, s11)),
              [pc] "i"(offsetof(Context, pc)), [status] "i"(offsetof(Context, status)), [csrepc] "i"(T::EPC),
              [csrstatus] "i"(T::STATUS));
        T::ret();
        __builtin_unreachable();
    }

    template <typename T = KernelMode> __attribute__((naked)) bool save() {
        asm("sd ra, %[ra](a0)\n"
            "sd gp, %[gp](a0)\n"
            "sd sp, %[sp](a0)\n"
            "sd zero,%[a0](a0)\n"
            "sd s0, %[s0](a0)\n"
            "sd s1, %[s1](a0)\n"
            "sd s2, %[s2](a0)\n"
            "sd s3, %[s3](a0)\n"
            "sd s4, %[s4](a0)\n"
            "sd s5, %[s5](a0)\n"
            "sd s6, %[s6](a0)\n"
            "sd s7, %[s7](a0)\n"
            "sd s8, %[s8](a0)\n"
            "sd s9, %[s9](a0)\n"
            "sd s10, %[s10](a0)\n"
            "sd s11, %[s11](a0)\n"
            "sd ra, %[pc](a0)\n"
            "csrr t1, %[csrstatus]\n"
            "or t1, t1, %[me2me]\n"
            "and t1, t1, %[pirqe]\n"
            "sd t1, %[status](a0)\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [sp] "i"(offsetof(Context, sp)),
              [a0] "i"(offsetof(Context, a0)), [s0] "i"(offsetof(Context, s0)), [s1] "i"(offsetof(Context, s1)),
              [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)), [s4] "i"(offsetof(Context, s4)),
              [s5] "i"(offsetof(Context, s5)), [s6] "i"(offsetof(Context, s6)), [s7] "i"(offsetof(Context, s7)),
              [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)), [s10] "i"(offsetof(Context, s10)),
              [s11] "i"(offsetof(Context, s11)), [pc] "i"(offsetof(Context, pc)), [csrstatus] "i"(T::STATUS),
              [me2me] "r"(T::ME2ME), [pirqe] "r"(~T::PIRQE), [status] "i"(offsetof(Context, status))
            : "memory");
        asm("li a0, 1\n"
            "ret\n");
    }

    template <typename T = KernelMode> __attribute__((always_inline)) static inline Context *push() {
        asm("addi sp, sp, %[size]\n"
            "sd ra, %[ra](sp)\n"
            "sd gp, %[gp](sp)\n"
            "sd t0, %[t0](sp)\n"
            "sd t1, %[t1](sp)\n"
            "sd t2, %[t2](sp)\n"
            "sd t3, %[t3](sp)\n"
            "sd t4, %[t4](sp)\n"
            "sd t5, %[t5](sp)\n"
            "sd t6, %[t6](sp)\n"
            "sd a0, %[a0](sp)\n"
            "sd a1, %[a1](sp)\n"
            "sd a2, %[a2](sp)\n"
            "sd a3, %[a3](sp)\n"
            "sd a4, %[a4](sp)\n"
            "sd a5, %[a5](sp)\n"
            "sd a6, %[a6](sp)\n"
            "sd a7, %[a7](sp)\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [t0] "i"(offsetof(Context, t0)),
              [t1] "i"(offsetof(Context, t1)), [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
              [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)), [t6] "i"(offsetof(Context, t6)),
              [a0] "i"(offsetof(Context, a0)), [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
              [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)), [a5] "i"(offsetof(Context, a5)),
              [a6] "i"(offsetof(Context, a6)), [a7] "i"(offsetof(Context, a7)), [size] "i"(-sizeof(Context))
            : "memory");

        asm("csrr t0, %0\n"
            "sd t0, %c[status](sp)\n"
            "csrr t0, %1\n"
            "sd t0, %c[pc](sp)" ::"i"(T::STATUS),
            "i"(T::EPC), [status] "i"(offsetof(Context, status)), [pc] "i"(offsetof(Context, pc)));
        register Context *sp asm("sp");
        return sp;
    }

    template <typename T = KernelMode> __attribute__((naked)) static void pop() {
        asm("ld t0, %[statuso](sp)\n"
            "csrw %[statusr], t0\n"
            "ld t0, %[pc](sp)\n"
            "csrw %[epcr], t0" ::[statusr] "i"(T::STATUS),
            [epcr] "i"(T::EPC), [pc] "i"(offsetof(Context, pc)), [statuso] "i"(offsetof(Context, status)));
        asm("ld ra, %[ra](sp)\n"
            "ld gp, %[gp](sp)\n"
            "ld t0, %[t0](sp)\n"
            "ld t1, %[t1](sp)\n"
            "ld t2, %[t2](sp)\n"
            "ld t3, %[t3](sp)\n"
            "ld t4, %[t4](sp)\n"
            "ld t5, %[t5](sp)\n"
            "ld t6, %[t6](sp)\n"
            "ld a0, %[a0](sp)\n"
            "ld a1, %[a1](sp)\n"
            "ld a2, %[a2](sp)\n"
            "ld a3, %[a3](sp)\n"
            "ld a4, %[a4](sp)\n"
            "ld a5, %[a5](sp)\n"
            "ld a6, %[a6](sp)\n"
            "ld a7, %[a7](sp)\n"
            "addi sp, sp, %[size]\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [t0] "i"(offsetof(Context, t0)),
              [t1] "i"(offsetof(Context, t1)), [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
              [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)), [t6] "i"(offsetof(Context, t6)),
              [a0] "i"(offsetof(Context, a0)), [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
              [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)), [a5] "i"(offsetof(Context, a5)),
              [a6] "i"(offsetof(Context, a6)), [a7] "i"(offsetof(Context, a7)), [size] "i"(sizeof(Context))
            : "memory");
        T::ret();
        __builtin_unreachable();
    }
};
