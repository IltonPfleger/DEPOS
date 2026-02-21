#pragma once

#include <network/ethernet/ip/IPv4.hpp>

class UDP {
  public:
    struct Header {
        uint16_t m_source;
        uint16_t m_destination;
        uint16_t m_length;
        uint16_t m_checksum;
    } __attribute__((packed));

    template <typename Driver>
    class Connection : public Observer<const unsigned char *, size_t>, public Observed<const unsigned char *, size_t> {
      public:
        Connection(uint16_t port = 0) : m_port(port) { m_connection.attach(this); }
        ~Connection() { m_connection.detach(this); }

        void update(const unsigned char *data, size_t length) {
            const auto *ip = reinterpret_cast<const IPv4::Header *>(data);
            uint8_t ihl = (ip->m_version & 0x0F) * 4;
            const auto *udp = reinterpret_cast<const Header *>(data + ihl);

            if (ip->m_protocol != IPv4::UDP) return;

            if (!m_port || CPU::be16toh(udp->m_destination) == m_port) {
                notify(data + ihl, length - ihl);
            }
        }

        void send(IPv4::Address dip, uint16_t d, void *data, size_t length) {
            auto *ethernet = reinterpret_cast<Ethernet::Header *>(data);
            auto *ip = reinterpret_cast<IPv4::Header *>(ethernet + 1);
            auto *header = reinterpret_cast<Header *>(ip + 1);

            header->m_source = CPU::htobe16(m_port);
            header->m_destination = CPU::htobe16(d);
            header->m_length = CPU::htobe16(length + sizeof(Header));
            header->m_checksum = 0;

            m_connection.send(dip, IPv4::UDP, data, length + sizeof(Header));
        }

      private:
        IPv4::Connection<Driver> m_connection;
        uint16_t m_port;
    };
};
