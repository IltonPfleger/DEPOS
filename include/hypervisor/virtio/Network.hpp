#pragma once

#include <Traits.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <hypervisor/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

namespace virtio {

struct NetworkHeader {
    unsigned char padding[10];
};

template <typename Device, uintptr_t Base> class Network : public Handler, public Device::Observer {
    enum { RX = 0, TX = 1 };

  public:
    void notify(unsigned int source) {
        if (source != TX) return;

        auto &queue = this->m_queues[TX];

        while (queue.available()) {
            int head              = queue.alloc();
            int current           = head;
            uint32_t length       = 0;
            bool first_descriptor = true;

            while (true) {
                auto *descriptor = queue.get(current);
                uint8_t *data    = reinterpret_cast<uint8_t *>(descriptor->address);
                uint32_t len     = descriptor->length;

                if (first_descriptor) {
                    data += sizeof(NetworkHeader);
                    len -= sizeof(NetworkHeader);
                    first_descriptor = false;
                }

                if (len > 0) {
                    typename Device::Buffer buffer(data, len);
                    m_device->send(&buffer);
                }

                length += descriptor->length;
                if (!(descriptor->flags & 0x1)) break;
                current = descriptor->next;
            }

            queue.free(head, length);
            this->interrupt() |= 0x1;
            m_owner->interrupt(IRQ);
        }
    }

    void update(const Device::Buffer *buffer) override {
        auto data = buffer->data();
        auto size = buffer->length();

        auto &queue = this->m_queues[RX];

        if (!queue.available()) return;

        int id = queue.alloc();

        auto descriptor   = queue.get(id);
        auto *destination = reinterpret_cast<uint8_t *>(descriptor->address);

        memset(destination, 0, sizeof(NetworkHeader));
        memcpy(destination + sizeof(NetworkHeader), data, size);

        descriptor->length = size + sizeof(NetworkHeader);

        queue.free(id, descriptor->length);
        this->interrupt() |= 0x1;
        m_owner->interrupt(IRQ);
    }

    Network(VirtualMachine *owner)
        : m_owner(owner) {
        this->m_header.m_magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version                   = 1;
        this->m_header.m_id                        = 1;
        this->m_header.m_vendor                    = 0x554d4551;
        this->m_header.m_host_features             = 0;
        this->m_header.m_max_number_of_descriptors = Number;
        m_device                                   = Device::instance();
        m_device->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr uintptr_t IRQ     = 50;
    static constexpr size_t Size       = sizeof(LegacyHeader);

  private:
    static constexpr uintptr_t Number = 128;

  private:
    Device *m_device;
    VirtualMachine *m_owner;
};

} // namespace virtio

} // namespace DEPOS
