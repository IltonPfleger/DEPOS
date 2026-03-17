#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class NIC : public Observed<const unsigned char *, size_t> {
  public:
    using Observer = DEPOS::Observer<const unsigned char *, size_t>;
    using Observed = DEPOS::Observed<const unsigned char *, size_t>;

    class Buffer {
      public:
        Buffer() = default;
        Buffer(unsigned char *data, size_t length)
            : m_data(data),
              m_length(length) {}
        auto &data() { return m_data; }
        auto &length() { return m_length; }
        auto &id() { return m_id; }

      private:
        unsigned char *m_data;
        size_t m_length;
        int m_id;
    };

    virtual ~NIC() {}
    virtual int send(const void *b, size_t s) = 0;
    virtual void release(Buffer *b)           = 0;
    virtual Buffer *receive()                 = 0;

  protected:
    void init() { new Thread(worker, this); }

  private:
    static void *worker(void *p) {
        auto *self = reinterpret_cast<NIC *>(p);
        while (1) {
            auto *buffer = self->receive();
            if (buffer) {
                self->notify(buffer->data(), buffer->length());
                self->release(buffer);
            }
        }
        return 0;
    }
};

} // namespace DEPOS
