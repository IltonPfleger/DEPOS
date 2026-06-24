#pragma once

#include <Meta.hpp>
#include <types.hpp>

namespace QUARK {

template <typename K, typename T, size_t Size, typename Hasher> class Hash {
  public:
    T &operator[](const K &key) { return table_[hasher_(key) % Size]; }
    const T &operator[](const K &key) const { return table_[hasher_(key) % Size]; }

  private:
    Meta::Array<Size, T> table_;
    Hasher hasher_;
};

} // namespace QUARK
