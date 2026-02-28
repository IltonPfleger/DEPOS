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
        if (!s_instance) s_instance = new Network();
        return s_instance;
    }

    void notify(unsigned int source) {
        if (source != k_tx_queue) return;

        VirtQueue &queue = this->m_queues[k_tx_queue];
        int head_id;

        while ((head_id = queue.available()) >= 0) {
            int current_id        = head_id;
            uint32_t total_length = 0;
            bool first_descriptor = true;

            while (true) {
                VirtQueue::RingDescriptor *descriptor = queue.get(current_id);
                uint8_t *data = reinterpret_cast<uint8_t *>(descriptor->address);
                uint32_t len  = descriptor->length;

                if (first_descriptor) {
                    data += sizeof(NetworkHeader);
                    len -= sizeof(NetworkHeader);
                    first_descriptor = false;
                }

                if (len > 0) {
                    NIC<Device>::instance()->send(data, len);
                }

                total_length += descriptor->length;
                if (!(descriptor->flags & 0x1)) break;
                current_id = descriptor->next;
            }

            queue.free(head_id, total_length);
            this->interrupts(0x1);
        }
    }

    void update(const unsigned char *buffer, size_t size) override {
        VirtQueue &queue = this->m_queues[k_rx_queue];
        int id           = queue.available();

        if (id < 0) return;

        auto descriptor   = queue.get(id);
        auto *destination = reinterpret_cast<uint8_t *>(descriptor->address);

        memset(destination, 0, sizeof(NetworkHeader));
        memcpy(destination + sizeof(NetworkHeader), buffer, size);

        descriptor->length = size + sizeof(NetworkHeader);

        queue.free(id, descriptor->length);
        this->interrupts(0x1);
        m_vcpu->interrupt(61);
    }

  private:
    Network() {
        this->m_header.m_magic            = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version          = 1;
        this->m_header.m_id               = 1;
        this->m_header.m_vendor           = 0x554d4551;
        this->m_header.m_queue_number_max = NumberOfDescriptors;
        m_vcpu                            = VirtualCPU::current();
        NIC<Device>::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t NumberOfDescriptors = 128;
    static constexpr uintptr_t Address             = Base;
    static constexpr size_t Size                   = sizeof(LegacyHeader);

  private:
    static constexpr uintptr_t k_rx_queue = 0;
    static constexpr uintptr_t k_tx_queue = 1;
    static inline Network *s_instance     = nullptr;

  private:
    VirtualCPU *m_vcpu;
};

} // namespace virtio

} // namespace DEPOS
