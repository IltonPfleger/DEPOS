#pragma once

#include <Traits.hpp>
#include <drivers/virtio/LegacyHeader.hpp>
#include <drivers/virtio/VirtQueue.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace virtio {

template <typename T> class Handler {
  public:
    struct Register {
        static constexpr uint32_t Magic = 0x000;
        static constexpr uint32_t Version = 0x004;
        static constexpr uint32_t DeviceID = 0x008;
        static constexpr uint32_t VendorID = 0x00c;
        static constexpr uint32_t DeviceFeatures = 0x010;
        static constexpr uint32_t DeviceFeaturesSelector = 0x014;
        static constexpr uint32_t DriverFeatures = 0x020;
        static constexpr uint32_t DriverFeaturesSelector = 0x024;
        static constexpr uint32_t GuestPageSize = 0x028;
        static constexpr uint32_t QueueSelector = 0x030;
        static constexpr uint32_t QueueSizeMax = 0x034;
        static constexpr uint32_t QueueSize = 0x038;
        static constexpr uint32_t QueueAlignment = 0x03c;
        static constexpr uint32_t QueuePFN = 0x040;
        static constexpr uint32_t QueueNotify = 0x050;
        static constexpr uint32_t InterruptStatus = 0x060;
        static constexpr uint32_t InterruptAck = 0x064;
        static constexpr uint32_t Status = 0x070;
    };

  public:
    bool readm(uintptr_t addr, uint32_t *dest) {
        const auto offset = addr - T::Address;

        switch (offset) {
        case Register::Magic:
        case Register::Version:
        case Register::DeviceID:
        case Register::VendorID:
        case Register::Status:
        case Register::DeviceFeatures:
        case Register::QueueSizeMax:
        case Register::InterruptStatus:
            *dest = header(offset);
            return true;
        case Register::QueuePFN:
            *dest = pfn();
            return true;
        default:
            return false;
        }
    }

    bool writem(uintptr_t addr, uint32_t value) {
        const auto offset = addr - T::Address;

        switch (offset) {
        case Register::GuestPageSize:
        case Register::Status:
        case Register::QueueSelector:
        case Register::DeviceFeaturesSelector:
        case Register::DriverFeaturesSelector:
        case Register::DriverFeatures:
        case Register::QueueSize:
        case Register::QueueAlignment:
            header(offset) = value;
            return true;

        case Register::QueuePFN:
            pfn(value);
            return true;

        case Register::InterruptAck:
            interruptc(value);
            return true;

        case Register::QueueNotify:
            T::instance()->notify(value);
            return true;

        default:
            return false;
        }
    }

  protected:
    uint32_t &header(uint32_t offset) { return reinterpret_cast<uint32_t *>(&m_header)[offset / 4]; }

    uint32_t pfn() {
        auto &queue = m_queues[m_header.m_queue_selector];
        if (m_header.m_guest_page_size == 0) return 0;
        return queue.m_address / m_header.m_guest_page_size;
    }

    void pfn(uint32_t source) {
        auto &queue = m_queues[m_header.m_queue_selector];
        const uint32_t address = source * m_header.m_guest_page_size;

        queue = VirtQueue(address, T::NumberOfDescriptors, m_header.m_queue_align);
        m_header.m_queue_page_frame_number = source;
    }

    void interruptc(uint32_t mask) { m_header.m_interrupt_status &= ~mask; }
    void interrupts(uint32_t mask) { m_header.m_interrupt_status |= mask; }

  public:
    static bool read(uintptr_t addr, uint32_t *dest) { return T::instance()->readm(addr, dest); }

    static bool write(uintptr_t addr, uint32_t val) { return T::instance()->writem(addr, val); }

  protected:
    static constexpr size_t MaxQueues = 2;

    VirtQueue m_queues[MaxQueues];
    LegacyHeader m_header;
};

} // namespace virtio

} // namespace DEPOS
