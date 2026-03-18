#pragma once

#include <Traits.hpp>
#include <machine/Machine.hpp>

namespace DEPOS {

namespace hypervisor {

class VirtualSwitch : NIC::Observer, NIC::Observed {
    struct Devices {
      public:
        Devices() { init<typename Traits<Ethernet>::Devices>(); }

        template <typename... Args> void send(Args &&...args) {
            for (unsigned int i = 0; i < NumberOfDevices; ++i)
                m_devices[i]->send(args...);
        }

        void attach(NIC::Observer *observer) {
            for (unsigned int i = 0; i < NumberOfDevices; ++i)
                m_devices[i]->attach(observer);
        }

      private:
        template <typename List, unsigned int Index = 0> void init();
        template <unsigned int Index> void init<Meta::TypeList<>, Index>() {}
        template <typename Head, typename... Tail, unsigned int Index>
        void init<Meta::TypeList<Head, Tail...>, Index>() {
            m_devices[Index] = new Head();
            init<Meta::TypeList<Tail...>, Index + 1>();
        }

      private:
        static constexpr int NumberOfDevices = Traits<Ethernet>::NumberOfDevices;
        NIC *m_devices[NumberOfDevices];
    };

    VirtualSwitch() {
        for (int i = 0; i < InternalQueueSize; i++) {
            m_free.insert(&m_links[i]);
        }
        m_devices.attach(this);
        worker = new Thread(worker, this);
    }

    void receive(const void *data, size_t length) {
        Link *link       = m_free.remove();
        Buffer &buffer   = link->value();
        buffer->data()   = data;
        buffer->length() = length();
        m_received.insert(&buffer);
    }

    int send(const void *data, size_t length) {
        receive(data, length);
        m_devices.send(data, length);
    }

    void update(const unsigned char *data, size_t length) { receive(data, length); }

    void *worker(void *pointer) {
        auto *self = reinterpret_cast<VirtualSwitch *>(pointer);
        while (1) {
            Link *link = m_received.remove();
            if (link) {
                Buffer &buffer = link->value();
                notify(buffer->data(), buffer->length());
            }
        }
    }

  private:
    using Buffer                           = NIC::Buffer;
    using Link                             = Node<Buffer>;
    using List                             = FIFO<Link>;
    static constexpr int InternalQueueSize = 16;

  private:
    Devices m_devices;
    Thread *m_worker;
    List m_free;
    List m_received;
    Link m_links[InternalQueueSize];
};

} // namespace hypervisor

} // namespace DEPOS
