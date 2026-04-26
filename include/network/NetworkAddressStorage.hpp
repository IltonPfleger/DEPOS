#ifndef __NETWORK_ADDRESS_STORAGE_HEADER__
#define __NETWORK_ADDRESS_STORAGE_HEADER__

#include <shared/libraries/libc/string.hpp>
#include <types.hpp>

namespace DEPOS {

template <size_t Length> struct NetworkAddressStorage {

    constexpr NetworkAddressStorage() = default;

    template <unsigned char... Args>
    constexpr NetworkAddressStorage(Args... args)
        : m_data{args...} {
        static_assert(sizeof...(Args) == Length);
    }

    [[nodiscard]] constexpr operator const unsigned char *() const { return m_data; }

    [[nodiscard]] constexpr bool operator==(const NetworkAddressStorage &other) const {
        return memcmp(m_data, other.m_data, sizeof(m_data)) == 0;
    }

    [[nodiscard]] constexpr bool operator!=(const NetworkAddressStorage &other) const { return !(*this == other); }

  private:
    unsigned char m_data[Length];
} __attribute__((packed));

} // namespace DEPOS

#endif
