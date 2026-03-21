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
              m_length(length) {}

        auto &data() const { return m_data; }
        auto &data() { return m_data; }

        auto &length() const { return m_length; }
        auto &length() { return m_length; }

        auto &id() { return m_id; }
        auto &id() const { return m_id; }

      private:
        unsigned char *m_data;
        size_t m_length;
        size_t m_id;
    };

    using Observer = DEPOS::Observer<const Buffer *>;
    using Observed = DEPOS::Observed<const Buffer *>;

  public:
    virtual ~NIC()                                    = default;
    virtual int send(const void *buffer, size_t size) = 0;
    void attach(Observer *o) { m_observed.attach(o); }
    void detach(Observer *o) { m_observed.detach(o); }

  protected:
    void init() { new Thread(worker, this); }
    virtual void free(Buffer *b) = 0;
    virtual Buffer *receive()    = 0;

  private:
    static void *worker(void *p) {
        auto *self = static_cast<NIC *>(p);

        while (true) {
            auto *buffer = self->receive();
            if (!buffer) continue;

            self->m_observed.notify(buffer);
            self->free(buffer);
        }
        return nullptr;
    }

  private:
    Observed m_observed;
};

} // namespace DEPOS
