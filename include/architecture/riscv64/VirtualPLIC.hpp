#pragma once

#include <Traits.hpp>

namespace riscv64 {

class VirtualPLIC {
    enum {
        PRIORITY = 0x000000,
        PENDING = 0x001000,
        ENABLED = 0x002000,
        THRESHOLD = 0x200000,
        CLAIM = 0x200004,
    };

  public:
    VirtualPLIC() = default;

    bool interrupt(unsigned int id) {
        unsigned int bank = id / 32;
        unsigned int bit = id % 32;
        bool enabled = (m_enabled[1][bank] >> bit) & 1;
        if (enabled) m_pending[bank] |= (1 << bit);
        return enabled;
    }

    bool read(unsigned long offset, unsigned int *destination) {
        if (offset > ENABLED && offset < THRESHOLD) {
            unsigned int context = (offset - ENABLED) / 0x80;
            unsigned int chunk = ((offset - ENABLED) % 0x80) / 4;
            *destination = m_enabled[context][chunk];
            return true;
        } else if (offset >= THRESHOLD) {
            unsigned int off = offset - THRESHOLD;
            unsigned int ctx = off / 0x1000;
            unsigned int reg = off % 0x1000;

            if (ctx >= k_number_of_contexts) return false;

            if (reg == 0) {
                *destination = m_threshold[ctx];
                return true;
            }

            if (reg == 4) {
                *destination = 0;
                for (unsigned int i = 0; i < 32; ++i) {
                    uint32_t active = m_pending[i] & m_enabled[ctx][i];

                    if (i == 0) active &= ~1;

                    if (active) {
                        unsigned int bit = __builtin_ctz(active);
                        m_pending[i] &= ~(1 << bit);
                        *destination = (i * 32) + bit;
                        return true;
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool write(unsigned long offset, unsigned int source) {
        if (offset > PRIORITY && offset < PENDING) {
            m_priority[offset / 4] = source;
            return true;
        } else if (offset > ENABLED && offset < THRESHOLD) {
            offset -= ENABLED;
            unsigned int context = offset / 0x80;
            unsigned int chunk = (offset % 0x80) / 4;
            m_enabled[context][chunk] = source;
            return true;
        } else if (offset >= THRESHOLD) {
            offset -= THRESHOLD;
            if (offset % 0x1000 == 0) {
                m_threshold[offset / 0x1000] = source;
            } else {
                csrc<MachineMode::IP>(SupervisorMode::EI);
            }
            return true;
        }

        return false;
    }

  private:
    static constexpr unsigned int k_number_of_contexts = 2;
    uint32_t m_priority[1024] = {}; // 32 Bits For Each IRQ
    uint32_t m_pending[32] = {};    // One Bit For Each IRQ
    uint32_t m_enabled[k_number_of_contexts][32] = {};
    uint32_t m_threshold[k_number_of_contexts] = {}; // Threshold Per Context
};

} // namespace riscv64
