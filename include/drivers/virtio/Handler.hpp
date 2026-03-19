#pragma once

#include <Traits.hpp>
#include <drivers/virtio/LegacyHeader.hpp>
#include <drivers/virtio/VirtQueue.hpp>
#include <memory/Heap.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace virtio {

template <typename T> class Handler {
  public:
    struct Register {
        static constexpr uint32_t Magic                  = 0x000;
        static constexpr uint32_t Version                = 0x004;
        static constexpr uint32_t DeviceID               = 0x008;
        static constexpr uint32_t VendorID               = 0x00c;
        static constexpr uint32_t DeviceFeatures         = 0x010;
        static constexpr uint32_t DeviceFeaturesSelector = 0x014;
        static constexpr uint32_t DriverFeatures         = 0x020;
        static constexpr uint32_t DriverFeaturesSelector = 0x024;
        static constexpr uint32_t GuestPageSize          = 0x028;
        static constexpr uint32_t QueueSelector          = 0x030;
        static constexpr uint32_t QueueSizeMax           = 0x034;
        static constexpr uint32_t QueueSize              = 0x038;
        static constexpr uint32_t QueueAlignment         = 0x03c;
        static constexpr uint32_t QueuePFN               = 0x040;
        static constexpr uint32_t QueueNotify            = 0x050;
        static constexpr uint32_t InterruptStatus        = 0x060;
        static constexpr uint32_t InterruptAck           = 0x064;
        static constexpr uint32_t Status                 = 0x070;
    };

  public:
    bool read(uintptr_t address, uint32_t *destination) {
        const auto offset = address - T::Address;
        switch (offset) {
        case Register::Magic:
        case Register::Version:
        case Register::DeviceID:
        case Register::VendorID:
        case Register::Status:
        case Register::DeviceFeatures:
        case Register::QueueSizeMax:
        case Register::InterruptStatus:
            *destination = header(offset);
            return true;
        case Register::QueuePFN:
            *destination = pfn();
            return true;
        default:
            return false;
        }
    }

    bool write(uintptr_t address, uint32_t value) {
        const auto offset = address - T::Address;
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
            this->interrupt() &= ~value;
            return true;
        case Register::QueueNotify:
            static_cast<T *>(this)->notify(value);
            return true;
        default:
            return false;
        }
    }

  protected:
    uint32_t &header(uint32_t offset) { return reinterpret_cast<uint32_t *>(&m_header)[offset / 4]; }

    uint32_t pfn() {
        if (m_header.m_guest_page_size == 0) return 0;
        return m_queues[m_header.m_queue_selector].m_address / m_header.m_guest_page_size;
    }

    void pfn(uint32_t source) {
        uint32_t address = source * m_header.m_guest_page_size;
        new (&m_queues[m_header.m_queue_selector]) VirtQueue(address, m_header.m_queue_number, m_header.m_queue_align);
        m_header.m_queue_page_frame_number = source;
    }

    auto &interrupt() { return m_header.m_interrupt_status; }

  protected:
    static constexpr size_t MaxQueues = 2;
    VirtQueue m_queues[MaxQueues];
    LegacyHeader m_header;
};

} // namespace virtio

} // namespace DEPOS
