#pragma once

#include <architecture/CPU.hpp>
#include <network/GenericAddress.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

struct Ethernet {
    enum { MTU = 1500 };
    typedef GenericAddress<6> Address;
    typedef uint16_t Protocol;
    typedef unsigned char Data[MTU];

    struct Header {
        Header(Address to, Address from, Protocol protocol)
            : m_to(to),
              m_from(from),
              m_protocol(CPU::htobe16(protocol)) {}

        const Address &to() const { return m_to; }
        const Address &from() const { return m_from; }
        Protocol protocol() const { return CPU::be16toh(m_protocol); }

      private:
        Address m_to;
        Address m_from;
        Protocol m_protocol;
    } __attribute__((packed));

    static constexpr auto broadcast() { return Address(255, 255, 255, 255, 255, 255); }
};

} // namespace DEPOS
