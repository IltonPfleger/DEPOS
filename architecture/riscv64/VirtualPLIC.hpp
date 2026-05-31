#pragma once

#include <Traits.hpp>

namespace DEPOS {

class VirtualPLIC {
    enum {
        PRIORITY  = 0x000000,
        PENDING   = 0x001000,
        ENABLED   = 0x002000,
        THRESHOLD = 0x200000,
        CLAIM     = 0x200004,
    };

  public:
    VirtualPLIC() = default;

    bool pending() { return m_irq; }

    void interrupt(uint32_t id) {
        uint32_t bank = id >> 5;
        uint32_t bit  = id & 31;
        uint32_t mask = (1U << bit);

        for (uint32_t c = 0; c < NumberOfContexts; ++c) {
            if (m_enabled[c][bank] & mask) {
                m_pending[bank] |= mask;
                m_irq = true;
                break;
            }
        }
    }

    uint32_t claim(uint32_t context) {
        for (uint32_t i = 0; i < 32; ++i) {
            uint32_t active = m_pending[i] & m_enabled[context][i];

            if (i == 0) active &= ~1U;

            if (active) {
                uint32_t bit = static_cast<uint32_t>(__builtin_ctz(active));
                m_pending[i] &= ~(1U << bit);
                m_irq = false;
                return (i << 5) | bit;
            }
        }
        return 0;
    }

    bool read(unsigned long offset, unsigned int *destination) {
        if (offset >= ENABLED && offset < THRESHOLD) {
            unsigned int context = (offset - ENABLED) / 0x80;
            unsigned int chunk   = ((offset - ENABLED) % 0x80) / 4;
            *destination         = m_enabled[context][chunk];
            return true;
        } else if (offset >= THRESHOLD) {
            unsigned int off     = offset - THRESHOLD;
            unsigned int context = off / 0x1000;
            unsigned int reg     = off % 0x1000;

            if (context >= NumberOfContexts) return false;

            if (reg == 0) {
                *destination = m_threshold[context];
                return true;
            } else if (reg == 4) {
                csrc<MachineMode::IP>(SupervisorMode::EI);
                *destination = claim(context);
                return true;
            }
        }
        return false;
    }

    bool write(unsigned long offset, unsigned int source) {
        if (offset >= PRIORITY && offset < PENDING) {
            m_priority[offset / 4] = source;
            return true;
        } else if (offset >= ENABLED && offset < THRESHOLD) {
            offset -= ENABLED;
            unsigned int context      = offset / 0x80;
            unsigned int chunk        = (offset % 0x80) / 4;
            m_enabled[context][chunk] = source;
            return true;
        } else if (offset >= THRESHOLD) {
            unsigned int off     = offset - THRESHOLD;
            unsigned int context = off / 0x1000;
            unsigned int reg     = off % 0x1000;
            if (reg == 0) {
                m_threshold[context] = source;
                return true;
            } else if (reg == 4) {
                return true;
            }
        }

        return false;
    }

  private:
    static constexpr unsigned int NumberOfContexts = 2;
    uint32_t m_priority[1024]                      = {0}; // 32 Bits For Each IRQ
    uint32_t m_pending[32]                         = {0}; // One Bit For Each IRQ
    uint32_t m_enabled[NumberOfContexts][32]       = {0};
    uint32_t m_threshold[NumberOfContexts]         = {0}; // Threshold Per Context
    bool m_irq                                     = false;
};

} // namespace DEPOS
