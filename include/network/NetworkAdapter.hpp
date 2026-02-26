#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Device> class NetworkAdapter : public Observed<const unsigned char *, size_t> {

    NetworkAdapter() {
        m_device = Device::instance();
        new Thread(worker);
    };

    static void *worker(void *) {
        auto *self = reinterpret_cast<NetworkAdapter *>(s_instance);
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
    static void init() { s_instance = new NetworkAdapter(); }

    static auto *instance() { return s_instance; }

    int send(const void *b, size_t s) { return m_device->send(b, s); };

  private:
    static inline NetworkAdapter *s_instance;

  private:
    Spin m_spin;
    Device *m_device;
};

} // namespace DEPOS
