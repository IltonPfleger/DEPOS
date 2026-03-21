#pragma once

#include <network/Address.hpp>
#include <types.hpp>
#include <utils/string.hpp>

namespace DEPOS {

template <size_t Length> class GenericAddress : public Address {

  public:
    constexpr GenericAddress()
        : m_data{} {}

    template <typename... Args>
    constexpr GenericAddress(Args... args)
        : m_data{static_cast<unsigned char>(args)...} {
        static_assert(sizeof...(Args) == Length);
    }

    GenericAddress(const char *s) {
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

    bool operator==(const Address &other) const override {
        if (other.size() != Length) return false;
        return memcmp(m_data, other.data(), Length) == 0;
    }

    virtual size_t size() const { return Length; }
    virtual const unsigned char *data() const { return m_data; }

  private:
    Meta::Array<Length, unsigned char> m_data;
};

} // namespace DEPOS
