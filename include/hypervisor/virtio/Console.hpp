#pragma once

#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <architecture/CPU.hpp>
// #include <drivers/uart/UART16550.hpp>
#include <hypervisor/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

namespace virtio {

template <typename Device, uintptr_t Base>
class Console : public Handler, public Observer<const unsigned char *, size_t> {

  public:
    void notify(unsigned int source) {
        if (source != k_tx_queue) return;

        VirtQueue &queue = this->m_queues[source];
        if (!queue.available()) return;

        int head        = queue.alloc();
        int current     = head;
        uint32_t length = 0;

        while (1) {
            auto *descriptor = queue.get(current);
            char *data       = reinterpret_cast<char *>(descriptor->address);

            for (uint32_t j = 0; j < descriptor->length; j++)
                Device::instance()->putc(data[j]);

            length += descriptor->length;

            if (!(descriptor->flags & 0x1)) break;
            current = descriptor->next;
        }
        queue.free(head, length);
    }

    void update(const unsigned char *buffer, size_t size) override {
        VirtQueue &queue = this->m_queues[k_rx_queue];

        if (!queue.available()) return;

        int id            = queue.alloc();
        auto *descriptor  = queue.get(id);
        auto *destination = reinterpret_cast<unsigned char *>(descriptor->address);

        descriptor->length = size;
        descriptor->flags  = 0;

        memcpy(destination, buffer, size);

        queue.free(id, size);
        this->interrupt() |= 1;
        m_owner->interrupt(IRQ);
    }

    Console(VirtualMachine *owner)
        : m_owner(owner) {
        this->m_header.m_magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version                   = 1;
        this->m_header.m_id                        = 3;
        this->m_header.m_vendor                    = 0x554d4551;
        this->m_header.m_host_features             = 1 << 27;
        this->m_header.m_max_number_of_descriptors = k_number;
        Device::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr int IRQ           = 32;
    static constexpr size_t Size       = sizeof(LegacyHeader);

  private:
    static const int k_number   = 32;
    static const int k_tx_queue = 1;
    static const int k_rx_queue = 0;

  private:
    VirtualMachine *m_owner;
};

} // namespace virtio

} // namespace DEPOS
