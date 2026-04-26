#pragma once

#include <Traits.hpp>
#include <hypervisor/virtio/LegacyHeader.hpp>
#include <hypervisor/virtio/Queue.hpp>
#include <shared/console/Debug.hpp>

namespace DEPOS {

namespace virtio {

class Handler {
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

  public:
    Handler() = default;

    template <typename Self> bool read(this Self &&self, uintptr_t address, uint32_t *destination) {
        const int offset = address - self.Address;
        switch (offset) {
        case Magic:
        case Version:
        case DeviceID:
        case VendorID:
        case Status:
        case DeviceFeatures:
        case QueueSizeMax:
        case InterruptStatus:
            *destination = self.header(offset);
            return true;
        case QueuePFN:
            *destination = self.pfn();
            return true;
        default:
            return false;
        }
    }

    template <typename Self> bool write(this Self &&self, uintptr_t address, uint32_t value) {
        const auto offset = address - self.Address;
        switch (offset) {
        case GuestPageSize:
        case Status:
        case QueueSelector:
        case DeviceFeaturesSelector:
        case DriverFeaturesSelector:
        case DriverFeatures:
        case QueueSize:
        case QueueAlignment:
            self.header(offset) = value;
            return true;
        case QueuePFN:
            self.pfn(value);
            return true;
        case InterruptAck:
            self.interrupt() &= ~value;
            return true;
        case QueueNotify:
            self.notify(value);
            return true;
        default:
            return false;
        }
    }

  protected:
    uint32_t &header(uint32_t offset) { return reinterpret_cast<uint32_t *>(&m_header)[offset / 4]; }

    uint32_t pfn() { return m_queues[header(QueueSelector)].m_address / header(GuestPageSize); }

    void pfn(uint32_t source) {
        ERROR(header(QueueSelector) >= MaxNumberOfQueues, header(QueueSelector));
        uint32_t address = source * header(GuestPageSize);
        new (&m_queues[header(QueueSelector)]) Queue(address, header(QueueSize), header(QueueAlignment));
        header(QueuePFN) = source;
    }

    auto &interrupt() { return header(InterruptStatus); }

  private:
    static constexpr size_t MaxNumberOfQueues = 2;

  protected:
    Meta::Array<MaxNumberOfQueues, Queue> m_queues{};
    LegacyHeader m_header{};
};

} // namespace virtio

} // namespace DEPOS
