#pragma once

#include <Spin.hpp>
#include <Traits.hpp>
#include <network/NIC.hpp>

namespace DEPOS {

namespace hypervisor {

template <typename Device> class VirtualSwitch : public NIC::Observer, public NIC::Observed {
    static constexpr unsigned int FrameSize = 1518;
    static constexpr int InternalQueueSize  = 16;

    struct InternalBuffer {
        unsigned char data[FrameSize];
        unsigned int size;

        InternalBuffer()
            : size(0) {}
    };

  public:
    static auto instance() {
        static VirtualSwitch instance;
        return &instance;
    }

  public:
    VirtualSwitch()
        : m_worker_thread(nullptr) {

        for (int i = 0; i < InternalQueueSize; i++) {
            m_free.insert(&m_links[i]);
        }

        m_devices = Device::instance();
        m_devices->attach(this);
        m_worker_thread = new Thread(entry, this);
    }

    int send(const unsigned char *data, unsigned int length) {
        m_devices->send(data, length);
        return enqueue_copy(data, length);
    }

    void update(const NIC::Buffer *buffer) {
        enqueue_copy(reinterpret_cast<const unsigned char *>(buffer->data()), buffer->length());
    }

  private:
    int enqueue_copy(const unsigned char *data, unsigned int length) {
        m_spin.acquire();
        Link *link = m_free.remove();
        m_spin.release();
        if (!link) return -1;

        InternalBuffer &buf = link->value();
        buf.size            = (length > FrameSize) ? FrameSize : length;
        memcpy(buf.data, data, buf.size);

        m_spin.acquire();
        m_received.insert(link);
        m_spin.release();
        return buf.size;
    }

    static void *entry(void *p) { return reinterpret_cast<VirtualSwitch *>(p)->run(); }

    void *run() {
        while (true) {
            CPU::Interruptions::disable();
            m_spin.acquire();
            Link *link = m_received.remove();
            m_spin.release();
            CPU::Interruptions::enable();

            if (link) {
                InternalBuffer &buf = link->value();

                NIC::Buffer temp_buffer(buf.data, buf.size, 0);
                notify(&temp_buffer);

                CPU::Interruptions::disable();
                m_spin.acquire();
                m_free.insert(link);
                m_spin.release();
                CPU::Interruptions::enable();
            }
        }
        return nullptr;
    }

  private:
    using Link = Node<InternalBuffer>;
    using List = FIFO<Link>;

    Device *m_devices;
    Thread *m_worker_thread;
    List m_free;
    List m_received;
    Link m_links[InternalQueueSize];
    Spin m_spin;
};

} // namespace hypervisor

} // namespace DEPOS
