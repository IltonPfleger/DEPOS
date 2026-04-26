#pragma once

#include <Semaphore.hpp>
#include <Traits.hpp>
#include <utils/Observer.hpp>
#include <utils/collections/AtomicLIFO.hpp>
#include <utils/collections/MPSC.hpp>

namespace DEPOS {

namespace hypervisor {

template <typename Device> class VirtualSwitch : Device::Observer {
    static constexpr int InternalQueueSize = 16;

  public:
    class Buffer {
        friend VirtualSwitch;

      public:
        Buffer()
            : m_length(0) {}

        auto data() const { return m_data; }
        auto &data() { return m_data; }

        auto length() const { return m_length; }
        auto &length() { return m_length; }

      private:
        unsigned char m_data[Device::MaximumFrameLength];
        unsigned int m_length;
    };

    using Observed = DEPOS::Observed<const Buffer *>;
    using Observer = DEPOS::Observer<const Buffer *>;
    void attach(Observer *o) { m_observers.attach(o); }
    void detach(Observer *o) { m_observers.detach(o); }

  private:
    using Link         = collections::Node<Buffer>;
    using ReceivedList = collections::MPSC<Link>;
    using FreeList     = collections::Atomic<Link>;

    VirtualSwitch() {
        for (int i = 0; i < InternalQueueSize; i++) {
            m_free.insert(&m_links[i]);
        }
        m_device = Device::instance();
        m_device->attach(this);
        m_worker = new Thread(worker, this);
    }

    void update(const typename Device::Buffer *buffer) override {
        enqueue(reinterpret_cast<const unsigned char *>(buffer->data()), buffer->length());
    }

    void enqueue(const unsigned char *data, unsigned int length) {
        if (length > Device::MaximumFrameLength) length = Device::MaximumFrameLength;

        Link *l = m_free.remove();

        if (!l) {
            return;
        }

        l->value().m_length = length;
        memcpy(l->value().m_data, data, length);

        m_received.insert(l);
        m_latch.v();
    }

    static void *worker(void *argument) {
        auto *self = reinterpret_cast<VirtualSwitch *>(argument);
        while (true) {
            self->m_latch.p();
            Link *l        = self->m_received.remove();
            Buffer &buffer = l->value();
            self->m_observers.notify(&buffer);
            self->m_free.insert(l);
        }
        return nullptr;
    }

  public:
    static VirtualSwitch *instance() {
        static VirtualSwitch instance;
        return &instance;
    }

    int send(const unsigned char *data, unsigned int length) {
        m_device->send(data, length);
        enqueue(data, length);
        return 0;
    }

  private:
    Link m_links[InternalQueueSize];
    FreeList m_free;
    ReceivedList m_received;
    Semaphore m_latch;
    Device *m_device;
    Thread *m_worker;
    Observed m_observers;
};

} // namespace hypervisor

} // namespace DEPOS
