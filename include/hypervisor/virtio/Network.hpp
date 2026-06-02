#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <hypervisor/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <network/NetworkDevice.hpp>

namespace DEPOS {

namespace virtio {

typedef unsigned char NetworkHeader[10];

template <typename Device, uintptr_t Base> class Network : public Handler, public Device::Observer {

    friend Handler;

    enum { RX = 0, TX = 1 };

  public:
    Network(VirtualMachine &owner)
        : owner_(owner),
          running_(true),
          thread_(entry, this) {
        this->m_header.magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.version                   = 1;
        this->m_header.id                        = 1;
        this->m_header.vendor                    = 0x554d4551;
        this->m_header.host_features             = 0;
        this->m_header.max_number_of_descriptors = Number;
        m_device                                 = Device::instance();
        m_device->attach(this);
    }

    ~Network() {
        m_device->detach(this);
        running_ = false;
        semaphore_.v();
    }

    void notify(unsigned int source) {
        if (source != TX) return;
        semaphore_.v();
    }

    void update(const NetworkBuffer &buffer) override {
        auto data = buffer.start();
        auto size = buffer.length();

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
        owner_.interrupt(IRQ);
    }

  private:
    static void *entry(void *self) { return reinterpret_cast<Network *>(self)->worker(); }

    void *worker() {
        while (true) {
            semaphore_.p();

            if (!running_) break;

            auto &queue = this->m_queues[TX];

            while (queue.available()) {
                int head      = queue.alloc();
                size_t length = process(queue, head);
                queue.free(head, length);
                this->interrupt() |= 0x1;
                owner_.interrupt(IRQ);
            }
        }
        return nullptr;
    }

    size_t process(Queue &queue, int head) {
        size_t total = 0;
        int current  = head;

        RingDescriptor *descriptor = queue.get(current);
        total += descriptor->length;

        if (descriptor->length >= sizeof(NetworkHeader)) {
            auto *data      = reinterpret_cast<uint8_t *>(descriptor->address) + sizeof(NetworkHeader);
            uint32_t length = descriptor->length - sizeof(NetworkHeader);
            send(data, length);
        }

        while (descriptor->flags & 0x1) {
            current    = descriptor->next;
            descriptor = queue.get(current);
            total += descriptor->length;
            auto *data = reinterpret_cast<uint8_t *>(descriptor->address);
            send(data, descriptor->length);
        }

        return total;
    }

    void send(const uint8_t *data, uint32_t length) {
        if (length == 0) return;
        NetworkBuffer *buffer = m_device->alloc(length);
        if (buffer) {
            buffer->shrink(buffer->offset());
            buffer->rewind(buffer->offset());
            memcpy(buffer->start(), data, length);
            m_device->send(buffer);
        }
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr uintptr_t IRQ     = 50;
    static constexpr size_t Size       = sizeof(LegacyHeader);

  private:
    static constexpr uintptr_t Number = 128;

  private:
    Device *m_device;
    VirtualMachine &owner_;
    volatile bool running_;
    Semaphore semaphore_;
    Thread thread_;
};

} // namespace virtio

} // namespace DEPOS
