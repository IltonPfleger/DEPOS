#ifndef __DEPOS_RISCV64_FPU__
#define __DEPOS_RISCV64_FPU__

namespace DEPOS {

class FPU {
    struct Registers {
        uint64_t f[32];
        uint64_t fcsr;
    };

  public:
    static void enable(ContextFrame *context) {
        context->status &= ~MASK;
        context->status |= INITIAL;
    }

    template <typename T> __attribute__((always_inline)) inline void save() {
        asm("csrr t0, %0; li t1, (3 << 13); and t0, t0, t1; bne t0, t1, 1f" ::"i"(T::STATUS));
        asm("li t0, (1 << 14); csrc %0, t0" ::"i"(T::STATUS));
        asm("mv t0, %0" ::"r"(this));
        asm("fsd f0,  0(t0)\n"
            "fsd f1,  8(t0)\n"
            "fsd f2,  16(t0)\n"
            "fsd f3,  24(t0)\n"
            "fsd f4,  32(t0)\n"
            "fsd f5,  40(t0)\n"
            "fsd f6,  48(t0)\n"
            "fsd f7,  56(t0)\n"
            "fsd f8,  64(t0)\n"
            "fsd f9,  72(t0)\n"
            "fsd f10, 80(t0)\n"
            "fsd f11, 88(t0)\n"
            "fsd f12, 96(t0)\n"
            "fsd f13, 104(t0)\n"
            "fsd f14, 112(t0)\n"
            "fsd f15, 120(t0)\n"
            "fsd f16, 128(t0)\n"
            "fsd f17, 136(t0)\n"
            "fsd f18, 144(t0)\n"
            "fsd f19, 152(t0)\n"
            "fsd f20, 160(t0)\n"
            "fsd f21, 168(t0)\n"
            "fsd f22, 176(t0)\n"
            "fsd f23, 184(t0)\n"
            "fsd f24, 192(t0)\n"
            "fsd f25, 200(t0)\n"
            "fsd f26, 208(t0)\n"
            "fsd f27, 216(t0)\n"
            "fsd f28, 224(t0)\n"
            "fsd f29, 232(t0)\n"
            "fsd f30, 240(t0)\n"
            "fsd f31, 248(t0)\n"
            "csrr t1, fcsr\n"
            "sd   t1, 256(t0)");
        asm("1:");
    }

    template <typename T> __attribute__((always_inline)) void load() {
        asm("csrr t0, %0; srli t0, t0, 13; andi t0, t0, 3; beqz t0, 1f" ::"i"(T::STATUS));
        asm("mv t0, %0" ::"r"(this));
        asm("fld f0,  0(t0)\n"
            "fld f1,  8(t0)\n"
            "fld f2,  16(t0)\n"
            "fld f3,  24(t0)\n"
            "fld f4,  32(t0)\n"
            "fld f5,  40(t0)\n"
            "fld f6,  48(t0)\n"
            "fld f7,  56(t0)\n"
            "fld f8,  64(t0)\n"
            "fld f9,  72(t0)\n"
            "fld f10, 80(t0)\n"
            "fld f11, 88(t0)\n"
            "fld f12, 96(t0)\n"
            "fld f13, 104(t0)\n"
            "fld f14, 112(t0)\n"
            "fld f15, 120(t0)\n"
            "fld f16, 128(t0)\n"
            "fld f17, 136(t0)\n"
            "fld f18, 144(t0)\n"
            "fld f19, 152(t0)\n"
            "fld f20, 160(t0)\n"
            "fld f21, 168(t0)\n"
            "fld f22, 176(t0)\n"
            "fld f23, 184(t0)\n"
            "fld f24, 192(t0)\n"
            "fld f25, 200(t0)\n"
            "fld f26, 208(t0)\n"
            "fld f27, 216(t0)\n"
            "fld f28, 224(t0)\n"
            "fld f29, 232(t0)\n"
            "fld f30, 240(t0)\n"
            "fld f31, 248(t0)\n"
            "ld  t1, 256(t0)\n"
            "csrw fcsr, t1");
        asm("1:");
    }

  private:
    static constexpr uint64_t SHIFT   = 13;
    static constexpr uint64_t MASK    = (3ULL << SHIFT);
    static constexpr uint64_t OFF     = (0ULL << SHIFT);
    static constexpr uint64_t INITIAL = (1ULL << SHIFT);
    static constexpr uint64_t CLEAN   = (2ULL << SHIFT);
    static constexpr uint64_t DIRTY   = (3ULL << SHIFT);

  private:
    Registers registers_;
};

} // namespace DEPOS

#endif
