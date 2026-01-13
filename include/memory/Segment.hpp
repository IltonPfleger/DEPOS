#pragma once

#include <memory/Memory.hpp>

class Segment {
  public:
    Segment(unsigned long addr, unsigned int size, bool owned = true)
        : m_addr(reinterpret_cast<char *>(addr)), m_size(size), m_owned(owned) {}

    Segment(unsigned int size) : Segment(reinterpret_cast<unsigned long>(Memory::kmalloc(size)), size) {}

    ~Segment() {
        if (m_owned)
            Memory::kfree(m_addr, m_size);
    }

    char *base() const { return m_addr; }
    char *end() const { return m_addr + m_size; }
    unsigned int size() const { return m_size; }

  private:
    char *const m_addr;
    const unsigned int m_size;
    const bool m_owned;
};
