#pragma once

#include <utils/string.hpp>

template <size_t Length> struct GenericAddress {

    constexpr GenericAddress() = default;

    template <typename... Args> constexpr GenericAddress(Args... args) : m_data{static_cast<unsigned char>(args)...} {
        static_assert(sizeof...(Args) == Length);
    }

    GenericAddress(const char *s) {
        const char *token = s;
        const char *next = token;
        for (unsigned int i = 0; i < Length; i++) {
            if (token) {
                m_data[i] = atol(token);
                next = strchr(token, '.');
                if (!next) next = strchr(token, ':');
                if (next) token = next + 1;
            }
        }
    }

    constexpr operator const unsigned char *() const { return m_data; }
    constexpr bool operator==(const GenericAddress &other) const { return memcmp(m_data, other.m_data, sizeof(m_data)) == 0; }
    constexpr bool operator!=(const GenericAddress &other) const { return !(*this == other); }

  private:
    unsigned char m_data[Length];
} __attribute__((packed));
