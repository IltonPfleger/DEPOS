#ifndef __META_ARRAY_HEADER__
#define __META_ARRAY_HEADER__

#include <types.hpp>

namespace DEPOS {

namespace meta {

template <size_t N, typename T> class Array {
  public:
    constexpr Array() {}

    template <typename... Args>
    constexpr Array(Args... args)
        : m_data{static_cast<T>(args)...} {
        static_assert(sizeof...(Args) == N);
    }

    [[nodiscard]] constexpr const T *data() const { return m_data; }

    [[nodiscard]] constexpr T *data() { return m_data; }

    [[nodiscard]] constexpr T &operator[](size_t i) { return m_data[i]; }

    [[nodiscard]] constexpr const T &operator[](size_t i) const { return m_data[i]; }

    [[nodiscard]] constexpr size_t length() const { return N; }

    [[nodiscard]] constexpr size_t size() const { return N * sizeof(T); }

    [[nodiscard]] constexpr bool empty() const { return N == 0; }

    [[nodiscard]] constexpr bool operator!=(const Array &other) const { return !(*this == other); }

    [[nodiscard]] constexpr bool operator==(const Array &other) const {
        for (size_t i = 0; i < N; ++i)
            if (!(m_data[i] == other.m_data[i])) return false;
        return true;
    }

  private:
    T m_data[N];
};

} // namespace meta

} // namespace DEPOS

#endif
