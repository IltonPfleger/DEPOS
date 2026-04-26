#ifndef __NETWORK_BUFFER_HEADER__
#define __NETWORK_BUFFER_HEADER__

#include <Meta.hpp>
#include <types.hpp>

namespace DEPOS {

class NetworkBuffer {
  public:
    NetworkBuffer(uint8_t *start, size_t size, uint8_t *data = nullptr)
        : m_start(start),
          m_size(size),
          m_current(data ? data : start),
          m_current_length(0) {}

    template <typename T = const uint8_t *> T data() const { return reinterpret_cast<T>(m_current); }

    template <typename T = uint8_t *> T data() { return reinterpret_cast<T>(m_current); }

    size_t length() const { return m_current_length; }
    void length(size_t l) { m_current_length = l; }

  protected:
    const uint8_t *start() const { return m_start; }
    size_t size() const { return m_size; }

    size_t offset() const { return (m_current >= m_start) ? static_cast<size_t>(m_current - m_start) : 0; }

    size_t remaining() const { return (offset() <= m_size) ? (m_size - offset()) : 0; }

    void reset() { m_current = m_start; }

    bool advance(size_t bytes) {
        if (bytes > remaining()) return false;
        m_current += bytes;
        return true;
    }

    bool rewind(size_t bytes) {
        if (bytes > offset()) return false;
        m_current -= bytes;
        return true;
    }

  private:
    uint8_t *m_start;
    size_t m_size;
    uint8_t *m_current;
    size_t m_current_length;
};

} // namespace DEPOS

#endif
