#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <machine/Traits.hpp>
#include <network/GenericAddress.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Checksum.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <network/ethernet/ip/ARP.hpp>
#include <network/ethernet/ip/IGMP.hpp>

namespace DEPOS {

namespace IPv4 {

enum { Protocol = 0x0800 };
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
    Address m_to;

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
        m_to             = destination;
        m_checksum       = 0;
        m_checksum       = Checksum::calculate(this, sizeof(Header));
    }

    uint8_t length() const { return (m_version & 0x0F) * 4; }

} __attribute__((packed));

class Packet {
  public:
    auto header() const { return reinterpret_cast<const Header *>(&m_header); }
    auto length() { return CPU::be16toh(header()->m_length) - header()->length(); }
    auto *data() { return reinterpret_cast<uint8_t *>(this) + header()->length(); }
    auto *data() const { return reinterpret_cast<const uint8_t *>(this) + header()->length(); }

  private:
    Header m_header;
};

// namespace IGMP {
//
// enum { Protocol = 2, Query = 0x11, Report = 0x16, Leave = 0x17 };
//
// struct Header {
//     uint8_t m_type;
//     uint8_t m_max;
//     uint16_t m_checksum;
//     IPv4::Address m_group;
//
//     Header(uint8_t type, IPv4::Address group)
//         : m_type(type),
//           m_max(0),
//           m_checksum(0),
//           m_group(group) {
//         m_checksum = Checksum::calculate(this, sizeof(Header));
//     }
// } __attribute__((packed));
//
// } // namespace IGMP

class Multicast {
  public:
    static bool valid(const Address &address) { return (address[0] & 0xF0) == 0xE0; }
    static Ethernet::Address mac(const Address &ip) { return {0x01, 0x00, 0x5E, ip[1] & 0x7F, ip[2], ip[3]}; }
};

//
//     template <typename Network> static void join(IPv4::Address group) {
//         unsigned char buffer[1024];
//
//         new (buffer + sizeof(Ethernet::Header) + sizeof(IPv4::Header))
//             IGMP::Header(IGMP::Report, group);
//
//         auto dmac = mac(group);
//         Network::instance()->send(dmac, group, IGMP::Protocol, buffer, sizeof(IGMP::Header));
//     }
//
//     // template <typename Driver> static bool leave(IP group) {
//     //     IPv4::Connection<Driver> socket;
//     //     unsigned char buffer[sizeof(IGMPv2) + sizeof(IPv4::Header) +
//     sizeof(Ethernet::Header)];
//
//     //    new (buffer) IGMPv2(0x17, group);
//     //    // IGMPv2 *report = new (buffer) IGMPv2(0x17, group);
//     //    //  report->m_checksum = IPv4::checksum(report, sizeof(IGMPv2));
//
//     //    MAC mac   = convert_multicast_group_mac(All);
//     //    MAC mymac = Driver::instance()->mac();
//     //    IP myip   = Driver::instance()->ip();
//
//     //    return socket.send(mac, All, mymac, myip, IPv4::IGMP, buffer, sizeof(IGMPv2));
//     //}
//};

template <typename NIC> class Network : public NIC::Observer, public Observed<const Packet *> {
    using ARP = DEPOS::ARP<NIC, Network>;

  public:
    enum { Protocol = IPv4::Protocol };
    using Packet  = IPv4::Packet;
    using Header  = IPv4::Header;
    using Address = IPv4::Address;

  protected:
    Network() {
        m_nic = NIC::instance();
        m_arp = new ARP(m_nic, this);
        m_nic->attach(this);
    }

    ~Network() { m_nic->detach(this); }

    void update(const NIC::Buffer *buffer) {
        auto data      = buffer->data();
        auto *ethernet = reinterpret_cast<const Ethernet::Header *>(data);
        auto *packet   = reinterpret_cast<const Packet *>(ethernet + 1);

        if (ethernet->protocol() == Protocol) {
            if (accepts(packet)) this->notify(packet);
        }
    }

  public:
    static auto instance() {
        if (!s_instance) s_instance = new Network();
        return s_instance;
    }

    bool accepts(const Packet *) const {
        // const Header *header = packet->header();
        // if (Multicast::valid(header->m_to) || header->m_to == Address::broadcast() ||
        //     header->m_to == address())
        //     return true;
        return true;
    }

    auto address() const { return GenericAddress<4>(Traits<NIC>::IP); }

    void send(Address dip, uint8_t protocol, void *data, uint16_t length) {
        Ethernet::Address mac;
        if (dip == Address::broadcast()) {
            mac = Ethernet::Broadcast;
        } else if (Multicast::valid(dip)) {
            mac = Multicast::mac(dip);
        } else {
            mac = m_arp->resolve(dip);
        }

        send(mac, dip, protocol, data, length);
    }

    void send(Ethernet::Address dmac, Address dip, uint8_t protocol, void *data, uint16_t length) {
        uint16_t total = sizeof(Ethernet::Header) + sizeof(Header) + length;
        auto *ethernet = new (data) Ethernet::Header(dmac, m_nic->address(), Protocol);
        new (ethernet + 1) Header(dip, this->address(), protocol, length);
        m_nic->send(data, total);
    }

  protected:
    NIC *m_nic;
    ARP *m_arp;

  private:
    static inline Network *s_instance;
};

} // namespace IPv4

} // namespace DEPOS
