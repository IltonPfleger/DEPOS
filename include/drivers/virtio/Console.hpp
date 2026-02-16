#pragma once

#include <Traits.hpp>
#include <drivers/uart/UART16550.hpp>
#include <drivers/virtio/Handler.hpp>
#include <drivers/virtio/LegacyHeader.hpp>
#include <drivers/virtio/VirtQueue.hpp>
#include <memory/Heap.hpp>
#include <utils/Observer.hpp>

namespace virtio {

static constexpr int N = 128;

template <uintptr_t Base> class Console : public Handler, public Observer<unsigned char *, size_t> {
    typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result Device;

  public:
    static Console *instance() {
        if (!s_instance) s_instance = new Console();
        return s_instance;
    }

    unsigned int *header() { return &m_header.m_magic; }
    unsigned int pfn() { return m_queues[m_header.m_queue_selector].m_address / m_header.m_guest_page_size; }
    void ack(unsigned int source) { m_header.m_interrupt_status &= ~source; }

    void pfn(unsigned int value) {
        new (&m_queues[m_header.m_queue_selector])
            VirtQueue(value * m_header.m_guest_page_size, k_number_of_descriptors, m_header.m_queue_align);
        m_header.m_queue_page_frame_number = value;
    }

    void notify(unsigned int source) {
        VirtQueue &queue = m_queues[source];

        if (source == k_tx_queue) {
            int id = queue.available();
            if (id < 0) return;
            while (1) {
                VirtQueue::RingDescriptor *descriptor = queue.get(id);
                char *data = reinterpret_cast<char *>(descriptor->address);
                for (uint32_t j = 0; j < descriptor->length; j++) {
                    ::Console::cout << data[j];
                }
                if (!(descriptor->flags & 0x1)) break;
                id = descriptor->next;
            }
            queue.free(id);
        }
    }

    void update(unsigned char *buffer, size_t size) override {
        VirtQueue &queue = m_queues[k_rx_queue];
        int id = queue.available();
        if (id < 0) return;
        VirtQueue::RingDescriptor *descriptor = queue.get(id);
        descriptor->length = size;
        descriptor->flags = 0;

        unsigned char *destination = reinterpret_cast<unsigned char *>(descriptor->address);
        memcpy(destination, buffer, size);
        m_header.m_interrupt_status |= 0x1;
        queue.free(id, size);
    }

    static bool read(uintptr_t addr, unsigned int *destination) { return instance()->Handler::read(addr, destination); }
    static bool write(uintptr_t addr, unsigned int source) { return instance()->Handler::write(addr, source); }

  private:
    Console() {
        m_header.m_magic = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        m_header.m_version = 1;
        m_header.m_id = 3;
        m_header.m_vendor = 0x554d4551;
        m_header.m_host_features = 1 << 27;
        m_header.m_queue_number_max = k_number_of_descriptors;
        Device::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr uintptr_t Size = sizeof(Console);
    LegacyHeader m_header;

  private:
    static constexpr uintptr_t k_number_of_queues = 2;
    static constexpr uintptr_t k_number_of_descriptors = 128;
    static constexpr uintptr_t k_tx_queue = 1;
    static constexpr uintptr_t k_rx_queue = 0;
    static inline Console *s_instance = nullptr;
    VirtQueue m_queues[k_number_of_queues];
};

} // namespace virtio
