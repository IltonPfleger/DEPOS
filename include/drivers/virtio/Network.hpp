#pragma once

#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <network/NIC.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

namespace virtio {

struct NetworkHeader {
    unsigned char _[10];
};

template <typename Device, uintptr_t Base>
class Network : public Handler<Network<Device, Base>>,
                public Observer<const unsigned char *, size_t> {
  public:
    static Network *instance() {
        static Network instance;
        return &instance;
    }

    void notify(unsigned int source) {
        if (source != k_tx_queue) return;

        VirtQueue &queue = this->m_queues[k_tx_queue];

        while (queue.available()) {
            int head              = queue.alloc();
            int current           = head;
            uint32_t length       = 0;
            bool first_descriptor = true;

            while (true) {
                VirtQueue::RingDescriptor *descriptor = queue.get(current);
                uint8_t *data = reinterpret_cast<uint8_t *>(descriptor->address);
                uint32_t len  = descriptor->length;

                if (first_descriptor) {
                    data += sizeof(NetworkHeader);
                    len -= sizeof(NetworkHeader);
                    first_descriptor = false;
                }

                if (len > 0) {
                    m_device->send(data, len);
                }

                length += descriptor->length;
                if (!(descriptor->flags & 0x1)) break;
                current = descriptor->next;
            }

            queue.free(head, length);
            this->interrupts(0x1);
        }
    }

    void update(const unsigned char *buffer, size_t size) override {
        VirtQueue &queue = this->m_queues[k_rx_queue];

        if (!queue.available()) return;

        int id = queue.alloc();

        auto descriptor   = queue.get(id);
        auto *destination = reinterpret_cast<uint8_t *>(descriptor->address);

        memset(destination, 0, sizeof(NetworkHeader));
        memcpy(destination + sizeof(NetworkHeader), buffer, size);

        descriptor->length = size + sizeof(NetworkHeader);

        queue.free(id, descriptor->length);
        this->interrupts(0x1);
        m_vcpu->interrupt(IRQ);
    }

  private:
    Network() {
        this->m_header.m_magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version                   = 1;
        this->m_header.m_id                        = 1;
        this->m_header.m_vendor                    = 0x554d4551;
        this->m_header.m_max_number_of_descriptors = k_number;
        m_vcpu                                     = VirtualCPU::current();
        m_device                                   = Device::instance();
        m_device->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr uintptr_t IRQ     = 50;
    static constexpr size_t Size       = sizeof(LegacyHeader);

  private:
    static constexpr uintptr_t k_number   = 128;
    static constexpr uintptr_t k_rx_queue = 0;
    static constexpr uintptr_t k_tx_queue = 1;

  private:
    Device *m_device;
    VirtualCPU *m_vcpu;
};

} // namespace virtio

} // namespace DEPOS
