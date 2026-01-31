#pragma once

#include <utils/string.hpp>

template <size_t Length> struct GenericAddress {
    GenericAddress() = default;
    GenericAddress(const unsigned char *str) {
        const char *token = str;
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

    operator const unsigned char *() const { return m_data; }
    bool operator==(const GenericAddress &other) const { return memcmp(m_data, other.m_data, sizeof(m_data)) == 0; }
    bool operator!=(const GenericAddress &other) const { return !(*this == other); }

  private:
    unsigned char m_data[Length];

} __attribute__((packed));
