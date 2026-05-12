#pragma once

#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>

namespace DEPOS {

template <typename Device> class VirtualSwitch : public Device::Observer, public Device::Observed {
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
        : device_(Device::instance()) {

        for (int i = 0; i < InternalQueueSize; i++) {
            m_free.insert(&m_links[i]);
        }

        device_->attach(this);
        m_worker_thread = new Thread(entry, this);
    }

    NetworkBuffer *alloc(size_t length) { return device_->alloc(length); }

    int send(NetworkBuffer *buffer) {
        device_->send(buffer);
        enqueue_copy(buffer->start(), buffer->length());
        return 0;
    }

    void update(NetworkBuffer buffer) {
        enqueue_copy(reinterpret_cast<const unsigned char *>(buffer.start()), buffer.length());
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
            CPU::Interrupt::disable();
            m_spin.acquire();
            Link *link = m_received.remove();
            m_spin.release();
            CPU::Interrupt::enable();

            if (link) {
                InternalBuffer &buf = link->value();

                NetworkBuffer temp_buffer(buf.data, 0, buf.size);
                this->notify(temp_buffer);

                CPU::Interrupt::disable();
                m_spin.acquire();
                m_free.insert(link);
                m_spin.release();
                CPU::Interrupt::enable();
            }
        }
        return nullptr;
    }

  private:
    using Link = collections::Node<InternalBuffer>;
    using List = collections::FIFO<Link>;

    Device *device_;
    Thread *m_worker_thread;
    List m_free;
    List m_received;
    Link m_links[InternalQueueSize];
    Spin m_spin;
};

} // namespace DEPOS
