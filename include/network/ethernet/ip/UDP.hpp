#pragma once

#include <network/ethernet/ip/IPv4.hpp>

namespace DEPOS {

namespace UDP {

enum { Protocol = 17 };

struct Header {
    uint16_t m_source;
    uint16_t m_destination;
    uint16_t m_length;
    uint16_t m_checksum;
} __attribute__((packed));

struct Datagram {
    auto header() const { return &m_header; }
    uint16_t length() const { return CPU::be16toh(m_header.m_length) - sizeof(Header); }
    const uint8_t *data() const { return reinterpret_cast<const uint8_t *>(&m_header + 1); }
    uint8_t *data() { return reinterpret_cast<uint8_t *>(&m_header + 1); }

  private:
    Header m_header;
};

template <typename Network>
class Channel : private Observer<const typename Network::Packet *>, public Observed<const Datagram *> {

    void update(const Network::Packet *packet) {
        const auto *datagram = reinterpret_cast<const Datagram *>(packet->data());

        if (packet->header()->m_protocol != Protocol) return;

        if (!m_port || CPU::be16toh(datagram->header()->m_destination) == m_port) {
            notify(datagram);
        }
    }

  public:
    typedef Datagram Buffer;
    typedef DEPOS::Observer<const Datagram *> Observer;

    Channel(uint16_t port = 0)
        : m_network(Network::instance()),
          m_port(port) {
        m_network->attach(this);
    }

    ~Channel() { m_network->detach(this); }

    void send(Network::Address address, uint16_t port, unsigned char *data, size_t length) {
        auto *ethernet = reinterpret_cast<Ethernet::Header *>(data);
        auto *ip       = reinterpret_cast<Network::Header *>(ethernet + 1);
        auto *header   = reinterpret_cast<Header *>(ip + 1);

        header->m_source      = CPU::htobe16(m_port);
        header->m_destination = CPU::htobe16(port);
        header->m_length      = CPU::htobe16(length + sizeof(Header));
        header->m_checksum    = 0;

        m_network->send(address, Protocol, data, length + sizeof(Header));
    }

  private:
    Network *m_network;
    uint16_t m_port;
};

} // namespace UDP

} // namespace DEPOS
