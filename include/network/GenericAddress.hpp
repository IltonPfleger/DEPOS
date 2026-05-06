#pragma once

#include <libraries/libc/string.h>
#include <types.hpp>

namespace DEPOS {

template <size_t Length> struct GenericAddress {

    constexpr GenericAddress() = default;

    template <typename... Args>
    constexpr GenericAddress(Args... args)
        : _data{static_cast<unsigned char>(args)...} {
        static_assert(sizeof...(Args) == Length);
    }

    constexpr GenericAddress(const unsigned char *data) {
        for (size_t i = 0; i < Length; ++i)
            _data[i] = data[i];
    }

    // GenericAddress(const char *s) {
    //     const char *token = s;
    //     const char *next  = token;
    //     for (unsigned int i = 0; i < Length; i++) {
    //         if (token) {
    //             _data[i] = atol(token);
    //             next      = strchr(token, '.');
    //             if (!next) next = strchr(token, ':');
    //             if (next) token = next + 1;
    //         }
    //     }
    // }

    constexpr operator const unsigned char *() const { return _data; }

    constexpr bool operator==(const GenericAddress &other) const {
        return memcmp(_data, other._data, sizeof(_data)) == 0;
    }
    constexpr bool operator!=(const GenericAddress &other) const { return !(*this == other); }

    // constexpr GenericAddress operator|(const GenericAddress &other) const {
    //     GenericAddress result;
    //     for (size_t i = 0; i < Length; ++i) {
    //         result._data[i] = _data[i] | other._data[i];
    //     }
    //     return result;
    // }

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
