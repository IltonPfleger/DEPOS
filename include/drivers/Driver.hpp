#pragma once

#include <types.hpp>

namespace DEPOS {

class Driver {
  public:
    Driver()
        : m_base(0) {}

    Driver(uintptr_t base)
        : m_base(base) {}

    volatile auto &Reg64(size_t offset) { return *reinterpret_cast<volatile uintptr_t *>(m_base + offset); }

    volatile auto &Reg32(size_t offset) { return *reinterpret_cast<volatile uint32_t *>(m_base + offset); }

    // volatile auto &Reg16(size_t offset) { return *reinterpret_cast<volatile uint16_t *>(m_base + offset); }

    volatile auto &Reg8(size_t offset) { return *reinterpret_cast<volatile uint8_t *>(m_base + offset); }

    static volatile auto &Reg64(uintptr_t base, size_t offset) {
        return *reinterpret_cast<volatile uint64_t *>(base + offset);
    }

    static volatile auto &Reg32(uintptr_t base, size_t offset) {
        return *reinterpret_cast<volatile uint32_t *>(base + offset);
    }

    // static volatile auto &Reg16(uintptr_t base, size_t offset) {
    //     return *reinterpret_cast<volatile uint16_t *>(base + offset);
    // }

    static volatile auto &Reg8(uintptr_t base, size_t offset) {
        return *reinterpret_cast<volatile uint8_t *>(base + offset);
    }

  private:
    uintptr_t m_base;
};

} // namespace DEPOS
