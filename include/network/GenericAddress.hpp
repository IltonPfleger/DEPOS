#pragma once

#include <libraries/libc/string.h>
#include <network/NetworkAddress.hpp>
#include <types.hpp>

namespace DEPOS {

template <size_t Length> struct GenericAddress {

    constexpr GenericAddress() = default;

    template <typename... Args>
    constexpr GenericAddress(Args... args)
        : _data{static_cast<unsigned char>(args)...} {}

    constexpr GenericAddress(const unsigned char *data) {
        for (size_t i = 0; i < Length; ++i)
            _data[i] = data[i];
    }

    constexpr GenericAddress(const NetworkAddress &address) {
        for (size_t i = 0; i < Length && i < address.length(); i++) {
            _data[i] = address[i];
        }
    }

    [[nodiscard]] operator NetworkAddress() const { return NetworkAddress(_data, Length); }

    [[nodiscard]] bool operator==(const GenericAddress &) const = default;

    // Remove

    constexpr operator const unsigned char *() const { return _data; }

    static constexpr GenericAddress broadcast() {
        GenericAddress result{};
        for (size_t i = 0; i < Length; ++i) {
            result._data[i] = 0xFF;
        }
        return result;
    }

  private:
    unsigned char _data[Length];
} __attribute__((packed));

} // namespace DEPOS
