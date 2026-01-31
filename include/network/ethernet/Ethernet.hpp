#pragma once

#include <network/GenericAddress.hpp>
#include <utils/Observer.hpp>

class Ethernet : public Observed {

  public:
    typedef GenericAddress<6> Address;

    enum EtherType : uint16_t {
        IPv4 = 0x0800,
        ARP = 0x0806,
    };

    struct Header {
        Address m_destination;
        Address m_source;
        uint16_t m_type;

        Header(Address d, Address s, EtherType t) : m_destination(d), m_source(s), m_type(CPU::htobe16(t)) {}

    } __attribute__((packed));

    void notify() {
        Observed::notify();
        TraceIn();
    }
};
