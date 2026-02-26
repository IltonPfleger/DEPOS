#pragma once

#include <network/GenericAddress.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class Ethernet {

  public:
    typedef GenericAddress<6> Address;
    typedef Address MAC;
    typedef uint16_t Ethertype;

    enum : Ethertype {
        IPv4 = 0x0800,
        ARP = 0x0806,
    };

    struct Header {
        Address m_destination;
        Address m_source;
        Ethertype m_type;

        Header(Address d, Address s, Ethertype t) : m_destination(d), m_source(s), m_type(CPU::htobe16(t)) {}

    } __attribute__((packed));

    static constexpr MAC Broadcast = MAC(255, 255, 255, 255, 255, 255);
};

} // namespace DEPOS
