#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <machine/Traits.hpp>
#include <network/GenericAddress.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Checksum.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <network/ethernet/ip/ARP.hpp>

namespace DEPOS {

class IPv4 {
  public:
    enum : uint8_t { ICMP = 1, TCP = 6, UDP = 17 };
    typedef GenericAddress<4> Address;

    struct Header {
        uint8_t m_version;
        uint8_t m_tos;
        uint16_t m_length;
        uint16_t m_identification;
        uint16_t m_fragment;
        uint8_t m_ttl;
        uint8_t m_protocol;
        uint16_t m_checksum;
        Address m_source;
        Address m_destination;

        Header(Address destination,
               Address source,
               uint8_t protocol,
               uint16_t length,
               uint8_t tos             = 0,
               uint16_t identification = 0,
               uint16_t fragment       = 0,
               uint8_t ttl             = 64) {
            m_version        = 0x45;
            m_tos            = tos;
            m_length         = CPU::htobe16(sizeof(Header) + length);
            m_identification = CPU::htobe16(identification);
            m_fragment       = CPU::htobe16(fragment);
            m_ttl            = ttl;
            m_protocol       = protocol;
            m_source         = source;
            m_destination    = destination;
            m_checksum       = 0;
            m_checksum       = Checksum::calculate(this, sizeof(Header));
        }

        uint8_t length() const { return (m_version & 0x0F) * 4; }

    } __attribute__((packed));

    struct Packet {
        auto header() const { return &m_header; }
        auto length() const { return CPU::be16toh(m_header.m_length) - m_header.length(); }
        auto *data() const { return reinterpret_cast<const uint8_t *>(this) + m_header.length(); }

        Header m_header;
    };

  public:
    template <typename NIC>
    class Connection : public NIC::Observer, public Observed<const Packet *> {
        using ARP = DEPOS::ARP<NIC, Connection>;

      public:
        using Address                      = IPv4::Address;
        static constexpr Address Broadcast = Address(255, 255, 255, 255);
        static constexpr uint16_t Protocol = Ethernet::IPv4;

        Connection() {
            m_nic = NIC::instance();
            m_arp = new ARP(m_nic, this);
            m_nic->attach(this);
        }

        ~Connection() { m_nic->detach(this); }

        auto address() { return GenericAddress<4>(Traits<typename NIC::Device>::IP); }

        void update(const unsigned char *data, size_t) {
            auto *ethernet = reinterpret_cast<const Ethernet::Header *>(data);
            auto *ipv4     = reinterpret_cast<const Packet *>(ethernet + 1);

            if (CPU::be16toh(ethernet->m_type) == Ethernet::IPv4) {
                notify(ipv4);
            }
        }

        void send(Address dip, uint8_t protocol, void *data, uint16_t length) {
            uint16_t total = sizeof(Ethernet::Header) + sizeof(Header) + length;
            auto mac       = (dip == Broadcast) ? Ethernet::Broadcast : m_arp->resolve(dip);
            auto *ethernet = new (data) Ethernet::Header(mac, m_nic->address(), Ethernet::IPv4);
            new (ethernet + 1) Header(dip, address(), protocol, length);
            m_nic->send(data, total);
        }

      private:
        NIC *m_nic;
        ARP *m_arp;
    };
};

} // namespace DEPOS
