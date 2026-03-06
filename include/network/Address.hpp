#pragma once

#include <utils/string.hpp>

class Address {
  public:
    virtual ~Address()                        = default;
    virtual const unsigned char *data() const = 0;
    virtual size_t size() const               = 0;

    virtual unsigned char operator[](size_t index) const { return data()[index]; };
    virtual bool operator==(const Address &other) const {
        if (size() != other.size()) return false;
        return memcmp(data(), other.data(), size()) == 0;
    }

    virtual bool operator!=(const Address &other) const { return !(*this == other); }
};

template <size_t Length> class SizedAddress : public Address {
  public:
    constexpr SizedAddress() = default;

    template <typename... Args>
    constexpr SizedAddress(Args... args)
        : m_data{static_cast<unsigned char>(args)...} {
        static_assert(sizeof...(Args) == Length);
    }

    SizedAddress(const char *s) {
        const char *token = s;
        const char *next  = token;
        for (unsigned int i = 0; i < Length; i++) {
            if (token) {
                m_data[i] = atol(token);
                next      = strchr(token, '.');
                if (!next) next = strchr(token, ':');
                if (next) token = next + 1;
            }
        }
    }

    const unsigned char *data() const override { return m_data; }
    size_t size() const override { return Length; }

    static constexpr SizedAddress broadcast() {
        SizedAddress b;
        for (size_t i = 0; i < Length; ++i) {
            b.m_data[i] = 0xFF;
        }
        return b;
    }

  private:
    unsigned char m_data[Length];
} __attribute__((packed));
