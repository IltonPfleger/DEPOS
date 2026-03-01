#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Derived> class NIC : public Observed<const unsigned char *, size_t> {

    NIC() {
        m_device = Derived::instance();
        new Thread(worker, this);
    };

    static void *worker(void *p) {
        auto *self = reinterpret_cast<NIC *>(p);
        while (1) {
            auto *buffer = self->m_device->receive();
            if (buffer) {
                self->notify(buffer->data(), buffer->length());
                self->m_device->free(buffer);
            }
        }
        return 0;
    }

  public:
    // class Buffer {
    //   public:
    //     Buffer()
    //         : m_offset(0),
    //           m_length(0) {}

    //    void push(unsigned int size) {
    //        m_offset -= size;
    //        m_length += size;
    //    };
    //    void pop(unsigned int size) {
    //        m_offset += size;
    //        m_length -= size;
    //    };

    //    size_t length() const { return m_length; }

    //    unsigned char *data() { return m_data + m_offset; };

    //  private:
    //    unsigned char m_data[2048];
    //    unsigned int m_offset;
    //    unsigned int m_length;
    //};

    using Observer = DEPOS::Observer<const unsigned char *, size_t>;
    using Observed = DEPOS::Observed<const unsigned char *, size_t>;
    using Device   = Derived;

    static void init() { s_instance = new NIC(); }

    static auto *instance() { return s_instance; }

    int send(const void *b, size_t s) { return m_device->send(b, s); };

    auto address() { return Derived::instance()->address(); }

  private:
    static inline NIC *s_instance;

  private:
    Spin m_spin;
    Derived *m_device;
};

} // namespace DEPOS
