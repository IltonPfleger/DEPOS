#pragma once

#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/hypervisor/VirtualSwitch.hpp>
#include <drivers/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <network/NIC.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

namespace virtio {

struct NetworkHeader {
    unsigned char padding[10];
};

template <typename Device, uintptr_t Base> class Network : public Handler<Network<Device, Base>>, public NIC::Observer {
    enum { RX, TX };

  public:
    static Network *instance() {
        static Network instance;
        return &instance;
    }

    void notify(unsigned int source) {
        if (source != TX) return;
        VirtQueue &queue = this->m_queues[TX];
        while (queue.available()) {
            int head       = queue.alloc();
            uint32_t total = send(head);
            queue.free(head, total);
            this->interrupts(0x1);
        }
    }

    size_t send(int head) {
        bool first   = true;
        size_t total = 0;
        while (true) {
            auto *descriptor = this->m_queues[TX].get(head);
            uint8_t *data    = reinterpret_cast<uint8_t *>(descriptor->address);
            uint32_t length  = descriptor->length;

            if (first) {
                data += sizeof(NetworkHeader);
                length -= sizeof(NetworkHeader);
                first = false;
            }

            if (length > 0) m_device->send(data, length);

            total += descriptor->length;
            if (!(descriptor->flags & 0x1)) break;
            head = descriptor->next;
        }
        return total;
    }

    void update(const NIC::Buffer *buffer) override {
        auto data   = buffer->data();
        auto length = buffer->length();

        VirtQueue &queue = this->m_queues[RX];

        if (!queue.available()) return;

        int id = queue.alloc();

        auto descriptor   = queue.get(id);
        auto *destination = reinterpret_cast<uint8_t *>(descriptor->address);

        memset(destination, 0, sizeof(NetworkHeader));
        memcpy(destination + sizeof(NetworkHeader), data, length);

        descriptor->length = length + sizeof(NetworkHeader);

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
    static constexpr uintptr_t k_number = 128;

  private:
    Device *m_device;
    VirtualCPU *m_vcpu;
};

} // namespace virtio

} // namespace DEPOS
