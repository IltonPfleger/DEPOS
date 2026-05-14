#pragma once

#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <hypervisor/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <utility/Observer.hpp>

namespace DEPOS {

namespace virtio {

template <typename Device, uintptr_t Base>
class Console : public Handler, public Observer<const unsigned char *, size_t> {

    friend Handler;

    enum { RX = 0, TX = 1 };

  public:
    void notify(unsigned int source) {
        if (source != TX) return;

        auto &queue = this->m_queues[TX];

        while (queue.available()) {
            int head      = queue.alloc();
            size_t length = process(queue, head);
            queue.free(head, length);
        }
    }

    void update(const unsigned char *buffer, size_t size) override {
        VirtQueue &queue = this->m_queues[RX];

        if (!queue.available()) return;

        int id            = queue.alloc();
        auto *descriptor  = queue.get(id);
        auto *destination = reinterpret_cast<unsigned char *>(descriptor->address);

        descriptor->length = size;
        descriptor->flags  = 0;

        memcpy(destination, buffer, size);

        queue.free(id, size);
        this->interrupt() |= 1;
        owner_.interrupt(IRQ);
    }

    size_t process(VirtQueue &queue, int head) {
        size_t total = 0;
        int current  = head;

        VirtQueue::RingDescriptor *descriptor = queue.get(current);

        total += print(descriptor);

        while (descriptor->flags & 0x1) {
            current    = descriptor->next;
            descriptor = queue.get(current);
            total += print(descriptor);
        }

        return total;
    }

    size_t print(VirtQueue::RingDescriptor *descriptor) {
        auto *data      = reinterpret_cast<uint8_t *>(descriptor->address);
        uint32_t length = descriptor->length;
        for (uint32_t j = 0; j < descriptor->length; j++)
            Device::instance()->putc(data[j]);
        return length;
    }

    Console(VirtualMachine &owner)
        : owner_(owner) {
        this->m_header.magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.version                   = 1;
        this->m_header.id                        = 3;
        this->m_header.vendor                    = 0x554d4551;
        this->m_header.host_features             = 1 << 27;
        this->m_header.max_number_of_descriptors = k_number;
        Device::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr int IRQ           = 32;

  private:
    static const int k_number = 32;

  private:
    VirtualMachine &owner_;
};

} // namespace virtio

} // namespace DEPOS
