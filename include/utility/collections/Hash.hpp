#pragma once

#include <Meta.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename K, typename T, size_t Size, typename Hasher> class Hash {
  public:
    T &operator[](const K &key) { return m_table[m_hasher(key) % Size]; }
    const T &operator[](const K &key) const { return m_table[m_hasher(key) % Size]; }

  private:
    Meta::Array<Size, T> m_table;
    Hasher m_hasher;
};

} // namespace DEPOS
