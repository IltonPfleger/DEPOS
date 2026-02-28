#pragma once

#include <network/GenericAddress.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class Ethernet {

  public:
    enum { MTU = 1500 };
    typedef GenericAddress<6> Address;
    typedef Address MAC;
    typedef uint16_t Protocol;
    typedef unsigned char Data[MTU];
    typedef uint32_t CRC;

    class Header {
      public:
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

    class Frame {
      public:
        Frame(Address to, Address from, Protocol protocol)
            : m_header(to, from, protocol) {}

        const Header *header() const { return &m_header; }
        const Data &data() const { return m_data; }
        Data &data() { return m_data; }

      private:
        Header m_header;
        Data m_data;
        CRC m_crc;
    } __attribute__((packed));

    static constexpr MAC Broadcast = MAC(255, 255, 255, 255, 255, 255);
};

} // namespace DEPOS
