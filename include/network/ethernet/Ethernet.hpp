#pragma once

#include <network/GenericAddress.hpp>
#include <utils/Observer.hpp>

class Ethernet {

  public:
    typedef GenericAddress<6> Address;

    enum EtherType : uint16_t {
        IPv4 = 0x0008,
        ARP = 0x0608,
    };

    struct Header {
        Address m_destination;
        Address m_source;
        EtherType m_type;

        Header(Address d, Address s, EtherType t) : m_destination(d), m_source(s), m_type(t) {}

    } __attribute__((packed));
};
