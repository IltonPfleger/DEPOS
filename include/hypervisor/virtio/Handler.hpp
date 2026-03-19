#pragma once

#include <Traits.hpp>
#include <hypervisor/virtio/LegacyHeader.hpp>
#include <hypervisor/virtio/VirtQueue.hpp>
#include <memory/Heap.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace virtio {

class Handler {
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
    template <typename Self> bool read(this Self &&self, uintptr_t address, uint32_t *destination) {
        const auto offset = address - self.Address;

        switch (offset) {
        case Register::Magic:
        case Register::Version:
        case Register::DeviceID:
        case Register::VendorID:
        case Register::Status:
        case Register::DeviceFeatures:
        case Register::QueueSizeMax:
        case Register::InterruptStatus:
            *destination = self.header(offset);
            return true;
        case Register::QueuePFN:
            *destination = self.pfn();
            return true;
        default:
            return false;
        }
    }

    template <typename Self> bool write(this Self &&self, uintptr_t address, uint32_t value) {
        const auto offset = address - self.Address;

        switch (offset) {
        case Register::GuestPageSize:
        case Register::Status:
        case Register::QueueSelector:
        case Register::DeviceFeaturesSelector:
        case Register::DriverFeaturesSelector:
        case Register::DriverFeatures:
        case Register::QueueSize:
        case Register::QueueAlignment:
            self.header(offset) = value;
            return true;
        case Register::QueuePFN:
            self.pfn(value);
            return true;
        case Register::InterruptAck:
            self.interrupt() &= ~value;
            return true;
        case Register::QueueNotify:
            self.notify(value);
            return true;
        default:
            return false;
        }
    }

  protected:
    uint32_t &header(this auto &self, uint32_t offset) {
        return reinterpret_cast<uint32_t *>(&self.m_header)[offset / 4];
    }

    uint32_t pfn(this auto &self) {
        if (self.m_header.m_guest_page_size == 0) return 0;
        return self.m_queues[self.m_header.m_queue_selector].m_address / self.m_header.m_guest_page_size;
    }

    void pfn(this auto &self, uint32_t source) {
        uint32_t address = source * self.m_header.m_guest_page_size;
        new (&self.m_queues[self.m_header.m_queue_selector])
            VirtQueue(address, self.m_header.m_queue_number, self.m_header.m_queue_align);
        self.m_header.m_queue_page_frame_number = source;
    }

    auto &interrupt(this auto &self) { return self.m_header.m_interrupt_status; }

  private:
    static constexpr size_t MaxNumberOfQueues = 2;

  protected:
    LegacyHeader m_header;
    VirtQueue m_queues[MaxNumberOfQueues];
};

} // namespace virtio

} // namespace DEPOS
