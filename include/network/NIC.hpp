#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
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
