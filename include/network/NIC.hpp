#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class NIC {
  public:
    class Buffer {
      public:
        Buffer() = default;
        Buffer(unsigned char *data, size_t length)
            : m_data(data),
              m_length(length),
              m_references(0) {}

        auto data() const { return m_data; }
        auto &data() { return m_data; }

        auto length() const { return m_length; }
        auto &length() { return m_length; }

        auto references() const { return m_references; }
        auto &references() { return m_references; }

        auto id() const { return m_id; }
        auto &id() { return m_id; }

      private:
        unsigned char *m_data;
        size_t m_length;
        size_t m_id;
        mutable size_t m_references;
    };

    using Observer = DEPOS::Observer<const Buffer *>;
    using Observed = DEPOS::Observed<const Buffer *>;

    virtual ~NIC() {}

    void attach(Observer *o) { m_observed.attach(o); }
    void detach(Observer *o) { m_observed.detach(o); }

    void hold(Buffer *buffer) { CPU::Atomic::finc(buffer->references()); }

    void release(Buffer *buffer) {
        if (CPU::Atomic::fdec(buffer->references()) == 1) {
            this->free(buffer);
        }
    }

    virtual int send(const void *b, size_t s) = 0;
    virtual Buffer *receive()                 = 0;

  protected:
    void init() { new Thread(worker, this); }
    virtual void free(Buffer *b) = 0;

  private:
    static void *worker(void *p) {
        auto *self = static_cast<NIC *>(p);

        while (true) {
            auto *buffer = self->receive();
            if (!buffer) continue;
            self->m_observed.notify(buffer);
            self->free(buffer);

            // size_t observers_count = self->m_observed.size();
            // buffer->references()   = observers_count;

            // if (observers_count > 0) {
            //     self->m_observed.notify(buffer);
            // } else {
            //     self->free(buffer);
            // }
        }
        return nullptr;
    }

  private:
    Observed m_observed;
};

} // namespace DEPOS
