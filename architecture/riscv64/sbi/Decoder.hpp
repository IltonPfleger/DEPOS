#ifndef __DEPOS_RISCV64_SBI_DECODER__
#define __DEPOS_RISCV64_SBI_DECODER__

namespace DEPOS::sbi {

class Decoder {
  public:
    struct FP {
        static constexpr uint8_t LD     = 0x07;
        static constexpr uint8_t SD     = 0x27;
        static constexpr uint8_t OP     = 0x53;
        static constexpr uint8_t FMADD  = 0x43;
        static constexpr uint8_t FMSUB  = 0x47;
        static constexpr uint8_t FNMSUB = 0x4B;
        static constexpr uint8_t FNMADD = 0x4F;
        [[nodiscard]] static constexpr bool valid(uint8_t opcode) {
            return (opcode == LD) || (opcode == SD) || (opcode == OP) || (opcode == FMADD) || (opcode == FMSUB) ||
                   (opcode == FNMSUB) || (opcode == FNMADD);
        }
    };

    struct LD {};
    struct SD {};

    static uint8_t opcode(uintptr_t instruction) {
        if ((instruction & 0x3) != 0x3) return instruction & 0x3;
        return instruction & 0x7F;
    }

    static uint8_t rd(LD, uintptr_t pc) {
        const uint16_t instruction16 = *reinterpret_cast<const uint16_t *>(pc);

        if ((instruction16 & 0x3) != 0x3) {
            return 8 + ((instruction16 >> 2) & 0x7);
        }

        uint32_t instruction = *reinterpret_cast<const uint32_t *>(pc);
        uint8_t opcode       = instruction & 0x7F;
        uint8_t funct3       = (instruction >> 12) & 0x7;

        if (opcode == 0x03 && funct3 == 0x3) {
            return (instruction >> 7) & 0x1F;
        }

        return 0;
    }

    static uint8_t rs2(SD, uintptr_t pc) {
        const uint16_t instruction16 = *reinterpret_cast<const uint16_t *>(pc);

        if ((instruction16 & 0x3) != 0x3) {
            uint8_t funct3 = (instruction16 >> 13) & 0x7;
            if (funct3 == 0x7) return 8 + ((instruction16 >> 2) & 0x7);
            return 0;
        }

        uint32_t instruction = *reinterpret_cast<const uint32_t *>(pc);
        uint8_t opcode       = instruction & 0x7F;
        uint8_t funct3       = (instruction >> 12) & 0x7;
        if (opcode == 0x23 && funct3 == 0x3) return (instruction >> 20) & 0x1F;
        return 0;
    }
};

} // namespace DEPOS::sbi

#endif
