#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <machine/Traits.hpp>
#include <network/GenericAddress.hpp>
#include <network/NetworkAdapter.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <network/ethernet/ip/ARP.hpp>

class IPv4 {
  public:
    typedef unsigned char Protocol;
    enum : Protocol { ICMP = 1, TCP = 6, UDP = 17 };
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

        Header(Address destination, Address source, Protocol protocol, uint16_t length, uint8_t tos = 0,
               uint16_t identification = 0, uint16_t fragment = 0, uint8_t ttl = 64) {
            m_version = 0x45;
            m_tos = tos;
            m_length = CPU::htobe16(sizeof(Header) + length);
            m_identification = CPU::htobe16(identification);
            m_fragment = CPU::htobe16(fragment);
            m_ttl = ttl;
            m_protocol = protocol;
            m_source = source;
            m_destination = destination;
            m_checksum = 0;
            m_checksum = checksum(this, sizeof(Header));
        }

    } __attribute__((packed));

    static uint16_t checksum(const void *data, size_t length) {
        uint32_t sum = 0;
        const uint16_t *ptr = reinterpret_cast<const uint16_t *>(data);

        for (; length > 1; length -= 2)
            sum += *ptr++;
        if (length > 0) sum += *reinterpret_cast<const uint8_t *>(ptr);

        while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16);
        return static_cast<uint16_t>(~sum);
    }

    static constexpr Address Broadcast = Address(255, 255, 255, 255);

  public:
    template <typename Driver>
    class Connection : public Observer<const unsigned char *, size_t>, public Observed<const unsigned char *, size_t> {
      public:
        Connection() {
            m_nic = NetworkAdapter<Driver>::instance();
            m_nic->attach(this);
        }

        ~Connection() { m_nic->detach(this); }

        void update(const unsigned char *data, size_t length) {
            if (length < sizeof(Ethernet::Header) + sizeof(Header)) return;

            const Ethernet::Header *ethernet = reinterpret_cast<const Ethernet::Header *>(data);
            if (CPU::be16toh(ethernet->m_type) == Ethernet::IPv4) {
                notify(data + sizeof(Ethernet::Header), length - sizeof(Header));
            }
        }

        void send(Address destination, Protocol protocol, void *data, uint16_t length) {
            uint16_t total = sizeof(Ethernet::Header) + sizeof(Header) + length;

            Ethernet::Address dmac = (destination == Broadcast) ? Ethernet::Broadcast : ARP<Driver>::resolve(destination);

            Ethernet::Header *ethernet = new (data) Ethernet::Header(dmac, Driver::instance()->mac(), Ethernet::IPv4);

            new (ethernet + 1) Header(destination, Driver::instance()->ip(), protocol, length);

            m_nic->send(data, total);
        }

      private:
        NetworkAdapter<Driver> *m_nic;
    };
};
