#pragma once

#include <Traits.hpp>
#include <utils/Debug.hpp>

namespace virtio {

class Handler {
    enum {
        MMIO_MAGIC = 0x000,
        MMIO_VERSION = 0x004,
        MMIO_DEVICE_ID = 0x008,
        MMIO_VENDOR_ID = 0x00c,
        MMIO_DEVICE_FEATURES = 0x010,
        MMIO_DRIVER_FEATURES = 0x020,
        MMIO_DEVICE_FEATURES_SELECTOR = 0x014,
        MMIO_DRIVER_FEATURES_SELECTOR = 0x024,
        MMIO_GUEST_PAGE_SIZE = 0x028,
        MMIO_QUEUE_SELECTOR = 0x030,
        MMIO_QUEUE_NUMBER_MAXIMUM = 0x034,
        MMIO_QUEUE_SIZE = 0x038,
        MMIO_QUEUE_ALIGNAMENT = 0x03c,
        MMIO_QUEUE_PAGE_FRAME_NUMBER = 0x040,
        MMIO_QUEUE_NOTIFY = 0x050,
        MMIO_INTERRUPT_STATUS = 0x060,
        MMIO_INTERRUPT_ACK = 0x064,
        MMIO_STATUS = 0x070,
    };

  public:
    template <typename Self> bool read(this Self &&self, uintptr_t addr, unsigned int *destination) {
        uintptr_t offset = addr - self.Address;
        switch (offset) {
        case MMIO_MAGIC:
        case MMIO_VERSION:
        case MMIO_DEVICE_ID:
        case MMIO_VENDOR_ID:
        case MMIO_STATUS:
        case MMIO_DEVICE_FEATURES:
        case MMIO_QUEUE_NUMBER_MAXIMUM:
        case MMIO_INTERRUPT_STATUS:
            *destination = self.header()[offset / 4];
            return true;
        case MMIO_QUEUE_PAGE_FRAME_NUMBER:
            *destination = self.pfn();
            return true;
        default:
            return false;
        }
        return false;
    }

    template <typename Self> bool write(this Self &&self, uintptr_t addr, unsigned int source) {
        uintptr_t offset = addr - self.Address;
        switch (offset) {
        case MMIO_GUEST_PAGE_SIZE:
        case MMIO_STATUS:
        case MMIO_QUEUE_SELECTOR:
        case MMIO_DEVICE_FEATURES_SELECTOR:
        case MMIO_DRIVER_FEATURES_SELECTOR:
        case MMIO_DRIVER_FEATURES:
        case MMIO_QUEUE_SIZE:
        case MMIO_QUEUE_ALIGNAMENT:
            *(self.header() + (offset / 4)) = source;
            return true;
        case MMIO_QUEUE_PAGE_FRAME_NUMBER:
            self.pfn(source);
            return true;
        case MMIO_INTERRUPT_ACK:
            self.ack(source);
            return true;
        case MMIO_QUEUE_NOTIFY:
            self.notify(source);
            return true;
        default:
            return false;
        }
    }
};

} // namespace virtio
