#pragma once

#include <network/ethernet/ip/IPv4.hpp>

namespace DEPOS {

class UDP {
  public:
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

      private:
        Header m_header;
    };

    template <typename Driver>
    class Connection : private Observer<const IPv4::Packet *>, public Observed<const Datagram *> {
        void update(const IPv4::Packet *packet) {
            const auto *datagram = reinterpret_cast<const Datagram *>(packet->data());

            if (packet->header()->m_protocol != IPv4::UDP) return;

            if (!m_port || CPU::be16toh(datagram->header()->m_destination) == m_port) {
                notify(datagram);
            }
        }

      public:
        Connection(uint16_t port = 0)
            : m_port(port) {
            m_handler.attach(this);
        }

        ~Connection() { m_handler.detach(this); }

        void send(IPv4::Address dip, uint16_t d, void *data, size_t length) {
            auto *ethernet = reinterpret_cast<Ethernet::Header *>(data);
            auto *ip       = reinterpret_cast<IPv4::Header *>(ethernet + 1);
            auto *header   = reinterpret_cast<Header *>(ip + 1);

            header->m_source      = CPU::htobe16(m_port);
            header->m_destination = CPU::htobe16(d);
            header->m_length      = CPU::htobe16(length + sizeof(Header));
            header->m_checksum    = 0;

            m_handler.send(dip, IPv4::UDP, data, length + sizeof(Header));
        }

      private:
        IPv4::Connection<NIC<Driver>> m_handler;
        uint16_t m_port;
    };
};

} // namespace DEPOS
