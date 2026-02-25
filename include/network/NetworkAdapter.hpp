#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Driver> class NetworkAdapter : public Observed<const unsigned char *, size_t> {

    NetworkAdapter() { new Thread(worker, this); };

    static int worker(void *p) {
        auto *self = reinterpret_cast<NetworkAdapter *>(p);
        while (1) {
            auto *buffer = Driver::instance()->receive();
            if (buffer) {
                self->notify(buffer->data(), buffer->length());
                Driver::instance()->free(buffer);
            }
        }
        return 0;
    }

  public:
    static void init() { s_instance = new NetworkAdapter(); }

    static auto *instance() { return s_instance; }

    int send(const void *b, size_t s) {
        bool enabled = CPU::Interruptions::disable();
        int response = Driver::instance()->send(b, s);
        if (enabled) CPU::Interruptions::enable();
        return response;
    };

  private:
    static inline NetworkAdapter *s_instance;

  private:
    Spin m_spin;
};

} // namespace DEPOS
