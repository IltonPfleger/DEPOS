#pragma once

#include <types.hpp>

namespace DEPOS {

struct NetworkBuffer {
    NetworkBuffer() = default;
    NetworkBuffer(void *data, size_t length)
        : m_data(static_cast<unsigned char *>(data)),
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

} // namespace DEPOS
