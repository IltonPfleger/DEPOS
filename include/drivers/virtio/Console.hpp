#pragma once

#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/uart/UART16550.hpp>
#include <drivers/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <network/NetworkAdapter.hpp>
#include <utils/Observer.hpp>

namespace virtio {

template <typename Device, uintptr_t Base>
class Console : public Handler<Console<Device, Base>>, public Observer<const unsigned char *, size_t> {

  public:
    static Console *instance() {
        if (!s_instance) s_instance = new Console();
        return s_instance;
    }

    void notify(unsigned int source) {
        VirtQueue &queue = this->m_queues[source];

        if (source == k_tx_queue) {
            int head_id = queue.available();
            if (head_id < 0) return;

            int current_id = head_id;
            uint32_t total_length = 0;

            while (1) {
                VirtQueue::RingDescriptor *descriptor = queue.get(current_id);
                char *data = reinterpret_cast<char *>(descriptor->address);

                for (uint32_t j = 0; j < descriptor->length; j++) {
                    Device::instance()->putc(data[j]);
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
        int id = queue.available();
        if (id < 0) return;

        auto descriptor = queue.get(id);
        descriptor->length = size;
        descriptor->flags = 0;

        auto *destination = reinterpret_cast<unsigned char *>(descriptor->address);
        memcpy(destination, buffer, size);

        this->interrupts(0x1);
        m_vcpu->interrupt(Traits<Device>::IRQs[0]);

        queue.free(id, size);
    }

  private:
    Console() {
        this->m_header.m_magic = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version = 1;
        this->m_header.m_id = 3;
        this->m_header.m_vendor = 0x554d4551;
        this->m_header.m_host_features = 1 << 27;
        this->m_header.m_queue_number_max = NumberOfDescriptors;
        m_vcpu = VirtualCPU::current();
        Device::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t NumberOfDescriptors = 32;
    static constexpr uintptr_t Address = Base;
    static constexpr uintptr_t Size = sizeof(LegacyHeader);

  private:
    static constexpr uintptr_t k_tx_queue = 1;
    static constexpr uintptr_t k_rx_queue = 0;
    static inline Console *s_instance = nullptr;

  private:
    VirtualCPU *m_vcpu;
};

} // namespace virtio
