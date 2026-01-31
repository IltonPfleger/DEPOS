#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <machine/Traits.hpp>

template <size_t N> struct GenericAddress {
    GenericAddress() = default;
    GenericAddress(const unsigned char (&data)[N]) { memcpy(m_data, data, sizeof(m_data)); }

    operator const unsigned char *() const { return m_data; }
    bool operator==(const GenericAddress &other) const { return memcmp(m_data, other.m_data, sizeof(m_data)) == 0; }
    bool operator!=(const GenericAddress &other) const { return !(*this == other); }

  private:
    unsigned char m_data[N];
} __attribute__((packed));

class Ethernet {

  public:
    typedef GenericAddress<6> Address;
    enum EtherType : uint16_t { IPv4 = 0x0800 };
    static constexpr size_t MTU = 1518;

    struct Frame {
        struct Header {
            Address m_destination;
            Address m_source;
            uint16_t m_type;
        } __attribute__((packed));

        Frame(Address destination, Address source, EtherType type) {
            new (&m_header) Header(destination, source, CPU::htobe16(type));
        }
        unsigned char *data() { return reinterpret_cast<unsigned char *>(&m_header + 1); }

        Header m_header;
    };
};

class IPv4 {
  public:
    enum Protocol : unsigned char { ICMP = 1, TCP = 6, UDP = 17 };
    typedef GenericAddress<4> Address;
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NIC;

    struct PendingPacket {
        uint8_t *m_buffer;
        size_t m_received;
        size_t m_id;
    };

    enum { MF = 0x2000 };

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
            Console::print("THERE: ", fragment, '\n');
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

    } __attribute__((packed));

    class Assembler {

      public:
        Assembler() = default;
        Assembler(uint8_t *buffer, size_t buffer_length) : m_buffer(buffer), m_buffer_length(buffer_length) {}

        void add(Header *header) {
            uint16_t fragment = CPU::be16toh(header->m_fragment);
            uint16_t offset = (fragment & 0x1FFF) * 8;
            bool more_fragments = (fragment & MF);
            uint16_t id = header->m_identification;
            uint8_t ihl = (header->m_version & 0x0F) * 4;
            uint16_t payload_length = CPU::be16toh(header->m_length) - ihl;
            uint8_t *payload = (uint8_t *)header + ihl;

            if (m_received == 0) m_id = id;
            if (m_id != id || offset + payload_length > m_buffer_length) return;

            memcpy(m_buffer + offset, payload, payload_length);

            m_received += payload_length;

            if (!more_fragments) m_expected = offset + payload_length;
        }

        bool completed() const { return m_expected > 0 && m_received >= m_expected; }
        size_t length() const { return m_received; }

      private:
        uint8_t *m_buffer = nullptr;
        size_t m_buffer_length = 0;
        uint16_t m_id = 0;
        size_t m_received = 0;
        size_t m_expected = 0;
    };

  public:
    IPv4() { m_nic = NIC::instance(); }

    size_t receive(unsigned char *destination, unsigned int length, Protocol protocol) {
        uint8_t *buffer = new unsigned char[Ethernet::MTU];
        Assembler assembler(destination, length);

        while (!assembler.completed()) {
            size_t received = m_nic->receive(buffer, Ethernet::MTU);
            if (received) {
                Ethernet::Frame *frame = reinterpret_cast<Ethernet::Frame *>(buffer);

                if (frame->m_header.m_type == CPU::be16toh(Ethernet::IPv4)) {
                    Header *header = reinterpret_cast<Header *>(frame->data());
                    if (header->m_protocol != protocol) continue;
                    assembler.add(header);
                }
            }
        }

        delete[] buffer;
        return assembler.length();
    }

  private:
    NIC *m_nic;
};

class UDP {
    struct Header {
        uint16_t m_source;
        uint16_t m_destination;
        uint16_t m_length; 
        uint16_t m_checksum;
    } __attribute__((packed));

  public:
    UDP() {}

    size_t receive(unsigned char *destination, unsigned int length) {
        uint8_t *ip_payload_buffer = new uint8_t[length + sizeof(Header)];

        while (true) {
            size_t ip_payload_size = m_ipv4.receive(ip_payload_buffer, length + sizeof(Header), IPv4::UDP);

            if (ip_payload_size < sizeof(Header)) continue;

            Header *udp_header = reinterpret_cast<Header *>(ip_payload_buffer);

            uint16_t udp_total_len = CPU::be16toh(udp_header->m_length);
            size_t data_len = udp_total_len - sizeof(Header);

            size_t final_size = (data_len > length) ? length : data_len;

            memcpy(destination, ip_payload_buffer + sizeof(Header), final_size);

            delete[] ip_payload_buffer;
            return final_size;
        }
    }

  private:
    IPv4 m_ipv4;
};

// class UDP {
//     struct Header {
//         uint16_t m_source;
//         uint16_t m_destination;
//         uint16_t m_length;
//         uint16_t m_checksum;
//     } __attribute__((packed));
//
//   public:
//     UDP() {}
//
//     size_t receive(unsigned char *destination, unsigned int length) {
//         while (1) {
//             unsigned int received = m_ipv4.receive(destination, length);
//             auto *header =
//                 reinterpret_cast<IPv4::Header *>(destination + sizeof(Ethernet::Frame::Header) + sizeof(IPv4::Header));
//             if (header->m_protocol == IPv4::UDP) {
//                 return received;
//             }
//         }
//     }
//
//   private:
//     IPv4 m_ipv4;
// };

// void send() {
//     Address destination({0xFF, 0xFF, 0xFF, 0xFF});
//     Address source({0xFF, 0xFF, 0xFF, 0xFF});
//     Protocol protocol = UDP;

//    constexpr unsigned int k_max_frame_payload_size = 1480;

//    constexpr unsigned int length = 4000;

//    uint16_t sent = 0;
//    while (sent < length) {
//        size_t remaining = length - sent;
//        size_t current = (remaining > k_max_frame_payload_size) ? k_max_frame_payload_size : remaining;

//        uint16_t offset = (sent / 8);
//        offset |= (sent + current) < length ? 0x2000 : 0;

//        unsigned int total = sizeof(Ethernet::Frame) + sizeof(IPv4::Header) + current;
//        uint8_t *buffer = new unsigned char[total];
//        new (buffer + sizeof(Ethernet::Frame)) Header(destination, source, protocol, current, 0, m_id, offset);

//        Ethernet::Address dest_mac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
//        Ethernet::Address src_mac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
//        new (buffer) Ethernet::Frame(dest_mac, src_mac, Ethernet::IPv4);

//        NIC::instance()->send(buffer, total);

//        sent += current;

//        delete[] buffer;
//    }
//}
