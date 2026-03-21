#pragma once

#include <network/Address.hpp>
#include <types.hpp>

namespace DEPOS {

class NetworkDevice {
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

    virtual ~NetworkDevice()      = default;
    virtual int send(Buffer *)    = 0;
    virtual Buffer *receive()     = 0;
    virtual void alloc(Buffer *b) = 0;
    virtual void free(Buffer *b)  = 0;
    virtual Address address()     = 0;
};

} // namespace DEPOS
