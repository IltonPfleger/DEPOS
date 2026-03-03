#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Family> class NIC : public Observed<const unsigned char *, size_t> {
  public:
    using Observer = DEPOS::Observer<const unsigned char *, size_t>;
    using Observed = DEPOS::Observed<const unsigned char *, size_t>;

    class Buffer {
      public:
        auto data() { return m_data; }
        auto length() { return m_length; }
        size_t m_length;
        unsigned int m_id;
        unsigned char m_data[sizeof(typename Family::Frame)];
        unsigned int m_references;
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
