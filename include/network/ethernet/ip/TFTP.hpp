#pragma once

#include <network/ethernet/ip/UDP.hpp>

template <typename Driver> class TFTP : Observer<const unsigned char *, size_t> {

    enum class Opcode : uint16_t {
        RRQ = 1,   // Read Request
        WRQ = 2,   // Write Request
        DATA = 3,  // Data Packet
        ACK = 4,   // Acknowledgment
        ERROR = 5, // Error Packet
    };

  public:
    TFTP(IPv4::Address server_ip) : m_server_ip(server_ip) { m_udp.attach(this); }

    void *header(uint8_t *buffer) {
        return buffer + sizeof(Ethernet::Header) + sizeof(IPv4::Header) + sizeof(UDP::Header);
    }

    void request(const char *filename, void *buffer, size_t size) {
        m_user_buffer = reinterpret_cast<unsigned char *>(buffer);
        m_buffer_size = size;
        m_received_size = 0;
        m_expected_block = 1;
        m_server_port = 69;

        unsigned char packet[1024];
        size_t name_length = strlen(filename);

        auto *opcode = reinterpret_cast<uint16_t *>(header(packet));
        *opcode = CPU::htobe16(static_cast<uint16_t>(Opcode::RRQ));

        char *ptr = reinterpret_cast<char *>(opcode + 1);
        memcpy(ptr, filename, name_length + 1);
        ptr += name_length + 1;

        memcpy(ptr, "octet", 6);
        ptr += 6;

        size_t tftp_payload_size = 2 + (name_length + 1) + 6;

        TraceIn();
        m_udp.send(m_server_ip, m_server_port, packet, tftp_payload_size);
    }

    void update(const unsigned char *data, size_t length) {
        const auto *udp = reinterpret_cast<const UDP::Header *>(data);
        const uint8_t *tftp = reinterpret_cast<const uint8_t *>(udp + 1);
        size_t tftp_length = length - sizeof(UDP::Header);
        auto opcode = static_cast<Opcode>(CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp)));
        m_server_port = CPU::be16toh(udp->m_source);

        Console::cout << m_server_port << Console::endl;

        if (opcode == Opcode::DATA) {
            uint16_t block = CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp + 2));
            Console::cout << block << " " << m_server_port << " " << tftp_length << Console::endl;
            const uint8_t *data = tftp + 4;
            size_t data_length = tftp_length - 4;

            ERROR(block != m_expected_block);
            ERROR(m_received_size + data_length > m_buffer_size);

            memcpy(m_user_buffer + m_received_size, data, data_length);
            m_received_size += data_length;

            ack(block);
            m_expected_block++;
        }

        ERROR(opcode == Opcode::ERROR);
    }

    void ack(uint16_t block) {
        unsigned char buffer[1024];
        auto *packet = reinterpret_cast<uint16_t *>(header(buffer));
        packet[0] = CPU::htobe16(static_cast<uint16_t>(Opcode::ACK));
        packet[1] = CPU::htobe16(block);
        m_udp.send(m_server_ip, m_server_port, buffer, 4);
    }

  private:
    UDP::Connection<Driver> m_udp;
    IPv4::Address m_server_ip;
    uint16_t m_expected_block;
    uint8_t *m_user_buffer;
    size_t m_buffer_size;
    size_t m_received_size;
    uint16_t m_server_port;
};
